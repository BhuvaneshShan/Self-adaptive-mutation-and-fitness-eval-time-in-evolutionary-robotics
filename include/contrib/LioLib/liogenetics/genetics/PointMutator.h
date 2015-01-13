/*
 * $Id: PointMutator.h 1338 2009-07-31 14:33:01Z evert $
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

#ifndef POINTMUTATOR_H_
#define POINTMUTATOR_H_

#include <genetics/GeneticConfig.h>
#include <lioutils/Mathlib.h>

#include <stdlib.h>

namespace Genetics
{

/** \addtogroup Genetics */
/// @{
/**
 * Implements mutation of one gene in a chromosome.
 *
 * @tparam Chromosome is a policy class that implements a container of \c Genes (e.g, <tt>std::vector\<Fixed32\></tt>)
 * @tparam Gene is a placeholder that defines the gene type.
 */

template<template <typename Gene> class Chromosome, typename Gene>
class PointMutator
{
public:
	/// Useful shorthand for the chromosome template argument
	typedef Chromosome<Gene> Container;

	/**
	 * Vanilla constructor.
	 *
	 * Sets \c _probability to 0.1.
	 * Sets \c _epsilon to 0.1.
	 */
	PointMutator(void) :
		_probability(0.1)
	{};


	/**
	 * Constructor.
	 *
	 * @param probability Sets \c _probability.
	 * @param epsilon Sets \c _epsilon.
	 */
	PointMutator(ProbabilityType probability) :
		_probability(probability)
	{};

	/**
	 * If required, based on the flip a biased coin (bias defined by _probability), mutates one randomly selected \c Gene from \c chromosome.
	 * @param chromosome To be modified
	 * @param mutator The BaseMutator to use
	 *
	 * @tparam BaseMutator policy class that defines the actual mutation per gene, 
	 *  for example <tt>RealMutation\<Gene\><\tt>.
	 *
	 * @return Boolean indicating whether \c chromosome has indeed changed.
	 */
	template< class BaseMutator>
	bool mutate(Container& chromosome, const BaseMutator& baseMutator)
	{
		if (Mathlib::getBool(_probability))
		{
			size_t loc = (size_t) Mathlib::getInt(chromosome.size());
			
			return baseMutator.mutate(chromosome[loc]);			
		}
		// else
		
		return false;
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

#endif /* POINTMUTATOR_H_ */
