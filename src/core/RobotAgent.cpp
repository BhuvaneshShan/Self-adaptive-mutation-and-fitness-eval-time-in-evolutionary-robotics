/*
 *  RobotAgent.cpp
 *  roborobo
 *
 *  Created by Nicolas on 17/12/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <cmath>
#include <cstring>
#include <vector>
#include <utility>
#include <boost/lexical_cast.hpp>

#include "Config/GlobalConfigurationLoader.h"

#include "Agents/RobotAgent.h"
#include "WorldModels/EvolvingRobotAgentWorldModel.h"
#include "BehaviorControlArchitectures/EnergyPerceptronControlArchitecture.h"
#include "SDL_collide.h"
#ifdef MACOSX
#include "SDL_ttf/SDL_ttf.h"
#else
#include "SDL/SDL_ttf.h"
#endif

using namespace std;

RobotAgent::RobotAgent(World *__world) {
	_wm = gConfigurationLoader->make_RobotAgentWorldModel(); // TODO: externalize object referenced to create the new instance
	_registered = false;

	_wm->_world = __world;

	_wm->_agentId = gAgentCounter;
	gAgentCounter++;

	_organism = NULL;
	connected = new std::vector<RobotAgentPtr>();
	//Process agent specification (ie. IR/US/laser sensors)

	// create dynamic array
	// parse image and add (sensor.x/y, orientation) or (sensorStart.x/y, sensorEnd.x/y)

	_wm->_sensorCount = 0;


	//count sensors
	for (int x = 0; x != gAgentWidth; x++) // image is analysed a first time to count the number of sensors (faster than dynamically re-allocating array size for every new sensor)
		for (int y = 0; y != gAgentHeight; y++) {
			Uint32 pixel = getPixel32(gAgentSpecsImage, x, y);
			if (pixel != SDL_MapRGB(gAgentSpecsImage->format, 0xFF, 0xFF, 0xFF))
				_wm->_sensorCount++;
		}
	
	_wm->_sensors = new double[_wm->_sensorCount][7]; // see header for details.
	_wm->_ditchSensor = new double[_wm->_sensorCount];
    _wm->_energySensor = new double[_wm->_sensorCount];
	for (int i = 0; i < _wm->_sensorCount; i++) {
		_wm->_sensors[i][0] = -1;
		// 1 - 4 set below
		_wm->_sensors[i][5] = -1;
		_wm->_sensors[i][6] = -1;
		
		_wm->_ditchSensor[i] = -1;
        _wm->_energySensor[i] = -1;
	}
	//int sensorIt = 0;

	//register sensors
	for (int x = 0; x != gAgentWidth; x++)
		for (int y = 0; y != gAgentHeight; y++) {
			Uint32 pixel = getPixel32(gAgentSpecsImage, x, y);
			if (pixel != SDL_MapRGB(gAgentSpecsImage->format, 0xFF, 0xFF, 0xFF)) {
				// sensor found, register sensor.

				Uint8 r, g, b;
				SDL_GetRGB(pixel, gAgentSpecsImage->format, &r, &g, &b);

				if (_wm->_sensors[r][0] != -1) {
					std::cout << "[ERROR] robot sensor id already in use -- check agent specification image." << std::endl;
					exit(-1);
				}

				if (r >= _wm->_sensorCount) {
					std::cout << "[ERROR] robot sensor id is not permitted (must be defined btw 0 and " << (_wm->_sensorCount - 1) << ", got: " << r << ") -- check agent specification image." << std::endl;
					exit(-1);
				}


				_wm->_sensors[r][0] = r; // no. sensor

				// sensor origini point location wrt. agent center
				_wm->_sensors[r][1] = sqrt((x - gAgentWidth / 2) * (x - gAgentWidth / 2) + (y - gAgentHeight / 2) * (y - gAgentHeight / 2)); // length
				double angleCosinus = ((x - (gAgentWidth / 2)) / _wm->_sensors[r][1]);
				double angleSinus = ((y - (gAgentHeight / 2)) / _wm->_sensors[r][1]);
				if (angleSinus >= 0)
					_wm->_sensors[r][2] = acos(angleCosinus) + M_PI / 2; // angle (in radian)
				else
					_wm->_sensors[r][2] = -acos(angleCosinus) + M_PI / 2 + M_PI * 2; // angle (in radian)

				// sensor target point location wrt. agent center -- sensor target angle is (green+blue) component values
				double angle = g + b - 90; // note: '-90deg' is due to image definition convention (in image, 0° means front of agent, which is upward -- while 0° in simulation means facing right)
				double xTarget = (x - gAgentWidth / 2) + cos(angle * M_PI / 180) * gSensorRange;
				double yTarget = (y - gAgentHeight / 2) + sin(angle * M_PI / 180) * gSensorRange;
				_wm->_sensors[r][3] = sqrt(xTarget * xTarget + yTarget * yTarget); // length (**from agent center**)
				angleCosinus = xTarget / _wm->_sensors[r][3];
				angleSinus = yTarget / _wm->_sensors[r][3];
				if (angleSinus >= 0)
					_wm->_sensors[r][4] = acos(angleCosinus) + M_PI / 2; // angle (in radian) wrt. agent center
				else
					_wm->_sensors[r][4] = -acos(angleCosinus) + M_PI / 2 + M_PI * 2;
				r++;
			}
		}

	_agentObserver = gConfigurationLoader->make_AgentObserver(_wm); // TODO: externalize
	_behavior = gConfigurationLoader->make_BehaviorControlArchitecture(_wm); // TODO: externalize
         
        //bot_fitness=0; //our bot fitness
        
	reset();

}

RobotAgent::~RobotAgent() {
	delete _agentObserver;
	delete connected;
	delete _wm;
	delete _behavior;
}



void RobotAgent::reset() {
	//Initialize general
	_iterations = 0;
	_wm->_isAlive = true;


	/**/
	int x = 0, y = 0;
	std::string s = "";
	s += "agent[";
	std::stringstream out;
	out << _wm->_agentId;
	s += out.str();
	s += "].x";

	bool randomStart = false;
	if (gProperties.hasProperty(s))
		convertFromString<int>(x, gProperties.getProperty(s), std::dec);
	else {
		if (gVerbose) {
			std::cout << "[warning] Initial X coordinate for agent #" << _wm->_agentId << " not found. Random pick ([!] reset may alter environment, check with x-ray mode [!])." << std::endl;
		}
		randomStart = true;
		x = (int) gAgentWidth + (ranf() * (double) (gSpawnWidth - gAgentWidth));
	}

	s = "agent[";
	s += out.str();
	s += "].y";
	if (gProperties.hasProperty(s))
		convertFromString<int>(y, gProperties.getProperty(s), std::dec);
	else {
		if (gVerbose) {
			std::cout << "[warning] Initial Y coordinate for agent #" << _wm->_agentId << " not found. Random pick ([!] reset may alter environment, check with x-ray mode [!])." << std::endl;
		}
		randomStart = true;
		y = (int) gAgentHeight + (ranf() * (double) (gSpawnHeight - gAgentHeight));
	}

	setCoordReal(x, y);
	setCoord(x, y);
	
	while (randomStart && isCollision()) {
		//std::cout << "Collision detected on " << x << ", " << y << " rerolling" << std::endl;
		x = (int) 20 + (ranf() * (double) (gSpawnWidth));
		y = (int) 20 + (ranf() * (double) (gSpawnHeight));
		setCoordReal(x, y);
		setCoord(x, y);
	}

	// register agent so others don't start on top of it
	registerAgent();

	//_wm->_xReal = x;
	//_wm->_yReal = y;
	/**/

	//Initialize coordinate and displacement
	//_wm->_xReal=gAgentXStart;
	//_wm->_yReal=gAgentYStart;
	_xDelta = 0;
	_yDelta = 0;

	//Initialize internal variables
	_xDeltaReal = 0; //gAgentXStart;
	_yDeltaReal = 0; //gAgentYStart;


	s = "agent[";
	s += out.str();
	s += "].orientation";
	if (gProperties.hasProperty(s))
		convertFromString<double>(_wm->_agentAbsoluteOrientation, gProperties.getProperty(s), std::dec);
	else {
		if (gVerbose) {
			std::cout << "[warning] Initial orientation  for agent #" << _wm->_agentId << " not found. Random pick." << std::endl;
		}
		_wm->_agentAbsoluteOrientation = ranf() * 360. - 180.;
	}

	_wm->_agentAbsoluteLinearSpeed = 0;

	_wm->_desiredRotationalVelocity = 0;
	_wm->_desiredTranslationalValue = 0;

	_wm->_maxRotationalDeltaValue = gMaxRotationalSpeed; // ie. change will be instantenous
	_wm->_maxTranslationalDeltaValue = gMaxTranslationalDeltaValue; //gMaxTranslationalSpeed ; // idem.

	_wm->_actualTranslationalValue = 0;
	_wm->_actualRotationalVelocity = 0;

	for (int i = 0; i != _wm->_sensorCount; i++) // initialize sensor values to max range, no contact
	{
		_wm->_sensors[i][5] = gSensorRange; // range: max
		_wm->_sensors[i][6] = 0; // type:  none
	}
	_wm->_floorSensor = 0; // floor sensor value (taken from gZoneImage)

	// Initialize agent observer and Behavior Control Architecture

	_agentObserver->reset();
	_behavior->reset();

}

