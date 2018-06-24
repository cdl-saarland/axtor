/*
 * Serializer.cpp
 *
 *  Created on: 12.06.2010
 *      Author: gnarf
 */

#include <axtor/pass/Serializer.h>

#include <axtor/pass/TargetProvider.h>

#include <axtor/config.h>
#include <axtor/util/llvmDebug.h>
#include <axtor/util/ResourceGuard.h>
#include <axtor/util/llvmTools.h>
#include <axtor/util/llvmLoop.h>
#include <axtor/pass/RestructuringPass.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>

using namespace llvm;

namespace axtor
{
	llvm::RegisterPass<Serializer> __regSerializer("serialize", "AST serialization pass", true, true);

	char Serializer::ID = 0;

	/*
	 * translates the instruction list contained in @bb
	 */
	void Serializer::writeBlockInstructions(SyntaxWriter * writer, llvm::BasicBlock * bb, IdentifierScope & identifiers)
	{
#ifdef DEBUG
		std::cerr << "### TRANSLATING BB : " << bb->getName().str() << std::endl;
#endif
		assert(bb && " must not be 0");
		llvm::BasicBlock::iterator it = bb->begin();

		assert(it != bb->end() && "encountered empty bb");

		for(;! llvm::isa<llvm::TerminatorInst>(it);++it)
		{
			llvm::Instruction & inst = *it;

			if (llvm::isa<llvm::GetElementPtrInst>(inst))
			{
				continue;
			}

			const VariableDesc * desc = identifiers.lookUp(&inst);

			if (! desc) {
				if (! isType(inst, llvm::Type::VoidTyID)) {
					Log::warn(&inst, "undeclared variable! (may cause problems if instruction result is not ignored)");
				}
				writer->writeInstruction(0, &inst, identifiers);
			} else {
				writer->writeInstruction(desc, &inst, identifiers);
			}
		};
	}

	void Serializer::processBranch(SyntaxWriter * writer, llvm::BasicBlock * source, llvm::BasicBlock * target, IdentifierScope & locals, const LoopContext & LC)
	{
		IF_DEBUG std::cerr << "### processing branches from " << source->getName().str() << " to " << target->getName().str() << "\n";

		for(llvm::BasicBlock::iterator inst = target->begin(); llvm::isa<llvm::PHINode>(inst); ++inst)
		{
			llvm::PHINode * phi = cast<llvm::PHINode>(inst);
			if (phi == LC.phi)
				continue;

			writer->writePHIAssign(*phi, source, locals);
		}
	}


	/*
	 * create variable bindings for function arguments
	 */
	void Serializer::createArgumentDeclarations(llvm::Function * func, ConstVariableMap & declares, std::set<llvm::Value*> & parameters)
	{
		for (Argument & arg : func->args())
		{
			//const llvm::Type * argType = type->getParamType(i);

			std::string argName = arg.getName();

			declares[&arg] = VariableDesc(&arg, argName);
			parameters.insert(&arg);
		}
	}

