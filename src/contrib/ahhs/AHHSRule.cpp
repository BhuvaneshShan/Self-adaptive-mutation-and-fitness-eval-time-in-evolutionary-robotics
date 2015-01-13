/**
 * RuleClass.cpp
 *
*  Created on: 23.07.2009
 * Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 */
#include "ahhs/AHHSRule.h"

AHHSRule::AHHSRule()
{
  weights_.resize(NR_OF_RULE_TYPES, 0.0);
  window_ = 0.0;
  window_center_ = 0.0;
  dependent_dose_ = 0.0;
  fixed_dose_ = 0.0;
  sensor_input_ = 0.0;
  hormone_input_ = 0.0;
  hormone_output_ = 0.0;
  actuator_output_ = 0.0;
}

AHHSRule::AHHSRule(AHHSRule *src) 
{
  weights_.clear();
  for(unsigned int i = 0; i < NR_OF_RULE_TYPES; i++) {
    if (NR_OF_RULE_TYPES != src->weights_.size()) {
      cout << "AHHSRule::AHHSRule(AHHSRule *). Error. Rule to copy has wrong nr of weigths. Exit." << endl;
      exit(1);
    }
    weights_.push_back(src->weights_[i]);
  }
  window_  = src->window_;
  window_center_  = src->window_center_;
  dependent_dose_ = src->dependent_dose_;
  fixed_dose_ = src->fixed_dose_;
  sensor_input_ = src->sensor_input_;
  hormone_input_ = src->hormone_input_;
  hormone_output_ = src->hormone_output_;
  actuator_output_ = src->actuator_output_;
}

AHHSRule::~AHHSRule()
{
  //std::cout << "AHHSRule::~AHHSRule(). dead" << endl; 
}

double AHHSRule::getWeight(int pos) 
{
  if (pos >= (int) weights_.size()) {
    cout << "AHHSRule::getWeight(). Error: nr. of weigths exceeded. Exit." << endl;
    exit (0);
  }
  return weights_[pos];
}

double AHHSRule::getWindow() 
{
  return window_;
}

double AHHSRule::getWindowCenter() 
{
  return window_center_;
}

double AHHSRule::getDependentDose() 
{
  return dependent_dose_;
}

double AHHSRule::getFixedDose() 
{
  return fixed_dose_;
}

double AHHSRule::getSensorInput() 
{
  return sensor_input_;
}
    
double AHHSRule::getHormoneInput()
{
  return hormone_input_;
}

double AHHSRule::getHormoneOutput()
{
  return hormone_output_;
}

double AHHSRule::getActuatorOutput()
{
  return actuator_output_;
}

void AHHSRule::setWeight(double weight, int pos) 
{
  if (pos >= (int) weights_.size()) {
    cout << "AHHSRule::getWeight(). Error: nr. of weigths exceeded. Exit." << endl;
    exit (0);
  }
  weights_[pos] = weight;
}

void AHHSRule::setWindow(double window)
{
  window_ = window;  
}

void AHHSRule::setWindowCenter(double window_center)
{
  window_center_ = window_center;  
}

void AHHSRule::setDependentDose(double dependent_dose)
{
  dependent_dose_ = dependent_dose;  
}
 
void AHHSRule::setFixedDose(double fixed_dose)
{
  fixed_dose_ = fixed_dose;  
}

void AHHSRule::setSensorInput(double sensor_input)
{
  //cout << "AHHSRule::setSensorInput(). sensor input changed from outside from " << sensor_input_ << " to " << sensor_input << endl;
  sensor_input_ = sensor_input;  
}

void AHHSRule::setHormoneInput(double hormone_input)
{
  //cout << "AHHSRule::setHormoneInput(). hormone input changed from outside from " << hormone_input_ << " to " << hormone_input << endl;
  hormone_input_ = hormone_input;  
}

void AHHSRule::setHormoneOutput(double hormone_output)
{
  //cout << "AHHSRule::setHormoneOutput(). hormone output changed from outside from " << hormone_output_ << " to " << hormone_output << endl;
  hormone_output_ = hormone_output;  
}
 
