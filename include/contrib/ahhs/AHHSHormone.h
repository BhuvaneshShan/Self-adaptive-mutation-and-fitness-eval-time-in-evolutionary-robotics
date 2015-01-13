/** HormoneClass.h
 * Definitions of the rules.
 *
 *  @date: 23.07.2009
 *  @author: Heiko Hamann, Jürgen Stradner, Thomas Schmickl
*/

#ifndef AHHSHORMONE_H_
#define AHHSHORMONE_H_


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <limits>

using namespace std;

#define NR_OF_VARIABLES_IN_HORMONE 5

class AHHSHormone
{
private:
  double base_emission_rate_;
  double decay_rate_;
  double diffusion_coefficient_;
  double min_value_;
  double max_value_;

public:
  AHHSHormone();
  AHHSHormone(AHHSHormone *src);
  virtual ~AHHSHormone();
  
  double getBaseEmissionRate();
  double getDecayRate();
  double getDiffusionCoefficient();
  double getMinValue();
  double getMaxValue();

  void setBaseEmissionRate(double base_emission_rate);
  void setDecayRate(double decay_rate);
  void setDiffusionCoefficient(double diffusion_coefficient);
  void setMinValue(double min);
  void setMaxValue(double max);

  void randomInit();
  void handCodedInit(int hormone_number);
  void logging(std::ofstream &File);
  void readFromFile(std::string line);
  int mutate(double MutProbHormoneChange, double MutValueChangePercent);
  int mutateExactOne(double MutProbHormoneChange, double MutValueChangePercent);
};

#endif /* AHHSHORMONE_H_ */
