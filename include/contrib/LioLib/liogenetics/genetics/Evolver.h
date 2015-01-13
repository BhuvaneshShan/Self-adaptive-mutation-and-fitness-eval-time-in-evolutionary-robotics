/*
 *  $Id: Evolver.h 1995 2010-01-14 13:38:14Z evert $
 *
 *  Created on: 12-Feb-2009
 *
 * Copyright (c) 2009, VU University Amsterdam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef EVOLVER_H_
#define EVOLVER_H_

#include "Task.h"
#include "syslog.h"

#include <genetics/GeneticConfig.h>
#include <lioutils/Mathlib.h>
#include <lioutils/Fixed32.h>

#include <functional>
#include <algorithm>

#include <math.h>
//#include <plugins/controller/MDL2e/core/globals.h>

extern worldModel_t wm; // World Model global variable

namespace Genetics
{
/**
 * \addtogroup Genetics
 * @{
 */

/**
 * Task ID for Evolver task (needed to recognise hook messages)
 */
extern const char EvolverTaskID[];

/**
 *  Task that executes the on-board evolutionary process. The idea is that this
 *  task runs as a separate thread. It tracks robot performance through the
 *  Fitness policy and initiates reproduction.
 *  Through the Listener mechanism, any changes to the active
 *  genome are broadcast to registered listeners. These can then react to
 *  these changes by reinitialising their genetically controlled parameters.
 *  To register a class, simply derive from Listener and pass a reference to the
 *  Evolver to the constructor.
 *
 *  @see Listener
 *
 *  There is in principle no reason why there should be only a single Evolver
 *  task running: it is perfectly feasible to have, for instance, one Evolver
 *  that evolves sensor fusion and another that evolves a main movement neural
 *  net.
 *
 *  @tparam Genome The type of genome stored
 *  @tparam reservoirSize The number of candidate replacements to maintain
 *  @tparam Fitness Class that calculates fitness increments at every timestep of the EA
 *  (called through \c Fitness::operator(Worldmodel&). Requires default contstructor. See
 *  Genetics::ObstacleAvoidanceFitnessD3D for an example.
 *  @tparam Compare Fitness comparison function, defaults to std::greater : higher
 *  number is better fitness.
 *
 *  @author evert haasdijk
 */
template <typename __Genome, size_t reservoirSize, typename Fitness, typename Compare = std::greater<LIOReal> >
class Evolver : public Task
{
public:
	typedef __Genome Genome;

	/**
	 * @return Task ID for Evolver task (needed to recognise hook messages)
	 */
	static const char* getTaskID(void)
	{
		return EvolverTaskID;
	}

	/**
	 * Constructor.
	 */
	Evolver(void) :
		Task(getTaskID(),
			configMINIMAL_STACK_SIZE + 100,
			tskIDLE_PRIORITY,
			true,			// hookable
			true),			// has queue
		_listeners(0),
		_evaluationState(INITIALISING),
		_generation(0),
		_evaluationCycle(30),
		_startLifetimeValue(-30),
		_reevaluateRate(0.2),
		_randomSeed(1243), // TODO: -- use sensor instead? (relevant if sensor are noisy!) use internal clock? (if exists?)
		_generationsPerExperiment(500),
		_run(1)
		{
			//
			// Random seed is currently set by hand...

			//
			Mathlib::setRandSeed( _randomSeed );
		}

	virtual ~Evolver(void) { };

	/**
	 * @return The currently active genome
	 */
	const Genome& getGenome(void) const { return _genome; }

	/**
	 * Submit a \c Genome to be included in the _genomeReservoir. Looks for the first
	 * entry for which fitness is not set and, if it finds one, inserts \c candidate
	 * at that point. If no unevaluated entries exist, tries to replace the worst current
	 * entry.
	 *
	 * @param candidate The \c Genome to add
	 * @param force If true, \c candidate replaces the worst entry in
	 * _genomeReservoir, otherwise, \c candidate is accepted only if it is better than
	 * the worst entry.
	 *
	 * @return true if \c candidate was accepted for consideration.
	 *
	 * @note: If you call consider() with a genome for which the fitness has
	 * not been set, it may be replaced in the next call to consider(), unless
	 * it has been evaluated in the meantime.
	 */
	bool consider(Genome& candidate, bool force = false)
	{
		if (reservoirSize == 0)
			return false;
		// else

		//
		// Find first unevaluated entry, or, if there is none, replace last (i.e., worst) entry.
		// TODO Should we mark the last entered genome so that it won't be replaced unless the
		//      reservoir is full?
		//
		Genome* entry = std::find_if(_genomeReservoir, _genomeReservoir + reservoirSize, HasNoFitness());
		if (entry == _genomeReservoir + reservoirSize)
		{
			--entry;
		}

		//
		// Notice the inverted comparison: if both genomes have no or equal fitness,
		// we favour the candidate.
		//
		if (force || !_fitnessCompare(*entry, candidate)) {
//
//			LOG << "entry:     " << entry->hasFitness() << ' ' << entry->getFitness();
//			LOG << "\ncandidate: " << candidate.hasFitness() << ' ' << candidate.getFitness();

			*entry = candidate;

			//
			// If candidate has no fitness yet, it can remain in the last
			// position: it's no better than any of the others.
			//
			if (candidate.hasFitness())
			{
				sortReservoir();
			}

			return true;
		}
		// else

		return false;
	}

