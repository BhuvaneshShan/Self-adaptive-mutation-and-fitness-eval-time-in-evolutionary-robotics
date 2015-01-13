/*
 *  $Id: Genome.h 2729 2010-09-23 10:16:08Z evert $
 *
 * Authors evert & christopher
 */
#ifndef GENOME_H
#define GENOME_H

#include <genetics/GeneticConfig.h>
#include <genetics/RealMutation.h>
#include <genetics/AveragingCrossover.h>

#include <algorithm>

#include <math.h>

namespace Genetics
{

/** \addtogroup Genetics
 * @{
 */
/**
 * Implements genome functionality. A Genome contains a \c Chromosome (a
 * container of \c Genes), combined with mutation, recombination and
 * initialisation policies.
 * Also stores (but does not calculate) the fitness (utility) of the genome.
 *
 * @tparam BaseMutator is a policy class that implements mutation for a single gene. @see Genetics::PointMutator::mutate().
 * @tparam Mutation is a policy class that implements the mutation operator \c Genetics::PointMutator is a simple example.
 * @tparam Recombination is a policy class that implements crossover: \c Genetics::OnePointCrossover is a simple example.
 * @tparam Initialiser is a policy class that implements the (initial) generation of random \c Genes.
 * @tparam Chromosome is a policy class that implements a container of \c Genes (e.g, <tt>std::vector\<Fixed32\></tt>, or just a simple array).
 * @tparam Gene is a placeholder that defines the gene type, e.g., Fixed32.
 */
template<
	template< typename Gene > class BaseMutator,
	template< template< typename Gene > class Chromosome, typename Gene > class Mutator,
	template< template< typename Gene > class Chromosome, typename Gene > class Recombinator,
	template< template< typename Gene > class Chromosome, typename Gene > class Initialiser,
	template< typename Gene > class Chromosome,
	typename Gene = Fixed32>
class Genome
{
public:

    /// Vanilla constructor; creates an Initialiser and uses that to generate an initial Chromosome
	Genome(void)
    {
		reset();
    }

	Genome(const Initialiser<Chromosome, Gene>& initialiser) :
		_init(initialiser)
	{
		reset();
	}

    virtual ~Genome(void)
    {
    }

    // TODO: copy ctor (although acompiler-generated one will do for now)
	// TODO: setting of various parameters from settings file.

	/**
	 * Calls mutator to modify this genome
	 *
	 * @return A boolean indicating whether the genome changed
	 */
	bool mutate(void)
    {
        if (_mutator.mutate(_content, _baseMutator))
        {
        	_hasFitness = false;
        	_evaluationCount = 0;
        	_offspringCount = 0;
        	return true;
        }
        // else

        return false;
    }


	/**
	 * Calls mutator to modify this genome. For use with individual sigma values per gene
	 *
	 * @return A boolean indicating whether the genome changed
	 */
	bool mutateWithIndividualSigmas(bool adaptSigma)
    {
        if (_mutator.mutate(_content, _baseMutator, _sigmas))
        {
        	_hasFitness = false;
        	_evaluationCount = 0;
        	_offspringCount = 0;
        	return true;
        }
        // else

        return false;
    }

	bool mutateWithAdaptiveSigma(bool adaptSigma)
    {
		//Learnin
		double tau(1/sqrt(2*GENECOUNT));

		// mutate sigma
		if(adaptSigma){
			//Use one value copied along the array
			Gene newSigma( *(_sigmas.begin()) * exp( tau * Mathlib::getGaussianRand(Gene(0.0), Gene(1.0)) ));

			//
			// Extension: limit sigma ralitive to the range between upper- and lower bounds
			//
			if (!_sigmaBounds.empty())
			{
				_sigmaBounds.boundHard(newSigma);
			}

			for(LIOReal* gene = _sigmas.begin(); gene != _sigmas.end(); ++gene) *gene = newSigma;
		}
		//Mutate genes (use same mutator, set sigma adaptation to false)
        if (_mutator.mutate(_content, _baseMutator, _sigmas))
        {
        	_hasFitness = false;
        	_evaluationCount = 0;
        	_offspringCount = 0;
        	return true;
        }
        // else

        return false;
    }