void AHHSRule::setActuatorOutput(double actuator_output)
{
  //cout << "AHHSRule::setActuatorOutput(). actuator output changed from outside from " << actuator_output_ << " to " << actuator_output << endl;
  actuator_output_ = actuator_output;  
}

double AHHSRule::executeRuleSensorSource(int compartment_id, int hormone_id, Sensor *my_sensor, int number_of_hormones)
{
  if((compartment_id == my_sensor->getCompartmentID()) && ((hormone_id == (int)hormone_output_) || (hormone_id == ((int) hormone_output_ + 1) % number_of_hormones))) {
    double hormone_id_weight = 1.0f - fabs(hormone_output_ - (double)hormone_id);
    if (my_sensor->getSensorValue() > my_sensor->getMaxValue())
      my_sensor->setSensorValue(my_sensor->getMaxValue());
    if (my_sensor->getSensorValue() < my_sensor->getMinValue())
      my_sensor->setSensorValue(my_sensor->getMinValue());
    double sensor_value = (double)my_sensor->getSensorValue();
    
    return weights_[AHHS_SENSOR_SOURCE] * hormone_id_weight * checkTrigger(sensor_value) * (sensor_value * dependent_dose_ + fixed_dose_);
  } 
  else {
    return 0.0;
  }
}

double AHHSRule::executeRuleActuatorTarget(int compartment_id, int hormone_id, Actuator *my_actuator, double hormone_value, int number_of_hormones) 
{
  //cout << "AHHSRule::executeRuleActuatorTarget(int..). w2: " << weights_[AHHS_ACTUATOR_TARGET] << endl;
  if( (compartment_id == my_actuator->getCompartmentID()) && ( (hormone_id == (int)hormone_input_) || (hormone_id == ((int)hormone_input_ + 1)%number_of_hormones) ) ) {
    double hormone_id_weight = 1.0 - fabs(hormone_input_ - (double)hormone_id);
    double actuator_change = weights_[AHHS_ACTUATOR_TARGET] * hormone_id_weight * checkTrigger(hormone_value) * (hormone_value * dependent_dose_ + fixed_dose_);
    //cout << actuator_change << " " << weights_[AHHS_ACTUATOR_TARGET] << " " << hormone_id_weight << " " << checkTrigger(hormone_value) << " " << hormone_value  << " " << fixed_dose_ << endl;
    return actuator_change;
  } 
  else {
    //cout << " wrong comp, no h input " << " " << weights_[AHHS_ACTUATOR_TARGET] << " " << "no weight" << " " << checkTrigger(hormone_value) << " " << hormone_value  << " " << fixed_dose_ << endl;
    return 0.0;
  }
}


double AHHSRule::executeRuleHormone(int compartment_id, int hormone_id, double source_value, double target_value, AHHSHormone *target_hormone) 
{
  double hormone_change = 0.0;

  if ((int)hormone_input_ == hormone_id) 
  {
    hormone_change = weights_[AHHS_HORMONES] * checkTrigger(source_value) * (source_value * dependent_dose_ + fixed_dose_);
  }
  return hormone_change;
}

double AHHSRule::executeRuleHormoneNonLinear(int compartment_id, int hormone_id, double source_value, double target_value, AHHSHormone *target_hormone) {
  double hormone_change = 0.0;

  if ((int)hormone_input_ == hormone_id) {
    if(AHHS_NONLINEAR >= NR_OF_RULE_TYPES) {
      cout << "Rule::ExecuteRule_HormoneNonLinear - ERROR: Increase RULE_TYPES to use non-linear rules. Exit" << endl;
      exit(0);
    }
    hormone_change= weights_[AHHS_NONLINEAR] * checkTrigger(source_value) * (source_value * target_value * dependent_dose_ + fixed_dose_);
  }
  return hormone_change;
}

