#ifndef FYPCONFIGURATIONLOADER_H
#define	FYPCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"
#include "PropertiesReadingConfigurationLoader.h"

class fypConfigurationLoader : public PropertiesReadingConfigurationLoader {
public:
    fypConfigurationLoader();
    ~fypConfigurationLoader();

    WorldObserver *make_WorldObserver(World* wm);
    RobotAgentWorldModel *make_RobotAgentWorldModel();
    AgentObserver *make_AgentObserver(RobotAgentWorldModel* wm);
    BehaviorControlArchitecture *make_BehaviorControlArchitecture(RobotAgentWorldModel* wm);
    RobotAgentWorldInterface *make_RobotAgentWorldInterface(); // interface created once in constructor, returns a pointer

    bool readProperties(ExtendedProperties *gProperties);
protected:
    bool readSubscenarios(ExtendedProperties *gProperties);
};

#endif
