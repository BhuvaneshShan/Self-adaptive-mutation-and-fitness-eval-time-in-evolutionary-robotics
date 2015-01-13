/*
 * $Id $
 *
 *  Created on: 30 janv. 2009
 *      Author: nicolas.bredeche(at)lri.fr
 */

// A simple braitenberg vehicle for wall avoidance.

#include "lioutils/lio_global.h"

#include "behaviors/WallAvoidanceBehavior.h"

WallAvoidanceBehavior::WallAvoidanceBehavior()
{
	// nothing to do.
}

WallAvoidanceBehavior::~WallAvoidanceBehavior(void)
{
	// nothing to do.
}

void WallAvoidanceBehavior::init( LIOReal *__parameters, int __size)
{
	// nothing to do.
}

void WallAvoidanceBehavior::performStep( LIOReal *__sensorValues ) // btw -1 and 1
{
	//_actuatorValues[0] = ( __sensorValues[7] + __sensorValues[6] + __sensorValues[5] ) / (LIOReal)3;
	//_actuatorValues[1] = ( __sensorValues[0] + __sensorValues[1] + __sensorValues[2] ) / (LIOReal)3;
	_actuatorValues[0] = ( __sensorValues[7] + __sensorValues[7] + __sensorValues[6] + __sensorValues[5] ) / 4 ;
	_actuatorValues[1] = ( __sensorValues[0] + __sensorValues[0] + __sensorValues[1] + __sensorValues[2] ) / 4;
}

void WallAvoidanceBehavior::getActuators( LIOReal *__actuatorValues ) // btw -1 and 1
{
//	double e_puck_matrix[8][2] =
//{ {150, -35}, {100, -15}, {80, -10}, {-10, -10},
//{-10, -10}, {-10, 80}, {-30, 100}, {-20, 150} }
	for ( int i = 0 ; i != GetArrayLength(_actuatorValues) ; i++ )
	{
		__actuatorValues[i] = _actuatorValues[i];
	}
}


//-n
