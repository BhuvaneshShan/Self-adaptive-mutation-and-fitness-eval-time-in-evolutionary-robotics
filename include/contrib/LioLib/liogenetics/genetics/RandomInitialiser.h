/*
 * $Id: RandomInitialiser.h 2651 2010-08-20 09:41:23Z evert $
 *
 *      Author: ehaasdi
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

#ifndef RANDOMINITIALISER_H_
#define RANDOMINITIALISER_H_

#include "genetics/GeneticConfig.h"
#include <lioutils/Mathlib.h>

#include <algorithm>

//#define SEED {1.83427,-0.661236,2.11812,1.51796,-5.94952,-1.97846,-2.11108,2.09684,0.211501,1.70628,-0.824878,1.50674,0.910844,4.36569,-0.65015,2.10551,-1.28388,-2.58415}
//#define SEED {0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}
namespace Genetics
{

/** \addtogroup Genetics
 * @{
 */
/**
 *
 * Implements initialisation of a chromosome using uniform random values. If
 * bounds are specified, the initial values are bounded.
 *
 * @tparam Chromosome is a policy class that implements a container of \c Genes (e.g, <tt>std::vector\<Fixed32\></tt>)
 * @tparam Gene is a placeholder that defines the gene type.
 */
template< template <typename Gene> class Chromosome, typename Gene>
class RandomInitialiser
{
public:
	/// Useful shorthand for the chromosome template argument
	typedef Chromosome<Gene> Container;

	/// Vanilla constructor. Sets _lowerBound and _upperBound to 0
	RandomInitialiser(void) :
		_lowerBound(0),
		_upperBound(0)
	{}

	/**
	 * Fills the chromosome with random values
	 *
	 * @param chromosome To be modified
	 */
	void initialise(Container& chromosome)
	{
		std::generate(chromosome.begin(), chromosome.end(), *this);
	//	double value[] = SEED;
	//	std::copy(value, value+GENECOUNT, chromosome.begin());
	}

	/**
	 * Generates a single Gene value with Mathlib::getRand().
	 *
	 * If _lowerBound and _upperBound specify a range (i.e., they are not the
	 * same value), the return value is uniformly sampled within this range.
	 * Otherwise, just returns Mathlib::getRand().
	 *
	 * Allows instances to be used as functors in, e.g., std::generate().
	 */
	Gene operator()(void) {
		double range(_upperBound-_lowerBound);
		if (range > 0)
		{
			return Gene(Mathlib::getRand() * (LIOReal)range - LIOReal(range/2)); // uniform sampling within bounds
		}
		// else

		return Mathlib::getRand();
	}

	/**
	 * @return _lowerBound
	 */
	double getLowerBound() const
	{
		return _lowerBound;
	}

	/**
	 * Sets _lowerBound
	 */
	void setLowerBound(double lowerBound)
	{
		_lowerBound = lowerBound;
	}

	/**
	 * @return _upperBound
	 */
	double getUpperBound() const
	{
		return _upperBound;
	}

	/**
	 * Sets _upperBound
	 */
	void setUpperBound(double upperBound)
	{
		_upperBound = upperBound;
	}
	
	/**
	 * Set both lower- and upperBound
	 */
	void setBounds(double upper, double lower)
	{
		setLowerBound(lower);
		setUpperBound(upper);
	}

private:
	/**
	 * Minimum allowed value for a Gene.
	 *
	 * If _lowerBound and _upperBound have the
	 * same value, range checking is disabled.
	 */
	double _lowerBound;
	/**
	 * Maximum allowed value for a Gene.
	 *
	 * If _lowerBound and _upperBound have the
	 * same value, range checking is disabled.
	 */
	double _upperBound;
};
/// @}
}

#endif /* RANDOMINITIALISER_H_ */
