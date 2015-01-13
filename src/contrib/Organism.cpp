/*
 *  Organism.cpp
 *  roborobo
 *
 *  Created by Berend Weel.
 *
 */

#include "Organism.h"
#include "Agents/Agent.h"
#include "World/World.h"
#include <algorithm>
#include <vector>
#include <stack>
#include <cstdlib> 


/**
 *  Globals definitions
 * 
 * Globals can used by all Organism functions as well as in projects
 * 
 * removeOrganism, addOrganism and destroyOrganisms functions
 * maintain a global set of organisms in the simulation
 * 
 * Below that is the implementation of the Organism Class
 */
int Organism::nbOrganisms = 0;
std::vector<Organism*> *Organism::organisms = NULL;
std::ofstream Organism::sizeLog;

void Organism::remove(Organism *organism) {
	std::vector<Organism*>::iterator it;
	for (it = organisms->begin(); it != organisms->end(); it++) {
		if (*it == organism) {
			organisms->erase(it);
			return; // do not continue, the iterator is now invalid
		}
	}
}

void Organism::add(Organism *organism) {
	organisms->push_back(organism);
}

void Organism::reset() {
	if (organisms != NULL) {
		Organism::destroy();
	}
	organisms = new std::vector<Organism*>();

	sizeLog.open(gOrganismSizesLogFilename.c_str());
	if (!sizeLog) {
		std::cout << "[error] Cannot open log file " << gOrganismSizesLogFilename << "." << std::endl << std::endl;
	}

	sizeLog << "Iteration,Organism,Size,Age,x,y,Agents,a,b,c,d,e,f,g" << std::endl;
}

void Organism::destroy() {
	std::ofstream oLog;
	oLog.open(gOrganismLogFilename.c_str());

	if (!oLog) {
		std::cout << "[error] Cannot open log file " << gOrganismLogFilename << "." << std::endl << std::endl;
	}

	std::vector<Organism*>::iterator it;
	for (it = organisms->begin(); it != organisms->end(); it++) {
		if (oLog) {
			oLog << "Organism: " << (*it)->id << " creation: " << (*it)->creationIteration << " death: ";
			if ((*it)->isActive()) {
				oLog << gWorld->getIterations();
				oLog << " agents: ";
				std::vector<RobotAgentPtr>::iterator it2;
				for (it2 = (*it)->agents.begin(); it2 != (*it)->agents.end(); it2++) {
					oLog << (*it2)->getWorldModel()->_agentId << " ";
				}
			} else {
				oLog << (*it)->deathIteration;
			}
			oLog << std::endl;
		}
		delete (*it);
	}

	if (oLog) {
		oLog.close();
	}

	delete organisms;
	organisms = NULL;
}

void Organism::showAll() {
	std::vector<Organism*>::iterator it;
	for (it = organisms->begin(); it != organisms->end(); it++) {
		(*it)->show();
	}
}

void Organism::logAll() {
	if (sizeLog) {
		std::vector<Organism*>::iterator it;
		for (it = organisms->begin(); it != organisms->end(); it++) {
			(*it)->logOrganism();
		}
	}
}

/**
 *  Class Implementation
 */
Organism::Organism() {
	this->id = ++Organism::nbOrganisms;
	//this->agents = new std::vector<RobotAgentPtr>();
	this->hasMoved = false;
	this->creationIteration = gWorld->getIterations();
	this->active = true;
}

Organism::~Organism() {
}

void Organism::setId(int _id) {
	this->id = _id;
}

int Organism::getId() {
	return this->id;
}

void Organism::addRobot(RobotAgentPtr robot) {
    // Mark: Adding a robot to an organism now implies that in case the organsim consists of a single agent the leader should be determined. Otherwise, the leader has already been determined.
    
    if (this->agents.size() == 0){
        // If this is the first robot to join the organsism, this is the leader.
        this->leader = robot;
    }else if (this->agents.size() == 1){
        // If this is the second, a random leader is selected.
        int r = rand();
        if (r < 0.5*RAND_MAX){
            this->leader = robot;
        }
    }else{
        // Otherwise we will just stick to the previous leader.
    }
	
    this->agents.push_back(robot);
    
    robot->getWorldModel()->joinedOrganism = true;
}

