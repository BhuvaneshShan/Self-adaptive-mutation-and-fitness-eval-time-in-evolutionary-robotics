/*
 *  $Id: AveragingCrossover.h 2565 2010-07-01 10:09:09Z evert $
 *
 *  Created on: Oct 5, 2009
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

#ifndef AVERAGINGCROSSOVER_H_
#define AVERAGINGCROSSOVER_H_

#include <genetics/GeneticConfig.h>
#include "lioutils/Mathlib.h"

#include <algorithm>
#include <stdexcept>

namespace Genetics
{
/** \addtogroup Genetics
 * @{
 */
/**
 * Implements recombination of two chromosomes by averaging individual genes.
 *
 * @tparam Chromosome is a policy class that implements a container of \c Genes (e.g, <tt>std::vector\<Fixed32\></tt>)
 * @tparam Gene is a placeholder that defines the gene type.
 */
template< template <typename Gene> class Chromosome, typename Gene>
class AveragingCrossover
{
public:
	/// Useful shorthand for the chromosome template argument
	typedef Chromosome<Gene> Container;

	enum AveragingType {
		ARITHMETIC,
		GEOMETRIC
	};

	/**
	 * Vanilla constructor.
	 *
	 * Sets \c _probability to 1.0
	 */
	AveragingCrossover(void) :
		_probability(1.0),
		_averagingType(ARITHMETIC)
	{
		// nix
	}

	/**
	 * Constructor.
	 *
	 * @param probability Sets \c _probability of crossover.
	 */
	AveragingCrossover(ProbabilityType probability, AveragingType averagingType) :
		_probability(probability),
		_averagingType(averagingType)
	{
		// nix
	}

	virtual ~AveragingCrossover(void) {};

	/**
	 * Modifies \c dad, averaging individual genes with the values from
	 * \c mom (with \c _probability determining the likelihood of actually
	 * performing crossover).
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

		bool change = Mathlib::getBool(_probability);

		if (change)
		{
			for (unsigned i = 0; i < size; ++i)
			{
				switch (_averagingType)
				{
					case ARITHMETIC:
					{
							dad[i] = (dad[i] + mom[i]) / Gene(2.0);
							break;
					}
					case GEOMETRIC:
					{
						dad[i] = sqrt(dad[i]*mom[i]);
						break;
					}
					default:
					{
						throw std::logic_error("unexpected _averagingType");
						// break;
					}
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
private:
	/// Probability of crossover actually occurring
	ProbabilityType _probability;

	/// How to calculate the average value of genes
	AveragingType _averagingType;
};

}

#endif /* AVERAGINGCROSSOVER_H_ */
