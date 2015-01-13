/*
 * ScalePowerConfigurationLoader.h
 *
 */

#ifndef AHHSSCALEPOWERCONFIGURATIONLOADER_H
#define	AHHSSCALEPOWERCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"
#include "PropertiesReadingConfigurationLoader.h"

class AHHSScalePowerConfigurationLoader : public PropertiesReadingConfigurationLoader {
public:
	AHHSScalePowerConfigurationLoader();
    virtual ~AHHSScalePowerConfigurationLoader();

    WorldObserver *make_WorldObserver(World* wm);
    RobotAgentWorldModel *make_RobotAgentWorldModel();
    AgentObserver *make_AgentObserver(RobotAgentWorldModel* wm);
    BehaviorControlArchitecture *make_BehaviorControlArchitecture(RobotAgentWorldModel* wm);
    RobotAgentWorldInterface *make_RobotAgentWorldInterface(); // interface created once in constructor, returns a pointer

    bool readProperties(ExtendedProperties *gProperties);
};

#endif