void Organism::removeRobot(RobotAgentPtr robot) {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = this->agents.begin(); it != this->agents.end(); it++) {
		if ((*it)->getWorldModel()->_agentId == robot->getWorldModel()->_agentId) {
			this->agents.erase(it);
			break;
		}
	}
	robot->setOrganism(NULL);
    robot->getWorldModel()->joinedOrganism = false;
    
	// Remove the organism if there is only 1 robot left in it
	if (this->agents.size() == 1) {
		this->agents.front()->setOrganism(NULL);
		this->agents.clear();
		this->leader = RobotAgentPtr();

		gLogFile << "Organism: " << this->id << " created at iteration: " << this->creationIteration << " destroyed at iteration " << gWorld->getIterations()
				<< std::endl;

		//Organism::remove(this);
		this->setInactive();

		//delete this;
	} else if (this->agents.size() < 1) {
		gLogFile << "Organism: " << this->id << " created at iteration: " << this->creationIteration << " destroyed at iteration " << gWorld->getIterations()
				<< std::endl;
		//Organism::remove(this);

		this->leader = RobotAgentPtr();
		this->setInactive();
		//delete this;
	}else{
        // Create another leader in case the leader has just been removed. We'll just take the first one in the agents vector.
        if (this->leader == robot){
            this->leader = agents.at(0);
        }
    }
}

bool Organism::contains(RobotAgentPtr robot) {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = this->agents.begin(); it != this->agents.end(); it++) {
		if ((*it)->getWorldModel()->_agentId == robot->getWorldModel()->_agentId) {
			return true;
		}
	}
	return false;
}

void Organism::prepareAllMove() {
	std::vector<Organism*>::iterator it;
	for (it = organisms->begin(); it != organisms->end(); it++) {
		(*it)->prepareMove();
	}
}

void Organism::checkAllIntegrity() {
	std::vector<Organism*>::iterator it;
	std::vector<Organism*> neworgs = std::vector<Organism*>();
	for (it = organisms->begin(); it != organisms->end(); it++) {
		std::vector<Organism*> tmp = (*it)->checkIntegrity();
		neworgs.insert(neworgs.end(), tmp.begin(), tmp.end());
	}
	organisms->insert(organisms->end(), neworgs.begin(), neworgs.end());
}

int Organism::size() {
	return this->agents.size();
}

void Organism::setActive() {
	this->active = true;
}

void Organism::setInactive() {
	this->agents.clear();
	this->active = false;
	this->deathIteration = gWorld->getIterations();
}

bool Organism::isActive() {
	return this->active;
}

/**
 * Merges the organism into another organism. Do not forget to
 * deallocate the organism that merged into the other.
 * 
 * @param other the Organism to merge into
 */
void Organism::mergeInto(Organism* other) {
    // We're going to merge two organisms. The leader should be the one with the highest number of agents.
  
    if (other->agents.size() > agents.size()){
        this->leader = other->leader;
        gLogFile << "New leader elected when combining organisms" << std::endl;
    }
    
	other->agents.insert(other->agents.end(), this->agents.begin(), this->agents.end());
	std::vector<RobotAgentPtr>::iterator it;
	for (it = this->agents.begin(); it != this->agents.end(); it++) {
		(*it)->setOrganism(other);
        (*it)->getWorldModel()->joinedOrganism = true;
	}
	gLogFile << "Organism: " << this->id << " created at iteration: " << this->creationIteration << " destroyed at iteration " << gWorld->getIterations()
			<< std::endl;
}

/**
 * Check whether the organism is still connected
 */
