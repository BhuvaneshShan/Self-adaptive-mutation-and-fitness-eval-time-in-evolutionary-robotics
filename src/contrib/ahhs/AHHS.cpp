/*
 * AHHSClass.cpp
 *
*  Created on: 23.07.2009
 *      Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 */

#include "ahhs/AHHS.h"

AHHS::AHHS() 
{
  number_lookup_table_bins_ = 128;
  lookup_table_uncomputed_flag_ = 999999999999.0;
  lookup_table_computed_ = false;
  neighbor_value_ = 0.0;
  configuration_ = new AHHSConfiguration();
}

AHHS::AHHS(AHHS* src) 
{ 
  //cout << "AHHS::AHHS(AHHS*) begin" << endl;
  
  ahhs_type_ = src->ahhs_type_;
  use_lookup_=src->use_lookup_;

  number_lookup_table_bins_ = src->number_lookup_table_bins_;
  lookup_table_uncomputed_flag_ = src->lookup_table_uncomputed_flag_ ;
  lookup_table_computed_ = false;
  neighbor_value_ = src->neighbor_value_;
  do_diffusion_with_neighbors_ = src->do_diffusion_with_neighbors_;

  configuration_ = new AHHSConfiguration(src->configuration_);
  
  for(unsigned int i=0; i < src->hormones_.size(); i++) {
    addHormone(new AHHSHormone(src->getHormone(i)));
  }

  for(unsigned int i=0; i < src->rules_.size();i++) {
    addRule(new AHHSRule(src->getRule(i)));
  }

  for(unsigned int i=0; i < src->neighbors_.size();i++) {
    neighbors_[i] = src->neighbors_[i];
  }

  hormone_concentrations_.resize(configuration_->getNumberOfCompartments()*hormones_.size(), 0.0);
  hormone_concentrations_next_step_.resize(configuration_->getNumberOfCompartments()*hormones_.size(), 0.0);

  //cout << "AHHS::AHHS(AHHS*) end" << endl;
}

AHHS::AHHS(bool initWithRandomHormones, bool initWithRandomRules, int nr_of_hormones, int nr_of_rules, int nr_of_actuators, int nr_of_sensors, double sensor_arr[], int nr_of_compartments, int ahhs_type, bool use_lookup, bool do_diffusion_with_neighbors, bool customConfiguration)
{
  number_lookup_table_bins_ = 128;
  lookup_table_uncomputed_flag_ = 999999999999.0;
  lookup_table_computed_ = false;
  neighbor_value_ = 0.0;
  configuration_ = new AHHSConfiguration();
  initializeAHHS(initWithRandomHormones, initWithRandomRules, nr_of_hormones, nr_of_rules, nr_of_actuators, nr_of_sensors, sensor_arr, nr_of_compartments, ahhs_type, use_lookup, do_diffusion_with_neighbors, customConfiguration);

  //cout << "AHHS::AHHS(bool, int, int, int, int bool, bool) end" << endl;
}

AHHS::AHHS(string path) 
{
  number_lookup_table_bins_ = 128;
  lookup_table_uncomputed_flag_ = 999999999999.0;
  lookup_table_computed_ = false; 
  neighbor_value_ = 0.0;

  configuration_ = new AHHSConfiguration();
  readFromFile(path);
  
  hormone_concentrations_.resize(configuration_->getNumberOfCompartments()*hormones_.size(), 0.0);
  hormone_concentrations_next_step_.resize(configuration_->getNumberOfCompartments()*hormones_.size(), 0.0);
  //cout << "AHHS::AHHS(string) end" << endl;
}

AHHS::~AHHS() 
{
  //cout << "AHHS::~AHHS(): I'm dead" << endl; 
  
  if(configuration_ != 0) {
    delete configuration_;
    configuration_ = 0;
  }
  
  if(!hormones_.empty()) {
    for(int i=0; i < (int) hormones_.size();i++) {
      if(hormones_[i]!=0) {
	delete hormones_[i];
	hormones_[i]=0;
      }
    }
    hormones_.clear();
  }
  
  if(!rules_.empty()) {
    for(int i=0; i < (int) rules_.size();i++) {
      if(rules_[i] != 0) {
	delete rules_[i];
	rules_[i] = 0;
      }
    }
    rules_.clear();
  }
  
  if (!hormone_concentrations_.empty()) {
    hormone_concentrations_.clear();
  }
  if (!hormone_concentrations_next_step_.empty()) {
    hormone_concentrations_next_step_.clear();
  }
}

void AHHS::initializeAHHS(bool init_with_random_hormones, bool init_with_random_rules, int nr_of_hormones, int nr_of_rules, int nr_of_actuators, int nr_of_sensors, double sensor_arr[], int nr_of_compartments, int ahhs_type, bool use_lookup, bool do_diffusion_with_neighbor, bool customConfiguration)
{
  //cout << "AHHS::InitializeAHHS(bool, int, int, int, bool, bool) begin. Actuators:" << nr_of_actuators << " sens: " << nr_of_sensors << " comp: " << nr_of_compartments << endl;

  ahhs_type_ = ahhs_type;
  use_lookup_= use_lookup;
  do_diffusion_with_neighbors_ = do_diffusion_with_neighbor;
  
  //cout << "AHHS::InitializeAHHS(bool, int, int, int, bool, bool) 1" << endl;

  //configuration_ = new AHHSConfiguration();
  if(customConfiguration){
	  configuration_->setCustomConfiguration(nr_of_actuators, nr_of_sensors, nr_of_compartments,sensor_arr);
  }else{
	  configuration_->setConfiguration(nr_of_actuators, nr_of_sensors, nr_of_compartments);
  }

  //cout << "AHHS::InitializeAHHS(bool, int, int, int, bool, bool) 2" << endl;

  if(init_with_random_hormones) {
    setHormonesRandom(nr_of_hormones);
  }
  else {
    setHormonesHandCoded();
  }
  //cout << "AHHS::InitializeAHHS(bool, int, int, int, bool, bool) 2.5" << endl;
  if(init_with_random_rules) {
    setRulesRandom(nr_of_rules, ahhs_type, nr_of_hormones, configuration_->getNumberOfSensors(), configuration_->getNumberOfActuators());
  }
  else {
    setRulesHandCoded();
  }

  hormone_concentrations_.resize(configuration_->getNumberOfCompartments()*hormones_.size(), 0.0); 
  hormone_concentrations_next_step_.resize(configuration_->getNumberOfCompartments()*hormones_.size(), 0.0); 

  //cout << "AHHS::InitializeAHHS(bool, int, int, int, bool, bool) done" << endl;
}

