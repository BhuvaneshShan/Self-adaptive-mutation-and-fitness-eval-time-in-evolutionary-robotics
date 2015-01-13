/*
 *  $Id: ObstacleAvoidanceFitnessPlayerStage.cpp 2565 2010-07-01 10:09:09Z evert $
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

#include <genetics/ObstacleAvoidanceFitnessPlayerStage.h>

#include <algorithm>
#include <cmath>

#include "syslog.h"

namespace Genetics
{

ObstacleAvoidanceFitnessPlayerStage::ObstacleAvoidanceFitnessPlayerStage()
{
	// nix

}

ObstacleAvoidanceFitnessPlayerStage::~ObstacleAvoidanceFitnessPlayerStage()
{
	// nix
}

LIOReal ObstacleAvoidanceFitnessPlayerStage::operator()(worldModel_t& wm, LIOReal currentFitness)
{
	//
	// retrieve sensor values to range closest obstacle
	//
	double minSensorValue(1.0);
	for ( int i = 0; i != 8; i++ )
	{
		double d = (static_cast<double>(wm.irSensor[i])) / 70.0; // normalize btw 0.0 and 1.0 for Player/Stage sensor

#ifdef DEBUG_FITNESS
		LOG << wm.irSensor[i] << " => " << LIOReal(d) << LOG << "; ";
#endif

		minSensorValue = std::min(minSensorValue, d);
	}

	//
	// retrieve actuator values to estimate speeds
	//
	double v1 = double(wm.velocity[0]) / 100.0;
	double v2 = double(wm.velocity[1]) / 100.0;

	double translationalValue = std::min(fabs(v1), fabs(v2));
	double rotationalValue = fabs(v1-v2);

	double epsilon (0.5); // sensitivity threshold: values lower than this are interpreted as touching some obstacle
	double minFitness(0.1 * 0.1 * minSensorValue);
	LIOReal delta(minFitness);
	if (minSensorValue > epsilon)
	{
		delta += LIOReal(
				translationalValue
				* ( 1.0 - rotationalValue )
				* minSensorValue);


#ifdef DEBUG_FITNESS
		LOG << "incrementing fitness: " ;
		printReal(delta);
		LOG << ", minSensorValue: "; printReal(LIOReal(minSensorValue)); LOG <<'\n';
#endif
	}
	else{
#ifdef DEBUG_FITNESS
		LOG << " ...bumpity bump\n";
#endif
	}

	return currentFitness + delta;
}


}
