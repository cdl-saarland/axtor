/*
 * PredicateRestruct.h
 *
 *  Created on: 27 Feb 2012
 *      Author: v1smoll
 */

#ifndef PREDICATERESTRUCT_H_
#define PREDICATERESTRUCT_H_

#include "RestructuringProcedure.h"

#include <axtor/config.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/llvmDuplication.h>

namespace axtor
{
	/*
	 * Interface for restructuring procedures for acyclic control-flow
	 *
	 * (Implement as Singleton)
	 */

	class PredicateRestruct : public RestructuringProcedure
	{
	public:

		PredicateRestruct();
		~PredicateRestruct();
		/*
		 * converts the @regions into valid regions with respect to the single exit node criterion of acyclic abstract high-level nodes
		 * returns
		 * 		requiredExit (if defined) or a newly generated exit block otw. (if any)
		 */
		llvm::BasicBlock * resolve(RegionVector & regions, llvm::BasicBlock * requiredExit, const ExtractorContext & context, AnalysisStruct & analysis);
	};
}


#endif /* PREDICATERESTRUCT_H_ */
