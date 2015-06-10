#include <axtor/parsers/LoopParser.h>

#include <axtor/console/CompilerLog.h>
#include <axtor/ast/BasicNodes.h>

#include <axtor/util/stringutil.h>
#include <axtor/util/llvmShortCuts.h>

#include <axtor/util/llvmLoop.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>

using namespace llvm;

namespace axtor {
	LoopParser LoopParser::instance;

	LoopParser::LoopBuilderSession::LoopBuilderSession(RegionVector regions, llvm::BasicBlock * entry, llvm::BasicBlock * requiredExit, ForLoopInfo * _forInfo)
	: BuilderSession(regions, entry, requiredExit)
	, forInfo(_forInfo)
	{}

	std::string LoopParser::LoopBuilderSession::getName() const
	{
		return "LoopBuilderSession";
	}

	RestructuringProcedure * LoopParser::LoopBuilderSession::getSolver() const
	{
		return NULL; //loop restructuring is currently done with the loop exit enumeration pass
	}

	ast::ControlNode * LoopParser::LoopBuilderSession::build(ast::NodeMap children, llvm::BasicBlock * exitBlock)
	{
		assert(children.size() == 1 && "expected a single loop body");

		ast::ControlNode * bodyNode = children[getEntryBlock()];

		if (forInfo) {
			return new ast::ForLoopNode(forInfo, bodyNode);
		} else {
			return new ast::LoopNode(bodyNode);
		}
	}

	void LoopParser::LoopBuilderSession::dump() {}

	PrimitiveParser::BuilderSession * LoopParser::tryParse(llvm::BasicBlock * entry, ExtractorContext context, AnalysisStruct & analysis)
	{
		llvm::Loop * loop = analysis.getLoopFor(entry);

		if (!loop || loop == context.parentLoop)
			return 0;

		BlockSet exits;
		getUniqueExitBlocks(*loop, exits);

		if (exits.size() > 1 && loop->getParentLoop())
		{
			for(BlockSet::iterator itExit = exits.begin(); itExit != exits.end(); ++itExit)
			{
				std::cerr << (*itExit)->getName().str() << ",\n";
			}

			Log::fail(entry, "(loop exits == " + str<int>(exits.size()) + " > 1) Use the loop exit enumeration pass to normalize inner loops");
		}

		ForLoopInfo * forInfo = new ForLoopInfo;

		BasicBlock * bodyBlock = nullptr;
		if (inferForLoop(loop, *forInfo)) {
			bodyBlock = forInfo->bodyBlock; // default to a while loop
		} else {
			delete forInfo;
			forInfo = nullptr;
			bodyBlock = entry; // default to a while loop
		}

		// move all destinations of a outer-most loop into its body
		llvm::BasicBlock * breakTarget = exits.size() == 1 ? *exits.begin() : 0;

		ExtractorContext bodyContext(context);
		bodyContext.parentLoop = loop;
		bodyContext.exitBlock = entry;
		bodyContext.continueBlock = entry;
		bodyContext.breakBlock = breakTarget;

		RegionVector regions;
		regions.push_back(ExtractorRegion(bodyBlock, bodyContext));

		return new LoopBuilderSession(regions, bodyBlock, breakTarget, forInfo);
	}

	LoopParser * LoopParser::getInstance()
	{
		return &instance;
	}
}
