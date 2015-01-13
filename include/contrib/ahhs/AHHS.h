/*
 * Ahhs2.h
 *
 *  Created on: 23.07.2009
 *      Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 */

#ifndef AHHS_H_
#define AHHS_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "Actuator.h"
#include "AHHSHormone.h"
#include "AHHSRule.h"
#include "AHHSConfiguration.h"
#include "Sensor.h"

using namespace std;

class AHHS 
{
private:
  //[number of compartments* number of hormones]
  vector<double> hormone_concentrations_;
  //[number of compartments* number of hormones]
  vector<double> hormone_concentrations_next_step_;

  vector<AHHSRule*> rules_;          //[Max_Number_of_Rules]
  vector<AHHSHormone*> hormones_;    //[Max_Number_of_Hormones]
  vector<AHHS*> neighbors_;    //[Max_Number_of_Hormones]
  int ahhs_type_;
  bool use_lookup_;
  unsigned int number_lookup_table_bins_;
  bool lookup_table_computed_;
  double lookup_table_uncomputed_flag_;
  vector< vector< vector<double> > > lookup_sensor2hormone_c0_;
  vector< vector< vector<double> > > lookup_sensor2hormone_c1_;
  vector< vector< vector<double> > > lookup_hormone2hormone_;
  vector< vector< vector< vector<double> > > > lookup_hormone2hormone_nonlin_;
  vector< vector< vector<double> > > lookup_hormone2actuator_c0_;
  vector< vector< vector<double> > > lookup_hormone2actuator_c1_;
  double neighbor_value_;
  bool do_diffusion_with_neighbors_;
 
  void executeRules (int compartment_id, int hormone_id);
  void checkRuleAssertions(AHHSRule *my_rule);
  void performDiffusion();
  bool addRule(AHHSRule *new_rule);
  bool addHormone(AHHSHormone *new_hormone);
  void computeLookupTable();

public:
  AHHS();
  AHHS(AHHS* src);
  AHHS(bool initWithRandomHormones, bool initWithRandomRules, int nr_of_hormones, int nr_of_rules, int nr_of_actuators, int nr_of_sensors, double sensor_arr[], int nr_of_compartments, int ahhs_type, bool use_lookup, bool do_diffusion_with_neighbors, bool customConfiguration);
  AHHS(string path);
  virtual ~AHHS();

  AHHSConfiguration* configuration_;

  void initializeAHHS(bool initWithRandomHormones, bool initWithRandomRules, int nr_of_hormones, int nr_of_rules, int nr_of_actuators, int nr_of_sensors, double sensor_arr[], int nr_of_compartments, int ahhs_type, bool use_lookup, bool do_diffusion_with_neighbor, bool customConfiguration);
  AHHSHormone* getHormone(int pos);
  AHHSRule* getRule(int pos); 
  int getNumberOfHormones();
  int getNumberOfRules();
  int getNumberOfHormoneConcentrations();
  double getHormoneConcentration(int compartment_id, int hormone_id);
  double getHormoneConcentrationNextStep(int compartmentID, int hormone_id);
  double getValueFromActuator(int ActuatorID);
  int getAHHSType();

  void setHormonesRandom(int number_of_hormones);
  void setRulesHandCoded(); 
  void setHormonesHandCoded();
  void setRulesRandom(int number_of_rules, int ahhs_type, int nr_of_hormones, int nr_of_sensors, int nr_of_actuators);
  void setValueToSensor(int sensor_id, double value);
  void setHormoneConcentration(int compartment_id, int hormone_id, double value);
  void setAHHSType(int ahhs_type);
  void setUseLookup(bool use_lookup);
  void setNeighborValue(double value);

  int act();
  void logging(string complete_path);
  void readFromFile(string filenamePrefix);
  void reset();
  unsigned int mutate(double MutProbHormoneChange, double MutProbRuleChange, double MutValueChangePercent);
  unsigned int mutateExactOne(double MutProbHormoneChange, double MutProbRuleChange, double MutValueChangePercent);
  int recombinateWith(AHHS *Partner, double RecProbHormones, double RecProbRules);
  void resetHormoneConcentrations();
  void resetHormones();
  void resetRules();
  bool linkNeighbor(AHHS* neighbor); 
  bool unlinkNeighbor(int index_of_neighbor); 
  void unlinkAllNeighbors();
  int getNumberOfNeighbors();
  AHHS* getNeighbor(int pos);
};

#endif /* AHHS_H */