AHHSHormone* AHHS::getHormone(int pos)
{
  if (pos >= (int) hormones_.size()) {
    cout << "AHHS::getHormone(). Error: Requested hormone does not exist. Requested hormone: " << pos << ", but there are only " << hormones_.size() << " hormones registered (0-(n-1)). Exit" << endl;
  }
  return hormones_[pos];
}

AHHSRule* AHHS::getRule(int pos)
{
  if (pos >= (int) rules_.size()) {
    cout << "AHHS::getRule(). Error: Requested rule does not exist. Requested rule: " << pos << ", but there are only " << rules_.size() << " rules registered (0-(n-1)). Exit" << endl;
  }
  return rules_[pos];
}

int AHHS::getNumberOfHormones()
{
  return hormones_.size();
}

int AHHS::getNumberOfRules()
{
  return rules_.size();
}

int AHHS::getNumberOfHormoneConcentrations()
{
  return hormone_concentrations_.size();
}

double AHHS::getHormoneConcentration(int compartment_id, int hormone_id) 
{
  if (compartment_id >= configuration_->getNumberOfCompartments()) {
    std::cout << "AHHS::getHormoneConcentration: Error! Compartment ID " << compartment_id
	      << " >=, " << configuration_->getNumberOfCompartments() << "thus, only max number to call comp is  " << configuration_->getNumberOfCompartments() - 1
	      << ". Exit.\n";
    exit(0);
  } 
  else {
    if ((unsigned int)hormone_id >= hormones_.size()) {
      std::cout << "AHHS::getHormoneConcentration: Error! Hormone " << hormone_id << " does not exist, only " << hormones_.size() << " hormones are registered. Exit. \n";
      exit(0);
    } 
    else {
      return hormone_concentrations_[compartment_id*hormones_.size()+hormone_id];
    }
  }
  return 0.0;
}


double AHHS::getHormoneConcentrationNextStep(int compartment_id, int hormone_id) 
{
  if (compartment_id >= configuration_->getNumberOfCompartments()) {
    std::cout << "AHHS::getHormoneConcentrationNextStep: Error! Compartment " << compartment_id
	      << " does not exist, only " << configuration_->getNumberOfCompartments()
	      << " compartments are registered. Exit.\n";
    exit(0);
  } 
  else {
    if ((unsigned int)hormone_id >= hormones_.size()) {
      std::cout << "AHHS::getHormoneConcentrationNextStep: Error! Hormone " << hormone_id << " does not exist, only " << hormones_.size() << " hormones are registered. Exit. \n";
      exit(0);
    } 
   else {
      if (!hormone_concentrations_next_step_.empty()) {
	return hormone_concentrations_next_step_[compartment_id*hormones_.size()+hormone_id];
      }
      else {
	return 111111111.111;
      }
    }
  }
  return 0.0;
}

double AHHS::getValueFromActuator(int actuator_id) 
{
  return configuration_->getActuator(actuator_id)->getActuatorValue();
}

int AHHS::getAHHSType()
{
  return ahhs_type_;
}

void AHHS::setHormonesRandom(int number_of_hormones) 
{
  for (int i = 0; i < number_of_hormones; i++) {
    AHHSHormone *my_hormone = new AHHSHormone();
    my_hormone->randomInit();
    addHormone(my_hormone);
  }
}

void AHHS::setRulesHandCoded() 
{
  for (int i = 0; i < 6; i++) {
    AHHSRule *my_rule = new AHHSRule();
    my_rule->handCodedInit(i);
    addRule(my_rule);
  }
}

void AHHS::setHormonesHandCoded() 
{
  for (int i = 0; i < 2; i++) {
    AHHSHormone *my_hormone = new AHHSHormone();
    my_hormone->handCodedInit(i);
    addHormone(my_hormone);
  }
}

void AHHS::setRulesRandom(int number_of_rules, int ahhs_type, int nr_of_hormones, int nr_of_sensors, int nr_of_actuators) 
{
  //cout << "AHHS::setRulesRandom(int, int, int, int) begin: rules: " << number_of_rules <<  " type: " << ahhs_type << " sens: " << nr_of_sensors << " horm: " << nr_of_hormones << " act: " << nr_of_actuators << endl;
  for (int i = 0; i < number_of_rules; i++) {
    AHHSRule *my_rule = new AHHSRule();
    my_rule->randomInit(ahhs_type, nr_of_sensors, nr_of_hormones, nr_of_actuators);
    addRule(my_rule);
  }
}

void AHHS::setValueToSensor(int sensor_id, double value)
{
  configuration_->getSensor(sensor_id)->setSensorValue(value);
}

