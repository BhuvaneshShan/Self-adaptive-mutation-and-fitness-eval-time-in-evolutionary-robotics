#include "Config/AHHSScalePowerConfigurationLoader.h"

#include "AHHSScalePower/include/AHHSScalePowerWorldObserver.h"
#include "AHHSScalePower/include/AHHSScalePowerAgentWorldModel.h"
#include "AHHSScalePower/include/AHHSScalePowerAgentObserver.h"
#include "AHHSScalePower/include/AHHSScalePowerControlArchitecture.h"
#include "AHHSScalePower/include/AHHSScalePowerSharedData.h"

#include "World/RobotAgentWorldInterface.h"

AHHSScalePowerConfigurationLoader::AHHSScalePowerConfigurationLoader()
{
	// create the single instance of Agent-World Interface.
	_AgentWorldInterface = new RobotAgentWorldInterface();
	readProperties(&gProperties);
}

AHHSScalePowerConfigurationLoader::~AHHSScalePowerConfigurationLoader()
{
	delete _AgentWorldInterface;
}


WorldObserver* AHHSScalePowerConfigurationLoader::make_WorldObserver(World* wm)
{
	return new AHHSScalePowerWorldObserver(wm);
}

RobotAgentWorldModel* AHHSScalePowerConfigurationLoader::make_RobotAgentWorldModel()
{
	return new AHHSScalePowerAgentWorldModel();
}

AgentObserver* AHHSScalePowerConfigurationLoader::make_AgentObserver(RobotAgentWorldModel* wm)
{
	return new AHHSScalePowerAgentObserver(wm);
}

BehaviorControlArchitecture* AHHSScalePowerConfigurationLoader::make_BehaviorControlArchitecture(RobotAgentWorldModel* wm)
{
	return new AHHSScalePowerControlArchitecture(wm);
}

RobotAgentWorldInterface* AHHSScalePowerConfigurationLoader::make_RobotAgentWorldInterface()
{
	return _AgentWorldInterface;
}

