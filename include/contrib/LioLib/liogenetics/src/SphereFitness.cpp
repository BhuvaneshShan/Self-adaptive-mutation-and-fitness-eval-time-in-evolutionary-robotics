/*
 * $Id: SphereFitness.cpp$
 *
 *  Created on: Jun 23, 2010
 *      Author: ehaasdi
 */
#ifdef SPHERE_TEST

#include <genetics/SphereFitness.h>
#include<math.h>

namespace Genetics
{
	SphereFitness::SphereFitness(){}
	SphereFitness::~SphereFitness(){}

	LIOReal SphereFitness::operator()(worldModel_t& wm)
	{
		double sum=0;
		for(int i=0; i<GENECOUNT; i++){
			sum += pow((wm.genes[i]/4),2);
		}

		return 1/sum;
	}
}

#endif
