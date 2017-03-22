/*
 * RestructuringProcedure.h
 *
 *  Created on: 20.02.2011
 *      Author: Simon Moll
 */

#ifndef RESTRUCTURINGPROCEDURE_HPP_
#define RESTRUCTURINGPROCEDURE_HPP_

#include <axtor/CommonTypes.h>

#include <axtor/util/AnalysisStruct.h>
#include <axtor/util/ExtractorRegion.h>

namespace axtor
{
	/*
	 * Interface for restructuring procedures for acyclic control-flow
	 *
	 * (Implement as Singleton)
	 */

	class RestructuringProcedure
	{
	public:
		virtual ~RestructuringProcedure() {};
		/*
		 * converts the @regions into valid regions with respect to the single exit node criterion of acyclic abstract high-level nodes
		 * 	oExitBlock
		 * 		requiredExit (if defined) or a newly generated exit block otw. (if any)
		 * returns
		 * 		if the CFG was modified and needs to be reparsed
		 */
		virtual bool resolve(RegionVector & regions, llvm::BasicBlock * requiredExit, const ExtractorContext & context, AnalysisStruct & analysis, llvm::BasicBlock *& oExitBlock) = 0;
	};
}

#endif /* RESTRUCTURINGPROCEDURE_HPP_ */
