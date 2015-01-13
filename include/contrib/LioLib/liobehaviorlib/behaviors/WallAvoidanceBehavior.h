/*
 * $Id $
 *
 *  Created on: 30 janv. 2009
 *      Author: nicolas.bredeche(at)lri.fr
 */

#ifndef WALL_AVOIDANCE_BEHAVIOR_H
#define WALL_AVOIDANCE_BEHAVIOR_H

#include <lioutils/lio_global.h>

#include <lioutils/Fixed32.h>

//#include "behaviors/Behavior.h"

class WallAvoidanceBehavior //: public Behavior
{
	private:
		LIOReal _actuatorValues[2];

	protected:

	public:
		WallAvoidanceBehavior();
		~WallAvoidanceBehavior(void);

		void init( LIOReal *__parameters = 0, int __size = 0);

		void performStep( LIOReal *__sensorValues ); // btw -1 and 1

		void getActuators( LIOReal *__actuatorValues ); // btw -1 and 1

		//size_t serialize(void* buffer) const;
		//void deserialize(void* buffer);

};


#endif // WALL_AVOIDANCE_BEHAVIOR_H

//-n
