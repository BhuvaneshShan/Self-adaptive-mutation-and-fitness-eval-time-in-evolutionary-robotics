/*
 * $Id: CollectivePatrolling.h$
 *
 *  Created on: Jul 21, 2010
 *      Author: ehaasdi
 */

#ifndef COLLECTIVEPATROLLING_H_
#define COLLECTIVEPATROLLING_H_

#include <genetics/GeneticConfig.h>

namespace Genetics
{
class CollectivePatrolling
{
public:
	CollectivePatrolling();
	virtual ~CollectivePatrolling();

	//Fitness for collective patrolling task using pheromones. Calculates fitness by penalizing
	// the sensory input of pheromone.
	LIOReal operator()(worldModel_t& wm);
};

}

#endif /* COLLECTIVEPATROLLING_H_ */
