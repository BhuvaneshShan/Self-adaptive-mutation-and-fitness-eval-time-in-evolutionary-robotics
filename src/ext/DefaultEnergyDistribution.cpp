#include "World/DefaultEnergyDistribution.h"

#include "RoboroboMain/roborobo.h"

DefaultEnergyDistribution::DefaultEnergyDistribution() {
}

DefaultEnergyDistribution::~DefaultEnergyDistribution() {
}

Point2d DefaultEnergyDistribution::draw() {
    double x = (rand() % (gAreaWidth-20)) + 10;
    double y = (rand() % (gAreaHeight-20)) + 10;

    return Point2d(x,y);
}