	/**
	 * Main task loop. Note, that the first thing the loop does, is initialising the
	 * evolution. Thus, one should not start the Evolver task before registering (i.e., creating) all listeners.
	 */
	virtual void task(void)
	{
		static QueueHookable::QueueMessage databuf;
		initDisplay();

		for (;;) {
			// TODO: if the time is ripe (when is that?) seek out partner or select. Requires comms between robots

			if (_evaluationState == INITIALISING)
			{
				initEvolution();
				continue;
			}
			//else

			//
			// check if we received a message
			//
			if (xQueueReceive( queue, (void*)&databuf, portMAX_DELAY)== pdTRUE)
			{
				//
				// check if we received a message from controller
				//
				if (!strncmp(databuf.id, "Cml", 3))
				{

					// Player/Stage specific hack:
					// security control (prior to a new behavior, get out of crash situation) -- avoid "STALL" status in player/stage
					// inter-generation security setup : get away from the wall (note: when crash, robot cannot move except backward. in real setup it would bump.)
					// strategy: random choice btw several implementations (last one being random walk (trade-off absolute efficiency vs. short-term efficiency)

					if (_freeSteps > 0)
					{
						--_freeSteps;
						continue;
					}
					// else


					if ( _iteration >= 0 )
					{
						//
						// compute fitness only if iteration >= 0:
						// if iteration<0, this is free-of-charge lifetime. (used for parent reevaluation)
						//
						updateFitness();
					}
					_iteration++;

					if ( _iteration == _evaluationCycle )
					{
						_genome.setFitness(_currentFitness);

						//
						// finished evaluating the current genome: set fitness, compare
						// to previous solutions, start next evaluation, etc.
						//
						bool bestEver = _fitnessCompare( _currentFitness, _bestFitnessEver);
						if (bestEver)
						{
							_bestFitnessEver = _genome.getFitness();
						}

						bool improvement = false;

						switch (_evaluationState) {

						case EVALUATING:
						{
							//
							// (re-)evaluating: update fitness in reservoir, or --if the original has somehow
							// disappeared-- reconsider the current genome
							//
							Genome* original = findInReservoir(_genome);

							if (original)
							{
								improvement  = _fitnessCompare( _currentFitness, original->getFitness());

								//
								// Exponential moving average
								//
								LIOReal newFitness = (_currentFitness + original->getFitness()) / LIOReal(2.0);
								original->setFitness( newFitness );

							} else {
								//
								// the original has meanwhile been replaced; reconsider this
								//
								consider(_genome);
							}
							break;
						}

						case CONSIDERING:
						{
							improvement  = _fitnessCompare( _currentFitness, _parent.getFitness());

							//
							// consider current genome for insertion in the reservoir. If that fails
							// (because the new individual is worse than the worst genome in the
							// reservoir), increase the parent's sigma
							//
							bool goodEnough = consider(_genome);

							if (!goodEnough)
							{
								Genome* parent = findInReservoir(_parent);
								if (parent)
								{
									parent->increaseSigma();
								}
							}

							break;
						}

						case INITIALISING:
						{
							DEBUG("can't happen!");
						}
						// default: nix
						}

						display(bestEver, improvement);

						if (_generationsPerExperiment > 0 && _generation == _generationsPerExperiment)
						{
							LOG << "\n### END OF RUN " << _run << "\n";

							++_run;
							_evaluationState = INITIALISING;
						} else {
							startEvaluation();
						}
					}

				}
			} /* End of main infinite task loop */

		}
	}