double AHHSRule::checkTrigger(double value) 
{
  if(fabs(value-window_center_) < window_) {
    return (window_-fabs(value-window_center_))/window_;
  } 
  return 0.0;
}

void AHHSRule::handCodedInit(int rule_number) 
{

  for (int i=0; i < NR_OF_RULE_TYPES; i++) {
    weights_[i] = 0.0;
  }
  hormone_input_ = 0.0;
  hormone_output_ = 0.0;

  // sensor rules:
  if ((rule_number == 0) || (rule_number == 1)) {
    weights_[1] = 1.0;
    window_center_ = 1.0;
    window_ = 1.0;
    fixed_dose_ = 1.0;
    dependent_dose_ = 0.0;
    sensor_input_ = (float)rule_number; //3.0;
    hormone_input_ = 0.0;
    hormone_output_ = 0.0;
    actuator_output_ = 0.0;
  }
  else if (rule_number == 2) {
    // rule 1 - actuator rule 0: STOPPING
    weights_[2] = 1.0;
    window_center_ = 1.0;
    window_ = 1.0;
    fixed_dose_ = -2.0;
    dependent_dose_ = 0.0;
    sensor_input_ = 0.0;
    hormone_input_ = 0.0;
    hormone_output_ = 0.0;
    actuator_output_ = 0.0;
  }
  else if (rule_number == 3) {
    // rule 2 - actuator rule 1: STOPPING
    weights_[2] = 1.0;
    window_center_ = 1.0;
    window_ = 1.0;
    fixed_dose_ = -2.0;
    dependent_dose_ = 0.0;
    sensor_input_ = 0.0;
    hormone_input_ = 0.0;
    hormone_output_ = 0.0;
    actuator_output_ = 1.0;
  }
  /*
  else if (rule_number == 4) {
   // rule 3 - actuator rule 0: BASE SPEED
    window_center_ = 1.0;
    window_ = 1.0;
    weights_[2] = 1.0;
    fixed_dose_ = 1.0;
    dependent_dose_ = 0.0;
    sensor_input_ = 0.0;
    actuator_output_ = 0.0;
    hormone_input_ = 1.0;
  }
  else if (rule_number == 5) {
    window_center_ = 1.0;
    window_ = 1.0;
    // rule 4 - actuator rule 1: BASE SPEED
    weights_[2] = 1.0;
    fixed_dose_ = 1.0;
    dependent_dose_ = 0.0;
    sensor_input_ = 0.0;
    actuator_output_ = 1.0;
    hormone_input_ = 1.0;
  }
  */
}

void AHHSRule::randomInit(int ahhs_type, int number_of_sensors, int number_of_hormones, int number_of_actuators) 
{
  //cout << "AHHSRule::randomInit(int, int, int, int) begin: type: " << ahhs_type << " sens: " << number_of_sensors << " horm: " << number_of_hormones << " act: " << number_of_actuators << endl;
  if (ahhs_type == 0) {
    setRuleProbRandom(3);
  }
  else {
    setRuleProbRandom(NR_OF_RULE_TYPES);
  }
  window_center_ = (2.0*(double)rand()/(double)RAND_MAX - 1.0) * 2.0;
  window_ = (double)rand()/(double)RAND_MAX * 1.0; // was: 0.25
  fixed_dose_ = (2.0*(double)rand()/(double)RAND_MAX - 1.0) * 0.2;//was: 0.02
  dependent_dose_ = (2.0*(double)rand()/(double)RAND_MAX - 1.0) * 2.0;
  sensor_input_ = (double)(rand() % number_of_sensors) + (double)rand()/(double)RAND_MAX * 0.999;
  hormone_input_ = (double)(rand() % number_of_hormones) + (double)rand()/(double)RAND_MAX * 0.999;
  hormone_output_ = (double)(rand() % number_of_hormones) + (double)rand()/(double)RAND_MAX * 0.999;
  actuator_output_ = (double)(rand() % number_of_actuators) + (double)rand()/(double)RAND_MAX * 0.999;

  //cout << "AHHSRule::randomInit(int, int, int, int) done"<< endl;
}

