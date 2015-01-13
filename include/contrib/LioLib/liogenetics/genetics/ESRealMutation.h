/*
 *  $Id: ESRealMutation.h 2733 2010-09-23 10:18:22Z evert $
 *
 *  Created on: Oct 6, 2009
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

#ifndef ESREALMUTATION_H_
#define ESREALMUTATION_H_

#include <lioutils/Bounds.h>
#include <lioutils/Mathlib.h>

#include <algorithm>
#include <cmath>

/**
 *  @author ehaasdi
 */
namespace Genetics
{

/** \addtogroup Genetics
 * @{
 */
/**
 * Atomic modification of single real-valued gene. If bounds are specified, the
 * result values are bounded. Co-evolves sigma values per Gene.
 *
 * @tparam Gene Underlying real type. Must be castable to double
 */
template <typename Gene>
class ESRealMutation
{
public:
	ESRealMutation(void) :
		_tauLocal (0.3),
		_tauGlobal(0.15)
		{


			// From EO code:
			// unsigned size = bounds.size(); << we know this to be 18 in our experiments
			// TauLcl = _init.TauLcl();
			//         TauGlb = _init.TauGlb();
			//         // renormalization
			//         TauLcl /= sqrt( 2.0 * sqrt(double(size)) ); ~ 3
			//         TauGlb /= sqrt( 2.0 * double(size) ); ~ 6
		}

	virtual ~ESRealMutation() {}

	/**
	 * Uncorrelated Mutation with adapting local step size.
	 * Modifies \c gene  by adding to it from gaussian distribution with
	 * \c gene.getSigma() standard deviation. First, however, mutates the
	 * gene's sigma.
	 * The resulting value is guaranteed to
	 * be between \c _lowerBound and \c upperBound by virtue of bouncing
	 * bounds.
	 *
	 * Algorithm from Eiben & Smith, Introduction to Evolutionary Computation, p. 76
	 *
	 * @param gene Reference to the Gene to be modified
	 * @param sigma Reference to the associated sigma
	 * @return Boolean indicating whether \c gene has indeed changed.
	 */
	bool mutate(Gene& gene, Gene& sigma) const
	{
//		if (adaptSigma)
//		{
//			const Gene epsilon(1.0e-40);
//
//			//
//			// Extension: limit sigma ralitive to the range between upper- and lower bounds
//			//
//			if (!_sigmaBounds.empty())
//			{
//				_sigmaBounds.boundHard(sigma);
//			}
//
//			Gene global = Gene(_tauGlobal) * Gene(Mathlib::getGaussianRand(Gene(0.0), Gene(1.0)));
//			Gene local = Gene(_tauLocal) * Gene(Mathlib::getGaussianRand(Gene(0.0), Gene(1.0)));
//
//			sigma = std::max(epsilon, sigma * Gene(exp( global + local)));
//
//			//
//			// Extension: limit sigma ralitive to the range between upper- and lower bounds
//			//
//			if (!_sigmaBounds.empty())
//			{
//				_sigmaBounds.boundHard(sigma);
//			}
//		}


        gene += sigma * Gene(Mathlib::getGaussianRand(Gene(0.0), Gene(1.0)));
        if (!_bounds.empty())
        {
        	_bounds.boundBouncing(gene);
        }

        return true;
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

	/**
	 * Set _tauLocal and _tauGlobal to optimal values for linear
	 * chromosome of length \c chromLen. Algorithm from
	 * Eiben & Smith, Introduction to Evolutionary Computation, p. 76
	 */
	void deriveTau(unsigned chromLen, bool multi_step)
	{
		double n(chromLen);
		_tauGlobal = 1.0 / sqrt(2.0 * n);
		_tauLocal = multi_step ? (1.0 / sqrt(2.0 * sqrt(n))) : 0.0;
	}

	void setSigmaBounds(double lower, double upper)
	{
		_sigmaBounds.lower() = Gene(lower);
		_sigmaBounds.upper() = Gene(upper);
	}

private:
	/// Upper- and lower bounds
	LIOUtilities::Bounds<Gene> _bounds;
	/// Upper- and lower bounds for self-adaptive sigma, set to 3*_bouds.range()
	LIOUtilities::Bounds<Gene> _sigmaBounds;
 	/// Local factor for mutation of std deviations.
	double 	_tauLocal;
 	/// Global factor for mutation of std deviations.
	double 	_tauGlobal;

	/// Set _sigmaBounds from _bounds
	void setSigmaBounds()
	{
		if (!_bounds.empty())
		{
			_sigmaBounds.lower() = Gene(0.0);
			_sigmaBounds.upper() = _bounds.range() * Gene(3.0);
		}
	}
};

}

#endif /* ESREALMUTATION_H_ */