void AHHS::setHormoneConcentration(int compartment_id, int hormone_id, double value) 
{
  if (compartment_id >= configuration_->getNumberOfCompartments()) {
    std::cout << "AHHS::SetHormoneConcentration: Error! Compartment " << compartment_id
	      << " does not exist, only " << configuration_->getNumberOfCompartments()
	      << " compartments are registered. Exit.\n";
    exit(0);
  } 
  else {
    if ((unsigned int)hormone_id >= hormones_.size()) {
      std::cout << "AHHS::SetHormoneConcentration: Error! Hormone " << hormone_id << " does not exist, only " << hormones_.size() << " hormones are registered. Exit. \n";
      exit(0);
    } 
    else {
      if (value > hormones_[hormone_id]->getMaxValue()) {
	hormone_concentrations_[compartment_id*hormones_.size()+hormone_id] = hormones_[hormone_id]->getMaxValue();
      }
      else if (value < hormones_[hormone_id]->getMinValue())
	hormone_concentrations_[compartment_id*hormones_.size()+hormone_id] = hormones_[hormone_id]->getMinValue();
      else {
	hormone_concentrations_[compartment_id*hormones_.size()+hormone_id] = value;
      }
    }
  }
}

void AHHS::setAHHSType(int ahhs_type)
{
  ahhs_type_ = ahhs_type;
}

void AHHS::setUseLookup(bool use_lookup)
{
  use_lookup_ = use_lookup;
}

void AHHS::setNeighborValue(double value) {
  neighbor_value_ = value;
}

int AHHS::act() 
{
  if(!lookup_table_computed_ && use_lookup_) {
    computeLookupTable();
  }
  
  for (int i = 0; i < configuration_->getNumberOfActuators();i++) {
      configuration_->getActuator(i)->setActuatorValue(0.0);
  }
    
  hormone_concentrations_next_step_ = hormone_concentrations_;
    
 if (!hormone_concentrations_.empty()) {
   for (unsigned int i = 0; i < hormone_concentrations_.size(); i++) {
     if (hormone_concentrations_next_step_[i] == hormone_concentrations_[i]) {
       // that's ok: do nothing
     }
     else {
       cout << "AHHS::Act() Warning: Values differ after copy process! Wrong STL usage. Exit. at pos i: " << i << hormone_concentrations_next_step_[i] <<  " != " << hormone_concentrations_[i] << endl;
     }
   }
 }
 
 for (int i = 0; i < configuration_->getNumberOfCompartments(); i++) {
     for (unsigned int j = 0; j < hormones_.size(); j++) {
      executeRules(i,j);
    }
  }


 //cout << "AHHS::act(): nr.of.comp: " << configuration_->getNumberOfCompartments() << endl;
 
for (int i = 0; i < configuration_->getNumberOfCompartments();i++) 
  {
    for (unsigned int j=0; j < hormones_.size(); j++) 
    {
      setHormoneConcentration(i,j,hormone_concentrations_next_step_[i*hormones_.size()+j]);
    }
  }

  performDiffusion();

  /*
    for (unsigned int i = 0; i < hormone_concentrations_.size(); i++) {
      if (hormone_concentrations_[i] > hormones_[i]->getMaxValue()) {
	hormone_concentrations_[i] = hormones_[i]->getMaxValue();
      }
      if (hormone_concentrations_[i] < hormones_[i]->getMinValue()) {
	hormone_concentrations_[i] = hormones_[i]->getMaxValue();
      }
  }
  */
	 /*
  for (int actuator = 0; actuator < configuration_->getNumberOfActuators(); actuator++) {
    AHHSActuator *my_actuator = configuration_->getActuator(actuator);
    my_actuator->checkBounds();
  }
	 */
  //cout << "AHHS: hor. con: " << hormone_concentrations_[0] << " " << hormone_concentrations_[1] << " output right: " << configuration_->getActuator(0)->getActuatorValue() << " "  << configuration_->getActuator(1)->getActuatorValue() << " input right: " << configuration_->getSensor(0)->getSensorValue() << " "  << configuration_->getSensor(1)->getSensorValue() << endl;

  for(int i = 0; i < configuration_->getNumberOfActuators(); i++) {
    if (configuration_->getActuator(i)->checkBounds()) {
      //cout << "AHHS::act(). Actuator val out of bounds: " <<  configuration_->getActuator(i)->getActuatorValue() << endl; 
      configuration_->getActuator(i)->setValueBackToBounds();
      //cout << "AHHS::act(). changed to: " <<  configuration_->getActuator(i)->getActuatorValue() << " min: " << configuration_->getActuator(i)->getMinValue() << endl; 
    }
    /*
    if (configuration_->getActuator(i)->getActuatorValue() > configuration_->getActuator(i)->getMaxValue()) {
      
      configuration_->getActuator(i)->setActuatorValue(configuration_->getActuator(i)->getMaxValue());
      cout << " changed to: " <<  configuration_->getActuator(i)->getActuatorValue() << endl; 
    }
    if (configuration_->getActuator(i)->getActuatorValue() < configuration_->getActuator(i)->getMinValue()) {
      configuration_->getActuator(i)->setActuatorValue(configuration_->getActuator(i)->getMinValue());
      }
    */
  }  	
  return 1;
}

