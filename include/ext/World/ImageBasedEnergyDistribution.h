/*
 *  ImageBasedEnergyDistribution.h
 *  roborobo
 *
 *  Created by Cristian Dinu on 07/12/12.
 *
 *  This is an energy distribution whose probability map is read from a PNG
 *  file (blacker areas => higher probability, with white meaning zero)
 */


#ifndef IMAGEBASEDENERGYDISTRIBUTION_H_
#define IMAGEBASEDENERGYDISTRIBUTION_H_

#include <string>
#include <stdint.h>
#include "EnergyDistribution.h"
#include "Utils/Distribution.h"

class ImageBasedEnergyDistribution: public EnergyDistribution {
public:
    ImageBasedEnergyDistribution(std::string filename);
    virtual ~ImageBasedEnergyDistribution();

    virtual void load();

    virtual Point2d draw();
protected:
    std::string imageFilename;
    Distribution<Point2d> distrib;
};

#endif /* IMAGEBASEDENERGYDISTRIBUTION_H_ */
