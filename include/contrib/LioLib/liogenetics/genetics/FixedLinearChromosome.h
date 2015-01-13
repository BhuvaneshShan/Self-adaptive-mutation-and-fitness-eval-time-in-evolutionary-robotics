/*
 *  $Id: FixedLinearChromosome.h 1338 2009-07-31 14:33:01Z evert $
 *
 *  Created on: Feb 19, 2009
 *      Author: ehaasdi
 *
 * Copyright (c) 2009, VU University Amsterdam
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

#ifndef FIXEDLINEARCHROMOSOME_H_
#define FIXEDLINEARCHROMOSOME_H_

#include <genetics/GeneticConfig.h>

#include <algorithm>

namespace Genetics
{
/** \addtogroup Genetics
 * @{
 */

/**
 *  Linear, fixed size genome. Similar funcionality is provdided -much more
 *  extensively- by boost::array, but to keep this library self-contained, we
 *  provide this version.
 *
 *  Method names and functionalities conform to STL.
 *
 *  @tparam __Gene The type of a gene, typically Fixed32, but \c int, \c bool,
 *  \c char all work.
 *
 *  @todo The length of the genome should be a template parameter, but that
 *  leads to issues in classes that depend on this one.
 */
template<class __Gene = Fixed32> class FixedLinearChromosome  {
public :
	/// Default constructor
	FixedLinearChromosome(void) {};

	/// Copy constructor
	FixedLinearChromosome(const FixedLinearChromosome< __Gene > & rhs)
	{
		std::copy(rhs.begin(), rhs.end(), begin());
	}

	/// Assignment operator
	FixedLinearChromosome& operator=(const FixedLinearChromosome< __Gene > & rhs)
	{
		std::copy(rhs.begin(), rhs.end(), begin());
		return *this;
	}

	/// @return The size of the array of chromosomes
	unsigned size(void) const { return GENECOUNT; }

	// todo: copy constructor & assignment, equality...
	// todo: chromosome length as template param.

	/// @return An iterator pointing to the beginning of the array.
	__Gene* begin(void) { return _content; }
	/// @return An iterator pointing to the beginning of the vector.
	__Gene* end(void) { return _content + GENECOUNT; }

	/// @return A const iterator pointing to the beginning of the array.
	const __Gene* begin(void) const { return _content; }
	/// @return A const iterator pointing to the beginning of the vector.
	const __Gene* end(void) const { return _content + GENECOUNT; }
	/// @return The n'th element.
	__Gene& operator[](int n) { return _content[n]; }
	/// @return The n'th element.
	__Gene operator[](int n) const { return _content[n]; }

protected:
	/// The underlying array of genes
	__Gene _content [GENECOUNT];
};

// Boost version of the above:
//#include <boost/array.hpp>
//template <typename Gene> class FixedLinearChromosome :
//	public boost::array<Gene, GENECOUNT>
//{};

///@}

}

#endif /* FIXEDLINEARCHROMOSOME_H_ */
