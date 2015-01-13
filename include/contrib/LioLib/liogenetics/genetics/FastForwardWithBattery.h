/*
 * $Id: FastForwardWithBattery.h$
 *
 *  Created on: Apr 16, 2010
 *      Author: ehaasdi
 */

#ifndef FASTFORWARDWITHBATTERY_H_
#define FASTFORWARDWITHBATTERY_H_

#include <genetics/GeneticConfig.h>

namespace Genetics{

	class FastForwardWithBattery{

	public:
		FastForwardWithBattery();
		virtual ~FastForwardWithBattery();

		//	Standard FastForward fitness (as in ObstacleAvoidanceFitnessWebots) with
		//	an additional component that penalizes according to battery status
		LIOReal operator()(worldModel_t& wm);
	};

}

#endif /* FASTFORWARDWITHBATTERY_H_ */
