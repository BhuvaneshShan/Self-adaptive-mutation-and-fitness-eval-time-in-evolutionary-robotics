/*
 *  agent.h
 *  roborobo
 *
 *  Created by Nicolas on 16/12/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef AGENT_H
#define AGENT_H 

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/SDL_gfxRoborobo.h"

//The dot

class Agent {
protected:

	//The X and Y offsets of the dot -- move() method should modify xVel and yVel instead
	int _x, _y;

	//The collision boxes of the dot
	//std::vector<SDL_Rect> box; 	// NOT IMPLEMENTED

	//The velocity of the dot - in fact, the number of pixels movement per step.
	int _xDelta, _yDelta;

	//Moves the collision boxes relative to the dot's offset
	//void shift_boxes();  	// NOT IMPLEMENTED

	// (re-)initialize variables (called by constructors)
	virtual void reset() = 0;

	int _connectToOthers;
	
public:

	//Initializes the variables
	Agent();

	virtual ~Agent() {};

	//Agent(SDL_Surface *__mask);	// NOT IMPLEMENTED

	void setConnectToOthers(int __connectToOthers);
	int getConnectToOthers();

	// controller step
	virtual void stepBehavior() = 0;
	virtual void stepBehavior(Uint8* __keyboardStates) = 0;

	//Moves the dot
	void move();

	//Shows the dot on the screen
	void show();

	//Sets the camera over the dot
	void set_camera();

	void getCoord(int &coord_x, int &coord_y) {
		coord_x = _x;
		coord_y = _y;
	}

	void updateCoord(int &__xUpdate, int &__yUpdate) {
		_x += __xUpdate;
		_y += __yUpdate;
	}

	void setCoord(int __x, int __y) // agent is centered on point
	{
		_x = __x - gAgentWidth / 2;
		_y = __y - gAgentHeight / 2;
	}

	static const int POSITIVE = 1;
	static const int NEUTRAL = 0;
	static const int NEGATIVE = -1;
};


#endif

