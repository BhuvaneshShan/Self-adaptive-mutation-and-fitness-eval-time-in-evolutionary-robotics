#include "Config/ConfigurationLoader.h"
#include <string.h>

//-------------------------
//Include under your loaders
#include "Config/SelfRepConfigurationLoader.h" 
#include "Config/StrongCausalityConfigurationLoader.h" 
#include "Config/OnePlusOneOnlineConfigurationLoader.h" 
#include "Config/BasicProjectConfigurationLoader.h" 
#include "Config/MedeaConfigurationLoader.h" 
#include "Config/BattleConfigurationLoader.h"
#include "Config/ReplayConfigurationLoader.h"
#include "Config/MedeaTwoSunsConfigurationLoader.h"
#include "Config/MedeaSpecializeConfigurationLoader.h"
#include "Config/ScalePowerConfigurationLoader.h"
#include "Config/ScalePowerSimpleConfigurationLoader.h"
#include "Config/RiverbedConfigurationLoader.h"
#include "Config/MPOConfigurationLoader.h"
#include "Config/HiveMindConfigurationLoader.h"
#include "Config/NewscastConfigurationLoader.h"
#include "Config/AHHSScalePowerConfigurationLoader.h"
#include "Config/ParcoursConfigurationLoader.h"
#include "Config/DynamicConfigurationLoader.h"
//#include "Config/VariableTConfigurationLoader.h"
#include "Config/fypconfigConfigurationLoader.h"

ConfigurationLoader::ConfigurationLoader()	
{
	//nothing to do
}

ConfigurationLoader::~ConfigurationLoader()
{
	//nothing to do
}

ConfigurationLoader* ConfigurationLoader::make_ConfigurationLoader (std::string configurationLoaderObjectName)
{
	if (configurationLoaderObjectName == "SelfRepConfigurationLoader" )
	{
		return new SelfRepConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "StrongCausalityConfigurationLoader" )
	{
		return new StrongCausalityConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "OnePlusOneOnlineConfigurationLoader" )
	{
		return new OnePlusOneOnlineConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "MedeaConfigurationLoader" ) // PPSN
	{
		return new MedeaConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "BattleConfigurationLoader" )
	{
		return new BattleConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "ReplayConfigurationLoader" )
	{
		return new ReplayConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "BasicProjectConfigurationLoader" )
	{
		return new BasicProjectConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "MedeaTwoSunsConfigurationLoader" ) // MCMDS
	{
		return new MedeaTwoSunsConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "MedeaSpecializeConfigurationLoader" )
	{
		return new MedeaSpecializeConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "ScalePowerConfigurationLoader")
	{
		return new ScalePowerConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "ScalePowerSimpleConfigurationLoader")
	{
		return new ScalePowerSimpleConfigurationLoader();
	}
    else if (configurationLoaderObjectName == "RiverbedConfigurationLoader")
	{
		return new RiverbedConfigurationLoader();
	}
    else if (configurationLoaderObjectName == "MPOConfigurationLoader")
	{
		return new MPOConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "HiveMindConfigurationLoader") {
		return new HiveMindConfigurationLoader();
	}
	else if (configurationLoaderObjectName == "NewscastConfigurationLoader") {
		return new NewscastConfigurationLoader();
	}
	else if(configurationLoaderObjectName == "AHHSScalePowerConfigurationLoader")
	{
		return new AHHSScalePowerConfigurationLoader();
	}
	else if(configurationLoaderObjectName == "ParcoursConfigurationLoader")
	{
		return new ParcoursConfigurationLoader();
	}
	else if(configurationLoaderObjectName == "DynamicConfigurationLoader")
	{
		return new DynamicConfigurationLoader();
	}
	/*
    else if(configurationLoaderObjectName == "VariableTConfigurationLoader")
	{
		return new VariableTConfigurationLoader();
	}*/
	else if(configurationLoaderObjectName == "fypconfigConfigurationLoader")
	{
		return new fypConfigurationLoader();
	}
	else
	{
		return NULL;
	}
}
