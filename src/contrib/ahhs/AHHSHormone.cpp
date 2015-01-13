/**
 * HormoneClass.cpp
 *
*  Created on: 23.07.2009
 * Author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 */
#include "ahhs/AHHSHormone.h"

AHHSHormone::AHHSHormone() 
{
  base_emission_rate_ = 0.0;
  decay_rate_ = 0.0;
  diffusion_coefficient_ = 0.0;
  min_value_ = 0.0;
  max_value_ = 1.0;  
}

AHHSHormone::AHHSHormone(AHHSHormone *src) 
{
  base_emission_rate_ = src->base_emission_rate_;
  decay_rate_ = src->decay_rate_;
  diffusion_coefficient_ = src->diffusion_coefficient_;
  min_value_ = src->min_value_;
  max_value_ = src->max_value_;  
}

AHHSHormone::~AHHSHormone()
{
  //std::cout << "AHHSHormone::~AHHSHormone(). dead" << std::endl;
}

double AHHSHormone::getBaseEmissionRate()
{
  return base_emission_rate_;
}

double AHHSHormone::getDecayRate()
{
  return decay_rate_;
}    

double AHHSHormone::getDiffusionCoefficient()
{
  return diffusion_coefficient_;
}

double AHHSHormone::getMinValue()
{
  return min_value_;
}    

double AHHSHormone::getMaxValue()
{
  return max_value_;
}    

void AHHSHormone::setBaseEmissionRate(double base_emission_rate)
{
  base_emission_rate_ = base_emission_rate;
}

void AHHSHormone::setDecayRate(double decay_rate)
{
  decay_rate_ = decay_rate;
}

void AHHSHormone::setDiffusionCoefficient(double diffusion_coefficient)
{
  diffusion_coefficient_ = diffusion_coefficient;
}

void AHHSHormone::setMinValue(double min)
{
  min_value_ = min;
}

void AHHSHormone::setMaxValue(double max)
{
  max_value_ = max;
}

void AHHSHormone::randomInit() 
{
   base_emission_rate_ = (double)rand()/(double)RAND_MAX * 0.1;
   decay_rate_ = (double)rand()/(double)RAND_MAX * 0.1;
   diffusion_coefficient_ = (double)rand()/(double)RAND_MAX * 0.1;
}


void AHHSHormone::handCodedInit(int hormone_number) 
{
  if(hormone_number==0) {
    base_emission_rate_ = 0.0;
    decay_rate_ = 0.01; // 0.3
    diffusion_coefficient_ = 0.05; //0.1
  } else if(hormone_number==1) {
    base_emission_rate_ = 0.5;
    decay_rate_ = 0.0;
    diffusion_coefficient_ = 0.0;
  }
}

void AHHSHormone::logging(std::ofstream &File) 
{
  File << std::scientific << std::setprecision(std::numeric_limits<double>::digits10 + 1) << base_emission_rate_ << "; ";
  File << std::scientific << std::setprecision(std::numeric_limits<double>::digits10 + 1) << decay_rate_ << "; ";
  File << std::scientific << std::setprecision(std::numeric_limits<double>::digits10 + 1) << diffusion_coefficient_ << "; ";
  File << std::scientific << std::setprecision(std::numeric_limits<double>::digits10 + 1) << min_value_ << "; ";
  File << std::scientific << std::setprecision(std::numeric_limits<double>::digits10 + 1) << max_value_;
  File << std::endl;
}

void AHHSHormone::readFromFile(std::string line) 
{
  std::string item;
  std::istringstream linestream(line);

  unsigned int field_counter = 0;
  while (getline (linestream, item, ';')) {
    double value = atof(item.c_str());
    switch (field_counter) {
    case 0:
      base_emission_rate_ = value;
      break;
    case 1:
      decay_rate_ = value;
      break;
    case 2:
      diffusion_coefficient_ = value;
      break;
    case 3:
      min_value_ = value;
      break;
    case 4:
      max_value_ = value;
      break;
    default:
      std::cout << "AHHSHormone::readFromFile: Too many fields in hormone definition file. Please check. Aborting.\n";
      exit(1);
      break;
    }
    //cout << "field_counter: " << field_counter << ": " << value << endl;
    field_counter++;
  }
  // fieldcounter is 1 greater than it should be
  if (field_counter != 5) {
    std::cout << "Hormone::readFromFile: Not enough fields in hormone definition file. Please check. Aborting.\n";
    exit(1);
  }
}