void RobotAgent::callObserver() {
	_agentObserver->step();
}

void RobotAgent::stepBehavior() {
	// update status
	_iterations++;

	if (_wm->_isAlive) // note: reviving a robot is performed in stepGenetics()
	{
		// compute commands
		_behavior->step();
	}

	displayInfo();
}

void RobotAgent::stepBehavior(Uint8* __keyboardStates) {
	_iterations++;

	// compute commands
	behaviorUpdate_remoteController(__keyboardStates);

	displayInfo();
}

void RobotAgent::displayInfo() {
	if (gVerbose && gDisplayMode <= 1)
		if (gInspectAgent && gAgentIndexFocus == _wm->_agentId) {
			std::cout << "Robot #" << _wm->_agentId << " : ( ";
			for (int i = 0; i != _wm->_sensorCount; i++) {
				std::cout << _wm->getSensorDistanceValue(i) << "/" << _wm->getSensorMaximumDistanceValue(i) << " ";
			}
			std::cout << " ) => ( " << _wm->_desiredTranslationalValue << " ; " << _wm->_desiredRotationalVelocity << " )." << std::endl;
		}
}

void RobotAgent::behaviorUpdate_remoteController(Uint8* __keyboardStates) // user controlled robot
{
	if (gAgentIndexFocus != _wm->_agentId)
		return;

	// key pressed
	gKeyUp = __keyboardStates[ SDLK_UP ];
	gKeyDown = __keyboardStates[ SDLK_DOWN ];
	gKeyLeft = __keyboardStates[ SDLK_LEFT ];
	gKeyRight = __keyboardStates[ SDLK_RIGHT ];

	// update speed

	if (gKeyUp)
		_wm->_desiredTranslationalValue += 1; // pixels
	if (gKeyDown)
		_wm->_desiredTranslationalValue += -1;

	if (_wm->_desiredTranslationalValue > gMaxTranslationalSpeed)
		_wm->_desiredTranslationalValue = gMaxTranslationalSpeed;
	else
		if (_wm->_desiredTranslationalValue < -gMaxTranslationalSpeed)
		_wm->_desiredTranslationalValue = -gMaxTranslationalSpeed;

	if (gKeyLeft)
		_wm->_desiredRotationalVelocity = -10; // degrees
	if (gKeyRight)
		_wm->_desiredRotationalVelocity = 10;

	if (!gKeyUp && !gKeyDown)
		_wm->_desiredTranslationalValue = 0;

	if (!gKeyRight && !gKeyLeft) {
		if (_wm->_desiredRotationalVelocity > 0)
			_wm->_desiredRotationalVelocity--;
		else
			if (_wm->_desiredRotationalVelocity < 0)
			_wm->_desiredRotationalVelocity++;
	}

	if (__keyboardStates[SDLK_o]) {
		_wm->_desiredTranslationalValue = 0;
		_wm->_desiredRotationalVelocity = 0;
	}
}

