/*
 * $Id: SphereFitness.h$
 *
 *  Created on: Jun 23, 2010
 *      Author: ehaasdi
 */

#ifndef SPHEREFITNESS_H_
#define SPHEREFITNESS_H_

#include <genetics/GeneticConfig.h>

namespace Genetics
{
	class SphereFitness
	{
	public:
		SphereFitness();
		virtual ~SphereFitness();

		//Fitness for the standard sphere problem
		LIOReal operator()(worldModel_t& wm);
	};
}


#endif /* SPHEREFITNESS_H_ */
