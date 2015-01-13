#include <stdexcept>

#include "World/EnergyDistribution.h"

EnergyDistribution::EnergyDistribution() {
}

EnergyDistribution::~EnergyDistribution() {
}

Point2d EnergyDistribution::draw() {
    throw std::runtime_error("Attempted to use draw() method in abstract class EnergyDistribution");
}
