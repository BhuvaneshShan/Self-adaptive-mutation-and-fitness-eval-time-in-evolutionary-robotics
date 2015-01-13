/*
 *  $Id: ObstacleAvoidanceFitnessPlayerStage.h 2565 2010-07-01 10:09:09Z evert $
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

#ifndef OBSTACLEAVOIDANCEFITNESSPLAYERSTAGE_H_
#define OBSTACLEAVOIDANCEFITNESSPLAYERSTAGE_H_

#include <genetics/GeneticConfig.h>

namespace Genetics
{

/**
 * \addtogroup Genetics
 * @{
 */
/**
 * Calculates fitness for e-puck robot in player/stage.
 */
class ObstacleAvoidanceFitnessPlayerStage
{
public:
	ObstacleAvoidanceFitnessPlayerStage();
	virtual ~ObstacleAvoidanceFitnessPlayerStage();

	/**
	 * Calculates fitness for e-puck robot in player/stage.
	 *
	 * @param wm The worldmodel to calculate fitness from.
	 * @return Fitness increment for the current time-step (think 'reward' in reinforcement learning)
	 */
	LIOReal updateFitness(worldModel_t& wm);
	LIOReal finaliseFitness(LIOReal fitness) { return fitness; }
        
        LIOReal operator()(worldModel_t& wm, LIOReal currentFitness);
};
/// @}

}

#endif /* OBSTACLEAVOIDANCEFITNESSPLAYERSTAGE_H_ */
