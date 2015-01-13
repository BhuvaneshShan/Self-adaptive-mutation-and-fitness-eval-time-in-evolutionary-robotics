/*
 * $Id: CollectiveForaging.cpp$
 *
 *  Created on: Aug 5, 2010
 *      Author: ehaasdi
 */

#ifdef COLLECTIVE_FORAGING

#include <genetics/CollectiveForaging.h>

namespace Genetics
{
	CollectiveForaging::CollectiveForaging(){}
	CollectiveForaging::~CollectiveForaging(){}

	LIOReal CollectiveForaging::operator()(worldModel_t &wm)
	{
		double minSensorValue(1.0);
		for ( int i=0; i!=8; i++) minSensorValue = std::min(minSensorValue, wm.irSensor[i]);

		return wm.foodSensor + minSensorValue - 1;
	}
}


#endif
