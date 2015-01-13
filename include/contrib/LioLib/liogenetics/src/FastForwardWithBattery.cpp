/*
 * $Id: FastForwardWithBattery.cpp$
 *
 *  Created on: Apr 16, 2010
 *      Author: ehaasdi
 */

#ifdef FF_BATTERY

#include<genetics/FastForwardWithBattery.h>
#include <algorithm>
#include <cmath>
#include <DistanceSensorNormaliser.h>

namespace Genetics
{
	FastForwardWithBattery::FastForwardWithBattery()
	{

	}

	FastForwardWithBattery::~FastForwardWithBattery()
	{

	}

	LIOReal FastForwardWithBattery::operator()(worldModel_t& wm)
	{
		//Calculate speeds
		double v1 = double(wm.velocity[0]) / 100.0;
		double v2 = double(wm.velocity[1]) / 100.0;
		double translationalSpeed = std::min(fabs(v1), fabs(v2));
		double rotationalSpeed = fabs(v1 - v2) / 2.0;

		//Fitness = SpeedTranslational * (1 - SpeedRotational) * log(batery)
		LIOReal delta(0);
		if(wm.battery > 0){
			delta += LIOReal(
				translationalSpeed
				* ( 1.0 - rotationalSpeed )
				* ( log(wm.battery)/5 + 1 ) );
		}

		return delta;
	}

}

#endif
