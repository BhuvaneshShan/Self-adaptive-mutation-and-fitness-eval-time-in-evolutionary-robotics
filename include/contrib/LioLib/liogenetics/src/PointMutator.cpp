/*
 *  $Id: PointMutator.cpp 1338 2009-07-31 14:33:01Z evert $
 *
 *  Created by evert haasdijk on 31/07/2009.
 *
 */

#include <genetics/PointMutator.h>

#ifdef UNITTEST

#include <test/TestHarness.h>
#include <algorithm>

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
struct SillyMutatorForTest
{
	bool mutate(double& x) const { x -= 1.0; return true; }
};
TEST( PointMutator, mutation )
{
	__ChromType<double> org;
	std::fill(org.begin(), org.end(), 3.14);

	Genetics::PointMutator<__ChromType, double > mutator(1.0);
	CHECK(mutator.getProbability() == (ProbabilityType) 1.0);
	for (int i =0 ; i< 1000; i++)
		CHECK(Mathlib::getBool(mutator.getProbability()));

	SillyMutatorForTest sillyMutator;

	bool change = mutator.mutate(org, sillyMutator);
	CHECK(change);

	if (change)
	{
		bool foundChange = false;

		// loop until first change, expect before end
		for (int i = 0; i < TESTCHROMLEN; ++i)
		{
			if (org[i] != 3.14)
			{
				foundChange = true;
				break;
			}
			// else
		}

		CHECK(foundChange);
	}

	Genetics::PointMutator< __ChromType, double > nop(0.0);

	CHECK(!nop.mutate(org, sillyMutator));
}

#endif // UNITTEST