void AHHS::executeRules(int compartment_id, int hormone_id) 
{
  double old_value = getHormoneConcentration(compartment_id, hormone_id);
  double diff_value = 0.0;
  
  //cout << "AHHS::executeRules(..): compartment_id: " << compartment_id << endl;
  
  if (hormone_id >= (int) hormones_.size())
  {
    std::cout << "AHHS::Update_Hormone_in_Compartment(). HormoneID: " << hormone_id << endl;
  }
  if (compartment_id >= configuration_->getNumberOfCompartments())
  {
    std::cout << "AHHS::Update_Hormone_in_Compartment(). CompartmentID: " << configuration_->getNumberOfCompartments() << endl;
  }

  //Perform constant emissions
  diff_value = hormones_[hormone_id]->getBaseEmissionRate();

  // Decay
  double decay_rate = hormones_[hormone_id]->getDecayRate();

  diff_value -= old_value * decay_rate;
  
  if (!use_lookup_) {	
   for (unsigned int rule = 0; rule < rules_.size(); rule++) {
     AHHSRule *my_rule = rules_[rule];
     checkRuleAssertions(my_rule);

     Sensor *my_sensor = configuration_->getSensor(my_rule->getSensorInput());
     diff_value += my_rule->executeRuleSensorSource(compartment_id, hormone_id, my_sensor, hormones_.size());
     diff_value += my_rule->executeRuleHormone(compartment_id, hormone_id, getHormoneConcentration(compartment_id, hormone_id), getHormoneConcentration(compartment_id, my_rule->getHormoneOutput()), hormones_[my_rule->getHormoneOutput()]);
     diff_value += my_rule->executeRuleHormoneNonLinear(compartment_id, hormone_id, getHormoneConcentration(compartment_id, hormone_id), getHormoneConcentration(compartment_id, my_rule->getHormoneOutput()), hormones_[my_rule->getHormoneOutput()]);
  	
     Actuator *my_actuator = configuration_->getActuator(my_rule->getActuatorOutput());
     double old_actuator_value = my_actuator->getActuatorValue();
     double diff_actuator_value = my_rule->executeRuleActuatorTarget(compartment_id, hormone_id, my_actuator, getHormoneConcentration(compartment_id, hormone_id), hormones_.size());
     //cout << "******* ActNr." << configuration_->getActuator(my_rule->getActuatorOutput()) << " " << old_actuator_value << " " << diff_actuator_value << endl;
     my_actuator->setActuatorValue(old_actuator_value + diff_actuator_value);
     //if (rule == rules_.size()-1)
     //  cout << endl;
   }
  } 
  else {
    if(compartment_id == 0) {
      for(int sensor = 0; sensor < configuration_->getNumberOfSensors(); sensor++) {
        Sensor *my_sensor = configuration_->getSensor(sensor);
        diff_value += lookup_sensor2hormone_c0_[sensor][hormone_id][(float)(my_sensor->getSensorValue())/(float)(my_sensor->getMaxValue())*(float)number_lookup_table_bins_];
      }
    } 
    else 
      {
      for(int sensor = 0; sensor < configuration_->getNumberOfSensors(); sensor++) 
      {
        Sensor *my_sensor = configuration_->getSensor(sensor);
        diff_value += lookup_sensor2hormone_c1_[sensor][hormone_id][(float)(my_sensor->getSensorValue())/(float)(my_sensor->getMaxValue())*(float)number_lookup_table_bins_];
      }
    }
    
    for(unsigned int hIn = 0; hIn < hormones_.size(); hIn++) {
      diff_value += lookup_hormone2hormone_[hIn][hormone_id][getHormoneConcentration(compartment_id, hIn)*(float)number_lookup_table_bins_];
      
      if(lookup_hormone2hormone_nonlin_[hIn][hormone_id][getHormoneConcentration(compartment_id, hIn)*(float)number_lookup_table_bins_][getHormoneConcentration(compartment_id, hormone_id)*(float)number_lookup_table_bins_] == lookup_table_uncomputed_flag_) {
	lookup_hormone2hormone_nonlin_[hIn][hormone_id][getHormoneConcentration(compartment_id, hIn)*(float)number_lookup_table_bins_][getHormoneConcentration(compartment_id, hormone_id)*(float)number_lookup_table_bins_]=0.0;
	for (int rule = 0; rule < (int) rules_.size(); rule++) {
	  AHHSRule *my_rule = rules_[rule];
	  if(my_rule->getHormoneInput() == hIn) {
	    lookup_hormone2hormone_nonlin_[hIn][hormone_id][getHormoneConcentration(compartment_id, hIn)*(float)number_lookup_table_bins_][getHormoneConcentration(compartment_id, hormone_id)*(float)number_lookup_table_bins_]
	      += my_rule->executeRuleHormoneNonLinear(0, hIn, getHormoneConcentration(compartment_id, hIn), getHormoneConcentration(compartment_id, hormone_id), hormones_[hormone_id]);
	  }
	}
      }
      diff_value += lookup_hormone2hormone_nonlin_[hIn][hormone_id][getHormoneConcentration(compartment_id, hIn)*(float)number_lookup_table_bins_][getHormoneConcentration(compartment_id, hormone_id)*(float)number_lookup_table_bins_];
    }

    if(compartment_id==0)  {
      for(int actuator=0; actuator < configuration_->getNumberOfActuators(); actuator++) {
	Actuator *my_actuator = configuration_->getActuator(actuator);
        my_actuator->setActuatorValue(my_actuator->getActuatorValue() + (float)(lookup_hormone2actuator_c0_[hormone_id][actuator][((getHormoneConcentration(compartment_id, hormone_id) - hormones_[hormone_id]->getMinValue()) / abs(hormones_[hormone_id]->getMaxValue() - hormones_[hormone_id]->getMinValue()))*(float)number_lookup_table_bins_]));
      }
    } 
    else {
      for(int actuator=0; actuator < configuration_->getNumberOfActuators(); actuator++) {
	Actuator *my_actuator = configuration_->getActuator(actuator);
        my_actuator->setActuatorValue(my_actuator->getActuatorValue() + (float)(lookup_hormone2actuator_c1_[hormone_id][actuator][((getHormoneConcentration(compartment_id, hormone_id) - hormones_[hormone_id]->getMinValue()) / abs(hormones_[hormone_id]->getMaxValue() - hormones_[hormone_id]->getMinValue()))*(float)number_lookup_table_bins_]));
      }
      
    }
   
  }
  //= OldValue + DiffValue;
  hormone_concentrations_next_step_[compartment_id*hormones_.size()+hormone_id] += diff_value;
}