void AHHSRule::setRuleProbRandom(int nr_of_rule_types) 
{
  //cout << "AHHSRule::setRuleProbRandom(int). begin" << endl;

  double randPoint[nr_of_rule_types];
  double min;
  int minIndex;
  double minOld;

  for (int i = 0; i < NR_OF_RULE_TYPES; i++) {
    weights_[i] = 0.0;
  }
  for(int i = 0; i < nr_of_rule_types; i++) {
    randPoint[i] = (double)rand()/(double)RAND_MAX;
  }
  minOld = 0.0;
  for(int i = 0; i < nr_of_rule_types-1; i++) {
    min = 1.0;
    minIndex = -1;
    for(int j = 0; j < nr_of_rule_types-1; j++) {
      if(randPoint[j] < min) {
	min = randPoint[j];
	minIndex = j;
      }
    }
    weights_[i] = min - minOld;
    randPoint[minIndex] = 1.0;
    minOld = min;
  }
  weights_[nr_of_rule_types-1] = 1.0 - minOld;

  //cout << "AHHSRule::setRuleProbRandom(int). done w0:" << weights_[0] << " w1: " << weights_[1] << " w2: " << weights_[2] << " w3: " << weights_[3]  << " w4: " << weights_[4]  <<  endl;
}

void AHHSRule::logging(ofstream &File) 
{
  for(int j = 0; j < NR_OF_RULE_TYPES;j++)
    File << std::scientific << std::setprecision(std::numeric_limits<double>::digits10 + 1) << weights_[j] << "; ";
  File << std::scientific << window_center_ << "; ";
  File << std::scientific << window_ << "; ";
  File << std::scientific << fixed_dose_ << "; ";
  File << std::scientific << dependent_dose_ << "; ";
  File << std::scientific << sensor_input_ << "; ";
  File << std::scientific << hormone_input_ << "; ";
  File << std::scientific << hormone_output_ << "; ";
  File << std::scientific << actuator_output_;
  File << std::endl;
}


void AHHSRule::writeRule() 
{
  for(int j = 0; j < NR_OF_RULE_TYPES; j++)
    cout << weights_[j] << "; ";
  cout << window_center_ << "; ";
  cout << window_ << "; ";
  cout << fixed_dose_ << "; ";
  cout << dependent_dose_ << "; ";
  cout << sensor_input_ << "; ";
  cout << hormone_input_ << "; ";
  cout << hormone_output_ << "; ";
  cout << actuator_output_ << "; ";
  cout << endl;
}

void AHHSRule::readFromFile(std::string line) 
{
  std::string item;
  std::istringstream linestream(line);
  
  int field_counter = 0;
  while (getline (linestream, item, ';')) {
    double value = atof(item.c_str());
    switch (field_counter) {
    case 0:
      weights_[0] = value;
      break;
    case 1:
      weights_[1] = value;
      break;
    case 2:
      weights_[2] = value;
      break;
    case 3:
      weights_[3] = value;
      break;
    case 4:
      weights_[4] = value;
      break;
    case 5:
      window_center_ = value;
      break;
    case 6:
      window_ = value;
      break;
    case 7:
      fixed_dose_ = value;
      break;
    case 8:
      dependent_dose_ = value;
      break;
    case 9:
      sensor_input_ = value;
      break;
    case 10:
      hormone_input_ = value;
      break;
    case 11:
      hormone_output_ = value;
      break;
    case 12:
      actuator_output_ = value;
      break;
    default:
      std::cout << "Rule::ReadFromFile: Too many fields in rule definition file. Please check. Aborting.\n";
      exit(1);
    }
    //std::cout << "field_counter: " << field_counter << ": " << value << std::endl;
   field_counter++;
  }
  if (field_counter != 13) {
    std::cout << "Rule::ReadFromFile: Not correct number of fields in rule definition file. There were "
	      << field_counter << " fields. Please check. Aborting.\n";
    exit(1);
  }
}

