/**
 * ActuatorClass.cpp
 *
*  Created on: 23.07.2009
 * Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 */
#include "ahhs/Actuator.h"

Actuator::Actuator()
{
  actuator_value_ = 0.0;
  compartment_id_ = -1;
  min_value_ = -1.0;
  max_value_ = 1.0;
  virtual_id_ = 0;
  sim_id_ = 0;
}

Actuator::Actuator(Actuator *src)
{
  actuator_value_ = src->actuator_value_;
  compartment_id_ = src->compartment_id_;
  min_value_ = src->min_value_;
  max_value_ = src->max_value_;
  virtual_id_ = src->virtual_id_;
  sim_id_ = src->virtual_id_;
}

Actuator::~Actuator()
{
  //cout << "Actuator::~Actuator(). dead." << endl;
}

double Actuator::getActuatorValue()
{
  return actuator_value_;
}

int Actuator::getCompartmentID()
{
  return compartment_id_;
}

double Actuator::getMinValue()
{
  return min_value_;
}

double Actuator::getMaxValue()
{
  return max_value_;
}	

int Actuator::getVirtualID()
{
  return virtual_id_;
}
 
int Actuator::getSimID()
{
  return sim_id_;
}

void Actuator::setActuatorValue(double value)
{
  //  if ( (value >= min_value_) && (value <= max_value_) )
  //{
    actuator_value_ = value;
    //}
    /*
else if (value < min_value_)
  {
    actuator_value_ = min_value_;
  }
  else if (value > max_value_)
  {
    actuator_value_ = max_value_;
  }
    */
}

void Actuator::setCompartmentID(int id)
{
  compartment_id_ = id;
}

void Actuator::setMinValue(double value)
{
  min_value_ = value;
}

void Actuator::setMaxValue(double value)
{
  if (value < 0.0) {
    cout << "Error: to small max value" << endl;
    exit(1);
  }
  max_value_ = value;
}

void Actuator::setVirtualID(int id)
{
  virtual_id_ = id;
}
void Actuator::setSimID(int id)
{
  sim_id_ = id;
}

bool Actuator::checkBounds()
{
  return ((actuator_value_ < min_value_) || (actuator_value_ > max_value_));
}

void Actuator::setValueBackToBounds()
{
  if (actuator_value_ < min_value_) {
    //cout << "Actuator::setValueBackToBounds(). Val: " << actuator_value_ << " Min: " << min_value_ << endl;
    actuator_value_ = min_value_;
  }
  else if (actuator_value_ > max_value_) {
    //cout << "Actuator::setValueBackToBounds(). Val: " << actuator_value_ << " Max: " << max_value_ << endl;
    actuator_value_ = max_value_;
  }
}

void Actuator::logging(std::ofstream &File) 
{
  File << compartment_id_ << "; ";
  File << min_value_ << "; ";
  File << max_value_ << "; ";
  File << virtual_id_ << "; ";
  File << sim_id_;
  File << std::endl;
}

void Actuator::readFromFile(std::string line) 
{
  std::string item;
  std::istringstream linestream(line);

  double double_value = 0.0; 
  int int_value = 0;
  
  int fieldcounter = 0;
  while (getline (linestream, item, ';'))
  {
    if ((fieldcounter == 1) || (fieldcounter == 2)) {
      double_value = atoi(item.c_str());
    }
    else {
      int_value = atoi(item.c_str());
    }
    switch (fieldcounter) 
    {
    case 0:
      compartment_id_ = int_value;
      break;
    case 1:
      min_value_ = double_value;
      break;
    case 2:
      max_value_ = double_value;
      break;
    case 3:
      virtual_id_ = int_value;
      break;
    case 4:
      sim_id_ = int_value;
      break;
    default:
      std::cout << "Actuator::ReadFromFile: Too many fields in sensor definition file. Please check. Aborting.\n";
      exit(1);
      break;
    }
    //std::cout << "field_counter: " << fieldcounter << ": " << int_value << std::endl;
    fieldcounter++;
  }
  if ((fieldcounter) != 5) 
  {
    std::cout << "Actuator::ReadFromFile: Not enough fields in actuator definition file. Please check. Aborting." << fieldcounter << std::endl;
    exit(1);
  }
  actuator_value_ = 0.0;
}



