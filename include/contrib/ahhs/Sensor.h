/** SensorClass.h
 * Definitions of the Sensor.
 *
 *  @date: 23.07.2009
 *  @author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
@todo: string member variable to name it as "sound", "distance", so it is easier to identify it 

*/

#ifndef SENSOR_H_
#define SENSOR_H_

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>

using namespace std;

class Sensor  
{
  private:
    double sensor_value_;
    int compartment_id_;
    double min_value_;
    double max_value_;
    int virtual_id_;
    int sim_id_;
    double position_angle_;

  public:
    Sensor();
    Sensor(Sensor *src);
    virtual ~Sensor();
    
    double getSensorValue();
    int getCompartmentID();
    double getMinValue();
    double getMaxValue();
    int getVirtualID();
    int getSimID();
    double getPositionAngle();

    void setSensorValue(double value);
    void setCompartmentID(int id);
    void setMinValue(double value);
    void setMaxValue(double value);
    void setVirtualID(int id);
    void setSimID(int id);
    void setPositionAngle(double value); 

    void readFromFile(std::string line);
    void logging(std::ofstream &File);
    
};

#endif /* SENSOR_H_ */
