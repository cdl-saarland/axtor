/*
 * LoopParser.h
 *
 *  Created on: 20.02.2011
 *      Author: Simon Moll
 */

#ifndef LOOPPARSER_HPP_
#define LOOPPARSER_HPP_

#include <axtor/parsers/PrimitiveParser.h>
#include <axtor/console/CompilerLog.h>

namespace llvm {
	class SCEVAddRecExpr;
}

namespace axtor {
	class ForLoopInfo;

	/*
	 * Infinite Loop Parser - will abort when applied to multi-exit loops (does not return a solver procedure)
	 *
	 * TODO abort on irreducible loops
	 */
	class LoopParser : public PrimitiveParser
	{
		static LoopParser instance;

	public:
		class LoopBuilderSession : public PrimitiveParser::BuilderSession
		{
			ForLoopInfo * forInfo;
		public:
			LoopBuilderSession(RegionVector regions, llvm::BasicBlock * entry, llvm::BasicBlock * requiredExit, ForLoopInfo * forInfo);

			virtual RestructuringProcedure * getSolver() const;

			virtual ast::ControlNode * build(ast::NodeMap children, llvm::BasicBlock * exitBlock);

			virtual std::string getName() const;

			virtual void dump();
		};

		virtual BuilderSession * tryParse(llvm::BasicBlock * entry, ExtractorContext context, AnalysisStruct & analysis);

		static LoopParser * getInstance();
	};
}


#endif /* LOOPPARSER_HPP_ */