std::vector<Organism*> Organism::checkIntegrity() {
	std::vector<Organism*> orgs = std::vector<Organism*>();
	if (!isActive()) {
		return orgs;
	}

	std::vector<RobotAgentPtr> not_visited = std::vector<RobotAgentPtr>(this->agents);
	std::vector<RobotAgentPtr> loners = std::vector<RobotAgentPtr>();

	std::vector<RobotAgentPtr> connected = std::vector<RobotAgentPtr>();
	std::stack<RobotAgentPtr> s = std::stack<RobotAgentPtr>();

	// Tree visiting algorithm using a stack
	// All nodes of the "tree" should be visited, otherwise
	// the organism is falling apart
	// Repeat until all robots in the (ex)organism are accounted for
	while (!not_visited.empty()) {
		s.push(not_visited.front());
		while (!s.empty()) {
			RobotAgentPtr a = s.top();
			s.pop();

			// This robot has been visited
			std::vector<RobotAgentPtr>::iterator it;
			it = std::find(not_visited.begin(), not_visited.end(), a);
			if (it != not_visited.end()) {
				not_visited.erase(it);
			}

			// so it is connected to the current organism
			connected.push_back(a);

			// Add the neighbours of this robot to the stack
			std::vector<RobotAgentPtr>::iterator it2;
			for (it2 = a->getConnected()->begin(); it2 != a->getConnected()->end(); it2++) {
				if (std::find(connected.begin(), connected.end(), (*it2)) == connected.end()) {
					s.push((*it2));
				}
			}
		}

		if (connected.size() == this->agents.size()) {
			// organism intact.
			return orgs;
		} else if (connected.size() > 1) {
			// a connected part of the organism exist
			Organism *o = new Organism();
			std::vector<RobotAgentPtr>::iterator it;
			for (it = connected.begin(); it != connected.end(); it++) {
				o->addRobot((*it));
				(*it)->setOrganism(o);
			}
			orgs.push_back(o);
		} else if (connected.size() == 1) {
			// this robot is not connected to any other
			connected.front()->letGoOfOrganism();
			loners.push_back(connected.front());
		}
		connected.clear();
	}

	gLogFile << "Organism " << this->id << " is falling apart into " << orgs.size() << " organisms and " << loners.size() << " single robots" << std::endl;
	std::vector<Organism *>::iterator it;
	for (it = orgs.begin(); it != orgs.end(); it++) {
		gLogFile << "Creating new organism " << (*it)->id << " out of: ";
		std::vector<RobotAgentPtr>::iterator it2;
		for (it2 = (*it)->agents.begin(); it2 != (*it)->agents.end(); it2++) {
			gLogFile << (*it2)->getWorldModel()->_agentId << " ";
		}
		gLogFile << std::endl;
	}
	std::vector<RobotAgentPtr>::iterator it2;
	gLogFile << "Lone agents: ";
	for (it2 = loners.begin(); it2 != loners.end(); it2++) {
		gLogFile << (*it2)->getWorldModel()->_agentId << " ";
		(*it2)->setOrganism(NULL);
	}
	gLogFile << std::endl;

	this->setInactive();

	return orgs;
}

/**
 *  Initialize values needed before movement
 */
void Organism::prepareMove() {
	if (this->isActive()) {
		this->hasMoved = false;
	}
}

/**
 * Perform the actual movement
 */
void Organism::move() {
	// Do not move more than once in an update
	if (hasMoved || !active)
		return;

    double angle, translation = 0;
	// Calculate the combined angle and speed of the robots
	combineVectors(angle, translation);

	// Move all the agents using the combined angle
	// and speed.
	moveOrganism(angle, translation);

	// Did we collide with anything?
	if (detectCollisions()) {
		// Then roll the movement back
		rollbackMove();
	} else {
		updateWorldModels();
	}

	// Anyway, we are done moving this movement
	this->hasMoved = true;

	// So update the sensors
	updateSensors();
}

