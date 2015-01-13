/*
 * $Id $
 *
 *  Created on: 30 janv. 2009
 *      Author: nicolas.bredeche(at)lri.fr
 *
 *
 * See below for a tutorial as to how to use this class.
 *
 */

#ifndef NEURAL_NET_WALL_AVOIDANCE_BEHAVIOR_H
#define NEURAL_NET_WALL_AVOIDANCE_BEHAVIOR_H


#include "lioutils/lio_global.h"

#include "lioutils/Fixed32.h"

#include "neuralnets/SimplePerceptron.h"
#include "activationfunctions/ActivationFunctionTanh.h"

//#include "behaviors/Behavior.h"

class PerceptronBehavior //: public Behavior
{
	private:
		LIOReal _sensorValues[8+1]; // 8 IR inputs + 1 bias
		LIOReal _actuatorValues[2]; // 2 motor outputs

		SimplePerceptron *_perceptron;

	protected:

	public:
		PerceptronBehavior();
		~PerceptronBehavior(void);

		void init( LIOReal *__parameters = 0, int __size = 0);

		void performStep( LIOReal *__sensorValues ); // btw -1 and 1

		void getActuators( LIOReal *__actuatorValues ); // btw -1 and 1

		//size_t serialize(void* buffer) const;
		//void deserialize(void* buffer);

};


#endif // NEURAL_NET_WALL_AVOIDANCE_BEHAVIOR_H


/**

//
// Tutorial: Testing Behavior with encapsulated Neural Network (obstacle avoidance demo)
// This will show you how to simply implement this neural network behavior-based controller.
// Let's say we start from a basic Task controller example as provided in the repository.
//

(...)

NeuralNetWallAvoidanceBehavior *behavior; // you may want to define in the header.

ExampleControllerTask::ExampleControllerTask() : Task("ExampleController", 300, 6, true, true)
{
	behavior = new NeuralNetWallAvoidanceBehavior(); // testing beh+nn

	// set hand written controller for obstacle avoidance
	LIOReal weights[18];

	// to motor no.0
	weights[0] = (LIOReal)-0.5;
	weights[1] = (LIOReal)-1;
	weights[2] = (LIOReal)-1;
	weights[3] = (LIOReal)0;
	weights[4] = (LIOReal)0;
	weights[5] = (LIOReal)0; // ir5->out0
	weights[6] = (LIOReal)2;
	weights[7] = (LIOReal)2;
	weights[8] = (LIOReal)0; // bias
	// to motor no.1
	weights[9]  = (LIOReal)0; // ir0->out1
	weights[10] = (LIOReal)2;
	weights[11] = (LIOReal)2;
	weights[12] = (LIOReal)0;
	weights[13] = (LIOReal)0;
	weights[14] = (LIOReal)-0.5; // slightly asymetric behavior
	weights[15] = (LIOReal)-1;
	weights[16] = (LIOReal)-1;
	weights[17] = (LIOReal)0; // bias

	behavior->init(weights,18); // load parameters.
 }

(...)

void ExampleControllerTask_neuralnet::task()
{

	(...)

	 LIOReal sensorValues[8];
	 LIOReal actuatorValues[2];

	 // normalize sensor values
	 for ( int i = 0 ; i != 8 ; i++ )
	 {
		 double d = (double)wm.irSensor[i]  / (double)70; // normalize btw 0.0 and 1.0
		 sensorValues[i] = (LIOReal)d;
	 }

	 // call behavior
	 behavior->performStep(sensorValues);
	 behavior->getActuators(actuatorValues);

	 // normalize and register actuator values
	 double v1 = (double)(actuatorValues[0]) * 100.0;
	 double v2 = (double)(actuatorValues[1]) * 100.0;
	 wm.velocity[0] = (int8_t)v1;
	 wm.velocity[1] = (int8_t)v2;

	 // display log.
	 (*SysLog::instance()) << "sensors: ";
	 for ( int i = 0 ; i != 8 ; i++ )
	 (*SysLog::instance()) << wm.irSensor[i] << " , ";
	 (*SysLog::instance()) << " --> ";
	 (*SysLog::instance()) << "velocity: " << wm.velocity[0] << " , " << wm.velocity[1] << "\n";

	(...)

}

(...)

 */

//-n

