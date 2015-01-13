/*
 *  DefaultEnergyDistribution.h
 *  roborobo
 *
 *  Created by Cristian Dinu on 07/12/12.
 *
 *  This is the default energy point distribution: a uniform distribution that
 *  extends all over the arena except for a 10-pixel border.
 */

#ifndef DEFAULTENERGYDISTRIBUTION_H_
#define DEFAULTENERGYDISTRIBUTION_H_

#include "EnergyDistribution.h"

class DefaultEnergyDistribution: public EnergyDistribution {
public:
    DefaultEnergyDistribution();
    virtual ~DefaultEnergyDistribution();

    virtual Point2d draw();
};

#endif /* DEFAULTENERGYDISTRIBUTION_H_ */
