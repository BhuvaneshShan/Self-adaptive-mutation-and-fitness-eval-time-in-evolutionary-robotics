/*
 * $Id: EnergyCollection.h$
 *
 *  Created on: Apr 16, 2010
 *      Author: ehaasdi
 */

#ifndef ENERGYCOLLECTION_H_
#define ENERGYCOLLECTION_H_

#include <genetics/GeneticConfig.h>

namespace Genetics
{
	class EnergyCollection
	{
	public:
		EnergyCollection();
		virtual ~EnergyCollection();

		//Fitness function that rewards collecting energy from chargers
		LIOReal operator()(worldModel_t& wm);
	};
}

#endif /* ENERGYCOLLECTION_H_ */
