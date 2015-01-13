/*
 * $Id: Chasing.cpp$
 *
 *  Created on: Apr 27, 2010
 *      Author: ehaasdi
 */
#ifdef CHASING

#include <genetics/Chasing.h>

namespace Genetics
{
	Chasing::Chasing(){}
	Chasing::~Chasing(){}

	LIOReal Chasing::operator()(worldModel_t& wm)
	{
		return LIOReal(wm.signalStrength);
	}
}

#endif
