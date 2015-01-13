/*
 * $Id: Chasing.h$
 *
 *  Created on: Apr 27, 2010
 *      Author: ehaasdi
 */

#ifdef CHASING

#ifndef CHASING_H_
#define CHASING_H_

#include <genetics/GeneticConfig.h>

namespace Genetics
{
	class Chasing
	{
	public:
		Chasing();
		virtual ~Chasing();

		//Fitness function that rewards closeness to the prey (as measured by the signal
		//	strength of the prey's emitter')
		LIOReal operator()(worldModel_t& wm);
	};
}

#endif /* CHASING_H_ */

#endif