/*
	Compute actual Translational value and Rotational velocity that can be applied given the agent/robot dynamics.
	By default, this means: apply min/max values bounds, apply threshold on maximum delta values wrt. current values.
	By extension, this means: also apply some specific environment impact on the agent/robot dynamics (e.g. slow down due to sticky terrain, etc.). Details:
	1. convert desired translation/rotation to possible translation/rotation values
	2. apply environment constraints (call external method)
	3. translate into internal delta position update (ie. 2d x/y delta values)
 */
void RobotAgent::applyDynamics() {

	this->_lastAgentAbsoluteOrientation = _wm->_agentAbsoluteOrientation; // in case of collision, original orientation should be restored.

	// * translation
	// assume controller gives desired speed (the following models a feedback system (fr.: systeme asservi))

	_wm->_actualTranslationalValue = _wm->_agentAbsoluteLinearSpeed;

	if (_wm->_desiredTranslationalValue != _wm->_agentAbsoluteLinearSpeed) {
		double diff = fabs(_wm->_desiredTranslationalValue - _wm->_agentAbsoluteLinearSpeed); // Note: using abs(...) would truncate to integer. Use fabs instead. (minor bug solved as of 2010 06 18)

		if (_wm->_desiredTranslationalValue > _wm->_agentAbsoluteLinearSpeed) {
			if (diff > _wm->_maxTranslationalDeltaValue)
				_wm->_actualTranslationalValue += _wm->_maxTranslationalDeltaValue;
			else
				_wm->_actualTranslationalValue += diff;
		} else // strictly below
		{
			if (diff > _wm->_maxTranslationalDeltaValue)
				_wm->_actualTranslationalValue -= _wm->_maxTranslationalDeltaValue;
			else
				_wm->_actualTranslationalValue -= diff;
		}

		// upper and lower bound for translation speed
		if (_wm->_actualTranslationalValue > gMaxTranslationalSpeed)
			_wm->_actualTranslationalValue = gMaxTranslationalSpeed;
		else
			if (_wm->_actualTranslationalValue < -gMaxTranslationalSpeed)
			_wm->_actualTranslationalValue = -gMaxTranslationalSpeed;
	}

	// * rotation
	// assume controller gives angular velocity.

	if (_wm->_desiredRotationalVelocity > 0) {
		if (_wm->_desiredRotationalVelocity <= _wm->_maxRotationalDeltaValue)
			_wm->_actualRotationalVelocity = _wm->_desiredRotationalVelocity;
		else
			_wm->_actualRotationalVelocity = _wm->_maxRotationalDeltaValue;
	} else
		if (_wm->_desiredRotationalVelocity < 0) {
		if (_wm->_desiredRotationalVelocity >= -_wm->_maxRotationalDeltaValue)
			_wm->_actualRotationalVelocity = _wm->_desiredRotationalVelocity;
		else
			_wm->_actualRotationalVelocity = -_wm->_maxRotationalDeltaValue;
	}

	// * compute true displacement (actual (thought to be true) and true (really effected) rotation and translation,

	// update both actual trans/rot values and internal data (see method definition for detailed comments)
	// after this call, wm->_agentAbsoluteLinearSpeed AND _wm->_agentAbsoluteOrientation are updated (internal data)
	gConfigurationLoader->make_RobotAgentWorldInterface()->applyInternalDynamics(_wm);


	// note: the next locomotion-related method called will be the "move" method. It handles collision.
	// In case of collision, actual translation/rotation variables will be updated accordingly
	// Keep in mind that roborobo philosophy assumes pixel-based level of details for collision.
}

/**
 * update the agent position in the environment. Apply simple physics (ie. obstacle collision detection and consequences).
 * if collision, translation and rotation speed are set to zero.
 * note: __recursiveIt is currently unused (maybe used for debug purpose, eg. checking for infinite loop.)
 */
