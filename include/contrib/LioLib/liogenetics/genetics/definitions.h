/*
 * $Id: definitions.h$
 *
 *  Created on: Jun 23, 2010
 *      Author: ehaasdi
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <genetics/NeuralNetGenome.h>
#include <neuralnets/SimplePerceptron.h>
#include <neuralnets/MultiLayeredPerceptron.h>
//#include <neuralnets/DTRNN.h>

/*-----------------------------------------------------------------------*/
//Settings for the tasks. To activate a task define the appropriate
//name in the project's build settings' preprocessor section. For
//each task the sensors/actuators used are defines. In some cases
//some of these can be deactivated.
// CAUTION: After setting the task here, the number of genes must be set
// in GeneticConfig.h
/*-----------------------------------------------------------------------*/

#ifdef OBSTACLE_AVOIDANCE

	//Description
	#define TASK_DESCRIPTION "Fast Forward"

	//Sensory input (possibly some can be turned off)
	#define USE_DIST_SENSORS 1 //Distance sensors MUST be used!!

	//Output actuators (possibly some can be turned off)
	#define EFFECTUATE_WHEELS 1 //Wheels MUST be used!!

	//Number of inputs and outputs depending on senors and actuators. Do NOT edit!!!
	#define INPUTS 8
	#define OUTPUTS 2

	//NN type (can be 'SimplePerceptron', 'MultiLayeredPerceptron' or 'DTRNN')
	typedef SimplePerceptron task_NN;
	#define HIDDEN 6	//Here set the number of hidden nodes (it will have no effect if a
						//SimplePerceptron is used)

	//The genome and fitness
	#include <genetics/ObstacleAvoidanceFitnessWebots.h>
	typedef Genetics::ObstacleAvoidanceFitnessWebots task_fitness;
	typedef Genetics::ESGenome task_genome;
	typedef std::greater<LIOReal> task_greater;

#endif

#ifdef PHOTOTAXIS

	//Descrption
	#define TASK_DESCRIPTION "Phototaxis"

	//Sensory input (possibly some can be turned off)
	#define USE_LIGHT_SENSORS 1 //Distance sensors MUST be used!!

	//Output actuators (possibly some can be turned off)
	#define EFFECTUATE_WHEELS 1 //Wheels MUST be used!!

	//Number of inputs and outputs depending on senors and actuators. Do NOT edit!!!
	#define INPUTS 8
	#define OUTPUTS 2

	//NN type (can be 'SimplePerceptron', 'MultiLayeredPerceptron' or 'DTRNN')
	typedef SimplePerceptron task_NN;
	#define HIDDEN 0	//Here set the number of hidden nodes (it will have no effect if a
							//SimplePerceptron is used)

	//The genome and fitness
	#include <genetics/Phototaxis.h>
	typedef Genetics::Phototaxis task_fitness;
	typedef Genetics::ESGenome task_genome;
	typedef std::greater<LIOReal> task_greater;

#endif

#ifdef COLLECTIVE_PATROLLING

	//Description
	#define TASK_DESCRIPTION "Collective Patrolling"

	//Sensory input (possibly some can be turned off)
	#define USE_DIST_SENSORS 1 //Distance sensors MUST be used!!!
	#define USE_PHEROMONES 1 //Pheromones can be turned off and only fast forward be used.
								//To turn off just comment this define.

	//Output actuators (possibly some can be turned off)
	#define EFFECTUATE_WHEELS 1 //Wheels MUST be used!!
	//#define EFFECTUATE_PHEROMONE_DROP 1 //Pheromone drop can be turned off and robots will constantly drop
										//at a fixed rate. To turn off just comment this define.

	//Number of inputs and outputs depending on senors and actuators. Do NOT edit!!!
	#ifdef USE_PHEROMONES
		#define INPUTS 12
	#else
		#define INPUTS 8
	#endif
	#ifdef EFFECTUATE_PHEROMONE_DROP
		#define OUTPUTS 3
	#else
		#define OUTPUTS 2
	#endif

	//NN type (can be 'SimplePerceptron', 'MultiLayeredPerceptron' or 'DTRNN')
	typedef SimplePerceptron task_NN;
	#define HIDDEN 6	//Here set the number of hidden nodes. Must be greater than 0 (it will have no
						//effect if a SimplePerceptron is used)

	//The genome and fitness
	#include <genetics/CollectivePatrolling.h>
	typedef Genetics::CollectivePatrolling task_fitness;
	typedef Genetics::ESGenome task_genome;
	typedef std::greater<LIOReal> task_greater;

