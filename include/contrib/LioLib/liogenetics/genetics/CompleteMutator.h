/*
 *  $Id: CompleteMutator.h 2565 2010-07-01 10:09:09Z evert $
 *
 *  Created on: Feb 19, 2009
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

#ifndef COMPLETEMUTATOR_H_
#define COMPLETEMUTATOR_H_

#include <genetics/GeneticConfig.h>
#include <lioutils/Mathlib.h>
#include <cstring>

namespace Genetics
{

/** \addtogroup Genetics */
/// @{

/**
 * Implements mutation of all genes in a chromosome. The mutation of each
 * individual gene is performed by a RealMutator instance.
 *
 * @tparam BaseMutator policy class that defines the actual mutation per gene,
 *  for example <tt>RealMutation\<Gene\><\tt>.
 * @tparam Chromosome is a policy class that implements a container of \c Genes
 *  (e.g, <tt>std::vector\<Fixed32\></tt>)
 * @tparam Gene is a placeholder that defines the gene type.
 *
 * TODO: Mutation rate from policy, see e.g., GeneBasedMutationRate
 */
template<
	template <typename Gene> class Chromosome,
	typename Gene >
class CompleteMutator
{
public:
	/// Useful shorthand for the chromosome template argument
	typedef Chromosome<Gene> Container;

	/**
	 * Vanilla constructor.
	 *
	 * Sets \c _probability to 1.0.
	 */
	CompleteMutator(void) :
		_probability(ProbabilityType(1.0))
	{};


	/**
	 * Constructor.
	 *
	 * @param probability Sets \c _probability.
	 */
	CompleteMutator(ProbabilityType probability) :
		_probability(probability)
	{};

	/**
	 * If required, based on the flip a biased coin (bias defined by _probability), mutates one randomly selected \c Gene from \c chromosome.
	 * @param chromosome To be modified
	 * @return Boolean indicating whether \c chromosome has indeed changed.
	 */
	template <class BaseMutator>
	bool mutate(Container& chromosome, const BaseMutator& baseMutator)
	{
		bool changed = false;
		for (size_t i = 0; i < chromosome.size(); ++i)
		{
			if (Mathlib::getBool(_probability))
			{
				changed = baseMutator.mutate(chromosome[i]) || changed;
			}
		}
		return changed;
	}
	/**
	 * If required, based on the flip a biased coin (bias defined by _probability), mutates one randomly selected \c Gene from \c chromosome.
	 * @param chromosome To be modified
	 * @return Boolean indicating whether \c chromosome has indeed changed.
	 */
	template <class BaseMutator>
	bool mutate(Container& chromosome, const BaseMutator& baseMutator, Container& sigmas)
	{
		bool changed = false;
		for (size_t i = 0; i < chromosome.size(); ++i)
		{
			if (Mathlib::getBool(_probability))
			{
				changed = baseMutator.mutate(chromosome[i], sigmas[i]) || changed;
			}
		}
		return changed;
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
	/// Probability of a mutation occurring
	ProbabilityType _probability;
};
/// @}
}

#endif /* COMPLETEMUTATOR_H_ */
