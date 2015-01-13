/*
 *  $Id: AveragingCrossover.cpp 2565 2010-07-01 10:09:09Z evert $
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

#include <genetics/AveragingCrossover.h>


#ifdef UNITTEST

#include <test/TestHarness.h>

using Genetics::ProbabilityType;

#define TESTCHROMLEN 18

template< class __Gene > class __ChromType  {
	public :
	__Gene foo [TESTCHROMLEN];
	unsigned size(void) const { return TESTCHROMLEN; }

	__Gene* begin(void) { return foo; }
	__Gene* end(void) { return foo + TESTCHROMLEN; }

	const __Gene* begin(void) const { return foo; }
	const __Gene* end(void) const { return foo + TESTCHROMLEN; }

	__Gene& operator[](int i) { return foo[i]; }
	__Gene operator[](int i) const { return foo[i]; }
};

//---------------------------------------------------------------------------
// Test case
TEST( AveragingCrossover, arithmeticAvg )
{
	__ChromType<float> dad;
	__ChromType<float> mom;

	std::fill(dad.begin(), dad.end(), 1.0);
	std::fill(mom.begin(), mom.end(), 0.0);

	Genetics::AveragingCrossover< __ChromType, float > xover((ProbabilityType)1.0, Genetics::AveragingCrossover< __ChromType, float >::ARITHMETIC);

	CHECK(xover.getProbability() == (ProbabilityType) 1.0);
	CHECK(dad.size() == mom.size());
	for (int i =0 ; i< 1000; i++)
		CHECK(Mathlib::getBool(xover.getProbability()));

	bool change = xover.recombine(dad, mom);
	CHECK(change);

	if (change)
	{
		unsigned length = dad.size();
		for (unsigned i = 0; i < length; ++i)
		{
			CHECK(dad[i] == 0.5);
		}
	}

	Genetics::AveragingCrossover< __ChromType, float > xover2(0.0, Genetics::AveragingCrossover< __ChromType, float >::ARITHMETIC);

	CHECK(!xover2.recombine(dad, mom));
}

TEST( AveragingCrossover, geometricAvg )
{
	__ChromType<float> dad;
	__ChromType<float> mom;

	std::fill(dad.begin(), dad.end(), 2.0);
	std::fill(mom.begin(), mom.end(), 8.0);

	Genetics::AveragingCrossover< __ChromType, float > xover((ProbabilityType)1.0, Genetics::AveragingCrossover< __ChromType, float >::GEOMETRIC);

	CHECK(xover.getProbability() == (ProbabilityType) 1.0);
	CHECK(dad.size() == mom.size());
	for (int i =0 ; i< 1000; i++)
		CHECK(Mathlib::getBool(xover.getProbability()));

	bool change = xover.recombine(dad, mom);
	CHECK(change);

	if (change)
	{
		unsigned length = dad.size();
		for (unsigned i = 0; i < length; ++i)
		{
			CHECK(dad[i] == 4);
		}
	}
}
#endif // UNITTEST
