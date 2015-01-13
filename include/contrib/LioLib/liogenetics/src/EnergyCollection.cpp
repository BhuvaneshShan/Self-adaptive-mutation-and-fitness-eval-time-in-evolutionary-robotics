/*
 * $Id: EnergyCollection.cpp$
 *
 *  Created on: Apr 16, 2010
 *      Author: ehaasdi
 */

#ifdef ENERGY_COLLECTION

#include <genetics/EnergyCollection.h>

namespace Genetics
{
	EnergyCollection::EnergyCollection()
	{
	}

	EnergyCollection::~EnergyCollection()
	{
	}

	LIOReal EnergyCollection::operator()(worldModel_t& wm)
	{
		//Fitness is the gain in battery energy.
		//Since the selection method is tournament scalar differences between
		//fitness levels are not important (only the ordering matters).
		return LIOReal(wm.battery - wm.prevBattery);
	}
}

#endif