void Organism::show() {
	if (isActive()) {
		std::cout << "Organism: " << this->getId() << " agents:";
		std::vector<RobotAgentPtr>::iterator it;
		for (it = this->agents.begin(); it != this->agents.end(); it++) {
			std::cout << " " << (*it)->getWorldModel()->_agentId;
		}
		std::cout << std::endl;
	}
	sizeLog << std::endl;
}

void Organism::logOrganism() {
	if (isActive()) {
		double x,y;
		this->getBaryCenter(x,y);
		sizeLog << gWorld->getIterations() << "," << this->getId();
		sizeLog << "," << agents.size();
		sizeLog << "," << (gWorld->getIterations() - this->creationIteration);
		sizeLog << "," << x << "," << y;

		std::vector<RobotAgentPtr>::iterator it;
		for (it = this->agents.begin(); it != this->agents.end(); it++) {
			sizeLog << "," << (*it)->getWorldModel()->_agentId;
		}
		sizeLog << std::endl;
	}
}

/**
 * Combines the vectors of the robots in the organism and
 * also unregisters each agent.
 * @param angle
 * @param translation
 */
void Organism::combineVectors(double &angle, double &translation) {
	std::vector<RobotAgentPtr>::iterator it;
	double Rx = 0, Ry = 0;

	// Now it just includes a single agent, so the calculation of the combineVectors is just about calculating the movement of the leader.
    for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel* worldModel = (*it)->getWorldModel();

		(*it)->applyDynamics();

		(*it)->xReal_old = worldModel->_xReal;
		(*it)->yReal_old = worldModel->_yReal;
    }
    
    RobotAgentWorldModel* wm = leader->getWorldModel();
    Rx = wm->_agentAbsoluteLinearSpeed * cos(wm->_agentAbsoluteOrientation * M_PI / 180);
    Ry = wm->_agentAbsoluteLinearSpeed * sin(wm->_agentAbsoluteOrientation * M_PI / 180);
	
    if (gRiverbed) {
		int riverAgents = this->agentsInRiverzone();
		double extraStrength = 0;
		if(riverAgents > 0){
			extraStrength = log(riverAgents);
			Rx *= riverAgents;
			Ry *= riverAgents;
		}
        
		Rx += extraStrength * gStreamStrength * cos(gStreamDirection * M_PI / 180);
		Ry += extraStrength * gStreamStrength * sin(gStreamDirection * M_PI / 180);
    }else{
        double extraStrength = 0;
        switch(gSpeedScale){
            case NEUTRAL:
                extraStrength = 1;
                break;
            case WORSE_SUBLINEAR:
            case BETTER_SUBLINEAR:
                extraStrength = log(this->size());
                break;
            case WORSE_LINEAR:
            case BETTER_LINEAR:
                extraStrength = this->size();
                break;
            case WORSE_SUPERLINEAR:
            case BETTER_SUPERLINEAR:
                extraStrength = exp(this->size());
                break;
        }
        if(gSpeedScale > NEUTRAL){
            Rx *= extraStrength;
            Ry *= extraStrength;
        }else if(gSpeedScale < NEUTRAL){
            Rx /= extraStrength;
            Ry /= extraStrength;
        }
    }
        
    angle = atan2(Ry, Rx);
    translation = sqrt(Rx * Rx + Ry * Ry);
}

void Organism::moveOrganism(double angle, double translation) {
	double _xDeltaReal, _yDeltaReal;
	std::vector<RobotAgentPtr>::iterator it;

	_xDeltaReal = translation * cos(angle);
	_yDeltaReal = translation * sin(angle);
	for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel* wm = (*it)->getWorldModel();

		if ((*it)->isRegistered()) {
			(*it)->unregisterAgent();
		}

		wm->_xReal += _xDeltaReal;
		wm->_yReal += _yDeltaReal;

		(*it)->setCoord((int) wm->_xReal + 0.5, (int) wm->_yReal + 0.5);
	}
}

bool Organism::detectCollisions() {
	// Collision detection
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		if ((*it)->isCollision()) {
			return true;
		}
	}
	return false;
}

