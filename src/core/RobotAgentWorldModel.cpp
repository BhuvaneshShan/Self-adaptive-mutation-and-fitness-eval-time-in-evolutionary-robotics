/*
 *  RobotAgentWorldModel.cpp
 *  roborobo-online
 *
 *  Created by Nicolas on 20/03/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "WorldModels/RobotAgentWorldModel.h"

#include "World/World.h"

RobotAgentWorldModel::RobotAgentWorldModel()
{
	_sensors = NULL;
    _ditchSensor = NULL;
    _energySensor = NULL;
	_robotLED_status = false; // default: LED is off.
	_robotLED_green = _robotLED_blue = 0; // default value (grey)
	_robotLED_red = 255;
     _agentAbsoluteOrientation = 0;
     _agentAbsoluteLinearSpeed  = 0;
     _actualRotationalVelocity = 0.0;
     _actualTranslationalValue = 0.0;
    
    _energyLevel = 100;
    _deltaEnergy = 0;
	_energyGained = 0;
    
    joinedOrganism = false;
    
    _fitnessValue=0;
}

RobotAgentWorldModel::~RobotAgentWorldModel()
{
	if ( _sensors != NULL )
		delete[] _sensors;
    if ( _ditchSensor != NULL )
		delete[] _ditchSensor;
    if ( _energySensor != NULL )
		delete[] _energySensor;
}

double RobotAgentWorldModel::getSensorDistanceValue( int i )
{
	return _sensors[i][5];
}

double RobotAgentWorldModel::getSensorTypeValue( int i )
{
	return _sensors[i][6];
}

double RobotAgentWorldModel::getSensorMaximumDistanceValue( int i )
{
	return gSensorRange; // BEWARE: sensor[i][3] is distance btw the end point of the sensor and the **center of the agent**
}

int RobotAgentWorldModel::getFloorSensor(){
    return _floorSensor;
}

double RobotAgentWorldModel::getDitchDistanceValue( int i )
{
	return _ditchSensor[i];
}

double RobotAgentWorldModel::getEnergyDistanceValue( int i)
{
    return _energySensor[i];
}

void RobotAgentWorldModel::setRobotNeighborhoodCounter( int __value )
{
	_robotNeighborhoodCounter = __value;
}

void RobotAgentWorldModel::incRobotNeighborhoodCounter( ) // increment
{
	_robotNeighborhoodCounter++;
}

int RobotAgentWorldModel::getRobotNeighborhoodCounter()
{
	return _robotNeighborhoodCounter;
}

double RobotAgentWorldModel::getXReal()
{
	return _xReal;
}

double RobotAgentWorldModel::getYReal()
{
	return _yReal;
}

bool RobotAgentWorldModel::getRobotLED_status()
{
	return _robotLED_status;
}

void RobotAgentWorldModel::setRobotLED_status(bool __status)
{
	_robotLED_status = __status;
}

int RobotAgentWorldModel::getRobotLED_redValue()
{
	return _robotLED_red;
}

int RobotAgentWorldModel::getRobotLED_greenValue()
{
	return _robotLED_green;
}

int RobotAgentWorldModel::getRobotLED_blueValue()
{
	return _robotLED_blue;
}

void RobotAgentWorldModel::setRobotLED_colorValues(int __r, int __g, int __b)
{
	_robotLED_red = __r;
	_robotLED_green = __g;
	_robotLED_blue = __b;
}

void RobotAgentWorldModel::setDeltaEnergy(double __deltaEnergy) {
	this->_deltaEnergy = __deltaEnergy;
}

double RobotAgentWorldModel::getDeltaEnergy() const {
	return _deltaEnergy;
}

void RobotAgentWorldModel::setEnergyLevel(double __energyLevel) {
	this->_energyLevel = __energyLevel;
}

double RobotAgentWorldModel::getEnergyLevel() const {
	return _energyLevel;
}

void RobotAgentWorldModel::setEnergyGained(double __energygained) {
	this->_energyGained = __energygained;
}

double RobotAgentWorldModel::getEnergyGained() const {
	return this->_energyGained;
}
