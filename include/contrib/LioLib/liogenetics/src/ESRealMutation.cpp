/*
 *  $Id: ESRealMutation.cpp 2736 2010-09-23 10:19:02Z evert $
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

#include <genetics/ESRealMutation.h>

#ifdef UNITTEST

#include <test/TestHarness.h>
#include <algorithm>
#include <iostream>

//---------------------------------------------------------------------------
// Test case
TEST( ESRealMutation, construction )
{
	typedef double real;
	Genetics::ESRealMutation<real> foo;

	foo.setBounds(0.1, 1.0);
	real bar(42);
	real sigma(1);
	CHECK(foo.mutate(bar, sigma));
	CHECK(bar <= real(1.0));
	CHECK(bar >= real(0.1));
}
TEST(ESRealMutation, calculations)
{
	double _tauLocal (2.9);
	double _tauGlobal(0.16);
	const double epsilon(1.0e-40);



	double sigma(1);

	for (int i = 0; i < 1000; i++)
	{
		double global = _tauGlobal * Mathlib::getGaussianRand(0.0, 1.0);
		double local = _tauLocal * Mathlib::getGaussianRand(0.0, 1.0);
		LIOUtilities::Bounds<double> _sigmaBounds(0.0, 24.0);

		sigma = std::max(epsilon, sigma * exp(global + local));

		_sigmaBounds.boundHard(sigma);

		std::cout << "sigma[" << i << "]: " << sigma << std::endl;
	}
}

#endif
