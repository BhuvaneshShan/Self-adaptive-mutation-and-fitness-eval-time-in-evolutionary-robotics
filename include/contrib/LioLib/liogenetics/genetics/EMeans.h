/*
 *  $Id: EMeans.h  $
 *
 *  Created on: 8 Dec. 2009
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

#ifndef EMEANS_H_
#define EMEANS_H_

#include "genetics/GeneticConfig.h"
#include <vector>

namespace Genetics
{

/**
 * \addtogroup Genetics
 * @{
 */

/**
 * Everything is based on epsilon-means clustering:
 * There is a class EMeans, curiosity is in the methods of this class, EMeans.cpp (first file below).
 *
 * The idea is: the robot is running. Online, the sensor vector x is sent to routine which-cluster:
 *
 *	int EMeans::whichCluster(std::vector<double> &point)
 *
 * is there a cluster C_i, the representative x_i of which is such that ||x_i - x|| < epsilon ?
 *
 * If yes, increase the number of people in C_i;
 * else, create a new cluster with x as representative.
 * at the end of the robot life, compute the quantity of information: routine QI_Gen.
 *
 * Does this help ? Please ask if this is not clear - and thanks in advance for your feedback.
 *
 * All the best, Michele and Pierre
 */
class EMeans {
public:
	enum FitnessType { SIMP, DYN, DYN_SIMP };

	/**
	 * Vanilla constructor.
	 *
	 * @param epsilon Defaults to 0.5: from an email conversation with Pierre: "Epsilon doesn't
	 * seems really to influence the performance (if it's not so small or not so high) . But it
	 * influences the number of centers. So i advise you to take a not too small Epsilon ( from
	 * 0.4 to 0.6, suppose your data are normalized in  [ -1, 1 ] ) , to keep a reasonable number
	 * of centers.
	 * @param fitnessType defaults to SIMP
	 */
	EMeans(double epsilon = 0.5, FitnessType fitnessType = SIMP);
	EMeans(const EMeans & e);
	virtual ~EMeans() {};

	const EMeans& operator=(const EMeans& rhs) { return copy(rhs); }

	double getQI(void) const { return qi;}
	double getQIS(void) const { return qi_s;}
	double getQID(void) const { return qi;}

	int getNbCluster(void) const {return repartition_d.size();}
	const std::vector<int>& getRepartitionS(void) const { return repartition_s;  }
//	int getNbPointsI(void) const { return nb_i; }
//	int getNbPointsT(void) const { return nb_total; }
//	int getLastCluster(void) const { return lastCluster;}

	void init_simple(void);
	void restart(void);

	void addPoint(const std::vector<double> &point);

	/**
	 * Calculate and store Qi fitness
	 */
	double QI(void);

#ifndef UNITTEST
	LIOReal updateFitness(worldModel_t& wm, LIOReal currentFitness) ;
	LIOReal finaliseFitness(LIOReal fitness) { return fitness; }
#endif

private:
	/**
	 * Calculate QI
	 *
	 * @param repartition Vector of incidence counts
	 * @param nbp Total number of points
	 * @return The entropy of the observations
	 */
	double QI_Gen(const std::vector<int> &repartition, int nbp);
	const EMeans& copy(const EMeans & e);

	static const double alpha=0.6;

	double _epsilon;
	int nb_total;
	int nb_i;

	double qi_d;
	double qi_s;

	double qi;

	FitnessType _fitnessType;

	std::vector<std::vector<double> > centre;
	std::vector<int> repartition_d;
	std::vector<int> repartition_s;

	int lastCluster;

	/**
	 * Find closest cluster to \a point if that distance is less than \c _epsilon.
	 * @param point
	 * @return The index of the closest cluster, or -1 if the distance to the closest cluster is larger or equal to \c _epsilon.
	 */
	int whichCluster(const std::vector<double> &point);

	double distance(const std::vector<double> & x, const std::vector<double> & ) const;
};

///@}

}

#endif /* EMEANS_H_ */
