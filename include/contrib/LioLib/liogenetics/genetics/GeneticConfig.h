/*
 *  $Id: GeneticConfig.h 2734 2010-09-23 10:18:35Z evert $
 */
#ifndef GENETICCONFIG_H
#define GENETICCONFIG_H
/**
 *  \defgroup Genetics Genetics Framework
 *
 *  The Genetics Framework provides a flexible, extensible framework for
 *  on-board evolutionary computation.
 */
/**
 * \addtogroup Genetics
 * @{
 */
/**
 * \file GeneticConfig.h
 * Put all states and genes you need in these enumerations.
 * Do not use specific constant values, rather use the enumerated names
 * so your ids remain valid if order and position of the name changes.
 */

//#define UNITTEST
#include <lioutils/lio_global.h>
#include <lioutils/Fixed32.h>

typedef enum StateID
{
	STATEONE, // placeholder
	STATETWO, // placeholder

	STATECOUNT
} StateID;

typedef enum GeneID
{
	GENEONE, // placeholder
	GENETWO, // placeholder
	GENETHREE, // placeholder

	GENECOUNT =	10
} GeneID;


#ifndef UNITTEST
#ifdef EMBEDDED
#include <plugins/controller/MDL2e/core/globals.h>
#else

#ifdef WEBOTS

struct worldModel_t
{
	double irSensor[8];	// Distance sensor inputs
	double lightSensor[8];	// Light sensor inputs
	double phrmSensor[4]; 	// Pheromone peripheral sensor inputs
	double phrmFloor;	//Pheromone under robot sensor
	double foodSensor;	// Food sensor
	double previousLightSensorValue;
	double battery;
	double prevBattery;
	double signalStrength;
	double signalDirection;
	double velocity[2];	/// < Wheel velocities
	double pheromone_drop;
	const unsigned char *image;
	double leds;

	double xAngle;			/// < pitch from accelerometer
	double yAngle;			/// < roll from accelerometer
	double zAngle;			/// < yaw from accelerometer

	double xPos;			/// < GPS x coordinate
	double yPos;			/// < GPS y coordinate
	double zPos;			/// < GPS z coordinate

	double bearing;			/// < Compass bearing between [-pi, pi]

	worldModel_t(void)
	{

		previousLightSensorValue = -1.0;

	}

        // WK: colorVals; how much is there a certain color present in the image?
	double colorAct[4]; // < R B G Y
	// WK: shapeVals: how much does the largest shape resemble a certain shape?
	double shapeAct[12]; // < shapes 1 - 12
};

#else
struct worldModel_t {
	uint8_t irSensor[12];
	int8_t velocity[2];
	int8_t vHinge;
	int8_t connector;
	uint8_t can0;
	uint8_t myID;
} ;
#endif
#endif
#endif

namespace Genetics
{
/**
 * Defines the type LIOLib uses internally for probabilities of mutation, crossover, etc.
 */
typedef double ProbabilityType;

}
///@}
#endif // GENETICCONFIG_H
