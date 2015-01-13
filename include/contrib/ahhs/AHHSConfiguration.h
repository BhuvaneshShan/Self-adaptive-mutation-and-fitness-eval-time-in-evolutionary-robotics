/*
 * AHHSConfiguration.h
 *
 *  Created on: 23.07.2009
 *      Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 *
 * @todo member variable agent_radius is hardcoded, but should come from the parameter file, because in main this value is used. If these two value do not correspond, wrong behavior is shown by the robot.
 */

#ifndef AHHSCONFIGURATION_H_
#define AHHSCONFIGURATION_H_

#include <iostream>
#include <vector>

#include "Sensor.h"
#include "Actuator.h"

using namespace std;

#define PI 3.14159265

class AHHSConfiguration 
{
  public:
    AHHSConfiguration();
    AHHSConfiguration(AHHSConfiguration *src);
    virtual ~AHHSConfiguration();
    
    void addSensor(Sensor *new_sensor);
    void addActuator(Actuator *new_actuator);
    
    int getNumberOfSensors();
    int getNumberOfActuators();
    int getNumberOfCompartments();
    Actuator* getActuator(int pos);
    Sensor* getSensor(int pos);
    vector<Actuator*> getActuators();
    vector<Sensor*> getSensors();
    vector<int> getNeighborCompIDs(int index);

    void setNrOfCompartments(int nr_of_compartments);
    void setConfiguration(int nr_of_actuators, int nr_of_sensors, int nr_of_compartments);
    void setCustomConfiguration(int nr_of_actuators, int nr_of_sensors, int nr_of_compartments, double sensor_arr[]);

  private:
    int number_of_compartments_;
    std::vector<Sensor*> sensors_;
    std::vector<Actuator*> actuators_;
};

#endif /* AHHSCONFIGURATION_H_ */
