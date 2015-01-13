/*
 * $Id: SeedInitializer.h$
 *
 *  Created on: Apr 22, 2010
 *      Author: ehaasdi
 */

#ifndef SEEDINITIALIZER_H_
#define SEEDINITIALIZER_H_

#include "genetics/GeneticConfig.h"
#include <lioutils/Mathlib.h>

#include <algorithm>

#define SEED {1.37247,0.842781,-2.48526,1.91424,-2.61263,3.83909,0.0972982,0.0809432,0.840944,-0.682271,2.50144,-5.95933,-1.69964,1.09972,4.05125,2.38814,3.5029,-0.848567}

namespace Genetics
{

template< template <typename Gene> class Chromosome, typename Gene>
class SeedInitialiser
{
public:
	/// Useful shorthand for the chromosome template argument
	typedef Chromosome<Gene> Container;

	SeedInitialiser(void){}


	void initialise(Container& chromosome)
	{
		LIOReal value[] = SEED;
		std::copy(value, chromosome.begin(), GENECOUNT);
	}

}
}

#endif /* SEEDINITIALIZER_H_ */