void AHHS::checkRuleAssertions(AHHSRule *my_rule) 
{
  if(my_rule->getSensorInput() >= (double)configuration_->getNumberOfSensors()) {
    cout  << scientific << "Warning: AHHS::checkRuleAssertions(). Sensor_input out of bounds (" << my_rule->getSensorInput() << " >= " << configuration_->getNumberOfSensors() << "). \n";
    my_rule->setSensorInput((double)configuration_->getNumberOfSensors() - 0.001);
    // exit(0);
  }

  if(my_rule->getHormoneOutput() >= (double)hormones_.size()) {
    cout  << scientific << "Warning: AHHS::checkRuleAssertions(). hormone_output (" << my_rule->getHormoneOutput() << " >= " << (double)hormones_.size() << ") out of bounds. \n";
    my_rule->setHormoneOutput((double)hormones_.size() - 0.001);
    //exit(0);
  }

  if(my_rule->getHormoneInput() >= (double)hormones_.size()) {
    cout  << scientific << "Warning: AHHS::checkRuleAssertions(). hormone_input out of bounds (" << my_rule->getHormoneInput() << " >= " << (double)hormones_.size() << "). \n";
    my_rule->setHormoneInput((double)hormones_.size() - 0.001);
    //exit(0);
  }
  if(my_rule->getActuatorOutput() >= (double)configuration_->getNumberOfActuators()) {
    cout  << scientific << "Warning: AHHS::checkRuleAssertions(). actuator_output out of bounds. (" << my_rule->getActuatorOutput() << " >= " << (double)configuration_->getNumberOfActuators() << "). \n";
    my_rule->setActuatorOutput((double)configuration_->getNumberOfActuators() - 0.001);
    //exit(0);
  }
}

void AHHS::performDiffusion() 
{
  //cout << "AHHS::performDiffusion(): diff - coev " << hormones_[0]->getDiffusionCoefficient() << endl;
  /*
  if (configuration_->getNumberOfCompartments() == 2) {
    for (unsigned int i = 0; i < hormones_.size(); i++) {
      double old_c0 = getHormoneConcentration(0,i);
      double old_c1 = getHormoneConcentration(1,i);
      double delta0;
      double delta1;
      delta0 = (old_c0 - old_c1) * hormones_[i]->getDiffusionCoefficient() / 2.0;
      delta1 = (old_c1 - old_c0) * hormones_[i]->getDiffusionCoefficient() / 2.0;
      
      if (do_diffusion_with_neighbors_) {
	delta0 += (old_c0 - neighbor_value_) * hormones_[i]->getDiffusionCoefficient();
	//delta1 += (old_1 - neighbor_value_) * hormones_[i]->getDiffusionCoefficient();
      }
      
      setHormoneConcentration(0, i, getHormoneConcentration(0, i) - delta0);
      setHormoneConcentration(1, i, getHormoneConcentration(1, i) - delta1);
      //cout << "AHHS::performDiffusion(): diff - coev " << hormones_[0]->getDiffusionCoefficient() << " delta0: " << delta0 << " delta1: " <<  delta0 << endl;
    }
  }
  else {
    cout << "Sorry, diffusion only inplemented for 2 compartents, not " << configuration_->getNumberOfCompartments() << endl; 
  }
  */
  vector<double> tmp_hormone_concentrations = hormone_concentrations_;
  for (unsigned int hormone = 0; hormone < hormones_.size(); hormone++) {
    for (int compartment_id = 0; compartment_id < configuration_->getNumberOfCompartments(); compartment_id++) {
      vector<int> neighbor_comp_IDs = configuration_->getNeighborCompIDs(compartment_id);
      for (unsigned int neigh_comp_id = 0; neigh_comp_id < neighbor_comp_IDs.size(); neigh_comp_id++) {
	tmp_hormone_concentrations[compartment_id*hormones_.size()+hormone] 
	  -= (hormone_concentrations_[compartment_id*hormones_.size()+hormone]
	      - hormone_concentrations_[neigh_comp_id*hormones_.size()+hormone])*hormones_[hormone]->getDiffusionCoefficient();
      }
    }
  }
  hormone_concentrations_ = tmp_hormone_concentrations;
}

bool AHHS::addRule(AHHSRule *new_rule) 
{
  rules_.push_back(new_rule);
  return true;
}

bool AHHS::addHormone(AHHSHormone *new_hormone)
{
  hormones_.push_back(new_hormone);
  return true;
}

void AHHS::logging(string filenamePrefix)
{
  ofstream file;
  string filename;
  
  filename.assign(filenamePrefix);
  filename.append("ahhs");
  file.open (filename.c_str());
  file << configuration_->getNumberOfCompartments() << "; ";
  file << hormones_.size() << "; ";
  file << rules_.size() << "; ";
  file << configuration_->getNumberOfSensors() << "; ";
  file << configuration_->getNumberOfActuators() << "; ";
  file << ahhs_type_ << "; ";
  file << use_lookup_ << "; ";
  file << do_diffusion_with_neighbors_;
  file << endl;
  file.close();
  
  filename.assign(filenamePrefix);
  filename.append("hormones");
  file.open (filename.c_str());
  for (unsigned int i = 0; i < hormones_.size(); i++) {
    hormones_[i]->logging(file);
  }
  file.close();
  
  filename.assign(filenamePrefix);
  filename.append("rules");
  file.open (filename.c_str());
  for (unsigned int i = 0; i < rules_.size(); i++) {
    rules_[i]->logging(file);
  }
  file.close();

  filename.assign(filenamePrefix);
  filename.append("sensors");
  file.open (filename.c_str());
  for (int i = 0; i < configuration_->getNumberOfSensors(); i++) { 
    configuration_->getSensor(i)->logging(file);
  }
  file.close();
  
  filename.assign(filenamePrefix);
  filename.append("actuators");
  file.open (filename.c_str());
  for (int i = 0; i < configuration_->getNumberOfActuators(); i++) { 
    configuration_->getActuator(i)->logging(file);
  }
}

