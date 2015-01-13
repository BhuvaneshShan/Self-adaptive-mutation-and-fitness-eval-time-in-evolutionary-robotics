/*
 *  $Id: Evolver.cpp 1338 2009-07-31 14:33:01Z evert $
 *
 *  Created on: 12-Feb-2009
 *      Author: ehaasdi
 *
 */

#include <genetics/Evolver.h>

namespace Genetics
{
const char EvolverTaskID[] = "Evolver"; // only three characters are used...
}

#ifdef UNITTEST

#include <test/TestHarness.h>
using namespace Genetics;

typedef Genome< RealMutation, PointMutator, OnePointCrossover, RandomInitialiser, FixedLinearChromosome, Fixed32 > SimpleGenome;

//---------------------------------------------------------------------------
// Test case

TEST( Evolver, all )
{
static	Genetics::Evolver<> e;
	SimpleGenome g;
	g.setFitness(9);

	CHECK(e.consider(g, false));
}

#endif
