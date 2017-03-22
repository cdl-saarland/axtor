/*
 * Serializer.h
 *
 *  Created on: 12.06.2010
 *      Author: Simon Moll
 */

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>
#include <llvm/PassSupport.h>

#include <axtor/backend/AxtorBackend.h>
#include <axtor/CommonTypes.h>

#include <axtor/ast/BasicNodes.h>

namespace llvm {
	class ScalarEvolution;
	class PHINode;
}
/*
 * This pass gets an AST from the ASTExtractor pass and serializes it using the backend specified by the TargetProvider pass
 */
namespace axtor
{

	struct LoopContext {
		llvm::BasicBlock * header;
		llvm::BasicBlock * loopExit;
		llvm::PHINode * phi;
		LoopContext()
		: header(nullptr)
		, loopExit(nullptr)
		, phi(nullptr)
		{}
	};

	class Serializer : public llvm::ModulePass
	{
	private:
		const llvm::ScalarEvolution * SE;
		void runOnFunction(AxtorBackend & backend, SyntaxWriter * modWriter, IdentifierScope & globals, ast::FunctionNode * funcNode);

	public:
		static char ID;
		Serializer() :
			llvm::ModulePass(ID)
		{}


		/*
		 * replace all resolved PHI-nodes by assignments except loopPHI
		 */
		void processBranch(SyntaxWriter * writer, llvm::BasicBlock * source, llvm::BasicBlock * target, IdentifierScope & locals, const LoopContext & LC);

		/*
		 * writes all instructions of a @bb except the terminator except those contained in @supresssedInsts
		 */
		void writeBlockInstructions(SyntaxWriter * writer, llvm::BasicBlock * bb, IdentifierScope & identifiers);

		/*
		 * creates identifier bindings for function arguments
		 */
		void createArgumentDeclarations(llvm::Function * func, ConstVariableMap & declares, std::set<llvm::Value*> & parameters);

		void getAnalysisUsage(llvm::AnalysisUsage & usage) const;
		bool runOnModule(llvm::Module & M);

                llvm::StringRef getPassName() const;

		/*
		 * writes a node using the given backend and writer.
		 * @return unique exiting block (if any)
		 */
		llvm::BasicBlock * writeNode(AxtorBackend & backend, SyntaxWriter * writer, llvm::BasicBlock * previousBlock, llvm::BasicBlock * exitBlock, ast::ControlNode * node, IdentifierScope & locals, const LoopContext & LC);
	};
}


#endif /* SERIALIZER_HPP_ */