void AHHS::reset() 
{
  for (int i = 0; i < configuration_->getNumberOfCompartments(); i++) {
    for (int j = 0; j < (int) hormones_.size(); j++) {
      hormone_concentrations_[i*hormones_.size()+j] = 0.0;
    }
  }
  for (int i = 0; i < configuration_->getNumberOfCompartments(); i++) {
    for (int j = 0; j < (int) hormones_.size(); j++) {
      hormone_concentrations_next_step_[i*hormones_.size()+j] = 0.0;
    }
  }
  for (int i = 0; i < configuration_->getNumberOfSensors(); i++) {
    configuration_->getSensor(i)->setSensorValue(0.0);
  }
  for (int i = 0; i < configuration_->getNumberOfActuators(); i++) {
    configuration_->getActuator(i)->setActuatorValue(0.0);
  }
  lookup_table_computed_ = false;
}

void AHHS::computeLookupTable() 
{
  int compartment;
	
  lookup_sensor2hormone_c0_.clear();
  lookup_sensor2hormone_c0_.resize(configuration_->getNumberOfSensors(), 
		std::vector< std::vector<double> >(hormones_.size(), 
		std::vector<double>(number_lookup_table_bins_+1, 0)) );
  
  lookup_sensor2hormone_c1_.clear();
  lookup_sensor2hormone_c1_.resize(configuration_->getNumberOfSensors(),  
		std::vector< std::vector<double> >(hormones_.size(),  
		std::vector<double>(number_lookup_table_bins_+1, 0)) );
  
  lookup_hormone2hormone_.clear();
  lookup_hormone2hormone_.resize(hormones_.size(),  
		std::vector< std::vector<double> >(hormones_.size(),  
		std::vector<double>(number_lookup_table_bins_+1, 0)) );
  
  lookup_hormone2hormone_nonlin_.clear();
  lookup_hormone2hormone_nonlin_.resize(hormones_.size(),
		std::vector< std::vector< std::vector<double> > >(hormones_.size(), 
		std::vector< std::vector<double> >(number_lookup_table_bins_+1, 
		std::vector<double>(number_lookup_table_bins_+1, 0)) ) );

  lookup_hormone2actuator_c0_.clear();  
  lookup_hormone2actuator_c0_.resize(hormones_.size(),  
	        std::vector< std::vector<double> >(configuration_->getNumberOfActuators(),  
		std::vector<double>(number_lookup_table_bins_+1, 0)) );
  
  lookup_hormone2actuator_c1_.clear();
  lookup_hormone2actuator_c1_.resize(hormones_.size(),  
	        std::vector< std::vector<double> >(configuration_->getNumberOfActuators(),  
		std::vector<double>(number_lookup_table_bins_+1, 0)) );


  for (unsigned int k=0; k<rules_.size();k++) {
      for (unsigned int i=0; i<number_lookup_table_bins_;i++) {
	
	AHHSRule *myRule = rules_[k];
	Sensor *mySensor = configuration_->getSensor((int)myRule->getSensorInput());
	Actuator *myActuator = configuration_->getActuator((int)myRule->getActuatorOutput());
	mySensor->setSensorValue((float)i/(float)number_lookup_table_bins_*(float)mySensor->getMaxValue());
	int hIn  = myRule->getHormoneInput();
	int hOut = myRule->getHormoneOutput();

	// sensor rule:
	int sensor = (int)myRule->getSensorInput();
	compartment=0;
	lookup_sensor2hormone_c0_[sensor][hOut][i] 
	  += myRule->executeRuleSensorSource(compartment, hOut, 
					      mySensor, hormones_.size());
	compartment=1;
	lookup_sensor2hormone_c1_[sensor][hOut][i] 
	  += myRule->executeRuleSensorSource(compartment, hOut, 
					      mySensor, hormones_.size());
	
	// actuator rule:
	int actuator = (int)myRule->getActuatorOutput();
	compartment=0;
	lookup_hormone2actuator_c0_[hIn][actuator][i]
	  += myRule->executeRuleActuatorTarget(compartment, hIn, myActuator, (float)i/(float)number_lookup_table_bins_, hormones_.size());
	compartment=1;
	lookup_hormone2actuator_c1_[hIn][actuator][i]
	  += myRule->executeRuleActuatorTarget(compartment, hIn, 
						myActuator, 
						(float)i/(float)number_lookup_table_bins_, 
						hormones_.size());

	// hormone2hormone rule:
	lookup_hormone2hormone_[hIn][hOut][i]
	  += myRule->executeRuleHormone(0, hIn,
					 (float)i/(float)number_lookup_table_bins_,
					 0.0,
					 hormones_[hOut]);

      } // for bins
  } // for rules
    
  // hormone2hormone nonLin rule:
  for(unsigned int hIn=0;hIn<hormones_.size();hIn++) {
    for(unsigned int hOut=0;hOut<hormones_.size();hOut++) {
      for (unsigned int i=0; i<number_lookup_table_bins_;i++) {
	for (unsigned int j=0; j<number_lookup_table_bins_;j++) {
	  lookup_hormone2hormone_nonlin_[hIn][hOut][i][j]
	    = lookup_table_uncomputed_flag_;
	}
      }
    }
  }
 lookup_table_computed_ = true;
}

unsigned int AHHS::mutate(double MutProbHormoneChange, double MutProbRuleChange, double MutValueChangePercent)
{
#if IZG_DEBUG_MESSAGES
    std::cout << "AHHS2Controller::Mutate(" << MutProbHormoneChange<< ", "
		  << MutProbRuleChange<< ", " << MutValueChangePercent<< ")" << std::endl;
  #endif

  int MutationCount = 0;

  for(unsigned int hormoneIndex=0;hormoneIndex<hormones_.size();hormoneIndex++) 
  {
    MutationCount += hormones_[hormoneIndex]->mutate(MutProbHormoneChange, MutValueChangePercent);
  } // for hormoneIndex

  for(unsigned int ruleIndex=0;ruleIndex<rules_.size();ruleIndex++) 
  {
    MutationCount += rules_[ruleIndex]->mutate(MutProbRuleChange, MutValueChangePercent,
					      configuration_->getNumberOfActuators(), configuration_->getNumberOfSensors(), hormones_.size(), ahhs_type_);
    
  }
  if (use_lookup_)
    computeLookupTable();

  return MutationCount;
}
 