int AHHSHormone::mutate(double MutProbHormoneChange, double MutValueChangePercent) {
#if IZG_DEBUG_MESSAGES
std::cout << "Hormone::Mutate()" << std::endl;
#endif
 
 int MutationCount = 0;
 int valueIndex;
 double change;
 
 if((double)rand()/(double)RAND_MAX < MutProbHormoneChange) {
   change = (double)rand()/(double)RAND_MAX * MutValueChangePercent;
   MutationCount++;
   valueIndex = rand() % 2;
   
   switch (valueIndex) {
   case 0:
     if((double)rand()/(double)RAND_MAX < 0.5) {
       decay_rate_ -= change;
       if(decay_rate_ < 0)
	 decay_rate_ = 0;
     } else {
       decay_rate_ += change;
       if(decay_rate_ > 1.0)
	 decay_rate_ = 1.0;
     }
     break;
   case 1:
     if((double)rand()/(double)RAND_MAX < 0.5) {
       diffusion_coefficient_ -= change;
       if(diffusion_coefficient_ < 0.0)
	 diffusion_coefficient_ = 0.0;
     } else {
       diffusion_coefficient_ += change;
       if(diffusion_coefficient_> 1.0)
	 diffusion_coefficient_ = 1.0;
     }
     break;
   case 2:
     if((double)rand()/(double)RAND_MAX < 0.5) {
       base_emission_rate_ -= change;
       if(base_emission_rate_ < 0.0)
	 base_emission_rate_ = 0.0;
     } else {
       base_emission_rate_ += change;
       if(base_emission_rate_ > 1.0)
	 base_emission_rate_ = 1.0;
     }
     break;
   default:
     std::cout << "ERROR: reached default in switch in Hormone::Mutate()" << std::endl;
     exit(0);
     
   } // switch value index
   
 } // if < MUT_PROP_HORMONE_CHANGE
 
 return MutationCount;
}

int AHHSHormone::mutateExactOne(double MutProbHormoneChange, double MutValueChangePercent) {
#if IZG_DEBUG_MESSAGES
  std::cout << "Hormone::Mutate()" << std::endl;
#endif
  
  int MutationCount = 0;
  int valueIndex;
  double change;
  
  change = (double)rand()/(double)RAND_MAX * MutValueChangePercent;
  MutationCount++;
  valueIndex = rand() % 2;
  
  switch (valueIndex) {
  case 0:
    if((double)rand()/(double)RAND_MAX < 0.5) {
      decay_rate_ -= change;
      if(decay_rate_ < 0)
	decay_rate_ = 0;
    } else {
      decay_rate_ += change;
      if(decay_rate_ > 1.0)
	decay_rate_ = 1.0;
    }
    break;
  case 1:
    if((double)rand()/(double)RAND_MAX < 0.5) {
       diffusion_coefficient_ -= change;
       if(diffusion_coefficient_ < 0.0)
	 diffusion_coefficient_ = 0.0;
    } 
    else {
      diffusion_coefficient_ += change;
      if(diffusion_coefficient_> 1.0)
	diffusion_coefficient_ = 1.0;
    }
    break;
  case 2:
    if((double)rand()/(double)RAND_MAX < 0.5) {
      base_emission_rate_ -= change;
      if(base_emission_rate_ < 0.0)
	base_emission_rate_ = 0.0;
    } 
    else {
      base_emission_rate_ += change;
      if(base_emission_rate_ > 1.0)
	base_emission_rate_ = 1.0;
    }
    break;
  default:
    std::cout << "ERROR: reached default in switch in Hormone::Mutate()" << std::endl;
    exit(0);
    
  } // switch value index
  return MutationCount;
}

