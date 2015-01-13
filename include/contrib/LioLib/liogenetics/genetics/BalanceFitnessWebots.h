/*
 *  $Id: BalanceFitnessWebots.h 2739 2010-09-23 10:21:03Z evert $
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

#ifndef BALANCEFITNESSD3D_H_
#define BALANCEFITNESSD3D_H_

#include <cmath>

#include <genetics/GeneticConfig.h>

//#define DEBUG_BALANCING_FITNESS

namespace Genetics
{

/**
 * \addtogroup Genetics
 * @{
 */
/**
 * Calculates fitness for screwdrive robot in Symbricator3D.
 */
class BalanceFitnessWebots
{
public:
	BalanceFitnessWebots()
	{}
	virtual ~BalanceFitnessWebots()
	{}

	/**
	 * Calculates fitness for webots e-puck with accelerometer.
	 *
	 * @param wm The worldmodel to calculate fitness from.
	 * @return Fitness increment for the current time-step (think 'reward' in reinforcement learning)
	 */
	double updateFitness(worldModel_t& wm, double currentFitness)
	{
		double fitness = degreesFromAccelerometer(wm.zAngle);

#ifdef DEBUG_BALANCING_FITNESS
		std::cout << "Pos(xyz): " << wm.xPos << ' ' << wm.yPos << ' ' << wm.zPos;
		std::cout << " dist: " << std::sqrt(wm.xPos*wm.xPos + wm.yPos*wm.yPos + wm.zPos*wm.zPos);
		std::cout << " Angles(xyz): " << degreesFromAccelerometer(wm.xAngle) << ' '
									  << degreesFromAccelerometer(wm.yAngle) << ' '
									  << degreesFromAccelerometer(wm.zAngle);
		std::cout << " Bearing: " << fmod(wm.bearing * 180.0 / M_PI,  360.0);
		std::cout << " Fitness: " <<  fitness;
		std::cout << std::endl;
#endif

		return (fitness + currentFitness) / 2;
	}

	double finaliseFitness(double fitness) { return fitness; }

private:
	double degreesFromAccelerometer(double measurement)
	{
		// Assumes that the measurement has been normalised for gravity to be between 0 and 1
		return (asin(measurement) * 180.0) / M_PI;
	}

};
///@}
}

#endif /* OBSTACLEAVOIDANCEFITNESSD3D_H_ */
