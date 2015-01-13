/*
 *  World.h
 *  roborobo
 *
 *  Created by Nicolas on 16/01/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef WORLD_H
#define WORLD_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Agents/RobotAgent.h"

#include "Utilities/Misc.h"

#include "Observers/WorldObserver.h"

class RobotAgent;

class World
{
	protected:

		std::vector<RobotAgentPtr> agents;//[gNbOfAgents];
		//std::vector<bool> agentRegister;//[gNbOfAgents]; // track agents which are registered (ie. drawn) in the foreground image (for agent-agent collision)

		//RobotAgent *agents[gNbOfAgents];
		//bool agentRegister[gNbOfAgents]; // track agents which are registered (ie. drawn) in the foreground image (for agent-agent collision)
		
		int _iterations;

		//True if there is a variation in the number of agent
		bool _agentsVariation;
		
		WorldObserver *_worldObserver;
		
	public:		

		World();
		~World();

		bool loadFiles();
		//bool loadProperties( std::string __propertiesFilename );

		void initWorld();
		void resetWorld();
		void resetEnergy();
		void updateWorld(Uint8 *__keyboardStates = NULL);
		
		RobotAgentPtr getAgent( int __agentIndex );
		//bool isAgentRegistered( int __agentIndex );

		//delete an agent from the simulator. No other functions to call
		void deleteAgent (int __agentIndex );
		//add an agent in the simulator. No other functions to call
		void addAgent(RobotAgent *agentToAdd);
		
		int getIterations();
		WorldObserver* getWorldObserver();
		int getNbOfAgent();
};



#endif // WORLD_H

