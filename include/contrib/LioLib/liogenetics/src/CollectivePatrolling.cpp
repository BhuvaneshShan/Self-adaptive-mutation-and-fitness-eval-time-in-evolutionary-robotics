/*
 * $Id: CollectivePatrolling.cpp$
 *
 *  Created on: Jul 21, 2010
 *      Author: ehaasdi
 */
#ifdef COLLECTIVE_PATROLLING

#include<genetics/CollectivePatrolling.h>
#include <DistanceSensorNormaliser.h>
#include <cmath>
#include <genetics/definitions.h>

namespace Genetics
{

	CollectivePatrolling::CollectivePatrolling(){}
	CollectivePatrolling::~CollectivePatrolling(){}

	LIOReal CollectivePatrolling::operator()(worldModel_t &wm)
	{
		//Fast forward component
		double minSensorValue(1.0);
		for ( int i=0; i!=8; i++) minSensorValue = std::min(minSensorValue, wm.irSensor[i]);

		// retrieve actuator values and estimate rotational and translational speeds
		double v1 = double(wm.velocity[0]) / 100.0;
		double v2 = double(wm.velocity[1]) / 100.0;
		double translationalSpeed = std::min(fabs(v1), fabs(v2));
		double rotationalSpeed = fabs(v1 - v2) / 2.0;
		double epsilon (0.05); // sensitivity threshold: values lower than this are interpreted as touching some obstacle
		double minFitness(0);//epsilon * 0.1 * minSensorValue);
		LIOReal delta(minFitness);
		if (minSensorValue > epsilon) delta += LIOReal(translationalSpeed * ( 1.0 - rotationalSpeed ) * minSensorValue);


		//Pheromone component
		double phf = 1 - wm.phrmFloor;


		#ifdef USE_PHEROMONES
			//	return phf;
				return phf*minSensorValue;
			//	return wm.pheromone_drop*phf;
			//  return 0.5*delta + 0.5*phf;
		#else
			return delta;
		#endif

	}

}

#endif