	//Mutate with derandomized self-adaptation
	bool mutateDerandomized()
	{
		//b factor (sqrt(1/n)),
		double b = sqrt(1/(double)GENECOUNT);

		//bscal factor (1/n)
		double bscal = 1/(double)GENECOUNT;

		//Mutation of sigmas depends on the mutation values of the genes themselves
		//	so order is not important and the sigmas can be mutated last

		//Create z vector
		Chromosome<Gene> z;
		for(LIOReal* pos = z.begin(); pos != z.end(); ++pos) *pos = Gene(Mathlib::getGaussianRand(0, 1));

		//Decide ksi
		double ksi;
		if(Mathlib::getBool()){
			ksi = 1.4;
		}else{
			ksi = 0.7142;
		}

		//Mutate with z vector and current sigmas
		LIOReal *s = _sigmas.begin();
		LIOReal *zi = z.begin();
		for(LIOReal* gene = _content.begin(); gene != _content.end(); ++gene){
			*gene += ksi * (*s) * (*zi);
			s++;
			zi++;
		}

		//Mutate sigmas with z vector
		zi = z.begin();
		for(s = _sigmas.begin(); s != _sigmas.end(); ++s){
			*s = *s * pow(ksi, b) * pow(exp(abs(*zi)-0.798), bscal);
			zi++;
		}

		return true;
	}

    /** Calls recombinator to replace this genome with one created from \c this and \c partner.
	 *
	 * @param partner
	 * @return A boolean indicating whether the genome changed
	 */
	bool recombine(const Genome & partner)
    {
		typedef Genetics::AveragingCrossover< Chromosome, Gene > SigmaCombinator;
		SigmaCombinator sigmaXOver(1.0, SigmaCombinator::GEOMETRIC);
		sigmaXOver.recombine(_sigmas, partner._sigmas);

        if (_xover.recombine(_content, partner._content))
        {
        	_hasFitness = false;
        	_evaluationCount = 0;
        	_offspringCount = 0;
        	return true;
        }
        // else

        return false;
    }

    /// @return The contained chromosome (as a const reference).
	const Chromosome<Gene> & getChromosome(void) const
    {
        return _content;
    }

	void setChromosome(Gene *genes)
	{
		int pos = 0;
		for(Gene *g = _content.begin(); g!=_content.end(); g++) *g = genes[pos++];
	}

    /// @return The contained sigmas (as a const reference).
	const Chromosome<Gene> & getSigmas(void) const
    {
        return _sigmas;
    }

	void setSigmas(Gene *sigmas)
	{
		int pos = 0;
		for(Gene *s = _sigmas.begin(); s!=_sigmas.end(); s++) *s = sigmas[pos++];
	}

	/**
	 * @return The utility of this genome
	 */
	LIOReal getFitness() const
    {
        return _fitness;
    }

	/**
	 * Set _fitness of this genome to \c fitness
	 */
    void setFitness(LIOReal fitness)
    {
        _fitness = fitness;
        _hasFitness = true;
        _evaluationCount++;
    }

    /// Indicates if the fitness for this genome has ever been set.
    bool hasFitness(void) const
    {
    	return _hasFitness;
    }

    void setID(unsigned int id)
    {
    	id_ = id;
    }

    unsigned int getID()
    {
    	return id_;
    }

    /// Re-initiliase the chromosome and unset the fitness
	void reset(void)
	{
		reset(0.8);
	}

