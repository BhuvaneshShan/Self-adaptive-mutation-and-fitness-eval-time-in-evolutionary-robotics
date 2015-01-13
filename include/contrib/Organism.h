/* 
 * File:   Organism.h
 * Author: bweel
 *
 * Created on April 13, 2011, 6:10 PM
 */

#ifndef ORGANISM_H
#define	ORGANISM_H

#include <vector>
#include <ostream>
#include "RoboroboMain/roborobo.h"

class Organism {
public:
	std::vector<RobotAgentPtr> agents;
    RobotAgentPtr leader;

	Organism();
	virtual ~Organism();

	void setId(int _id);
	int getId();

	int size();

	void addRobot(RobotAgentPtr robot);
	void removeRobot(RobotAgentPtr robot);
    
	void mergeInto(Organism *other);

	void prepareMove();
	void move();
	void registerOrganism();

	void show();
	void logOrganism();
	
	void getBaryCenter(double &x, double &y);
	
	std::vector<Organism*> checkIntegrity();
	
	bool contains(RobotAgentPtr robot);
	
	void setActive();
	void setInactive();
	bool isActive();

	static void remove(Organism *organism);
	static void add(Organism *organism);
	static void reset();
	static void destroy();
	static void showAll();
	static void logAll();
	static void prepareAllMove();
	static void checkAllIntegrity();
	
	static int nbOrganisms;

private:
	void combineVectors(double &angle, double &translation);
	void moveOrganism(double angle, double translation);
	bool detectCollisions();
	void rollbackMove();
	void updateWorldModels();
	void updateSensors();
	int agentsInRiverzone();

	static std::vector<Organism*> *organisms;
	static std::ofstream sizeLog;
	
	int id;
	bool hasMoved;
	bool active;
	
	unsigned int creationIteration;
	unsigned int deathIteration;
};

#endif	/* ORGANISM_H */