unsigned int AHHS::mutateExactOne(double MutProbHormoneChange, double MutProbRuleChange, double MutValueChangePercent)
{
#if IZG_DEBUG_MESSAGES
	std::cout << "AHHS2Controller::MutateExactOne(" << MutProbHormoneChange<< ", "
		  << MutProbRuleChange<< ", " << MutValueChangePercent<< ")" << std::endl;
#endif

	int hormoneIndex = rand() % hormones_.size();
	int ruleIndex = rand() % rules_.size();

	if((double)rand()/(double)RAND_MAX < 0.5)
		hormones_[hormoneIndex]->mutateExactOne(MutProbHormoneChange, MutValueChangePercent);
	else
		rules_[ruleIndex]->mutateExactOne(MutProbRuleChange, MutValueChangePercent,configuration_->getNumberOfActuators(), configuration_->getNumberOfSensors(), hormones_.size(), ahhs_type_);

	return 1;
}
 
int AHHS::recombinateWith(AHHS *Partner, double RecProbHormones, double RecProbRules)
{
  //std::cout << "AHHS2Controller::RecombinateWith(*Partner, " << RecProbHormones<< ", " << RecProbRules<< ")" << std::endl;

  int recombinated = 0;
  int minNumHormones;
  int minNumRules;
  
  //Recombination of Hormones
  if((double)rand()/(double)RAND_MAX < RecProbHormones) {
    recombinated = 1;

    if(Partner->hormones_.size() < this->hormones_.size())
      minNumHormones = Partner->hormones_.size();
    else
      minNumHormones = this->hormones_.size();

    unsigned int point1 = (unsigned int)(((double)(rand())/(double)(RAND_MAX)) * (double)(minNumHormones));
    unsigned int point2 = (unsigned int)(((double)(rand())/(double)(RAND_MAX)) * (double)(minNumHormones));
    //unsigned int point1 = (unsigned int)(rand() % minNumHormones);
    //unsigned int point2 = (unsigned int)(rand() % minNumHormones);
    
    if (point1 > point2 ) {
      unsigned int point3 = point2;
      point2 = point1;
      point1 = point3;
    } //if (point1 > point2)
    
    for (unsigned int i=point1; i<=point2; i++) {
      hormones_[i]->setBaseEmissionRate(Partner->getHormone(i)->getBaseEmissionRate());
      hormones_[i]->setDiffusionCoefficient(Partner->getHormone(i)->getDiffusionCoefficient());
      hormones_[i]->setDecayRate(Partner->getHormone(i)->getDecayRate());
      hormones_[i]->setMaxValue(Partner->getHormone(i)->getMaxValue());
      hormones_[i]->setMinValue(Partner->getHormone(i)->getMinValue());
    }// i
    
    //std::cout << "AHHS2Controller::RecombinateWith: Recombination of Hormones from " << point1 << " to " <<  point2 << "." << std::endl;

  } // if
  
  //Recombination of Rules
  if((double)rand()/(double)RAND_MAX < RecProbRules) {
    recombinated = 1;

    if(Partner->rules_.size() < this->rules_.size())
      minNumRules = Partner->rules_.size();
    else
      minNumRules = this->rules_.size();

    unsigned int point1 = (unsigned int)(((double)(rand())/(double)(RAND_MAX)) * (double)(minNumRules));
    unsigned int point2 = (unsigned int)(((double)(rand())/(double)(RAND_MAX)) * (double)(minNumRules));

    if (point1 > point2 ) {
      unsigned int point3 = point2;
      point2 = point1;
      point1 = point3;
    } //if (point1 > point2)
    
    for (unsigned int i=point1; i<=point2; i++) {
      for(int j=0;j<NR_OF_RULE_TYPES;j++)
	rules_[i]->setWeight(j, Partner->getRule(i)->getWeight(j));
      rules_[i]->setActuatorOutput(Partner->getRule(i)->getActuatorOutput());
      rules_[i]->setDependentDose(Partner->getRule(i)->getDependentDose());
      rules_[i]->setFixedDose(Partner->getRule(i)->getFixedDose());
      rules_[i]->setHormoneInput(Partner->getRule(i)->getHormoneInput());
      rules_[i]->setHormoneOutput(Partner->getRule(i)->getHormoneOutput());
      rules_[i]->setSensorInput(Partner->getRule(i)->getSensorInput());
      rules_[i]->setWindowCenter(Partner->getRule(i)->getWindowCenter());
      rules_[i]->setWindow(Partner->getRule(i)->getWindow());
    }// i

    //std::cout << "AHHS2Controller::RecombinateWith: Recombination of Hormones from " << point1 << " to " <<  point2 << "." << std::endl;

  } // if
  
  if (use_lookup_)
    computeLookupTable();
   
  return recombinated;
}

void AHHS::resetHormoneConcentrations()
{
  //cout << "AHHS::resetHormoneConcentrations(). comp: " << configuration_->getNumberOfCompartments() << " horm: " <<  hormones_.size() << " conc_arr size: " << hormone_concentrations_.size() << " next_arr: " << hormone_concentrations_next_step_.size() << endl;
  for (int i = 0; i < configuration_->getNumberOfCompartments(); i++) {
    for (unsigned int j = 0; j < hormones_.size(); j++) {
	hormone_concentrations_[i*hormones_.size()+j] = 0.0;
	hormone_concentrations_next_step_[i*hormones_.size()+j] = 0.0;
    }
  }
}