	llvm::BasicBlock * Serializer::writeNode(AxtorBackend & backend, SyntaxWriter * writer, llvm::BasicBlock * previousBlock, llvm::BasicBlock * exitBlock, ast::ControlNode * node, IdentifierScope & locals, const LoopContext & LC)
	{
		typedef ast::ControlNode::NodeVector NodeVector;

		BasicBlock * loopHeader = LC.header;
		BasicBlock * loopExit = LC.loopExit;

		llvm::BasicBlock * block = node->getBlock();
		IF_DEBUG llvm::errs() << "processing " << (block ? block->getName() : "none") << " exit : " << (exitBlock ? exitBlock->getName() : "null") << "\n";


		{
			switch (node->getType())
			{
				case ast::ControlNode::IF: {
					writeBlockInstructions(writer, block, locals);

					ast::ConditionalNode * condNode = reinterpret_cast<ast::ConditionalNode*>(node);
					llvm::Value * condition = condNode->getCondition();

					//determine properties
					bool hasElse = condNode->getOnTrue() && condNode->getOnFalse();
					bool negateCondition = !hasElse && !condNode->getOnTrue() && condNode->getOnFalse();

					ast::ControlNode * consNode;
					ast::ControlNode * altNode;
					llvm::BasicBlock * consBlock;
					llvm::BasicBlock * altBlock;
					if (negateCondition)
					{
						consNode = condNode->getOnFalse();
						altNode = condNode->getOnTrue();
						consBlock = condNode->getOnFalseBlock();
						altBlock = condNode->getOnTrueBlock();
					} else {
						consNode = condNode->getOnTrue();
						altNode = condNode->getOnFalse();
						consBlock = condNode->getOnTrueBlock();
						altBlock = condNode->getOnFalseBlock();
					}

					//write down
					//## consequence ##
					writer->writeIf(condition, negateCondition, locals);

					SyntaxWriter * consWriter = backend.createBlockWriter(writer);
						processBranch(consWriter, block, consBlock, locals, LC);
						writeNode(backend, consWriter, block, exitBlock, consNode, locals, LC);
					delete consWriter;

					//## alternative ##
					if (hasElse) { //real alternative case
						writer->writeElse();
						SyntaxWriter * altWriter = backend.createBlockWriter(writer);

						processBranch(altWriter, block, altBlock, locals, LC);

						llvm::BasicBlock * unprocessed = writeNode(backend, altWriter, block, exitBlock, altNode, locals, LC);
						if (unprocessed)
						{
							processBranch(altWriter, unprocessed, exitBlock, locals, LC);
						}

						delete altWriter;

					} else if (containsPHINodes(exitBlock)) { // alternative only contains PHI-assignments
						writer->writeElse();
						SyntaxWriter * altWriter = backend.createBlockWriter(writer);

						processBranch(altWriter, block, exitBlock, locals, LC);

						delete altWriter;
					}

					return 0;
				}

				case ast::ControlNode::FOR: {
					auto * forNode = static_cast<ast::ForLoopNode*>(node);
					ForLoopInfo * forInfo = forNode->getForLoopInfo();

					writer->writeForLoopBegin(*forInfo, locals);
						SyntaxWriter * bodyWriter = backend.createBlockWriter(writer);
						LoopContext nestedLC(LC);
						nestedLC.header = node->getEntryBlock();
						nestedLC.loopExit = exitBlock;
						nestedLC.phi = forInfo->phi;

						writeNode(backend, bodyWriter, previousBlock, node->getEntryBlock(), node->getNode(ast::LoopNode::BODY), locals, nestedLC);
						delete bodyWriter;
					// writer->writeForLoopEnd();
					return 0;
				}

				case ast::ControlNode::LOOP: {
					writer->writeInfiniteLoopBegin();
						SyntaxWriter * bodyWriter = backend.createBlockWriter(writer);

						LoopContext nestedLC(LC);
						nestedLC.header = node->getEntryBlock();
						nestedLC.loopExit = exitBlock;
						nestedLC.phi = nullptr;

						writeNode(backend, bodyWriter, previousBlock, node->getEntryBlock(), node->getNode(ast::LoopNode::BODY), locals, nestedLC);
						delete bodyWriter;
					writer->writeInfiniteLoopEnd();
					return 0;
				}

				case ast::ControlNode::BREAK: {
					if (block) {
						writeBlockInstructions(writer, block, locals);
						processBranch(writer, block, loopExit, locals, LC);
					} else {
						processBranch(writer, previousBlock, loopExit, locals, LC);
					}
					writer->writeLoopBreak();

					return 0;
				}

				case ast::ControlNode::CONTINUE: {
					assert(loopHeader && "continue outside of loop");

					if (block) {
						writeBlockInstructions(writer, block, locals);
						processBranch(writer, block, loopHeader, locals, LC);
					} else {
						processBranch(writer, previousBlock, loopHeader, locals, LC);
					}
					writer->writeLoopContinue();

					return 0;
				}

				case ast::ControlNode::LIST: {
					llvm::BasicBlock * childPrevBlock = previousBlock;

					for(NodeVector::const_iterator itNode = node->begin(); itNode != node->end() ; ++itNode)
					{
						ast::ControlNode * child = *itNode;

						llvm::BasicBlock * nextExitBlock;

						// last block case
						if (((itNode + 1) == node->end())) {
							nextExitBlock = exitBlock;

						// last block before empty block case (CONTINUE, BREAK)
						} else if (
								(itNode + 2) == node->end() &&
								(*(itNode + 1))->getEntryBlock() == 0
						) {
							ast::ControlNode * nextNode = *(itNode + 1);
							// TODO clean this up
							switch (nextNode->getType())
							{
							case ast::ControlNode::CONTINUE:
								assert(loopHeader && "can not CONTINUE w/o loop header");
								nextExitBlock = loopHeader; break;

							case ast::ControlNode::BREAK:
								assert(loopExit && "can not BREAK w/o loop exit");
								nextExitBlock = loopExit; break;

							default:
								nextExitBlock = exitBlock; break;
							}

						// inner node case
						} else {
							nextExitBlock =  (*(itNode + 1))->getEntryBlock();
						}

						childPrevBlock = writeNode(backend, writer, childPrevBlock, nextExitBlock, child, locals, LC);
					}

					return childPrevBlock;
				}

				case ast::ControlNode::BLOCK: {
					writeBlockInstructions(writer, block, locals);
					processBranch(writer, block, exitBlock, locals, LC);

					return block;
				}

				case ast::ControlNode::RETURN: {
					ast::ReturnNode * returnNode = reinterpret_cast<ast::ReturnNode*>(node);

					writeBlockInstructions(writer, block, locals);
					writer->writeReturnInst(returnNode->getReturn(), locals);
					return 0;
				}

				case ast::ControlNode::UNREACHABLE: {
					writeBlockInstructions(writer, block, locals);
					return 0;
				}

				default:
					Log::fail(block, "unsupported node type");
			}
		}
	}