int AHHSRule::mutate(double MutProbRuleChange, double MutValueChangePercent, int Number_of_Actuators, int Number_of_Sensors, int Number_of_Hormones, int ahhs_type) 
{

  int MutationCount = 0;
  int nr_of_rule_types;
  if (ahhs_type == 0) {
      nr_of_rule_types = 3;
  }
  else {
      nr_of_rule_types = NR_OF_RULE_TYPES;
    }
  if((double)rand()/(double)RAND_MAX < MutProbRuleChange) {
    
    MutationCount++;
    int valueIndex = rand() % 15;
    
    switch (valueIndex) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      mutateWeight(MutValueChangePercent, nr_of_rule_types);
      break;
    case 7:
      mutateWindow(MutValueChangePercent);
      break;
    case 8:
      mutateWindowCenter(MutValueChangePercent);
      break;
    case 9:
      mutateDependentDose(MutValueChangePercent);
      break;
    case 10:
      mutateFixedDose(MutValueChangePercent);
      break;
    case 11:
      mutateIndex(&actuator_output_, Number_of_Actuators);
      break;
    case 12:
      mutateIndex(&sensor_input_, Number_of_Sensors);
      break;
    case 13:
      mutateIndex(&hormone_input_, Number_of_Hormones);
      //cout << "AHHSRule::mutate(). Nr. of Hormones: " << Number_of_Hormones << endl;
      break;
    case 14:
      mutateIndex(&hormone_output_, Number_of_Hormones);
      break;
      //    case 15:
      //mutateIndex(&actuator_status, Number_of_Actuators, MutValueChangePercent);
      //break;
      //case 16:
      //mutateIndex(&sensor_status, Number_of_Sensors, MutValueChangePercent);
      //break;
      
    default:
      std::cout << "ERROR: reached default in switch in AHHS2Evolvable::Mutate()" << std::endl;
      exit(0);
      
    } // switch
  } // if < MUT_PROP_RULE_CHANGE
  
  return MutationCount;
}

int AHHSRule::mutateExactOne(double MutProbRuleChange, double MutValueChangePercent, int Number_of_Actuators, int Number_of_Sensors, int Number_of_Hormones, int ahhs_type) 
{
#if IZG_DEBUG_MESSAGES
  std::cout << "Rule::MutateExactOne" << std::endl;
#endif
  int nr_of_rule_types;
  int MutationCount = 0;
  if (ahhs_type == 0)
    {
      nr_of_rule_types = 3;
    }
  else
    {
      nr_of_rule_types = NR_OF_RULE_TYPES;
    }
  //if((double)rand()/(double)RAND_MAX < MutProbRuleChange) {
  
  MutationCount++;
  int valueIndex = rand() % 15;
  
  switch (valueIndex) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    mutateWeight(MutValueChangePercent, nr_of_rule_types);
    break;
  case 7:
    mutateWindow(MutValueChangePercent);
    break;
  case 8:
    mutateWindowCenter(MutValueChangePercent);
    break;
  case 9:
    mutateDependentDose(MutValueChangePercent);
    break;
  case 10:
    mutateFixedDose(MutValueChangePercent);
    break;
  case 11:
    mutateIndex(&actuator_output_, Number_of_Actuators);
    break;
  case 12:
    mutateIndex(&sensor_input_, Number_of_Sensors);
    break;
  case 13:
    mutateIndex(&hormone_input_, Number_of_Hormones);
    break;
  case 14:
    mutateIndex(&hormone_output_, Number_of_Hormones);
    break;
    //  case 15:
    //mutateIndex(&actuator_status, Number_of_Actuators, MutValueChangePercent);
    //break;
    //case 16:
    //mutateIndex(&sensor_status, Number_of_Sensors, MutValueChangePercent);
				break;
				
  default:
    std::cout << "ERROR: reached default in switch in AHHS2Evolvable::Mutate()" << std::endl;
    exit(0);
    
  } // switch
	//} // if < MUT_PROP_RULE_CHANGE
  
  return MutationCount;
}

