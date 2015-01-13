/*
 *  $Id: Phototaxis.cpp $
 *
 *  Created on: Mar 25, 2010
 *      Author: ehaasdi
 *
 * Copyright (c) 2010, VU University Amsterdam
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

#ifdef PHOTOTAXIS

#include <genetics/Phototaxis.h>

#include <algorithm>
#include <cmath>

//#define DEBUG_FITNESS

namespace Genetics
{

Phototaxis::Phototaxis()
{
	// nix
}

Phototaxis::~Phototaxis()
{
	// nix
}

LIOReal Phototaxis::operator()(worldModel_t& wm)
{
#ifdef DEBUG_FITNESS
	std::cout << "previousLightSensorValue: " << wm.previousLightSensorValue << ' ';
#endif
	//
	// retrieve brightest sensor value
	//
	double maxSensorValue (*std::max_element(wm.lightSensor, wm.lightSensor + 8));
	
	LIOReal delta(wm.previousLightSensorValue < 0.0 ? 0.0 : maxSensorValue - wm.previousLightSensorValue);
	wm.previousLightSensorValue = maxSensorValue;
	
#ifdef DEBUG_FITNESS
	std::cout << "SensorValue: " << maxSensorValue << ", delta: " << delta << std::endl;
#endif
	//return delta;
	return maxSensorValue;
}

}
#endif
