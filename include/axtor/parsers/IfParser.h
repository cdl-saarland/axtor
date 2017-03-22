/*
 * IfParser.h
 *
 *  Created on: 20.02.2011
 *      Author: Simon Moll
 */

#ifndef IFPARSER_HPP_
#define IFPARSER_HPP_

#include "PrimitiveParser.h"
#include <axtor/ast/BasicNodes.h>
#include <axtor/util/llvmShortCuts.h>

#include <axtor/solvers/NodeSplittingRestruct.h>

namespace axtor {
	/*
	 * combined IF and IF..ELSE parser class
	 *
	 * tryParse will return IF of IF..ELSE Builder Sessions depending on what kind of control-flow pattern was detected
	 *
	 * uses the Node-Splitting Solver procedure
	 */
	class IfParser : public PrimitiveParser
	{
		static IfParser instance;

	public:
		/*
		 * IF - builder (w/o ELSE)
		 */
		class IfBuilderSession : public PrimitiveParser::BuilderSession
		{
		public:
			IfBuilderSession(RegionVector regions, llvm::BasicBlock * exitBlock, llvm::BasicBlock * entryBlock);

			ast::ControlNode * build(ast::NodeMap children, llvm::BasicBlock * exitBlock);

			RestructuringProcedure * getSolver() const;

			virtual std::string getName() const;

			virtual void dump();
		};

		/*
		 * IF..ELSE builder
		 */
		class IfElseBuilderSession : public PrimitiveParser::BuilderSession
		{
		public:
			IfElseBuilderSession(RegionVector regions, llvm::BasicBlock * exitBlock, llvm::BasicBlock * entryBlock);

			ast::ControlNode * build(ast::NodeMap children, llvm::BasicBlock * exitBlock);

			RestructuringProcedure * getSolver() const;

			virtual std::string getName() const;

			virtual void dump();
		};

		/*
		 * tries to set up a builder session for IF primitives and falls back to IF..ELSE
		 */
		virtual BuilderSession * tryParse(llvm::BasicBlock * entry, ExtractorContext context, AnalysisStruct & analysis);

		static IfParser * getInstance();
	};
}

#endif /* IFPARSER_HPP_ */
