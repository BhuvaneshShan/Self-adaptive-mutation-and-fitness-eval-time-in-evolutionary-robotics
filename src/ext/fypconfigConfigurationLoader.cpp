#include "Config/fypconfigConfigurationLoader.h"

#include "fyp/include/fypWorldObserver.h"
#include "fyp/include/fypAgentWorldModel.h"
#include "fyp/include/fypAgentObserver.h"
#include "fyp/include/fypControlArchitecture.h"
#include "fyp/include/fypSharedData.h"
#include "fyp/include/fypController.h"
#include "fyp/include/fypSharedData.h"
#include "fyp/include/Subscenario.h"

#include "World/RobotAgentWorldInterface.h"

fypConfigurationLoader::fypConfigurationLoader()
{
	// create the single instance of Agent-World Interface.
	_AgentWorldInterface = new RobotAgentWorldInterface();
	readProperties(&gProperties);
}

fypConfigurationLoader::~fypConfigurationLoader()
{
	//nothing to do
	delete _AgentWorldInterface;
}


WorldObserver* fypConfigurationLoader::make_WorldObserver(World* wm)
{
	return new fypWorldObserver(wm);
}

RobotAgentWorldModel* fypConfigurationLoader::make_RobotAgentWorldModel()
{
	return new fypAgentWorldModel();
}

AgentObserver* fypConfigurationLoader::make_AgentObserver(RobotAgentWorldModel* wm)
{
	return new fypAgentObserver(wm);
}

BehaviorControlArchitecture* fypConfigurationLoader::make_BehaviorControlArchitecture(RobotAgentWorldModel* wm)
{
	return new fypControlArchitecture(wm);
}

RobotAgentWorldInterface* fypConfigurationLoader::make_RobotAgentWorldInterface()
{
	return _AgentWorldInterface;
}

bool fypConfigurationLoader::readProperties(ExtendedProperties *gProperties)
{
	bool returnValue = true;
	
	gProperties->checkAndGetPropertyValue("evolutionStepsPerCandidate", &EvolutionaryControlArchitecture::stepsPerCandidate, true);
	
	string fitnessCalculation;
	gProperties->checkAndGetPropertyValue("evolutionFitnessCalculation", &fitnessCalculation, true);
	if (fitnessCalculation == "average") {
		EvolutionaryControlArchitecture::fitnessCalculation = EvolutionaryControlArchitecture::AVERAGE;
	} else if (fitnessCalculation == "cumulative") {
		EvolutionaryControlArchitecture::fitnessCalculation = EvolutionaryControlArchitecture::CUMULATIVE;
	} else {
		std::cerr << "[CORRUPT] evolutionFitnessCalculation method '" << fitnessCalculation << "' is invalid." << std::endl;
		returnValue = false;
	}

	gProperties->checkAndGetPropertyValue("controllerInitialMutationStepSize", &Controller::initialMutationStepSize, true);
	gProperties->checkAndGetPropertyValue("populationMu", &PopulationControlArchitecture::mu, true);
	gProperties->checkAndGetPropertyValue("populationReEvaluationRate", &PopulationControlArchitecture::reEvaluationRate, true);
	gProperties->checkAndGetPropertyValue("populationCrossoverRate", &PopulationControlArchitecture::crossoverRate, true);
	gProperties->checkAndGetPropertyValue("populationMutationRate", &PopulationControlArchitecture::mutationRate, true);
	gProperties->checkAndGetPropertyValue("newscastCacheSize", &fypControlArchitecture::cacheSize, true);
	gProperties->checkAndGetPropertyValue("newscastItemTTL", &fypControlArchitecture::itemTTL, true);
	
	gProperties->checkAndGetPropertyValue("evolveEvaluationTime", &fypControlArchitecture::evolveEvaluationTime, true);
       
        gProperties->checkAndGetPropertyValue("useProposedEvalTimeMethod", &fypSharedData::useProposedEvalTimeMethod, true);
        gProperties->checkAndGetPropertyValue("useDerandomizedMuStepSizeApproach",&fypSharedData::useDerandomisedMuStepSizeApproach, true);
        
        gProperties->checkAndGetPropertyValue("minEvaluationTime", &fypControlArchitecture::minEvaluationTime, true);
	gProperties->checkAndGetPropertyValue("maxEvaluationTime", &fypControlArchitecture::maxEvaluationTime, true);

	gProperties->checkAndGetPropertyValue("gMaxEnergy", &fypSharedData::MAX_ENERGY, true);
	gProperties->checkAndGetPropertyValue("gStartEnergy", &fypSharedData::START_ENERGY, true);
    
    gProperties->checkAndGetPropertyValue("gEnableEnergySmellSensor", &fypController::USE_SMELL_SENSOR, true);
    gProperties->checkAndGetPropertyValue("gEnergySmellRange", &fypController::ENERGY_SMELL_RANGE, true);
    gProperties->checkAndGetPropertyValue("gEnableLocalEnergySensors", &fypController::USE_LOCAL_ENERGY_SENSORS, true);
    
    gProperties->checkAndGetPropertyValue("gAgentNNHiddenLayerSize", &fypController::HIDDEN_LAYER_SIZE, true);
    
	gProperties->checkAndGetPropertyValue("gFitnessLogFilename", &fypSharedData::gFitnessLogFilename, true);
        gProperties->checkAndGetPropertyValue("gNormalizedFitnessLogFilename", &fypSharedData::gNFitnesslogFilename, true);
	returnValue &= this->readSubscenarios(gProperties);

	return returnValue;
}

bool fypConfigurationLoader::readSubscenarios(ExtendedProperties *gProperties)
{
    char buf[1024];
    bool returnValue = true;

    if (gProperties->hasProperty("gNumSubscenarios")) {
        int numSubscenarios = 0;
        gProperties->checkAndGetPropertyValue("gNumSubscenarios", &numSubscenarios, true);

        for (int i = 1; i <= numSubscenarios; i++) {
            Subscenario ss;

            buf[sprintf(buf, "gSubscenario%dMaxEnergyPoints", i)] = 0;
            gProperties->checkAndGetPropertyValue(buf, &ss.maxEnergyPoints, true);
            buf[sprintf(buf, "gSubscenario%dRuntime", i)] = 0;
            gProperties->checkAndGetPropertyValue(buf, &ss.runtime, true);
            buf[sprintf(buf, "gSubscenario%dEnergyDensityImageFilename", i)] = 0;
            std::string filename = gProperties->getProperty(buf);
            ss.energyDistribution = new ImageBasedEnergyDistribution(filename);

            fypSharedData::subscenarios.push_back(ss);
        }
    }

    if (gProperties->hasProperty("gProgramme")) {
        std::string prog = gProperties->getProperty("gProgramme");

        istringstream iss(prog);
        unsigned int ssNo;
        while (iss >> ssNo) {
            if ((ssNo < 1) || (ssNo > fypSharedData::subscenarios.size())) {
                std::cerr << "[CORRUPT] Invalid subscenario number '" << ssNo
                        << "' in programme (expecting 1 to "
                        << fypSharedData::subscenarios.size()  << ")" << std::endl;
                return false;
            }

            fypSharedData::gProgramme.push_back(ssNo);
        }
    }

    return returnValue;
}
