/*
 * $Id: OnePointCrossover.cpp 1338 2009-07-31 14:33:01Z evert $
 */

#include <genetics/OnePointCrossover.h>

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
TEST( OnePointCrossover, crossover )
{
	static const unsigned chromLen = TESTCHROMLEN;

	__ChromType<char> dad;
	__ChromType<char> mom;

	for(unsigned i = 0; i < chromLen; ++i)
	{
		dad[i] = 'D';
		mom[i] = 'M';
	}

	Genetics::OnePointCrossover< __ChromType, char > xover((ProbabilityType)1.0);

	CHECK(xover.getProbability() == (ProbabilityType) 1.0);
	CHECK(dad.size() == mom.size());
	for (int i =0 ; i< 1000; i++)
		CHECK(Mathlib::getBool(xover.getProbability()));

	bool change = xover.recombine(dad, mom);
	CHECK(change);

	if (change)
	{
		unsigned length = dad.size();

		bool foundM = false;

		// loop until first 'M', expect before end, expect no further 'D'
		unsigned i = 0;
		while (dad[i] == 'D')
		{
			if (i == length - 1)
			{
				FAIL( "Xover reported, no change noted");
				break;
			}
			// else
			++i;
		}

		while (i < length)
		{
			if (dad[i] != 'M')
			{
				FAIL( "Expect only 'M', now" );
				break;
			} else {
				foundM = true;
			}
			++i;
		}

		CHECK(foundM);
	}

	Genetics::OnePointCrossover< __ChromType, char > xover2(0.0);

	CHECK(!xover2.recombine(dad, mom));
}

#endif // UNITTEST