	/**
	 *  Base class for Evolver clients.
	 *
	 *  Provides simple mechanics for linked-list maintenance in Evolver.
	 *
	 *  @note This class would have been abstract if the compiler would
	 *  have been able to handle that properly. You must overload parseGenome()
	 *  to implement a Listener
	 *
	 *  @see parseGenome
	 */
	//template<Genome>
	class Listener
	{
		public:
		/**
		 * Automatically registers this instance with the evolver
		 */
		Listener(Evolver<Genome, reservoirSize, Fitness, Compare>& evolver) :
			_next(0),
			_previous(0)
		{
			if (evolver._listeners) {
				evolver._listeners->push_back(this);
			} else {
				evolver._listeners = this;
			}
		}

		/**
		 * Calls remove() to maintain list integrity.
		 *
		 * @see unlink()
		 */
		virtual ~Listener(void)
		{
			remove();
		}

		/**
		 * Overload this method to react to changes in the active genome.
		 *
		 * Called when an Evolver activates a genome.
		 *
		 * @note If the listener needs to keep access to the genome after this
		 * call, make a deep copy.
		 *
		 * @note Pure virtual methods cause oversized binaries, otherwise this
		 * would surely be one.
		 */
		virtual void parseGenome(const Genome&, unsigned freeSteps )
		{
			//nix
		}

		/**
		 * Add \c listener to the end of the linked list of Listeners
		 * @param listener
		 */
		void push_back(Listener* listener)
		{
			if (listener)
			{
				if (_next == 0)
				{
					_next = listener;
					listener->_previous = this;

				} else {
					_next->push_back(listener);
				}
			}
		}

		/**
		 *	Removes this instance from the linked list
		 */
		void remove(void)
		{
			if (_next)
			{
				_next->_previous = _previous;
			}
			if (_previous)
			{
				_previous->_next = _next;
			}
			_next = 0;
			_previous = 0;
		}

		/**
		 * For looping through the linked list.
		 */
		Listener* next() {
			return _next;
		}

	private:
		Listener* _next;
		Listener* _previous;
	};

private:
	/// Utility class to compare genomes and fitnesses
	class FitnessCompare
	{
	public:
		/**
		 * Uses the Compare template argument to compare the fitness of x and y.
		 *
		 * @return False if x has no fitness set (see Genome::hasFitness()), else
		 * true if \c x is better than \c y or if \c y has no fitness.
		 */
		bool operator()(const Genome& x, const Genome& y) const
		{
			if (!x.hasFitness()) return false;
			// else
			if (!y.hasFitness()) return true;
			// else
			return _fitnessComparator(x.getFitness(), y.getFitness());
		}
		/**
		 * Uses the Compare template argument to compare x and the fitness of y.
		 *
		 * @return True if \c x is better than \c y.getFitness() or if \c y has no fitness.
		 */
		bool operator()(const LIOReal& x, const Genome& y) const
		{
			if (!y.hasFitness()) return true;
			// else
			return _fitnessComparator(x, y.getFitness());
		}

		/**
		 * Uses the Compare template argument to compare the fitness of x and y.
		 *
		 * @return False if x has no fitness set (see Genome::hasFitness()), else
		 * true if \c x.getFitness() is better than \c y.
		 */
		bool operator()(const Genome& x, const LIOReal& y) const
		{
			if (!x.hasFitness()) return false;
			// else
			return _fitnessComparator(x.getFitness(), y);
		}
		/**
		 * Uses the Compare template argument to compare x and y.
		 *
		 * @return False if x has no fitness set (see Genome::hasFitness()), else
		 * true if \c x.getFitness() is better than \c y.
		 */
		bool operator()(const LIOReal& x, const LIOReal& y) const
		{
			return _fitnessComparator(x, y);
		}
	protected:
		Compare _fitnessComparator;
	};

	/// Local comparator instance
	FitnessCompare _fitnessCompare;

	/**
	 * Helper class to find genomes with undefined fitness.
	 *
	 * Predicate for use in STL functions like std::find_if.
	 */
	class HasNoFitness
	{
	public:
		bool operator()(const Genome& genome) const
		{
			return !genome.hasFitness();
		}
	};

	/**
	 * Helper class to compare genetic code in Genomes. Simply calls operator==()
	 * on the genes in the chromosomes to compare the genomes' chromosomes.
	 *
	 * Predicate for use in STL functions like std::find_if.
	 */
    class CompareChromosome {
    public:
    	CompareChromosome(const Genome& genome) :
    		_genome(genome)
    		{};

    		bool operator()(const Genome& rhs) const
    		{
    			return std::equal(_genome.getChromosome().begin(), _genome.getChromosome().end(), rhs.getChromosome().begin());
    		}

    private:
		const Genome& _genome;
    };

    /**
     * Fitness calculator.
     */
    Fitness _fitness;