void AHHSRule::mutateFixedDose(double MutValueChangePercent) {
	double change = (double)rand()/(double)RAND_MAX*MutValueChangePercent*1.0/50.0;
	if((double)rand()/(double)RAND_MAX < 0.5)
		fixed_dose_ -= change;
	else
		fixed_dose_ += change;
	//if(fixed_dose < -1.0/50.0)
	//	fixed_dose = -1.0/50.0;
	//if(fixed_dose > 1.0/50.0)
	//	fixed_dose = 1.0/50.0;
}



  void AHHSRule::mutateWeight(double MutValueChangePercent, int nr_of_rule_types) {
	int index1;
	int index2;
	index1 = rand() % nr_of_rule_types;
	index2 = rand() % nr_of_rule_types;
	if(index2 == index1)
		index2++;
	if(index2 == nr_of_rule_types)
		index2=0;
	double change = rand() % (int)(MutValueChangePercent*100.0*1000.0);
	weights_[index1] -= (double)change/(100.0*1000.0);
	weights_[index2] += (double)change/(100.0*1000.0);
	if(weights_[index1] < 0.0) {
		weights_[index1] = 0.0;
		double sum = 0.0;
		for(int i=0;i<nr_of_rule_types;i++) {
			if(i != index2)
				sum += weights_[i];
		}
		weights_[index2] = 1.0 - sum;
	}
}


void AHHSRule::mutateWindow(double MutValueChangePercent) {
	double change = (double)rand()/(double)RAND_MAX*MutValueChangePercent*1.0;
	if((double)rand()/(double)RAND_MAX < 0.5) {
		window_ -= change;
		if(window_ < 0.0)
			window_ = 0.0;
	} else {
		window_ += change;
		if(window_ > 0.5)
		  window_ = 0.5;
	}
}

void AHHSRule::mutateDependentDose(double MutValueChangePercent) {
	double change = (double)rand()/(double)RAND_MAX*MutValueChangePercent*1.0;
	if((double)rand()/(double)RAND_MAX < 0.5)
		dependent_dose_ -= change;
	else
		dependent_dose_ += change;
	//if(dependent_dose < -1.0)
	//	dependent_dose = -1.0;
	//if(dependent_dose > 1.0)
	//	dependent_dose = 1.0;
}

void AHHSRule::mutateWindowCenter(double MutValueChangePercent) {
	double change = (double)rand()/(double)RAND_MAX*MutValueChangePercent*1.0;
	if((double)rand()/(double)RAND_MAX < 0.5)
		window_center_ -= change;
	else
		window_center_ += change;
	if(window_center_ < -1.0)
		window_center_ = -1.0;
	if(window_center_ > 1.0)
		window_center_ = 1.0;
}


void AHHSRule::mutateIndex(double *property, int MaxNumber) 
{
  //std::cout << "before=" << *property << " int: " << std::endl;

  double change = (double)rand()/(double)RAND_MAX*1.0;
  if((double)rand()/(double)RAND_MAX < 0.5) {
    *property -= change;
  }
  else {
    *property += change;
  }
   
  if(*property < 0.0) {  
    cout << "AHHSRule::mutateIndex(double ...) val: " << scientific << *property << fixed << " min: " <<  " 0.0 ";
    *property += (double)MaxNumber;
    cout << " => val set to: " << scientific << *property << fixed << endl;
  }
  if(*property >= (double)MaxNumber) {
    cout << "AHHSRule::mutateIndex(double ...) val: " << scientific << *property << fixed << " max: " << scientific <<  (double)MaxNumber << fixed << " int_max: " <<  MaxNumber;
    *property -= (double)MaxNumber;
    cout << " => val set to: " << scientific << *property << fixed << endl;
  }
  //std::cout << "after=" << *property << std::endl;
}






