/*
 * $Id: Genome.cpp 1338 2009-07-31 14:33:01Z evert $
 */

#include <genetics/Genome.h>

#ifdef UNITTEST

#include <test/TestHarness.h>

#include "genetics/GeneticConfig.h"
#include "genetics/Genome.h"
#include "genetics/RealMutation.h"
#include "genetics/PointMutator.h"
#include "genetics/UniformCrossover.h"
#include "genetics/RandomInitialiser.h"
#include "genetics/FixedLinearChromosome.h"

#include <lioutils/Fixed32.h>

using namespace Genetics;

TEST( Genome, all )
{
	typedef Genome< RealMutation, PointMutator, UniformCrossover, RandomInitialiser, FixedLinearChromosome, Fixed32 > SimpleGenome;
	Genetics::RandomInitialiser<FixedLinearChromosome, Fixed32 > init;
	init.setBounds(0.0, 10.0);
	SimpleGenome g(init);

	CHECK(g.getChromosome().size() == GENECOUNT);

	for (int i = 0; i < GENECOUNT; ++i)
	{
		CHECK (g.getChromosome()[i] <= Fixed32(10.0) && g.getChromosome()[i] >= Fixed32(0.0) );
	}

	g.getBaseMutator().setBounds(0.0, 10.0);
	g.getMutator().setProbability(1.0);
	CHECK(g.mutate());

	SimpleGenome h;
	g.getRecombinator().setProbability(1.0);
	CHECK(g.recombine(h));
}
#endif
