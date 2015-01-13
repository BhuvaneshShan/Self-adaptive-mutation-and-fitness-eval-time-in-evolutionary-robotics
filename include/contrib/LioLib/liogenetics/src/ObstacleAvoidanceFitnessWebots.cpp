/*
 *  $Id: ObstacleAvoidanceFitnessD3D.cpp 1683 2009-11-17 13:46:24Z evert $
 *
 *  Created on: Aug 20, 2009
 *      Author: ehaasdi
 *
 * Copyright (c) 2009, VU University Amsterdam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef OBSTACLE_AVOIDANCE

#include <genetics/ObstacleAvoidanceFitnessWebots.h>

#include <algorithm>
#include <cmath>
#include <DistanceSensorNormaliser.h>

//#define DEBUG_FITNESS

namespace Genetics
{

ObstacleAvoidanceFitnessWebots::ObstacleAvoidanceFitnessWebots()
{
	// nix
}

ObstacleAvoidanceFitnessWebots::~ObstacleAvoidanceFitnessWebots()
{
	// nix
}

LIOReal ObstacleAvoidanceFitnessWebots::operator()(worldModel_t& wm, LIOReal currentFitness)
{
	//
	// retrieve sensor values to range closest obstacle
	//
	double minSensorValue(1.0);
	for ( int i = 0; i != 8; i++ ) minSensorValue = std::min(minSensorValue, wm.irSensor[i]);

	//
	// retrieve actuator values and estimate rotational and translational speeds
	//
	double v1 = double(wm.velocity[0]) / 100.0;
	double v2 = double(wm.velocity[1]) / 100.0;
	double translationalSpeed = std::min(fabs(v1), fabs(v2));
	double rotationalSpeed = fabs(v1 - v2) / 2.0;
	double epsilon (0.05); // sensitivity threshold: values lower than this are interpreted as touching some obstacle
	double minFitness(0);//epsilon * 0.1 * minSensorValue);
	LIOReal delta(minFitness);
	if (minSensorValue > epsilon){
		delta += LIOReal(translationalSpeed * ( 1.0 - rotationalSpeed ) * minSensorValue);
	}else{
		#ifdef DEBUG_FITNESS
		std::cout << " ...bumpity bump\n";
		#endif
	}

	return currentFitness + delta;
}

/*
LIOReal ObstacleAvoidanceFitnessWebots::operator()(worldModel_t& wm, LIOReal currentFitness)
{
	//
	// retrieve sensor values to range closest obstacle
	//
	double minSensorValue(1.0);

	webotsSpecific::DistanceSensorNormaliser normaliser;
	for ( int i = 0; i != 8; i++ )
	{
		double d = normaliser(wm.irSensor[i]);

		#ifdef DEBUG_FITNESS
		std::cout << wm.irSensor[i] << " => " << d << "; ";
		#endif

		minSensorValue = std::min(minSensorValue, d);
	}

	//
	// retrieve actuator values and estimate rotational and translational speeds
	//
	double v1 = double(wm.velocity[0]) / 100.0;
	double v2 = double(wm.velocity[1]) / 100.0;

	#ifdef DEBUG_FITNESS
	std::cout << "vel. " << wm.velocity[0] << " => " << v1  << ';' << wm.velocity[1] << " => " << v2 << std::endl;
	if (fabs(v1) > 1.0 || fabs(v2) > 1.0) std::cerr << "**** Speed out of range: v1=" << v1 << ", v2=" << v2 << std::endl;
	#endif

	double translationalSpeed = std::min(fabs(v1), fabs(v2));
	double rotationalSpeed = fabs(v1 - v2) / 2.0;

	double epsilon (0.05); // sensitivity threshold: values lower than this are interpreted as touching some obstacle
	double minFitness(0);//epsilon * 0.1 * minSensorValue);
	LIOReal delta(minFitness);
	if (minSensorValue > epsilon)
	{
		delta += LIOReal(
				translationalSpeed
				* ( 1.0 - rotationalSpeed )
				* minSensorValue);


		#ifdef DEBUG_FITNESS
		std::cout << "incrementing fitness: " << delta << ", minSensorValue: " << minSensorValue << ", Vrot: " << rotationalSpeed << ", Vtrans: " << translationalSpeed << '\n';
		#endif
	}
	else{
		#ifdef DEBUG_FITNESS
		std::cout << " ...bumpity bump\n";
		#endif
	}

	return currentFitness + delta;
}
*/
}

#endif
