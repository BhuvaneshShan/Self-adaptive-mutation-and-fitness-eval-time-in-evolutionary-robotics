/*
 * $Id: CollectiveForaging.h$
 *
 *  Created on: Aug 5, 2010
 *      Author: ehaasdi
 */

#ifndef COLLECTIVEFORAGING_H_
#define COLLECTIVEFORAGING_H_

#include <genetics/GeneticConfig.h>

namespace Genetics
{
	class CollectiveForaging
	{
	public:
		CollectiveForaging();
		~CollectiveForaging();

		LIOReal operator()(worldModel_t &wm);
	};
}

#endif /* COLLECTIVEFORAGING_H_ */
