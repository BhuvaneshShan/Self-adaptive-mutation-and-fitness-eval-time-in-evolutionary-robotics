/*
 *  $Id: EMeans.cpp  $
 *
 *  Created on: 8 déc. 2009
 *      Author: delarbop
 *
 * Copyright (c) 2010
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <genetics/EMeans.h>
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace Genetics;

EMeans::EMeans(double epsilon, FitnessType fitnessType) :
	_epsilon(epsilon),
	nb_total(0),
	nb_i(0),
	qi_d(0),
	qi_s(0),
	_fitnessType(fitnessType)
{}

#ifndef UNITTEST
LIOReal EMeans::updateFitness(worldModel_t& wm, LIOReal currentFitness)
{ // WK:

	std::vector<double> point;
	// add all dsensors (irSensors)
	for (int i = 0; i < 8; ++i)
		point.push_back(wm.irSensor[i]);

	// add all lsensors (lightSensors)
	//for (int i = 0; i < 8; ++i)
	//	point.push_back(wm.lightSensor[i]);

	// add battery level
	// point.push_back(wm.battery);

	// add all colorActs
	for (int i = 0; i < 4; ++i)
		point.push_back(wm.colorAct[i]);

	// add all shapeActs
	for (int i = 0; i < 4; ++i)
		point.push_back(wm.shapeAct[i]);

	// add both motorspeeds
	for (int i = 0; i < 2; ++i)
		point.push_back(wm.velocity[i]);

	// add it.
	addPoint(point);

	return QI();
}

// finalizeFitness?
#endif


EMeans::EMeans(const EMeans &e)
{
	copy(e);
}

const EMeans& EMeans::copy(const EMeans & e)
{
	_epsilon = e._epsilon;
	nb_total = e.nb_total;
	nb_i = e.nb_i;

	repartition_d = e.repartition_d;
	centre = e.centre;
	_fitnessType = e._fitnessType;

	return *this;
}

void EMeans::addPoint(const std::vector<double> &point)
{
	int c = whichCluster(point);
	if( c == -1 )									// new cluster
	{
		centre.push_back(point);
		repartition_d.push_back(1);
		repartition_s.push_back(1);
		c=centre.size()-1;
	} else if(c >= 0 && c < (int)centre.size()) {	// add to existing cluster
		repartition_d[c]++;
		repartition_s[c]++;
	} else {
		throw std::logic_error("Cluster out of range in EMeans::addPoint()");
	}

	nb_total++;
	nb_i++;
	lastCluster=c;
}

void EMeans::init_simple(void)
{
	repartition_s.clear();
	repartition_s.assign(repartition_d.size(),0);
	nb_i=0;
}

void EMeans::restart(void)
{
	nb_i=0;
	nb_total=0;
	centre.clear();
	repartition_d.clear();
	repartition_s.clear();
}

int EMeans::whichCluster(const std::vector<double>& point)
{
    int ret=0;
    double minDistance = _epsilon;

    for(size_t i=0;  i < centre.size() ; i++)
    {
  	  double currentDistance = distance( point , centre[i] );
  	  if ( currentDistance <= minDistance )
  	  {
  		 minDistance = currentDistance;
  		 ret = (int)i;
  	  }
    }

    return (minDistance == _epsilon) ? -1 : ret;
}

double EMeans::distance(const std::vector<double> & x, const std::vector<double> & y) const
{
  double total = 0.0;

  if( x.size() != y.size()) return -1;

  std::vector<double>::const_iterator cpx=x.begin();
  std::vector<double>::const_iterator cpy=y.begin();
  for(;cpx!=x.end() ;++cpx,++cpy){
    double diff = *cpx - *cpy;
    total += (diff * diff);
  }
  //return total;  // no need to take sqrt, which is monotonic
  return sqrt(total);
}


double EMeans::QI_Gen(const std::vector<int> &repartition, int numPoints)
{
	double qi = 0;

	typedef std::vector<int>::const_iterator iterator;
	for (iterator i = repartition.begin(); i != repartition.end(); ++i)
	{
		if (*i != 0)
		{
			double fraction(*i);
			// FIXME: Pierre's original code used log() (i.e., ln), but this is easier to test
			qi -= ( fraction/ (double)numPoints) * log2(  fraction/(double)numPoints );
		}
	}

	return qi;
}


//double EMeans::KL(std::vector<int> &dist1, int t1, std::vector<int> &dist2 , int t2)
//{
//	double kl=0;
//
//	int v1,v2;
//
//	size_t maxL= std::max(dist1.size(), dist2.size());
//
//	for(size_t i = 0; i < maxL ; i++)
//	{
//		v1 = dist1.size() > i ? dist1[i] : 0 ;
//		v2 = dist2.size() > i ? dist2[i] : 0 ;
//
//		double p1=v1/(double)t1;
//		double p2=v2/(double)t2;
//
//		if( p1 != 0 && p2 != 0){
//			kl+= p1 * log(p1/p2);
//		}
//	}
//
//	return kl;
//}


double EMeans::QI()
{
	switch (_fitnessType)
	{
		case SIMP :
			qi_s = QI_Gen(repartition_s,nb_i);
//			std::cout << "QI : " << qi_s << std::endl;
			qi=qi_s;
			break;
		case DYN :
			qi_d = QI_Gen(repartition_d,nb_total);
			qi=qi_d;
			break;
		case DYN_SIMP :
			qi_d = QI_Gen(repartition_d,nb_total);
			qi_s = QI_Gen(repartition_s,repartition_s.size());
		//	qi=(1-alpha)*qi_s + alpha*qi_d;
			qi=qi_s + qi_d;

			break;
		default:
			throw std::logic_error("unexpected _fitnessType in EMeans::QI");
			// break;
	}

	return qi;
}


//---------------------------------------------------------------------------

#ifdef UNITTEST

#include <test/TestHarness.h>
// Test case
TEST( EMeans, clustering)
{
	double one[] = { 0.1, 0.1, 0.1, 0.1};
	double two[] = { 5.1, 5.1, 5.1, 5.1};

	Genetics::EMeans emean(0.4);
	for (int i = 0; i < 100; ++i)
		emean.addPoint(std::vector<double>(one, one + 4));

	int nrClusters = emean.getNbCluster();
	CHECK(nrClusters == 1);

	CHECK(emean.QI() == 0.0);

	for (int i = 0; i < 100; ++i)
		emean.addPoint(std::vector<double>(two, two + 4));

	CHECK(emean.getNbCluster() == 2);

	double qi = emean.QI();
	CHECK(qi == 1.0);

}

#endif // UNITTEST