    /**
     * Linked list of registered Listener
     */
	Listener* _listeners;

    /**
     * States for (re-)evaluation of genomes
     */
    enum  {
    	INITIALISING,		/// < initial state, no genome active, yet
    	EVALUATING,			/// < (re-)evaluating a genome in _genomeReservoir
    	CONSIDERING			/// < trying a new genome prior to consideration
    } _evaluationState;

	/// Number of 'generations' so far
	unsigned _generation;

	/**
	 * Array of prospective parents.
	 *
	 * The array can be filled in two ways:
	 * \li Generating mutants (or even completely new Genomes) a-sexually.
	 * Having only this is, in effect, an EvolutionsStrategy; we would then
	 * assess the fitness of each proposed replacement in a time-sharing manner.
	 *
	 * \li Receiving genomes from other individuals we came into contact with,
	 * then recombining these with the current genome.
	 * Finess could then be estimated as, e.g., the average of this and the
	 * partner's fitness.
	 * Requires comms.
	 */
	Genome _genomeReservoir[reservoirSize];
	// should have been: boost::array<Genome, reservoirSize> _genomeReservoir; but Nicolas persuaded me to remove boost dependencies

	/**
	 *  Currently active genome.
	 */
	Genome _genome;

	/**
	 * Parent of _genome. Only guaranteed to contain a 'proper' genome if _evaluationState == CONSIDERING
	 */
	Genome _parent;

	/// Best fitness so far
	LIOReal _bestFitnessEver;

	/// Currently calculated fitness
	LIOReal _currentFitness;

	/// Number of evaluations so far
	unsigned _numEvaluations;
	/// current iteration (reset for every evaluation)
	long _iteration;
	/// number of iterations used to evaluate a genome
	long _evaluationCycle;

	/**
	 * All individuals get a "free-of-charge" number of steps (ie. iterations
	 * step below zero are free) to get out of trouble before fitness calculation
	 * starts.
	 *
	 * Lifetime duration is twice the standard, half of it is free-of-charge (i.e.,
	 * no fitness update) to avoid bad evaluation because stuck in a non favourable
	 * location.
	 *
	 */
	long _startLifetimeValue;

	/// Likelihood of reevaluating a genome rather than generating offspring.
	ProbabilityType _reevaluateRate;

	/**
	 * Seed for random generator.
	 *
	 * @todo We could use some sensor value for a truly random value (if sensor are noisy), or use internal clock (if there is one?)
	 */
	unsigned _randomSeed;

	/**
	 * Evolution is reset when _generationsPerExperiment have passed. Set to 0 for neverending evolution.
	 */
	unsigned _generationsPerExperiment;

	/// Current experiment number
	unsigned _run;

	/**
	 * Number of timesteps spent to get a robot away from walls (hack to overcome Player/Stage sticky wall behaviour)
	 */
	unsigned _freeSteps;

	/**
	 * Sort reservoir using \c Compare. Since the array should be fairly small,
	 * this should not be a lot of overhead.
	 */
	void sortReservoir(void)
	{
		::std::sort(_genomeReservoir, _genomeReservoir + reservoirSize, _fitnessCompare);
	}

	/**
	 * Initialises the genomes, resets all fitnesses, etc.
	 */
	void initEvolution(void)
	{
		for (size_t i = 0; i < reservoirSize; ++i)
		{
			_genomeReservoir[i].reset();
		}

		_bestFitnessEver = _genome._minFitness;

		_numEvaluations = 0;
		_generation = 0;

		startEvaluation();
	}

	/**
	 * Select new parent, mutate as needed and initialise (re-)evaluation.
	 */
	void startEvaluation(void)
	{
		Genome* parent = selectParent();

		if (parent)
		{
			_parent = *parent;
			_genome = *parent;
		}
		// else, we have no reservoir: it's a 0+1 ES, i.e., random walk.

		_evaluationState = EVALUATING;

		if (_genome.hasFitness())  // i.e., genome not about to be evaluated for the first time ever
		{
			if (Mathlib::getBool(_reevaluateRate)) // check for parent re-evaluation
			{
				// re-evaluation; nothing to do here
			} else {
				_genome.mutate();	// create offspring

				_evaluationState = CONSIDERING;
				++_generation;
			}
		}

		_currentFitness = _genome._minFitness; // reset fitness

		_numEvaluations++;
		_iteration = _startLifetimeValue;

		postGenome();
	}

	/**
	 * Calculate _currentFitness from the world model through the Fitness policy.
	 */
	void updateFitness(void)
	{
		LIOReal delta(_fitness(wm));

		_currentFitness += delta;
	}

