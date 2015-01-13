/*
 *  $Id: NeuralNetGenome.h 2651 2010-08-20 09:41:23Z evert $
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

#ifndef NEURALNETGENOME_H_
#define NEURALNETGENOME_H_

#include <lioutils/lio_global.h>
#include <genetics/RealMutation.h>
#include <genetics/CompleteMutator.h>
#include <genetics/ESRealMutation.h>
#include <genetics/FixedLinearChromosome.h>
#include <genetics/Genome.h>
#include <genetics/AveragingCrossover.h>
#include <genetics/RandomInitialiser.h>



namespace Genetics
{

/**
 * \addtogroup Genetics
 * @{
 */
/**
 * 	Linear Genome for simple neural net control. Enforces minimum and maximum
 *  weights and adds funcionality to update the mutation's sigma (i.e., its
 *  volatility).
 *  Probably a bit specific for it's name.
 */
class NeuralNetGenome: public Genome< RealMutation, CompleteMutator, AveragingCrossover, RandomInitialiser, FixedLinearChromosome, LIOReal >
{
public:
	/// Minimum possible fitness value
	static const LIOReal _minFitness;
	static const LIOReal _sigmaMin;
	static const LIOReal _sigmaMax;

	/**
	 * Constructor. Sets bounds for initialiser and mutator.
	 */
	NeuralNetGenome(void)
	{
		double minValue = -4; // big enough to comply with low sensory values.
		double maxValue = +4;

		getInitialiser().setBounds(minValue,maxValue);

		getBaseMutator().setBounds(minValue,maxValue);
		getBaseMutator().setSigma(_sigmaMin);

		setSigmaBounds(getBaseMutator().getBounds());

		reset();
	}

	/**
	 * Increases the volatility of mutation.
	 */
	void increaseSigma(void)
	{
		LIOReal sigma = getBaseMutator().getSigma();
		// update mutation rate -- **increase** (in any case)
		//sigma = sigma * 2;
		//sigma = sigma * 1.1;
		sigma = std::min(sigma * LIOReal(2), _sigmaMax);

		getBaseMutator().setSigma(sigma);
	}

	/**
	 * Decreases the volatility of mutation.
	 */
	void decreaseSigma(void)
	{
		double sigma = getBaseMutator().getSigma();

		// update mutation rate -- **decrease**
		// 1.
		//sigma = sigma * 0.8408964152537145; // sigma = sigma * 2**(-1/4)
		// 2.
		//sigma = sigma * 0.9;
		//if ( sigma < sigmaMin ) sigma = sigmaMin;
		// 3.
		sigma = _sigmaMin; // TODO: add policy for updating sigma (+ and -)

		getBaseMutator().setSigma(sigma);
	}

	/**
	 * Sets volatility of mutation to the minimum
	 */
	void setMinSigma(void)
	{
		getBaseMutator().setSigma(_sigmaMin);
	}

	/**
	 * @return current volatility level of mutation
	 */
	LIOReal getSigma(void) const
	{
		return getBaseMutator().getSigma();
	}

	/**
	 * Set current volatility level of mutation
	 */
	void setSigma(LIOReal sigma) const
	{
	//	getBaseMutator().setSigma(sigma);
	}
};

/**
 * 	Linear Genome for fixed-length real-valued chromosome. Self-updates each individual gene's the mutation sigma (i.e., its
 *  volatility).
 */
class ESGenome: public Genome< ESRealMutation, CompleteMutator, AveragingCrossover, RandomInitialiser, FixedLinearChromosome, LIOReal >
{
public:

	/**
	 * Constructor. Sets bounds for initialiser and mutator.
	 */
	ESGenome(void)
	{
		double minValue = -4; // big enough to comply with low sensory values.
		double maxValue = +4;

		getInitialiser().setBounds(minValue,maxValue);

		getBaseMutator().setBounds(minValue,maxValue);
		getBaseMutator().deriveTau(GENECOUNT, true); //TODO: set multiStep

		//Set sigma bounds
		getBaseMutator().setSigmaBounds(0, 50000);

		reset();
	}

	/// Minimum possible fitness value
	static const LIOReal _minFitness;
};

/// @}
}
#endif /* NEURALNETGENOME_H_ */

