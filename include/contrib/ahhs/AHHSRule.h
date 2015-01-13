/** RuleClass.h
 * Definitions of the rules.
 *
 *  @date: 23.07.2009
 *  @author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
*/

#ifndef AHHSRULE_H_
#define AHHSRULE_H_


#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include <limits>
#include <vector>

#include "Sensor.h"
#include "Actuator.h"
#include "AHHSHormone.h"

using namespace std;

#define NR_OF_RULE_TYPES 5
#define NR_OF_VARIABLES_IN_RULE 8

enum _RuleType_ {AHHS_NEVER=0, AHHS_SENSOR_SOURCE=1, AHHS_ACTUATOR_TARGET=2, AHHS_HORMONES=3, AHHS_NONLINEAR=4};
typedef enum _RuleType_ RuleType;

class AHHSRule  
{
 private:
  vector<double> weights_;
  double window_center_;
  double window_;
  double fixed_dose_;
  double dependent_dose_;
  double sensor_input_;
  double hormone_input_;
  double hormone_output_;
  double actuator_output_;

  double checkTrigger(double value);
  void setRuleProbRandom(int nr_of_rule_types);
  void mutateWeight(double MutValueChangePercent, int nr_of_rule_types);
  void mutateWindow(double MutValueChangePercent);
  void mutateDependentDose(double MutValueChangePercent);
  void mutateIndex(double *property, int MaxNumber);
  void mutateFixedDose(double MutValueChangePercent);
  void mutateWindowCenter(double MutValueChangePercent);
 
public:
  AHHSRule();
  AHHSRule(AHHSRule *src);
  virtual ~AHHSRule();
  
  double getWeight(int pos);
  double getWindowCenter(); 
  double getWindow(); 
  double getDependentDose(); 
  double getFixedDose(); 
  double getSensorInput();
  double getHormoneInput();
  double getHormoneOutput();
  double getActuatorOutput();
  
  void setWeight(double weight, int pos);
  void setWindowCenter(double window_center);
  void setWindow(double window);
  void setDependentDose(double dependent_dose);
  void setFixedDose(double fixed_dose);
  void setSensorInput(double sensor_input);
  void setHormoneInput(double hormone_input);
  void setHormoneOutput(double hormone_output);
  void setActuatorOutput(double actuator_output);
  
  double executeRuleSensorSource(int compartment_id, int hormone_id, Sensor *my_sensor, int number_of_hormones);
  double executeRuleActuatorTarget(int compartment_id, int hormone_id, Actuator *my_actuator, double hormone_value, int number_of_hormones);
  double executeRuleHormone(int compartment_id, int hormone_id, double source_value, double target_value, AHHSHormone *target_hormone);
  double executeRuleHormoneNonLinear(int compartment_id, int hormone_id, double source_value, double target_value, AHHSHormone *target_hormone);
 
  void handCodedInit(int rule_number); 
  void randomInit(int ahhs_type, int number_of_sensors, int number_of_hormones, int number_of_actuators);
  void logging(ofstream &File);
  void writeRule();
  void readFromFile(std::string line);
  int mutate(double MutProbRuleChange, double MutValueChangePercent, int Number_of_Actuators, int Number_of_Sensors, int Number_of_Hormones, int ahhs_type);
  int mutateExactOne(double MutProbRuleChange, double MutValueChangePercent, int Number_of_Actuators, int Number_of_Sensors, int Number_of_Hormones, int ahhs_type);
};

#endif /* AHHSRULE_H_ */
