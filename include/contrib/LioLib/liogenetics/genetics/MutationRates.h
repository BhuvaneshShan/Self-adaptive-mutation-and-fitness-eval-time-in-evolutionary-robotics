/*
 *  $Id: MutationRates.h 1338 2009-07-31 14:33:01Z evert $
 *
 *  Created on: Jul 31, 2009
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

#ifndef MUTATIONRATES_H_
#define MUTATIONRATES_H_


#include <genetics/GeneticConfig.h>
#include <genetics/RealMutation.h>

namespace Genetics
{

/** \addtogroup Genetics
 * @{
 */

/**
 * Returns a single number as the fixed mutation rate for any Gene
 */
template<typename Gene>
class FixedMutationRate
{
public:
	/**
	 * Sets _probability
	 */
	FixedMutationRate(ProbabilityType probability) :
		_probability(probability)
	{
		//nix
	}

	/**
	 * Copy ctor
	 */
	FixedMutationRate(const FixedMutationRate& org) :
		_probability(org._probability)
	{
		//nix
	}

	/**
	 *  @param gene Ignored.
	 *  @return \c _probability\
	 */
	ProbabilityType getProbability(Gene& gene) const
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

/**
 * Returns mutation rate for every Gene.
 *
 * @tparam Gene The type of Gene, must support getMutationRate();
 */
template<typename Gene>
class GeneBasedMutationRate
{
public:
	GeneBasedMutationRate(void)
	{
		//nix
	}
	/**
	 *  @param gene The gene that determines its own mutation rate.
	 *  @return \c gene.getMutationRate()\
	 */

	ProbabilityType getProbability(Gene& gene) const
	{
		return gene.getMutationRate();
	}
};

 ///@}
#endif /* MUTATIONRATES_H_ */
