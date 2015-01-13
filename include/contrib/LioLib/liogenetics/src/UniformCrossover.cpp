/*
 * $Id: UniformCrossover.cpp 1338 2009-07-31 14:33:01Z evert $
 *
 *  Created by evert haasdijk on 31/07/2009.
 *
 */

#include <genetics/UniformCrossover.h>

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
TEST( UniformCrossover, crossover )
{	
	__ChromType<char> dad;
	__ChromType<char> mom;
	
	std::fill(dad.begin(), dad.end(), 'D');
	std::fill(mom.begin(), mom.end(), 'M');
	
	Genetics::UniformCrossover< __ChromType, char > xover((ProbabilityType)1.0);
	
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
		
		// loop until first 'M', expect before end
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
			if (dad[i] == 'M')
			{
				foundM = true;
			}
			++i;
		}
		
		CHECK(foundM);
	}
	
	Genetics::UniformCrossover< __ChromType, char > xover2(0.0);
	
	CHECK(!xover2.recombine(dad, mom));
}

#endif // UNITTEST
