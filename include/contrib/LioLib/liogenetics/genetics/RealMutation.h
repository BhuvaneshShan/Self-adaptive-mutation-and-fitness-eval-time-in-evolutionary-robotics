/*
 * $Id: RealMutation.h 2735 2010-09-23 10:18:52Z evert $
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

#ifndef REALMUTATION_H_
#define REALMUTATION_H_

#include <genetics/GeneticConfig.h>
#include <lioutils/Bounds.h>
#include <lioutils/Mathlib.h>

namespace Genetics
{

/** \addtogroup Genetics
 * @{
 */
/**
 * Atomic modification of single real-valued gene. If bounds are specified, the
 * result values are bounded.
 *
 * @tparam Gene Underlying real type. Must be castable to double and LIOReal.
 */
template <typename Gene = LIOReal>
class RealMutation
{
public:

	/**
	 * Constructor.
	 *
	 * Sets \c _sigma to 0.1
	 */
	RealMutation(void) :
		_sigma(0.1)
	{};

	/**
	 * Constructor.
	 *
	 * @param sigma Sets \c _sigma
	 */
	RealMutation(const Gene&  sigma) :
		_sigma(sigma)
	{};

	/**
	 * Modifies \c gene  by adding to it from gaussian distribution with
	 * \c _sigma standard deviation. The resulting value is guaranteed to
	 * be between \c _lowerBound and \c upperBound by virtue of bouncing
	 * bounds. Example: if the original value is 7, the upper bound is 10
	 * and mutation suggests adding 5, the value is updated as follows:
	 * 7..8..9..10 <BOING> ..9..8 = 8.
	 *
	 * @param gene Reference to the Gene to be modified
	 * @return Boolean indicating whether \c gene has indeed changed.
	 */
	bool mutate(Gene& gene) const
	{
		Gene delta(Mathlib::getGaussianRand(LIOReal(0.0), _sigma));
		Gene value = gene + delta;

		_bounds.boundBouncing(value);

		if (value == gene) return false;
		// else

		gene = value;
		return true;
	}

	/// @return _sigma
	Gene getSigma(void) const
	{
		return _sigma;
	}

	/// set _sigma
	void setSigma(const Gene& sigma)
	{
		_sigma = sigma;
	}

	/// @return const reference to _bounds
	const LIOUtilities::Bounds<Gene>& getBounds(void) const
	{
		return _bounds;
	}

	/// @return modifiable reference to _bounds
	LIOUtilities::Bounds<Gene>& getBounds(void)
	{
		return _bounds;
	}

	/**
	 * Set both lower- and upperBound
	 */
	void setBounds(double lower, double upper)
	{
		_bounds.lower() = Gene(lower);
		_bounds.upper() = Gene(upper);
	}

private:
	/// Maximum size of mutation
	Gene _sigma;

	/// Upper- and lower bounds
	LIOUtilities::Bounds<Gene> _bounds;
};
///@}
}

#endif /* REALMUTATION_H_ */
