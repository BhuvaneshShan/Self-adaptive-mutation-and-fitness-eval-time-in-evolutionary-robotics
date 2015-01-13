/*
 *  $Id: CompleteMutator.cpp 1453 2009-08-31 10:07:57Z evert $
 *  tests
 *
 *  Created by evert haasdijk on 30/07/2009.
 *
 */

#include <genetics/CompleteMutator.h>

#ifdef UNITTEST

#include <test/TestHarness.h>
#include <genetics/RealMutation.h>

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

class SetTo1
{
public:
	bool mutate(double& gene) const
	{
		gene = 1;
		return true;
	}
};
//---------------------------------------------------------------------------
// Test case
TEST( CompleteMutator, mutate)
{
	__ChromType<double> org;
	std::fill(org.begin(), org.end(), 3.14);

	Genetics::CompleteMutator<__ChromType, double > mutator(1.0);
	CHECK(mutator.getProbability() == (ProbabilityType) 1.0);
	for (int i =0 ; i< 1000; i++)
		CHECK(Mathlib::getBool(mutator.getProbability()));

	Genetics::RealMutation<double> foo;
	bool change = mutator.mutate(org, foo);
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

	Genetics::CompleteMutator< __ChromType, double> nop(0.0);

	CHECK(!nop.mutate(org, foo));

	CHECK(mutator.mutate(org, SetTo1()));

	CHECK(org[0] == 1);
	for (int i = 0; i < TESTCHROMLEN; ++i)
	{
		CHECK(org[i] == 1);
	}
}

#endif // UNITTEST
