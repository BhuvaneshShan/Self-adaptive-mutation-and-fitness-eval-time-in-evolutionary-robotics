/*
 * $Id: UniformCrossover.h 1338 2009-07-31 14:33:01Z evert $
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

#ifndef UNIFORMXOVER_H_
#define UNIFORMXOVER_H_

#include <algorithm>

#include <genetics/GeneticConfig.h>
#include <lioutils/Mathlib.h>

//#include <stddef.h>

namespace Genetics
{

/** \addtogroup Genetics
 * @{
 */
/**
 * Implements single-point crossover of two chromosomes.
 *
 * @tparam Chromosome is a policy class that implements a container of \c Genes (e.g, <tt>std::vector\<Fixed32\></tt>)
 * @tparam Gene is a placeholder that defines the gene type.
 */
template< template <typename Gene> class Chromosome, typename Gene>
class UniformCrossover
{
public:
	/// Useful shorthand for the chromosome template argument
	typedef Chromosome<Gene> Container;

	/**
	 * Vanilla constructor.
	 *
	 * Sets \c _probability to 0.5
	 */
	UniformCrossover(void) :
		_probability(0.5),
		_weight(0.5)
	{
		// nix
	}

	/**
	 * Constructor.
	 *
	 * @param probability Sets \c _probability of crossover
	 * @param weight Sets \c _weight of overwriting genes with values from partner
	 */
	UniformCrossover(ProbabilityType probability, ProbabilityType weight = 0.5) :
		_probability(probability),
		_weight(weight)
	{
		// nix
	}

	/**
	 * Modifies \c dad, overwriting individual genes with the values from 
	 * \c mom (with \c _probability determining the likelihood of 
	 * overwriting for every gene).
	 * 
	 * @note Will work if the chromosomes have different lengths: then, 
	 *  it just loops over the minimum length.
	 *
	 * @param dad "parent" that may be modified
	 * @param mom const reference to dad's partner
	 * @return Boolean indicating whether \c dad has indeed changed.
	 */
	bool recombine(Container& dad, const Container& mom)
	{
		unsigned size = std::min(dad.size(), mom.size());
		
		bool change = false;
		
		if (Mathlib::getBool(_probability))
		{
			for (unsigned i = 0; i < size; ++i)
			{
				if (Mathlib::getBool(_weight))
				{
					dad[i] = mom[i];
					change = true;
				}
			}
		}

		return change;
	}

	/// @return \c _probability
    ProbabilityType getProbability() const
    {
        return _probability;
    }
    /// sets \c _probability
    void setProbability(ProbabilityType probability)
    {
        _probability = probability;
    }
	
	/// @return \c _weight
    ProbabilityType getWeight() const
    {
        return _weight;
    }
    /// sets \c _weight
    void setWeight(ProbabilityType weight)
    {
        _weight = weight;
    }
	
private:
	/// Probability of crossover actually occurring
	ProbabilityType _probability;
	
	/// Weighting factor: the likelihood of overwriting a gene with the value from the partner
	ProbabilityType _weight;
};

///@}
}

#endif /* UNIFORMXOVER_H_ */