    /// Re-initiliase the chromosome and unset the fitness
    void reset(double sigma)
    {
    	_hasFitness = false;
    	_evaluationCount = 0;
    	_offspringCount = 0;
        _init.initialise(_content);
        std::fill(_sigmas.begin(), _sigmas.end(), sigma);
    }
	/**
	 * @return Reference to this Genome's BaseMutator. Use this to access it and modify any settings
	 */
	BaseMutator<Gene>& getBaseMutator(void) { return _baseMutator; }

	/**
	 * @return Const reference to this Genome's BaseMutator.
	 */
	const BaseMutator<Gene>& getBaseMutator(void) const { return _baseMutator; }

	/**
	 * @return Reference to this Genome's Mutator. Use this to access the Mutator and modify any settings
	 */
	Mutator<Chromosome,Gene>& getMutator(void) { return _mutator; }

	/**
	 * @return Const reference to this Genome's Mutator.
	 */
	const Mutator<Chromosome,Gene>& getMutator(void) const { return _mutator; }

	/**
     * @return Reference to this Genome's Recombinator. Use this to access the Recombinator and modify any settings
     */
    Recombinator<Chromosome,Gene>& getRecombinator(void) { return _xover; }

    /**
     * @return Const reference to this Genome's Recombinator.
     */
    const Recombinator<Chromosome,Gene>& getRecombinator(void) const { return _xover; }

    /**
     * @return Reference to this Genome's Initialiser. Use this to access the Initialiser and modify any settings.
     * @note You must re-initialise by calling \c reset() the genome for changes in the Initialiser to have any effect.
     */
    Initialiser<Chromosome,Gene>& getInitialiser(void) { return _init; }

    /**
     * @return Const reference to this Genome's Initialiser.
     */
    const Initialiser<Chromosome,Gene>& getInitialiser(void) const { return _init; }

    /**
     * @return The number of evaluations for this genome. I.e., the number of
     * times the fitness has been set since the last change (be it through
     * initialisation, mutation or crossover) to the contained chromosome.
     */
    unsigned getEvaluationCount(void) const
    {
    	return _evaluationCount;
    }

    void increaseOffspringCount(void)
    {
    	_offspringCount++;
    }

    unsigned getOffspringCount(void)
    {
    	return _offspringCount;
    }

	/// Set _sigmaBounds from bounds
	void setSigmaBounds(const LIOUtilities::Bounds<Gene>& bounds)
	{
		if (!bounds.empty())
		{
			_sigmaBounds.lower() = Gene(0.0);
			_sigmaBounds.upper() = bounds.range() * Gene(3.0);
		}
	}

	void setSigmas(double value)
	{
		Gene newSigma(value);
		// Limit sigma ralitive to the range between upper- and lower bounds
		if (!_sigmaBounds.empty()) _sigmaBounds.boundHard(newSigma);
		for(LIOReal* gene = _sigmas.begin(); gene != _sigmas.end(); ++gene) *gene = newSigma;
	}

private:
	/// The actual genetic material
    Chromosome<Gene> _content;

	/// Sigma values for individual genomes
    Chromosome<Gene> _sigmas;

    /// The Genome's fitness. Must be set from outside.
    LIOReal _fitness;

    /// Indicates whether or not the fitness has been set since initialisation or the last reset()
    bool _hasFitness;

    //Id used by distributed
    unsigned int id_;


	/// Base Mutation policy (i.e., per Gene)
	BaseMutator<Gene> _baseMutator;
    /// Mutation policy
    Mutator<Chromosome,Gene> _mutator;
    /// Recombination policy
    Recombinator<Chromosome,Gene> _xover;
    /// Initialisation policy
    Initialiser<Chromosome, Gene> _init;

    /// Maintains the number of evaluations
    unsigned _evaluationCount;
    // Number of offspring
    unsigned _offspringCount;

	/// Upper- and lower bounds for self-adaptive sigma, set to 3*_bouds.range()
	LIOUtilities::Bounds<Gene> _sigmaBounds;
};

/// @}

}
#endif // GENOME_H