void RobotAgent::move(int __recursiveIt) // the interface btw agent and world -- in more complex envt, this should be handled by the "world".
{
	// apply world dynamics onto this agent

	// compute real valued delta (convert to x/y delta coordinates)
	applyDynamics();

	// save position

	this->xReal_old = _wm->_xReal; // backup old position in case of collision
	this->yReal_old = _wm->_yReal;


	/*for (int i = 0; i < gNbOfAgents; i++) {
		gWorld->getAgent(i)->unregisterAgent();
	}*/

	// update x/y position

	_xDeltaReal = _wm->_agentAbsoluteLinearSpeed * cos(_wm->_agentAbsoluteOrientation * M_PI / 180);
	_yDeltaReal = _wm->_agentAbsoluteLinearSpeed * sin(_wm->_agentAbsoluteOrientation * M_PI / 180);

	_wm->_xReal += _xDeltaReal;
	_wm->_yReal += _yDeltaReal; // TODO: round is for positive values... (ok in this case however as 0,0 is up-left)

	setCoord((int) _wm->_xReal + 0.5, (int) _wm->_yReal + 0.5);

	// * collision with (image) border of the environment - position at border, then bounce

	//Move the collision boxes
	//shift_boxes(); // NOT IMPLEMENTED

	if (isCollision()) {
        _wm->_desiredTranslationalValue = 0;
        
		if (_wm->_agentAbsoluteLinearSpeed >= 1.0) {
			_wm->_agentAbsoluteLinearSpeed--;
		} else {
			if (_wm->_agentAbsoluteLinearSpeed <= -1.0)
				_wm->_agentAbsoluteLinearSpeed++;
		}

		if ((_wm->_agentAbsoluteLinearSpeed > -1.0) && (_wm->_agentAbsoluteLinearSpeed < 1.0)) {
			_wm->_agentAbsoluteLinearSpeed = 0; // roborobo assumes pixel-based resolution for solving collision. Ie. ignore sub-pixel translation values. (see _README.TXT for details)
		}

		_wm->_xReal = xReal_old;
		_wm->_yReal = yReal_old;

		if (std::isnan(_wm->_agentAbsoluteLinearSpeed)){
            _wm->_agentAbsoluteLinearSpeed = 0;
        }
            
        if(_wm->_agentAbsoluteLinearSpeed != 0) // if zero: move is canceled
		{
			__recursiveIt++;
			move(__recursiveIt);
		} else {
			// special case: agent cannot not move at all - restore original coordinate (remember, _x/yReal are global and modified during recursive call).
			_wm->_xReal = xReal_old;
			_wm->_yReal = yReal_old;
			setCoord((int) _wm->_xReal + 0.5, (int) _wm->_yReal + 0.5);
		}

		// update world model variables and internal data

		_wm->_agentAbsoluteLinearSpeed = 0;
		if (gLocomotionMode == 1) // consider obstacle friction or not for rotation?
		{
			_wm->_agentAbsoluteOrientation = _lastAgentAbsoluteOrientation;
			_wm->_actualRotationalVelocity = 0;
		} else
			_wm->_actualRotationalVelocity = _wm->_desiredRotationalVelocity;

		//_wm->_desiredTranslationalValue = _wm->_desiredRotationalVelocity = 0;
		_wm->_actualTranslationalValue = 0;

	} else {
		// actual rotational and translational values matches desired values
		_wm->_actualRotationalVelocity = _wm->_desiredRotationalVelocity;
		_wm->_actualTranslationalValue = _wm->_agentAbsoluteLinearSpeed; // (!) _wm->_desiredTranslationalValue is different as the "desired" translation speed may not be reached due to physical actuator limitations
	}

	// update sensors
	for (int i = 0; i < _wm->_sensorCount; i++) {
		// Warning: the following is repeated in the show method because coordinates are not stored, but are needed to display the sensor rays.
		double x1 = (_wm->_xReal + _wm->_sensors[i][1] * cos(_wm->_sensors[i][2] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double y1 = (_wm->_yReal + _wm->_sensors[i][1] * sin(_wm->_sensors[i][2] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double x2 = (_wm->_xReal + _wm->_sensors[i][3] * cos(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double y2 = (_wm->_yReal + _wm->_sensors[i][3] * sin(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));

		// cast sensor ray.
		_wm->_sensors[i][5] = castSensorRay(gEnvironmentImage, x1, y1, &x2, &y2, _wm->getSensorMaximumDistanceValue(i)); // x2 and y2 are overriden with collision coordinate if ray hits object. -- not used here.
		Uint8 r, g, b;
		Uint32 pixel = getPixel32(gEnvironmentImage, x2, y2);
		SDL_GetRGB(pixel, gEnvironmentImage->format, &r, &g, &b);
		_wm->_sensors[i][6] = (r << 16)+(g << 8) + b; // R=objects, B=agents, can only be one agent OR one object in each location.

		// Cast another sensor on the zone map to detect ditches
		double xx1 = (_wm->_xReal + _wm->_sensors[i][1] * cos(_wm->_sensors[i][2] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double yy1 = (_wm->_yReal + _wm->_sensors[i][1] * sin(_wm->_sensors[i][2] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double xx2 = (_wm->_xReal + _wm->_sensors[i][3] * cos(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double yy2 = (_wm->_yReal + _wm->_sensors[i][3] * sin(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));

		if (gEnableDitchSensors) {
			_wm->_ditchSensor[i] = castSensorRay(gZoneImage, xx1, yy1, &xx2, &yy2, _wm->getSensorMaximumDistanceValue(i));
		}

		if(gEnergyMode) {
			// Cast another sensor on the energy map to detect energy points
			xx2 = (_wm->_xReal + _wm->_sensors[i][3] * cos(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));
			yy2 = (_wm->_yReal + _wm->_sensors[i][3] * sin(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));
			_wm->_energySensor[i] = castSensorRay(gEnergyImage, xx1, yy1, &xx2, &yy2, _wm->getSensorMaximumDistanceValue(i));
		}
	} //(old: (r<<16)+(g<<8)+b;)

	
	Uint8 r, g, b;
	Uint32 pixel = getPixel32(gZoneImage, _wm->_xReal, _wm->_yReal);
	SDL_GetRGB(pixel, gZoneImage->format, &r, &g, &b);
	_wm->_floorSensor = r;
}

bool RobotAgent::isCollision() {
	// check collision with borders and environment objects.
	if ((_x < 0) || (_x + gAgentWidth >= gAreaWidth) ||
		(_y < 0) || (_y + gAgentHeight >= gAreaHeight)) {
		return true; // collision with border
	}

	Uint32 WHITE = SDL_MapRGB(gEnvironmentImage->format, 0xFF, 0xFF, 0xFF);

	static vector<pair<int,int> > agent_points;
	if (agent_points.size() == 0) {
		for (int i = 0; i != gAgentWidth; i++)
			for (int j = 0; j != gAgentHeight; j++)
				if (getPixel32(gAgentMaskImage , i , j) != WHITE)
					agent_points.push_back(make_pair(i,j));
	}

	// * environment objects
	for (vector<pair<int,int> >::iterator it = agent_points.begin(); it != agent_points.end(); it++) {
		int i = it->first;
		int j = it->second;

		if (getPixel32(gEnvironmentImage , _x+i , _y+j) != WHITE)
			return true;
	}

	return false;
}

int RenderTextToSurface(std::string Text, int x, int y, SDL_Surface *Dest, SDL_Color *TXT_Color) {
    
    SDL_Color txt_Color;
    if(TXT_Color == NULL) {
        txt_Color.r = 0x00;
        txt_Color.g = 0x00;
        txt_Color.b = 0x00;
    }else{
        txt_Color.r = TXT_Color->r;
        txt_Color.g = TXT_Color->g;
        txt_Color.b = TXT_Color->b;
    }
    
	SDL_Surface *TTF_Message;
	if (!TTF_WasInit()) {
		TTF_Init();
	}
	TTF_Font *font = TTF_OpenFont(gFontName.c_str(), 24);
	if (font == NULL) {
		char* error = TTF_GetError();
		std::cout << "TTF_Font: " << error << std::endl;
		return 1;
	}
	if (!(TTF_Message = TTF_RenderText_Solid(font, Text.c_str(), txt_Color))) {
		SDL_FreeSurface(TTF_Message);
		std::cout << "Error in function 'RenderTextToSurface': TTF_Message could not be blitted: returned 1" << std::endl;
		return 1;
	}
	TTF_CloseFont(font);
	apply_surface(x, y, TTF_Message, Dest);
	SDL_FreeSurface(TTF_Message);
	return 0;
}

/**
 * Display agent on screen. Add information caption if needed.
 *
 * (render mode only)
 */
void RobotAgent::show() // display on screen
{
	//Show the dot

	if (gNiceRendering) {
		if (gUseOrganisms && _connectToOthers == POSITIVE) {
			apply_surface(_x - gCamera.x, _y - gCamera.y, gAgentPositiveMaskImage, gScreen);
		} else if (gUseOrganisms && _connectToOthers == NEGATIVE) {
			apply_surface(_x - gCamera.x, _y - gCamera.y, gAgentNegativeMaskImage, gScreen);
		} else {
			apply_surface(_x - gCamera.x, _y - gCamera.y, gAgentMaskImage, gScreen); // OPTIONAL (agent is already visible/registered through the environment image -- but: may be useful for image capture
		}
	}

	if(gRenderRobotId){
		std::string str = "";
		str += boost::lexical_cast<std::string > (this->_wm->_agentId);
		RenderTextToSurface(str, _x + (gAgentWidth / 4) - gCamera.x, _y - gCamera.y, gScreen, NULL);
	}
    
    if(gShowAgentEnergyPoints){
		int value = this->getWorldModel()->getEnergyLevel();
        
        SDL_Color TXT_Color;
		TXT_Color.g = (value < 255 ? value : 255);
		TXT_Color.r = 255 - TXT_Color.g;
		TXT_Color.b = 0x00;
        
		RenderTextToSurface(boost::lexical_cast<std::string>(value), _x + (gAgentWidth / 4)  + 10 - gCamera.x, _y + 10 - gCamera.y, gScreen, &TXT_Color);
    }

	if (getWorldModel()->getRobotLED_status() == true) {
		int dx = 1;
		int dy = 1;
		int xcenter = (int) _wm->_xReal + 0.5;
		int ycenter = (int) _wm->_yReal + 0.5;
		int r = getWorldModel()->getRobotLED_redValue();
		int g = getWorldModel()->getRobotLED_greenValue();
		int b = getWorldModel()->getRobotLED_blueValue();

		for (int xTmp = xcenter - dx; xTmp != xcenter + dx + 1; xTmp++)
			for (int yTmp = ycenter - dy - 1; yTmp != ycenter + dy; yTmp++) {
				putPixel32secure(gScreen, xTmp - gCamera.x, yTmp + dy - gCamera.y, SDL_MapRGB(gScreen->format, r, g, b));
			}
	}
	/**
	 * Display agent on screen. Add information caption if needed.
	 */


	if (_wm->_agentId == gAgentIndexFocus && gUserCommandMode) // && _iterations%10 < 5)
	{
		int dx = 10;
		int dy = 10;
		int xcenter = (int) _wm->_xReal + 0.5;
		int ycenter = (int) _wm->_yReal + 0.5;
		int r = 255.0 * ranf();
		int g = 255.0 * ranf();
		int b = 255.0 * ranf();

		for (int xTmp = xcenter - dx; xTmp != xcenter + dx + 1; xTmp++) {
			putPixel32secure(gScreen, xTmp - gCamera.x, ycenter - dy - gCamera.y, SDL_MapRGB(gScreen->format, r, g, b));
			putPixel32secure(gScreen, xTmp - gCamera.x, ycenter + dy - gCamera.y, SDL_MapRGB(gScreen->format, r, g, b));
		}

		for (int yTmp = ycenter - dy; yTmp != ycenter + dy + 1; yTmp++) {
			putPixel32secure(gScreen, xcenter - dx - gCamera.x, yTmp - gCamera.y, SDL_MapRGB(gScreen->format, r, g, b));
			putPixel32secure(gScreen, xcenter + dx - gCamera.x, yTmp - gCamera.y, SDL_MapRGB(gScreen->format, r, g, b));
		}

		//			for ( int xTmp = (int)_wm->_xReal - dx ; xTmp != (int)_wm->_xReal + dx + 1 ; xTmp++ )
		//				for ( int yTmp = (int)_wm->_yReal - dy ; yTmp != (int)_wm->_yReal + dy + 1 ; yTmp++ )
		//						putPixel32secure( gScreen, xTmp - gCamera.x, yTmp - gCamera.y , SDL_MapRGB( gScreen->format, 0xFF, 0x00, ranf() ) );
	}

	if (gShowOrientationMarker) {
		this->_showOrientationMarker();
	}

	if (gDisplaySensors) {
		this->_showSensorRays();
	}

	// caption for storyzones

	if (gDisplayZoneCaption) {
		Uint32 pixel = getPixel32(gZoneImage, _x + gAgentWidth / 2, _y + gAgentHeight / 2);
		if (pixel != SDL_MapRGB(gZoneImage->format, 0x00, 0x00, 0x00)) // check if there's a "story" to display
		{
			// extract story index (if any)
			Uint8 r, g, b;
			SDL_GetRGB(pixel, gZoneImage->format, &r, &g, &b);
			int storyId = r; // assume the red component holds the story index.

			if (storyId >= 0 && storyId < 256 && gZoneCaptionImage[storyId] != NULL && gZoneStatus[storyId]) // security check: story exists?
			{
				// display story caption

				//set caption position
				int xCaption = 0, yCaption = 0;
				if (_x < gAreaWidth / 2)
					xCaption = _x - gCamera.x + 40;
				else
					xCaption = _x - gCamera.x - gZoneCaptionImage[storyId]->w - 40;
				if (_y < gAreaHeight / 2)
					yCaption = _y - gCamera.y + 40;
				else
					yCaption = _y - gCamera.y - gZoneCaptionImage[storyId]->h - 40;

				//display caption
				apply_surface(xCaption, yCaption, gZoneCaptionImage[storyId], gScreen);

				// update story flags (if needed)
				if (storyId >= 100 && storyId < 200 && storyId % 10 == 0) // key story, btw 100 and 199: activate story flag of the 9 next sub-stories
				{
					for (int i = 1; i != 10; i++)
						gZoneStatus[storyId + i] = true;
				} else
					if (storyId >= 200 && storyId < 256 && storyId % 10 != 9) // sub-story, btw 200 and 299: activate the next story flag (if not last)
				{
					gZoneStatus[storyId + 1] = true;
				}
			}
		}
	}
}

void RobotAgent::_showOrientationMarker()
{
	int xOrientationMarker = (int) (_wm->_xReal + 0.5) + gAgentWidth / 2 * cos(_wm->_agentAbsoluteOrientation * M_PI / 180);
	int yOrientationMarker = (int) (_wm->_yReal + 0.5) + gAgentWidth / 2 * sin(_wm->_agentAbsoluteOrientation * M_PI / 180);

	Uint32 color = SDL_MapRGBA(gScreen->format,
		(gOrientationMarkerColor >> 24) & 0xFF,
		(gOrientationMarkerColor >> 16) & 0xFF,
		(gOrientationMarkerColor >> 8) & 0xFF,
		(gOrientationMarkerColor >> 0) & 0xFF);

	if (_wm->_agentId == gAgentIndexFocus && gUserCommandMode) {
		int g, b;
		g = b = (32 * _iterations % 256) > 128 ? 0 : 255;

		color = SDL_MapRGB(gScreen->format, 0, b, g);
	}

	int R = gOrientationMarkerRadius;
	for (int xTmp = -R; xTmp < R; xTmp++)
		for (int yTmp = -R; yTmp < R; yTmp++)
			if (xTmp*xTmp + yTmp*yTmp <= R*R)
				putPixel32secure(gScreen, xOrientationMarker - gCamera.x + xTmp, yOrientationMarker - gCamera.y + yTmp, color);
}

void RobotAgent::_showSensorRays()
{
	for (int i = 0; i < _wm->_sensorCount; i++) {
		// Warning: the following is a repetition of code already in the move method (sensor ray casting) in order to display it (coordinates are not stored)
		double x1 = (_wm->_xReal + _wm->_sensors[i][1] * cos(_wm->_sensors[i][2] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double y1 = (_wm->_yReal + _wm->_sensors[i][1] * sin(_wm->_sensors[i][2] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double x2 = (_wm->_xReal + _wm->_sensors[i][3] * cos(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));
		double y2 = (_wm->_yReal + _wm->_sensors[i][3] * sin(_wm->_sensors[i][4] + _wm->_agentAbsoluteOrientation * M_PI / 180));

		// sensor ray casting is also performed in the move method -- this time we dont store data (already done). -- this one is only used to *display* the ray.
		/*_sensors[i][5] = */castSensorRay(gEnvironmentImage, x1, y1, &x2, &y2, _wm->getSensorMaximumDistanceValue(i)); // x2 and y2 are overriden with collision coordinate if ray hits object.

        Uint32 color;
        if (_wm->_sensors[i][5] < _wm->getSensorMaximumDistanceValue(i) - 1) //gSensorRange-1 )
			color = gSensorRayObstacleColor;
		else if (gEnergyMode && _wm->_energySensor[i] < _wm->getSensorMaximumDistanceValue(i) - 1)
			color = gSensorRayEnergyColor;
		else
			color = gSensorRayFreeColor;

        traceRayRGBA(gScreen, int(x1 + 0.5) - gCamera.x, int(y1 + 0.5) - gCamera.y, int(x2 + 0.5) - gCamera.x, int(y2 + 0.5) - gCamera.y,
        	(color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
	}
}

void RobotAgent::registerAgent() {
	if (gAgentRegistration && !this->_registered) {
		if (gUseOrganisms && _connectToOthers == POSITIVE) {
			register_surface(_x, _y, gAgentPositiveMaskImage, gEnvironmentImage, _wm->_agentId + 1000); // [!n] CONVENTION: _agentId+100 to avoid confusion with empty space (0: empty ; 1: static object; 2-99: reserved for other static objects)
		} else if (gUseOrganisms && _connectToOthers == NEGATIVE) {
			register_surface(_x, _y, gAgentNegativeMaskImage, gEnvironmentImage, _wm->_agentId + 1000); // [!n] CONVENTION: _agentId+100 to avoid confusion with empty space (0: empty ; 1: static object; 2-99: reserved for other static objects)
		} else {
			register_surface(_x, _y, gAgentMaskImage, gEnvironmentImage, _wm->_agentId + 1000); // [!n] CONVENTION: _agentId+100 to avoid confusion with empty space (0: empty ; 1: static object; 2-99: reserved for other static objects)
		}
		this->_registered = true;
	}
}

void RobotAgent::unregisterAgent() {
	if (gAgentRegistration && this->_registered) {
		if (gUseOrganisms && _connectToOthers == POSITIVE) {
			clean_surface(_x, _y, gAgentPositiveMaskImage, gEnvironmentImage); // original
		} else if (gUseOrganisms && _connectToOthers == NEGATIVE) {
			clean_surface(_x, _y, gAgentNegativeMaskImage, gEnvironmentImage); // original
		} else {
			clean_surface(_x, _y, gAgentMaskImage, gEnvironmentImage); // original
		}
		this->_registered = false;
	}
}

bool RobotAgent::isRegistered(){
	return _registered;
}

void RobotAgent::setCoordReal(int __x, int __y) // agent is centered on point
{
	//setCoord(gAgentXStart+__x,gAgentYStart+__y);
	//setCoord(__x,__y);
	_wm->_xReal = __x;
	_wm->_yReal = __y;
}

AgentObserver* RobotAgent::getObserver() {
	return _agentObserver;
}

BehaviorControlArchitecture* RobotAgent::getBehavior() {
	return _behavior;
}

RobotAgentWorldModel* RobotAgent::getWorldModel() {
	return _wm;
}

std::vector<int> RobotAgent::getNeighbors() {
	std::vector<int> neighborsList;

	if (gRadioNetwork) {
		int id = _wm->_agentId;

		// internal cuisine: for all existing agents, check if in the vicinity. Yes? add to neighbors list.
		for (int id_target = 0; id_target != gAgentCounter; id_target++) {
			if (gRadioNetworkArray[id][id_target] > 0) // note that the diagonal is always zero (ie. self-connection is not considered). check anyway (avoid an extra if statement).
				neighborsList.push_back(id_target);
		}
	} else
		if (gVerbose)
		std::cout << "[WARNING] RobotAgent::getNeighbors() is called but radio network is *inactive*! (always return empty list)" << std::endl;


	return neighborsList;
}

std::vector<RobotAgentPtr> RobotAgent::getNearRobots(){
	std::vector<RobotAgentPtr> agents;
	
	for(int i=0;i<_wm->_sensorCount;i++){
		if(_wm->_sensors[i][6] >= 1000 && _wm->_sensors[i][6] <= 2000){
			RobotAgentPtr other = gWorld->getAgent(_wm->_sensors[i][6]-1000);
			if(!gUseOrganisms || !this->isPartOfSameOrganism(other)){
				agents.push_back(gWorld->getAgent(_wm->_sensors[i][6]-1000));
			}
		}
	}
	
	return agents;
}

// ---- ---- ---- ---- ----

/**
 * This function sets up connections between robots who are close enough to
 * each other and have their connection ring set to 1.
 */
void RobotAgent::setUpConnections() {
	if (!gUseOrganisms) return;
    
	int nAgents = _wm->_world->getNbOfAgent();
	int id = _wm->_agentId;

	if (this->isPartOfOrganism()) {
		if (this->getConnectToOthers() == Agent::NEGATIVE) {
			this->letGoOfOrganism();
		}/* else if (this->getConnectToOthers() == Agent::NEUTRAL) {
			std::vector<RobotAgentPtr>::iterator it;
			for (it = this->connected->begin(); it != this->connected->end(); it++) {
				RobotAgentPtr other = (*it);
				if (other->getConnectToOthers() == Agent::NEUTRAL) {
					it = this->connected->erase(it);
					other->removeNeighbour(gWorld->getAgent(this->_wm->_agentId));
					break;
				}
			}
		}*/
		if (this->connected->empty()) {
			this->letGoOfOrganism();
		}
	}
	for (int i = 0; i < nAgents; i++) {
		RobotAgentPtr other = _wm->_world->getAgent(i);
		RobotAgentWorldModel *otherWM = (RobotAgentWorldModel*) other->getWorldModel();
		int otherId = otherWM->_agentId;

		if (otherId != id) {
			// If they're out of bounds, ignore.
			// This happens at the start of the simulation sometimes and can lead
			// to false positives
			if (this->isOutOfBounds() || other->isOutOfBounds()) {
				return;
			}

			double x1, y1, x2, y2;
			x1 = this->_wm->_xReal;
			y1 = this->_wm->_yReal;
			x2 = other->_wm->_xReal;
			y2 = other->_wm->_yReal;

			// Are they within range?
			if (SDL_CollideBoundingCircle(gAgentMaskImage, x1, y1, gAgentMaskImage, x2, y2, gConnectionGap)) {
				// connect to other robot
                
                // ---- Mark: but only connect in case you are willing to do so.
                
				if ((this->getConnectToOthers() == Agent::POSITIVE) && (other->getConnectToOthers() == Agent::POSITIVE)) {
					// || other->getConnectToOthers() == Agent::NEUTRAL)) {
					this->connectToRobot(other);
				}
//				else if (this->getConnectToOthers() == Agent::NEUTRAL && other->getConnectToOthers() == Agent::POSITIVE) {
//					this->connectToRobot(other);
//				}
			}
		}
	}
}

void RobotAgent::connectToRobot(RobotAgentPtr other) {
	if (!gUseOrganisms) return;

	if (this->isPartOfOrganism() && other->isPartOfOrganism()
		&& this->getOrganism()->getId() == other->getOrganism()->getId()) {
		// the same organism, do nothing.
	} else if (other->isPartOfOrganism() && this->isPartOfOrganism()
		&& other->getOrganism()->getId() != this->getOrganism()->getId()) {
		// two different organisms
		Organism* o1 = this->getOrganism();
		Organism* o2 = other->getOrganism();

		this->addNeighbour(other);
		other->addNeighbour(gWorld->getAgent(this->_wm->_agentId));

		// merge smaller into larger
		if (o1->size() >= o2->size()) {
			gLogFile << "Merging organism " << o2->getId() << " into " << o1->getId() << std::endl;

			//other->clickOnto(this, o2);

			o2->mergeInto(o1);
			o2->setInactive();
			//Organism::remove(o2);
			//delete o2;
		} else {
			gLogFile << "Merging organism " << o1->getId() << " into " << o2->getId() << std::endl;

			//this->clickOnto(other, o1);

			o1->mergeInto(o2);
			o1->setInactive();
			
			//Organism::remove(o1);
			//delete o1;
		}
	} else if (other->isPartOfOrganism()) {
		Organism* organism = other->getOrganism();
		organism->addRobot(gWorld->getAgent(this->_wm->_agentId));
		this->setOrganism(organism);

		this->addNeighbour(other);
		other->addNeighbour(gWorld->getAgent(this->_wm->_agentId));

		//this->clickOnto(other);
		gLogFile << "Adding agent " << this->_wm->_agentId << " to organism: " << organism->getId() << std::endl;
	} else if (this->isPartOfOrganism()) {
		Organism* organism = this->getOrganism();
		organism->addRobot(other);
		other->setOrganism(organism);

		this->addNeighbour(other);
		other->addNeighbour(gWorld->getAgent(this->_wm->_agentId));

		//other->clickOnto(this);
		gLogFile << "Adding agent " << other->_wm->_agentId << " to organism: " << organism->getId() << std::endl;
	} else {
		Organism* organism = new Organism();
		gLogFile << "Creating new organism: " << organism->getId() << " robots: " << this->_wm->_agentId << ", " << other->_wm->_agentId << std::endl;
		organism->addRobot(gWorld->getAgent(this->_wm->_agentId));
		organism->addRobot(other);

		this->setOrganism(organism);
		other->setOrganism(organism);

		this->addNeighbour(other);
		other->addNeighbour(gWorld->getAgent(this->_wm->_agentId));

		Organism::add(organism);

		//this->clickTogether(other);
	}
}

void RobotAgent::addNeighbour(RobotAgentPtr neighbour) {
	this->connected->push_back(RobotAgentPtr(neighbour));
}

void RobotAgent::removeNeighbour(RobotAgentPtr neighbour) {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = this->connected->begin(); it != this->connected->end(); it++) {
		if ((*it)->_wm->_agentId == neighbour->_wm->_agentId) {
			this->connected->erase(it);
			return;
		}
	}
}

std::vector<RobotAgentPtr> *RobotAgent::getConnected() {
	return this->connected;
}

bool RobotAgent::isOutOfBounds() {
	//If the dot went too far to the left or right
	if ((this->_x < 0) || (this->_x + gAgentWidth > gAreaWidth)) {
		return true;
	}
	//If the dot went too far up or down
	if ((this->_y < 0) || (this->_y + gAgentHeight > gAreaHeight)) {
		return true;
	}
	return false;
}

void RobotAgent::letGoOfOrganism() {
	if (!gUseOrganisms) return;

	if (this->isPartOfOrganism()) {
		gLogFile << "Agent: " << this->_wm->_agentId << " letting go of organism: " << this->getOrganism()->getId() << std::endl;
		this->getOrganism()->removeRobot(gWorld->getAgent(this->_wm->_agentId));

		std::vector<RobotAgentPtr>::iterator it;
		for (it = this->connected->begin(); it != this->connected->end(); it++) {
			(*it)->removeNeighbour(gWorld->getAgent(this->_wm->_agentId));
		}
		this->connected->clear();
	}
}

void RobotAgent::setOrganism(Organism* __organism) {
	if (!gUseOrganisms) return;

	_organism = __organism;
}

Organism* RobotAgent::getOrganism() {
	return _organism;
}

bool RobotAgent::isPartOfOrganism() {
	return _organism != NULL;
}

/**
 * Check whether the other robot is in the same organism
 * @param other
 * @return
 */
bool RobotAgent::isPartOfSameOrganism(RobotAgentPtr other) {
	std::vector<RobotAgentPtr>::iterator it;
	if (this->isPartOfOrganism()) {
		for (it = this->_organism->agents.begin(); it != this->_organism->agents.end(); it++) {
			if ((*it)->_wm->_agentId == other->_wm->_agentId) {
				return true;
			}
		}
	}
	return false;
}
