/*
 *  World.cpp
 *  roborobo
 *
 *  Created by Nicolas on 16/01/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "World/World.h"

#include "zsu/Properties.h"
#include "Organism.h"

#include "World/ImageBasedEnergyDistribution.h"
#include "Config/GlobalConfigurationLoader.h"

World::World() {
	_iterations = 0;
	_agentsVariation = false;
	_worldObserver = gConfigurationLoader->make_WorldObserver(this);
}

World::~World() {
	delete _worldObserver;
}

void World::initWorld() {

	// load environment and agents files
	if (loadFiles() == false) {
		std::cout << "[critical] cannot load image files." << std::endl;
		exit(1);
	}

	// initialize agents
	for (int i = 0; i != gNbOfAgents; i++) {
		agents.push_back(RobotAgentPtr(new RobotAgent(this)));
		//agentRegister.push_back(true);
	}

	// initialize communication network data

	//std::vector<std::vector<int> > gRadioNetworkArray;
	//gRadioNetworkArray = new std::vector( std::vector(gAgentCounter, std::vector<int>(gAgentCounter)) ) ;
	gRadioNetworkArray.resize(gAgentCounter);
	for (int i = 0; i != gAgentCounter; i++)
		(gRadioNetworkArray.at(i)).reserve(gAgentCounter);

	if (gEnergyMode) {
	    this->resetEnergy();
	}

	if (gUseOrganisms) {
		Organism::reset();
	}

	_worldObserver->reset();
}

void World::resetEnergy() {
    for (int i = 0; i < (int)gEnergyPoints.size(); i++) {
        gEnergyPoints[i].setActiveStatus(false);
    }
    gEnergyPoints.clear();

    for (int i = 0; i < gMaxEnergyPoints; i++) {
        if (gEnergyPointRespawnLagMaxValue > 0) {
            EnergyPoint point(i);
            gEnergyPoints.push_back(point);
        } else {
            EnergyPoint point;
            gEnergyPoints.push_back(point);
        }
    }
}

void World::resetWorld() {
	gAgentIndexFocus = 0;
	_iterations = 0;

	for (int i = 0; i != gAgentCounter; i++) {
		agents[i]->unregisterAgent();
		agents[i]->reset();
		//agents[i]->setCoordReal(gAgentXStart+(2*gAgentWidth)*(i%13),gAgentYStart+(2*gAgentWidth)*(i/13)); // check initWorld method
		agents[i]->registerAgent();
		//agentRegister[i] = true;
	}

	// Clean up organism things
	if (gUseOrganisms) {
		Organism::reset();
	}
}

void World::updateWorld(Uint8 *__keyboardStates) {
	// * update observer (e.g. update fitness, possibly update worldmodel, etc.)
	// note 1: if you are not doing evolution or do not need monitoring/logging, this part may be commented to omit one loop. If updateFitness method is empty, the compiler optimization may do it by itself, though.
	// note 2: depending on the evolution scheme, you might need only one of the two levels.
	//		e.g. 
	//			classic Evolutionary Robotics may be implemented only using world genetics level (or a combination of the two)
	//			on-line evolution may be implemented only using agent level genetics.
	// note 3: observer/genetics is done *before* control to enable initialization prior to the first world step update.
	// note 4: while observer was originally meant for world level genetics operator, you may do anything you want here (e.g. monitoring, logging...)
	// note 5: agents are updated *in random order* so as to avoid "effect" order (e.g. low index agents moves first).


	// Radio communication network 
	// (if active) update radio communication network. 
	// note : complexity is N(N-1)/2 (with N, the nb of agents). Meaning that this *DOES* actually impact speed.

	if (gRadioNetwork) {
		if (_agentsVariation == true) {
			gRadioNetworkArray.resize(gAgentCounter);
			for (int i = 0; i < gAgentCounter; i++)
				(gRadioNetworkArray.at(i)).reserve(gAgentCounter);
			_agentsVariation = false;
		}

		for (int i = 0; i < gAgentCounter - 1; i++)
			for (int j = i; j < gAgentCounter; j++) {
				int x1, y1, x2, y2;
				agents[i]->getCoord(x1, y1);
				agents[j]->getCoord(x2, y2);
				int distance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2); // d^2
				if (distance <= gMaxRadioDistanceToSquare)
					gRadioNetworkArray[i][j] = gRadioNetworkArray[j][i] = 1; // current implementation: either signal or no signal.
				else
					gRadioNetworkArray[i][j] = gRadioNetworkArray[j][i] = 0;
			}
	}

	if (gUseOrganisms) {
		Organism::checkAllIntegrity();
	}

	// update world level observer
	_worldObserver->step();

	// update energy points status, if needed
	if (gEnergyMode == true) {
		for (std::vector<EnergyPoint>::iterator it = gEnergyPoints.begin(); it < gEnergyPoints.end(); it++)
			it->step();
	}

	// * create an array that contains shuffled indexes. Used afterwards for randomly update agents.
	//    This is very important to avoid possible nasty effect from ordering such as "agents with low indexes moves first"
	//    outcome: among many iterations, the effect of ordering is reduced.
	//    As a results, roborobo is turn-based, with sotchastic updates within one turn

	int shuffledIndex[gAgentCounter];

	for (int i = 0; i < gAgentCounter; i++)
		shuffledIndex[i] = i;

	for (int i = 0; i < gAgentCounter - 1; i++) // exchange randomly indexes with one other
	{
		int r = i + (rand() % (gAgentCounter - i)); // Random remaining position.
		int tmp = shuffledIndex[i];
		shuffledIndex[i] = shuffledIndex[r];
		shuffledIndex[r] = tmp;
	}

	// update agent level observers
	for (int i = 0; i != gAgentCounter; i++) {
		// set up connections between agents if they are
		// close enough
		if (gUseOrganisms) {
			agents[i]->setUpConnections();
		}
		agents[i]->callObserver();
	}

	// * update world and agents 

	_iterations++;

	// controller step()
	for (int i = 0; i < gAgentCounter; i++) {
		if (__keyboardStates == NULL)
			agents[shuffledIndex[i]]->stepBehavior();
		else {
			if (shuffledIndex[i] == gAgentIndexFocus)
				agents[shuffledIndex[i]]->stepBehavior(__keyboardStates);
			else
				agents[shuffledIndex[i]]->stepBehavior();
		}
	}

	// * prepare world dynamics (if any)
	gConfigurationLoader->make_RobotAgentWorldInterface()->prepareInternalDynamics();

	if (gUseOrganisms) {
		Organism::prepareAllMove();
	}

	// * move the dot -- apply (limited) physics
	for (int i = 0; i < gAgentCounter; i++) {
		// move agent
		if (gUseOrganisms && agents[shuffledIndex[i]]->isPartOfOrganism()) {
			// Agents in an organism all move together
			Organism* organism = agents[shuffledIndex[i]]->getOrganism();
			organism->move();
			organism->registerOrganism();
		} else {
			// unregister itself (otw: own sensor may see oneself)
			if (agents[shuffledIndex[i]]->isRegistered()) {
				agents[shuffledIndex[i]]->unregisterAgent();
			}

			agents[shuffledIndex[i]]->move();
			agents[shuffledIndex[i]]->registerAgent();
		}
	}

	// This loop seems to be redundant
//	for (int i = 0; i < gAgentCounter; i++) {
//		if (!agents[i]->isRegistered()) {
//			agents[i]->registerAgent();
//		}
//	}
}

bool World::loadFiles() {
	bool returnValue = true;

	//Load the dot image
	gAgentMaskImage = load_image(gAgentMaskImageFilename);

	//Load the agent specifications image
	gAgentSpecsImage = load_image(gAgentSpecsImageFilename); // no jpg (color loss)

	if (gUseOrganisms) {
		gAgentPositiveMaskImage = load_image(gAgentPositiveMaskImageFilename);
		gAgentNegativeMaskImage = load_image(gAgentNegativeMaskImageFilename);
	}

	//Load the foreground image (active borders)
	gForegroundImage = load_image(gForegroundImageFilename); // RECOMMENDED: png rather than jpeg (pb with transparency otw)
	if (gForegroundImageFilename.compare(gForegroundImageFilename.length() - 3, 3, "jpg", 0, 3) == 0) {
		std::cerr << "foreground: PNG format is *mandatory* (JPG may feature transparency problems due to compression with loss)\n";
		returnValue = false;
	}

	gEnvironmentImage = load_image(gEnvironmentImageFilename);

	// load background image (parallax)
	gBackgroundImage = load_image(gBackgroundImageFilename);

	//Load the story zones
	gZoneImage = load_image(gZoneImageFilename);

	//Load stories and initialize "key" stories flag. 
	for (int i = 0; i != 256; i++) {
		std::ostringstream o;
		if (i < 10)
			o << gZoneCaptionPrefixFilename << "00" << i << ".png";
		else if (i < 100)
			o << gZoneCaptionPrefixFilename << "0" << i << ".png";
		else
			o << gZoneCaptionPrefixFilename << i << ".png";
		gZoneCaptionImage[i] = load_image(o.str());

		if (gZoneCaptionImage[i] == NULL) {
			o.str("");
			if (i < 10)
				o << gZoneCaptionPrefixFilename << "00" << i << ".jpg";
			else if (i < 100)
				o << gZoneCaptionPrefixFilename << "0" << i << ".jpg";
			else
				o << gZoneCaptionPrefixFilename << i << ".jpg";
			gZoneCaptionImage[i] = load_image(o.str()); // set to NULL if file does not exist
		}

		// set story flags
		if (gZoneCaptionImage[i] == NULL)
			gZoneStatus[i] = false;
		else {
			if (i < 100)
				gZoneStatus[i] = true;
			else {
				if (i % 10 == 0)
					gZoneStatus[i] = true; // key story will be displayed 
				else
					gZoneStatus[i] = false; // sub-stories will not be displayed for the moment
			}
		}
	}

	//If there was a problem in loading the agent mask and specs
	if (gAgentMaskImage == NULL) {
		std::cerr << "Could not load agent mask image\n";
		returnValue = false;
	}

	if (gUseOrganisms && gAgentPositiveMaskImage == NULL) {
		std::cerr << "Could not load agent positive mask image\n";
		returnValue = false;
	}

	if (gUseOrganisms && gAgentNegativeMaskImage == NULL) {
		std::cerr << "Could not load agent positive mask image\n";
		returnValue = false;
	}

	if (gAgentSpecsImage == NULL) {
		std::cerr << "Could not load agent specification image\n";
		returnValue = false;
	}

	//If there was a problem in loading the foreground image
	if (gForegroundImage == NULL) {
		std::cerr << "Could not load foreground image\n";
		returnValue = false;
	}

	if (gEnvironmentImage == NULL) {
		std::cerr << "Could not load environment image\n";
		returnValue = false;
	}

	//no background image (not a critical error)
	if (gBackgroundImage == NULL) {
		std::cout << "warning: could not load background image (will proceed anyway)\n";
	}

	// mandatory: image dimensions must be more than 1024x768 (otw: screen underfitting)
	if (gForegroundImage->w < gScreenWidth || gForegroundImage->h < gScreenHeight) {
		std::cerr << "foreground image dimensions must be " << gScreenWidth << "x" << gScreenHeight << " or higher (given: " << gForegroundImage->w << "x"
				<< gForegroundImage->h << ") \n";
		returnValue = false;
	}

	//If there was a problem in loading the storyzones image
	if (gZoneImage == NULL) {
		std::cerr << "Could not load storyzones image\n";
		returnValue = false;
	}

	if ((gZoneImage->w != gForegroundImage->w) || (gZoneImage->h != gForegroundImage->h)) {
		std::cerr << "Storyzones image dimensions do not match that of the foreground image\n";
		returnValue = false;
	}

	// set reference dimensions
	gAgentWidth = gAgentMaskImage->w;
	gAgentHeight = gAgentMaskImage->h;

	if (gMaxTranslationalSpeed > gAgentWidth || gMaxTranslationalSpeed > gAgentHeight) {
		std::cerr
				<< "[WARNING] gMaxTranslationalSpeed value *should not* be superior to agent dimensions (image width and/or height) -- may impact collision accuracy (e.g. teleporting through walls)\n";
		//		returnValue = false;
	}

	gAreaWidth = gForegroundImage->w;
	gAreaHeight = gForegroundImage->h;

	// set transparency color
	SDL_SetColorKey(gAgentMaskImage, SDL_SRCCOLORKEY, SDL_MapRGB(gAgentMaskImage->format, 0xFF, 0xFF, 0xFF));

	SDL_SetColorKey(gForegroundImage, SDL_SRCCOLORKEY, SDL_MapRGB(gForegroundImage->format, 0xFF, 0xFF, 0xFF));
	SDL_SetColorKey(gEnvironmentImage, SDL_SRCCOLORKEY, SDL_MapRGB(gEnvironmentImage->format, 0xFF, 0xFF, 0xFF));
	
	SDL_SetColorKey(gAgentSpecsImage, SDL_SRCCOLORKEY, SDL_MapRGB(gAgentSpecsImage->format, 0xFF, 0xFF, 0xFF));

	if (gUseOrganisms) {
		SDL_SetColorKey(gAgentPositiveMaskImage, SDL_SRCCOLORKEY, SDL_MapRGB(gAgentPositiveMaskImage->format, 0xFF, 0xFF, 0xFF));
		SDL_SetColorKey(gAgentNegativeMaskImage, SDL_SRCCOLORKEY, SDL_MapRGB(gAgentNegativeMaskImage->format, 0xFF, 0xFF, 0xFF));
	}

	// preparing Environment Image (ie. only the BLUE component is used)
	for (int x = 0; x != gEnvironmentImage->w; x++)
		for (int y = 0; y != gEnvironmentImage->h; y++) {
			Uint32 pixel = getPixel32(gEnvironmentImage, x, y);
			if (pixel != SDL_MapRGB(gEnvironmentImage->format, 0xFF, 0xFF, 0xFF))
				putPixel32(gEnvironmentImage, x, y, SDL_MapRGB(gEnvironmentImage->format, 0, 0, pixel & 0x0000FF));
		}
    
    if(gEnergyMode) {
        // Prepare a surface to hold the energy spots so sensors can detect them
        gEnergyImage = SDL_CreateRGBSurface (gScreen->flags, gAreaWidth, gAreaHeight, gScreen->format->BitsPerPixel, gScreen->format->Rmask, gScreen->format->Gmask, gScreen->format->Bmask, gScreen->format->Amask);
        SDL_SetColorKey(gEnergyImage, SDL_SRCCOLORKEY, SDL_MapRGB(gEnergyImage->format, 0xFF, 0xFF, 0xFF));
        SDL_FillRect(gEnergyImage, NULL, SDL_MapRGB(gEnergyImage->format,0xFF, 0xFF, 0xFF));

        // load energy distribution if an image is specified
        if (gEnergyDensityImageFilename.length() > 0) {
            ImageBasedEnergyDistribution *ibed = new ImageBasedEnergyDistribution(gEnergyDensityImageFilename);
            ibed->load();
            gEnergyDistribution = ibed;
        }
    }


	//If everything loaded fine
	if (returnValue == false)
		return false;
	else
		return true;
}

RobotAgentPtr World::getAgent(int __agentIndex) {
	return agents[__agentIndex];
}

/*bool World::isAgentRegistered(int __agentIndex) {
 return agentRegister[__agentIndex];
 }*/

void World::deleteAgent(int __agentIndex) {
	agents[__agentIndex]->unregisterAgent();
	agents.erase(agents.begin() + __agentIndex);
	//agentRegister.erase(agentRegister.begin() + __agentIndex);
	gAgentCounter--;
	_agentsVariation = true;
}

void World::addAgent(RobotAgent *agentToAdd) {
	agents.push_back(RobotAgentPtr(agentToAdd));
	//agentRegister.push_back(true);
	_agentsVariation = true;
}

int World::getIterations() {
	return _iterations;
}

WorldObserver* World::getWorldObserver() {
	return _worldObserver;
}

int World::getNbOfAgent() {
	return agents.size();
}

