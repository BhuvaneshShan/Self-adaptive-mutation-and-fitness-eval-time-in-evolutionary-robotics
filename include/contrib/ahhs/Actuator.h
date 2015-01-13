/** ActuatorClass.h
 * Definitions of the Actuator.
 *
 *  @date: 23.07.2009
 *  @author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
*/

#ifndef ACTUATOR_H_
#define ACTUATOR_H_

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>

using namespace std;

class Actuator 
{
  private:
    double actuator_value_;
    int compartment_id_;
    double max_value_;
    double min_value_;
    int virtual_id_;
    int sim_id_;
 
  public:
    Actuator();
    Actuator(Actuator *src);
    virtual ~Actuator();
    
    double getActuatorValue();
    int getCompartmentID();
    double getMinValue();
    double getMaxValue();
    int getVirtualID();
    int getSimID();
    
    void setActuatorValue(double value);
    void setCompartmentID(int id);
    void setMinValue(double value);
    void setMaxValue(double value);
    void setVirtualID(int id);
    void setSimID(int id);

    void readFromFile(std::string line);
    void logging(std::ofstream &File);
    bool checkBounds();
    void setValueBackToBounds();
};

#endif /* AHHSACTUATOR_H_ */