#endif

#ifdef COLLECTIVE_FORAGING

	//Description
	#define TASK_DESCRIPTION "Collective Foraging"

	//Sensory input (possibly some can be turned off)
	#define USE_DIST_SENSORS 1 //Distance sensors MUST be used!!!
	#define USE_CAMERA 1 		//Camera MUST be used!!!
	#define CAMERA_WIDTH 5
	#define CAMERA_ANGLE 3.14
	#define USE_FOOD_SENSOR 1	//Food sensor MUST be used!!!

	//Output actuators (possibly some can be turned off)
	#define EFFECTUATE_WHEELS 1	//Wheels MUST be used!!
	#define EFFECTUATE_LEDS 1	//LEDS can be turned off disabling the possibility for
									//robots to signal each other. To turn off just comment this define.

	//Number of inputs and outputs depending on senors and actuators. Do NOT edit!!!
	#define INPUTS 14
	#ifdef EFFECTUATE_LEDS
		#define OUTPUTS 3
	#else
		#define OUTPUTS 2
	#endif

	//NN type (can be 'SimplePerceptron', 'MultiLayeredPerceptron' or 'DTRNN')
	typedef SimplePerceptron task_NN;
	#define HIDDEN 10	//Here set the number of hidden nodes. Must be greater than 0 (it will have no
							//effect if a SimplePerceptron is used)

	//The genome and fitness
	#include <genetics/CollectiveForaging.h>
	typedef Genetics::CollectiveForaging task_fitness;
	typedef Genetics::ESGenome task_genome;
	typedef std::greater<LIOReal> task_greater;
#endif

#ifdef BALANCING

	//Description
	#define TASK_DESCRIPTION "Balancing"

	//Sensory input (possibly some can be turned off)
	#undef USE_DIST_SENSORS  //Distance sensors off

	//Output actuators (possibly some can be turned off)
	#define EFFECTUATE_WHEELS 1 //Wheels MUST be used!!

	//Number of inputs and outputs depending on senors and actuators. Do NOT edit!!!
	#define INPUTS 4
	#define OUTPUTS 2

	//NN type (can be 'SimplePerceptron', 'MultiLayeredPerceptron' or 'DTRNN')
	typedef SimplePerceptron task_NN;
	#define HIDDEN 6	//Here set the number of hidden nodes (it will have no effect if a
						//SimplePerceptron is used)

	//The genome and fitness
	#include <genetics/BalanceFitnessWebots.h>
	typedef Genetics::BalanceFitnessWebots task_fitness;
	typedef Genetics::ESGenome task_genome;
	typedef std::greater<LIOReal> task_greater;
#endif

#ifdef CURIOSITY
	#include <genetics/EMeans.h>
	#define DIMENSIONS 25					// TODO: appropriate number of inputs into NN here
	#define TASK_NAME "Rat's Life w. QI"
	typedef VU::MuPlusOne<Genetics::ESGenome, MU, Genetics::EMeans, std::greater<LIOReal> > MuPlusOne;
	typedef VU::NNWithEAControl<MU, Genetics::EMeans, std::greater<LIOReal>, DIMENSIONS, 2 > NeuralNet; //TODO: Appropriately complex NN
#endif

#endif /* DEFINITIONS_H_ */