	/**
	 * Advertise new genome to hooked-up listeners.
	 *
	 * @note Tried this with the OS task hooking mechanism, but couldn't achieve any semblance of
	 * symchronisation, and the genome got scrambled along the way.
	 */
	void postGenome(void)
	{
		_freeSteps = 6;

		for (Listener* listener = _listeners; listener != 0; listener = listener->next())
		{
			listener->parseGenome(_genome, _freeSteps); //e.g., load NN weights.
		}

		vTaskDelay(20);
	}

	/**
	 * Select the parent to create or copy the new current genome from.
	 * For now, simply selects either the first unevaluated entry or, if
	 * there is none, a random genome from _genomeReservoir.
	 *
	 * @return Const reference to the selected parent
	 *
	 * @todo Implement flexibly (as a strategy!?)
	 */
	Genome* selectParent(void)
	{
		if (reservoirSize == 0)
		{
			return 0;
		}
		// else

		Genome* retval = std::find_if(_genomeReservoir, _genomeReservoir + reservoirSize, HasNoFitness());
		if (retval == _genomeReservoir + reservoirSize) // i.e., all entries have a fitness set.
		{
			// Below's the bit that should be implemented through a policy
			return _genomeReservoir + Mathlib::getInt(reservoirSize);
		}
		// else

		return retval;
	}

    /**
     * Uses CompareChromose to find a clone of \c genome in \c _genomeReservoir
     *
     * @return Pointer to the first clone in _genomeReservoir or \c NULL if no
     * clone can be found
     */
	Genome* findInReservoir(const Genome& genome)
	{
		Genome* end = _genomeReservoir + reservoirSize;
		Genome* loc = std::find_if(_genomeReservoir, end, CompareChromosome(genome));

		return (loc == end) ? 0 : loc;
	}

	SysLog& printEvaluationState(void) const
	{
		switch (_evaluationState)
		{
			case INITIALISING:
				return LOG << "initialising";
				// break;

			case EVALUATING:
				return LOG << "re-evaluating";
				// break;

			case CONSIDERING:
				return LOG << "considering";
				// break;

			// default
		}
		return LOG << "unknown evalstate";
	}

	void initDisplay(void)
	{
		LOG << "# On-Line Evolution using random seed no."<< _randomSeed << "\n";
		LOG << "# generation _bestFitnessEver isNewBest improvement _currentFitness evaluationState \n";
		LOG << "# Per entry in the reservoir: Fitness Sigma EvaluationCount {Genome}\n";
		LOG << "# EvaluationCount: parent is re-evaluated sometimes (" << (int)(100*(double)_reevaluateRate) << "%). High number means robust wrt. to children/competitors.\n";
		LOG << "# isNewBest: true when the current genome is the best to date.\n";
		LOG << "# improvement: true whenever better child outperforms its parent.\n";
		LOG << "# Evolution is reset every " << _generationsPerExperiment << " evaluations.\n";

		vTaskDelay(20);
	}
	/**
	 * Dump EA statistics to log
	 * @param isNewBest
	 */
	void display(bool isNewBest, bool improvement)
	{
		static const char delim(' ');

		LOG << _generation << delim << _bestFitnessEver << delim << isNewBest << delim << improvement << delim << _currentFitness << delim; printEvaluationState() << '\n';

		vTaskDelay(20);

		LOG << "_genome ";
		for (const Fixed32* gene = _genome.getChromosome().begin(); gene != _genome.getChromosome().end(); ++gene)
		{
			LOG << gene->getRawValue(); // weights are not readable, but precise and reusable. (ie. internal encoding, value displayed may seem to be out of range, but in fact this is not true)
			LOG << delim;
		}
		LOG << '\n';

		vTaskDelay(20);

		for (unsigned i = 0; i < reservoirSize; ++i)
		{
			Genome& genome(_genomeReservoir[i]);

			LOG << "reservoir[" << i << "] ";
			LOG << delim << genome.getFitness();
			LOG << delim << genome.getSigma();
			LOG << delim << genome.getEvaluationCount();
			LOG << " {";

			for (const Fixed32* gene = genome.getChromosome().begin(); gene != genome.getChromosome().end(); ++gene)
			{
				LOG << gene->getRawValue(); // weights are not readable, but precise and reusable. (ie. internal encoding, value displayed may seem to be out of range, but in fact this is not true)
				LOG << delim;
			}

			LOG << "}\n";
			vTaskDelay(20);
		}
	}
};
/// @}
}

#endif /* EVOLVER_H_ */