	void Serializer::runOnFunction(AxtorBackend & backend, SyntaxWriter * modWriter, IdentifierScope & globals, ast::FunctionNode * funcNode)
	{
		ValueSet parameters;
		IdentifierScope locals(&globals);
		llvm::Function * func = funcNode->getFunction();

		SE = &getAnalysis<ScalarEvolutionWrapperPass>(*func).getSE();


		//### bind function parameters ###
		createArgumentDeclarations(func, locals.identifiers, parameters);

		//### create local identifiers ###
		for(BasicBlock & block : *func) {
			for(Instruction & inst : block)
			{
				VariableDesc desc(&inst, inst.getName());

				if (! isType(inst, llvm::Type::VoidTyID) &&
						! llvm::isa<llvm::GetElementPtrInst>(&inst)) //no pointer support: only indirectly required by value useds
				{
					//dont overwrite preceeding mappings by PHI-Nodes
					if (locals.lookUp(&inst) == 0 && backend.requiresDesignator(&inst))
						locals.bind(&inst, desc);
				}
			}
		}


		//print out a list of declarations (single occurrence,not a parameter)
		//### write function body ###
		{
			SyntaxWriter * funcWriter = backend.createFunctionWriter(modWriter, func);
			ResourceGuard<SyntaxWriter> __guardFuncWriter(funcWriter);

			funcWriter->writeFunctionHeader(func, &locals);
			{
				SyntaxWriter * bodyWriter = backend.createBlockWriter(funcWriter);
				ResourceGuard<SyntaxWriter> __guardBodyWriter(bodyWriter);

				bodyWriter->writeFunctionPrologue(func, locals);

				//### translate instructions ###
#ifdef DEBUG
				std::cerr <<  "##### translating instructions of func " << func->getName().str() << "\n";
#endif

				ast::ControlNode * body = funcNode->getEntry();
				LoopContext LC;
				writeNode(backend, bodyWriter,0, 0, body, locals, LC);
			}
		}
	}

	bool Serializer::runOnModule(llvm::Module & M)
	{
#ifdef DEBUG_PASSRUN
		std::cerr << "\n\n##### PASS: Serializer #####\n\n";
#endif
#ifdef DEBUG_DUMP_MODULES
		writeModuleToFile(&M, "Dump_serializer_pre.bc");
#endif

		TargetProvider & target = getAnalysis<TargetProvider>();
		AxtorBackend & backend = target.getBackend();
		ModuleInfo & modInfo = target.getModuleInfo();

		//not our module -> return
		if (! modInfo.isTargetModule(&M)) {
			return false;
		}

#ifdef DEBUG
		modInfo.dump();
		modInfo.verifyModule();
#endif

		IdentifierScope globalScope = modInfo.createGlobalBindings();

		SyntaxWriter * modWriter = backend.createModuleWriter(modInfo, globalScope);

		const ast::ASTMap & ASTs = getAnalysis<RestructuringPass>().getASTs();

		for(Function & func : M)
		{
			if (! func.isDeclaration())
			{
				ast::FunctionNode * funcNode = ASTs.at(&func);
#ifdef DEBUG
				funcNode->dump();
				func.dump();
#endif
				runOnFunction(backend, modWriter, globalScope, funcNode);
			}
		}

		delete modWriter;

		return false;
	}

	void Serializer::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
		usage.addRequired<ScalarEvolutionWrapperPass>();
		//usage.addRequired<OpaqueTypeRenamer>();
		usage.addRequired<RestructuringPass>();
		usage.addRequired<TargetProvider>();

		usage.setPreservesAll();
	}

        llvm::StringRef Serializer::getPassName() const
	{
		return "axtor - serializer";
	}
}