bool AHHSScalePowerConfigurationLoader::readProperties(ExtendedProperties *gProperties)
{
	bool returnValue = true;
	
	std::cout << "Reading properties..." << std::endl;
		
	if (gProperties->hasProperty("evolutionStepsPerCandidate")) {
		convertFromString<int>(EvolutionaryControlArchitecture::stepsPerCandidate, gProperties->getProperty("evolutionStepsPerCandidate"), std::dec);
	} else {
		std::cerr << "[MISSING] evolutionStepsPerCandidate value is missing." << std::endl;
		returnValue = false;
	}
	
	string fitnessCalculation;
	gProperties->checkAndGetPropertyValue("evolutionFitnessCalculation", &fitnessCalculation, true);
	if(fitnessCalculation == "average"){
		std::cout << "fitness calculation: " << fitnessCalculation << std::endl;
		EvolutionaryControlArchitecture::fitnessCalculation = EvolutionaryControlArchitecture::AVERAGE;
	}else if(fitnessCalculation == "cumulative"){
		std::cout << "fitness calculation: " << fitnessCalculation << std::endl;
		EvolutionaryControlArchitecture::fitnessCalculation = EvolutionaryControlArchitecture::CUMULATIVE;
	}
	
	
	if (gProperties->hasProperty("controllerInitialMutationStepSize")) {
		convertFromString<double>(Controller::initialMutationStepSize, gProperties->getProperty("controllerInitialMutationStepSize"), std::dec);
	} else {
		std::cerr << "[MISSING] controllerInitialMutationStepSize value is missing." << std::endl;
		returnValue = false;
	}
	
	if (gProperties->hasProperty("controllerMutationStepSizeMinimum")) {
		convertFromString<double>(Controller::controllerMutationStepSizeMinimum, gProperties->getProperty("controllerMutationStepSizeMinimum"), std::dec);
	} else {
		std::cerr << "[MISSING] controllerMutationStepSizeMinimum value is missing." << std::endl;
		returnValue = false;
	}

	if (gProperties->hasProperty("controllerStartHormones")) {
		convertFromString<int>(AHHSScalePowerSharedData::controllerStartHormones, gProperties->getProperty("controllerStartHormones"), std::dec);
	} else {
		std::cerr << "[MISSING] controllerStartHormones value is missing." << std::endl;
		returnValue = false;
	}

	if (gProperties->hasProperty("controllerStartRules")) {
		convertFromString<int>(AHHSScalePowerSharedData::controllerStartRules, gProperties->getProperty("controllerStartRules"), std::dec);
	} else {
		std::cerr << "[MISSING] controllerStartRules value is missing." << std::endl;
		returnValue = false;
	}

	if (gProperties->hasProperty("controllerStartCompartments")) {
		convertFromString<int>(AHHSScalePowerSharedData::controllerStartCompartments, gProperties->getProperty("controllerStartCompartments"), std::dec);
	} else {
		std::cerr << "[MISSING] controllerStartCompartments value is missing." << std::endl;
		returnValue = false;
	}

	if (gProperties->hasProperty("populationMu")) {
		convertFromString<unsigned int>(PopulationControlArchitecture::mu, gProperties->getProperty("populationMu"), std::dec);
	} else {
		std::cerr << "[MISSING] populationMu value is missing." << std::endl;
		returnValue = false;
	}
	
	if (gProperties->hasProperty("populationReEvaluationRate")) {
		convertFromString<double>(PopulationControlArchitecture::reEvaluationRate, gProperties->getProperty("populationReEvaluationRate"), std::dec);
	} else {
		std::cerr << "[MISSING] populationReEvaluationRate value is missing." << std::endl;
		returnValue = false;
	}
	
	if (gProperties->hasProperty("populationCrossoverRate")) {
		convertFromString<double>(PopulationControlArchitecture::crossoverRate, gProperties->getProperty("populationCrossoverRate"), std::dec);
	} else {
		std::cerr << "[MISSING] populationCrossoverRate value is missing." << std::endl;
		returnValue = false;
	}
	
	if (gProperties->hasProperty("populationMutationRate")) {
		convertFromString<double>(PopulationControlArchitecture::mutationRate, gProperties->getProperty("populationMutationRate"), std::dec);
	} else {
		std::cerr << "[MISSING] populationMutationRate value is missing." << std::endl;
		returnValue = false;
	}
	
	if (gProperties->hasProperty("populationNumberOfParents")) {
		convertFromString<int>(PopulationControlArchitecture::numberOfParents, gProperties->getProperty("populationNumberOfParents"), std::dec);
	} else {
		std::cerr << "[MISSING] populationNumberOfParents value is missing." << std::endl;
		returnValue = false;
	}

	gProperties->checkAndGetPropertyValue("gScaling", &AHHSScalePowerSharedData::SCALING, true);
	gProperties->checkAndGetPropertyValue("gMaxEnergy", &AHHSScalePowerSharedData::MAX_ENERGY, true);
	gProperties->checkAndGetPropertyValue("gStartEnergy", &AHHSScalePowerSharedData::START_ENERGY, true);

	gProperties->checkAndGetPropertyValue("gSunLifetime", &AHHSScalePowerSharedData::gSunLifetime, true);
	
	gProperties->checkAndGetPropertyValue("gGenomesLogFilename", &AHHSScalePowerSharedData::gGenomesLogFilename, true);
	gProperties->checkAndGetPropertyValue("gOutputLogFilename", &AHHSScalePowerSharedData::gOutputLogFilename, true);
	
	cout << "Logging genomes to: " << AHHSScalePowerSharedData::gGenomesLogFilename << std::endl;
	cout << "Logging output to: " << AHHSScalePowerSharedData::gOutputLogFilename << std::endl;
	
	cout << "Scaling: " << AHHSScalePowerSharedData::SCALING << endl;
	cout << "Start Energy: " << AHHSScalePowerSharedData::START_ENERGY << endl;
	cout << "Max Energy: " << AHHSScalePowerSharedData::MAX_ENERGY << endl;
	cout << "Sun Lifetime: " << AHHSScalePowerSharedData::gSunLifetime << endl;
	
	std::cout << "steps per candidate: " << EvolutionaryControlArchitecture::stepsPerCandidate << std::endl;
	std::cout << "initial mutation step size: " << Controller::initialMutationStepSize << std::endl;
	std::cout << "mu: " << PopulationControlArchitecture::mu << std::endl;
	std::cout << "re-evaluation rate: " << PopulationControlArchitecture::reEvaluationRate << std::endl;
	std::cout << "crossover rate: " << PopulationControlArchitecture::crossoverRate << std::endl;
	std::cout << "mutation rate: " << PopulationControlArchitecture::mutationRate << std::endl;
	std::cout << "number of parents: " << PopulationControlArchitecture::numberOfParents << std::endl;
	
	return returnValue;
}
