/**
 * SensorClass.cpp
 *
*  Created on: 23.07.2009
 * Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 */
#include "ahhs/Sensor.h"

Sensor::Sensor() 
{
  sensor_value_ = 0.0;
  compartment_id_ = -1;
  min_value_ = 0.0;
  max_value_ = 1.0;
  virtual_id_ = 0;
  sim_id_ = 0;
  position_angle_ = 0.0;
}

Sensor::Sensor(Sensor *src) 
{
  sensor_value_ = 0.0;
  compartment_id_ = src->compartment_id_;
  min_value_ = src->min_value_;
  max_value_ = src->max_value_;
  virtual_id_ = src->virtual_id_;
  sim_id_ = src->sim_id_;
  position_angle_ = src->position_angle_;
}

Sensor::~Sensor() 
{
  //cout << "Sensor::~Sensor(). dead" << endl; 
}

double Sensor::getSensorValue()
{
  return sensor_value_;
}

int Sensor::getCompartmentID()
{
  return compartment_id_;
}

double Sensor::getMinValue()
{
  return min_value_;
}

double Sensor::getMaxValue()
{
  return max_value_;
}	

int Sensor::getVirtualID()
{
  return virtual_id_;
}
 
int Sensor::getSimID()
{
  return sim_id_;
}

double Sensor::getPositionAngle()
{
  return position_angle_;
}


void Sensor::setSensorValue(double value)
{
  if ( (value >= min_value_) && (value <= max_value_) )
  {
    sensor_value_ = value;
  }
  else if (value < min_value_)
  {
    sensor_value_ = min_value_;
  }
  else if (value > max_value_)
  {
    sensor_value_ = max_value_;
  }
}

void Sensor::setCompartmentID(int id)
{
  compartment_id_ = id;
}

void Sensor::setMinValue(double value)
{
  min_value_ = value;
}

void Sensor::setMaxValue(double value)
{
  max_value_ = value;
}

void Sensor::setVirtualID(int id)
{
  virtual_id_ = id;
}
void Sensor::setSimID(int id)
{
  sim_id_ = id;
}

void Sensor::setPositionAngle(double value)
{
  position_angle_= value;
}

void Sensor::logging(std::ofstream &File) 
{
  File << compartment_id_ << "; ";
  File << min_value_ << "; ";
  File << max_value_ << "; ";
  File << virtual_id_ << "; ";
  File << sim_id_ << "; ";
  File << position_angle_;
  File << std::endl;
}

void Sensor::readFromFile(std::string line) {

	std::string item;
	std::istringstream linestream(line);

	double double_value = 0.0; 
	int int_value = 0;
	  
	int fieldcounter = 0;
	while (getline (linestream, item, ';')) {

	  if ((fieldcounter == 1) || (fieldcounter == 2) || (fieldcounter == 5))
	  {
	    double_value = atof(item.c_str());
	  }
	  else
	  {
	    int_value = atoi(item.c_str());
	  }
	  switch (fieldcounter) {
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
			case 5:
			        position_angle_ = double_value;
				break;
		        default:
				std::cout << "Sensor::ReadFromFile: Too many fields in sensor definition file. Please check. Aborting.\n";
				exit(1);
				break;
		}
	  //std::cout << "field_counter: " << fieldcounter << ": " << double_value << std::endl;
	fieldcounter++;
	}
	if ((fieldcounter) != 6) {
		std::cout << "Sensor::ReadFromFile: Not enough fields in sensor definition file. Please check. Aborting." << fieldcounter << std::endl;
		exit(1);
	}
	sensor_value_ = 0.0;
}