void AHHS::resetHormones()
{
  hormones_.clear();
}

void AHHS::resetRules()
{
  rules_.clear();
}

bool AHHS::linkNeighbor(AHHS* neighbor) 
{
  //if ((int)neighbors_.size() >= Max_Number_of_Neighbors) {
  //  std::cout << "AHHS2Controller::LinkNeighbor(): Error! " << neighbors_.size() << " neighbors are not allowed. you can only make " << Max_Number_of_Neighbors << " neighbors.\n";
  //  exit(0);
  //  return false;
  //}
  //else {
    neighbors_.push_back(neighbor);
    return true;
    //}
}

bool AHHS::unlinkNeighbor(int index_of_neighbor) 
{
  if (index_of_neighbor >=(int) neighbors_.size()) {
    std::cout << "Error: There is no neighbor #" << index_of_neighbor << "  linked to the hormone controller. There exists just " << neighbors_.size() << " neighbors.\n";
    return false;
  }
  else {
    for (int i = index_of_neighbor; i < (int)neighbors_.size(); i++) {
      neighbors_[i] = neighbors_[i+1];
    }
    neighbors_[neighbors_.size()-1] = NULL;
    neighbors_[neighbors_.size()] = NULL; //just for safety, should not be necessary
#if IZG_DEBUG_MESSAGES
    std::cout << "Success: Unlinked neighbor #" << index_of_neighbor << ".\n";
#endif
    return true;
  }
}

void AHHS::unlinkAllNeighbors()
{
  if (neighbors_.size() != 0) {
    //Neighbors.resize(0, NULL);
    neighbors_.clear();
  }
}

void AHHS::readFromFile(string filenamePrefix) 
{
  //cout << "AHHS::readFromFile(string): filenamePrefix: " << filenamePrefix << endl;

  string filename;
  ifstream *file;
  string line;
  
  filename.assign(filenamePrefix);
  filename.append("ahhs");
  //cout << filename << endl;
  file = new std::ifstream(filename.c_str());
  if (!(file->is_open())) {
    cout << "AHHS::readFromFile: File '" << filename << "' does not exist. Check your config file! Abort.\n";
    delete file;
    file = 0;
    exit(0);
  }
  while (getline (*file, line)) {
    string item;
    istringstream linestream(line);

    unsigned int field_counter = 0;
    while (getline (linestream, item, ';')) {
      int value = atoi(item.c_str());
      switch (field_counter) {
      case 0:
	configuration_->setNrOfCompartments(value);
	break;
      case 1:
	break;
      case 2:
	break;
      case 3:
	break;
      case 4:
	break;
      case 5:
	ahhs_type_ = value;
	break;
      case 6:
	use_lookup_ = (bool)(value);
	break;
      case 7:
	do_diffusion_with_neighbors_ = (bool)value;
	break;
      default:
	std::cout << "AHHS::readFromFile: Too many fields in ahhs definition file. Please check. Aborting.\n";
	delete file;
	file = 0;
	exit(1);
	break;
      }
      //std::cout << "field_counter: " << field_counter << ": " << value << std::endl;
      field_counter++;
    }
    // fieldcounter is 1 greater than it should be
    if (field_counter != 8) {
      std::cout << "AHHS::readFromFile: Not enough fields in ahhs definition file. Please check. Aborting.\n";
      delete file;
      file = 0;
      exit(1);
    }
  }
  delete file;
  file = 0;

  filename.assign(filenamePrefix);
  filename.append("hormones");
  //cout << "Hormones filename " << filename << endl;
  file = new std::ifstream(filename.c_str());
  if (!(file->is_open())) {
    std::cout << "AHHS::readFromFile: File '" << filename << "' does not exist. Check your config file! Abort.\n";
    delete file;
    file = 0;
    exit(0);
  }
  while (getline (*file, line)) {
    AHHSHormone *myHormone = new AHHSHormone();
    myHormone->readFromFile(line);
    addHormone(myHormone);
  }
  delete file;
  file = 0;

  filename.assign(filenamePrefix);
  filename.append("rules");
  //std::cout << filename << std::endl;
  file = new std::ifstream(filename.c_str());
  if (!(file->is_open())) {
    std::cout << "AHHS::readFromFile: File '" << filename << "' does not exist. Check your config file! Abort.\n";
    delete file;
    file = 0;
    exit(0);
  }
  while (getline (*file, line)) {
    AHHSRule *myRule = new AHHSRule();
    myRule->readFromFile(line);
    addRule(myRule);
  }
  delete file;
  file = 0;
    
  filename.assign(filenamePrefix);
  filename.append("actuators");
  //std::cout << filename << std::endl;
  file = new std::ifstream(filename.c_str());
  if (!(file->is_open())) {
    std::cout << "AHHS2Controller::ReadFromFile: File '" << filename << "' does not exist. Check your config file! Abort.\n";
    delete file;
    file = 0;
    exit(0);
  }
  while (getline (*file, line)) {
    Actuator *myActuator = new Actuator();
    myActuator->readFromFile(line);
    configuration_->addActuator(myActuator);
  }
  delete file;
  file = 0;

  filename.assign(filenamePrefix);
  filename.append("sensors");
  //std::cout << filename << std::endl;
  file = new std::ifstream(filename.c_str());
  if (!(file->is_open())) {
    std::cout << "AHHS::readFromFile: File '" << filename << "' does not exist. Check your config file! Abort.\n";
    delete file;
    file = 0;
    exit(0);
  }
  while (getline (*file, line)) {
    Sensor *mySensor = new Sensor();
    mySensor->readFromFile(line);
    configuration_->addSensor(mySensor);
  } 
  delete file;
  file = 0;
}

int AHHS::getNumberOfNeighbors()
{
  return neighbors_.size();
}
 
AHHS* AHHS::getNeighbor(int pos)
{
  return neighbors_[pos];
}