void Organism::rollbackMove() {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel *wm = (*it)->getWorldModel();
		wm->_desiredTranslationalValue = 0; // cancel any translation order as agent translation speed is set to zero after collision. (note that rotation is still ok)
		wm->_actualTranslationalValue = 0;
		wm->_agentAbsoluteLinearSpeed = 0;
		wm->_xReal = (*it)->xReal_old;
		wm->_yReal = (*it)->yReal_old;
		(*it)->setCoord((int) wm->_xReal + 0.5, (int) wm->_yReal + 0.5);

		if (gLocomotionMode == 1) // consider obstacle friction or not for rotation?
		{
			wm->_agentAbsoluteOrientation = (*it)->_lastAgentAbsoluteOrientation;
			wm->_actualRotationalVelocity = 0;
		} else {
			wm->_actualRotationalVelocity = wm->_desiredRotationalVelocity;
		}
	}
}

// actual rotational and translational values matches desired values
void Organism::updateWorldModels() {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel *wm = (*it)->getWorldModel();
		wm->_actualRotationalVelocity = wm->_desiredRotationalVelocity;
		wm->_actualTranslationalValue = wm->_agentAbsoluteLinearSpeed;
	}
}

void Organism::updateSensors() {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel *wm = (*it)->getWorldModel();
		// update sensors
		for (int i = 0; i != wm->_sensorCount; i++) {
			// Warning: the following is repeated in the show method because coordinates are not stored, but are needed to display the sensor rays.
			double x1 = (wm->_xReal + wm->_sensors[i][1] * cos(wm->_sensors[i][2] + wm->_agentAbsoluteOrientation * M_PI / 180));
			double y1 = (wm->_yReal + wm->_sensors[i][1] * sin(wm->_sensors[i][2] + wm->_agentAbsoluteOrientation * M_PI / 180));
			double x2 = (wm->_xReal + wm->_sensors[i][3] * cos(wm->_sensors[i][4] + wm->_agentAbsoluteOrientation * M_PI / 180));
			double y2 = (wm->_yReal + wm->_sensors[i][3] * sin(wm->_sensors[i][4] + wm->_agentAbsoluteOrientation * M_PI / 180));

			// cast sensor ray.
			wm->_sensors[i][5] = castSensorRay(gEnvironmentImage, x1, y1, &x2, &y2, wm->getSensorMaximumDistanceValue(i)); // x2 and y2 are overriden with collision coordinate if ray hits object. -- not used here.
			Uint8 r, g, b;
			Uint32 pixel = getPixel32(gEnvironmentImage, x2, y2);
			SDL_GetRGB(pixel, gEnvironmentImage->format, &r, &g, &b);
			wm->_sensors[i][6] = (r << 16) + (g << 8) + b; // R=objects, B=agents, can only be one agent OR one object in each location.
		} //(old: (r<<16)+(g<<8)+b;)

		Uint8 r, g, b;
		Uint32 pixel = getPixel32(gZoneImage, wm->_xReal + 0.5, wm->_yReal + 0.5);
		SDL_GetRGB(pixel, gZoneImage->format, &r, &g, &b);
		wm->_floorSensor = r;
	}
}

void Organism::registerOrganism() {
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		if (!(*it)->isRegistered()) {
			(*it)->registerAgent();
		}
	}
}

void Organism::getBaryCenter(double &x, double &y) {
	x = 0;
	y = 0;
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel* wm = (*it)->getWorldModel();
		x += wm->_xReal;
		y += wm->_yReal;
	}
	
	x = x / agents.size();
	y = y / agents.size();
}

int Organism::agentsInRiverzone() {
	int nAgents = 0;
	std::vector<RobotAgentPtr>::iterator it;
	for (it = agents.begin(); it != agents.end(); it++) {
		RobotAgentWorldModel* wm = (*it)->getWorldModel();
		if(wm->_floorSensor == gRiverbedZone || gRiverbedZone == -1)
			nAgents++;
	}
	return nAgents;
}
