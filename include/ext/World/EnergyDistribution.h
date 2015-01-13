/*
 *  EnergyDistribution.h
 *  roborobo
 *
 *  Created by Cristian Dinu on 07/12/12.
 *
 *  This is a class for modeling the distribution of energy points in the arena.
 *  The main method is 'draw()', which draws a random position from the
 *  set of all possible points, according to their relative probability as
 *  defined by the distribution.
 */

#ifndef ENERGYDISTRIBUTION_H_
#define ENERGYDISTRIBUTION_H_

#include "Utilities/Misc.h"

// This class is abstract
class EnergyDistribution {
public:
    EnergyDistribution();
    virtual ~EnergyDistribution();

    virtual Point2d draw();
};

#endif /* ENERGYDISTRIBUTION_H_ */
