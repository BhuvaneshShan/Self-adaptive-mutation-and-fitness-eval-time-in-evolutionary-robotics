/*
 * $Id $
 *
 *  Created on: 30 janv. 2009
 *      Author: nicolas.bredeche(at)lri.fr
 */

// A simple braitenberg vehicle for wall avoidance implemented with a perceptron

#include "lioutils/lio_global.h"

#include "behaviors/PerceptronBehavior.h"


PerceptronBehavior::PerceptronBehavior()
{
	_perceptron = new SimplePerceptron(8 + 1, 2);
	_perceptron->setActivationFunction(&ActivationFunctionTanh::apply);
}


PerceptronBehavior::~PerceptronBehavior(void)
{
	delete _perceptron;
}


void PerceptronBehavior::init( LIOReal *__parameters, int __size)
{
	_perceptron->loadParameters(__parameters,18); // load weights.
}


void PerceptronBehavior::performStep( LIOReal *__sensorValues ) // btw -1 and 1 (normalize beforehand)
{
	for ( int i = 0; i != 8; i++ )
		_sensorValues[i] = __sensorValues[i];
	_sensorValues[8] = 1; // add bias node value

	_perceptron->step(_sensorValues);
}


void PerceptronBehavior::getActuators( LIOReal *__actuatorValues ) // btw -1 and 1 (normalize afterwards)
{
	_perceptron->getOutputValues(_actuatorValues);	// read-out output values

	for ( int i = 0 ; i != GetArrayLength(_actuatorValues) ; i++ )
		__actuatorValues[i] = _actuatorValues[i];
}


//-n
