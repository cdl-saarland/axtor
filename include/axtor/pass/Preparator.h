/*
 * preparator.h
 *
 *  Created on: 12.02.2010
 *      Author: Simon Moll
 */

#ifndef PREPARATOR_HPP_
#define PREPARATOR_HPP_

#include <axtor/config.h>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Analysis/Passes.h>

#include <axtor/util/stringutil.h>
#include <axtor/util/llvmShortCuts.h>

#include <axtor/writer/SyntaxWriter.h>
#include <axtor/CommonTypes.h>

#include <axtor/pass/ExitUnificationPass.h>

/*
 * This pass assigns names to all types and instructions in the module. Opaque Type names are preserved.
 * Also it replaces constants in instructions by uses of fake assignment instructions
 */
namespace axtor
{
	class Preparator : public llvm::ModulePass
	{
	private:
		/*
		 * names all unnamed values in a function
		 */
		void nameAllInstructions(llvm::Function * func);

		void nameAllInstructions(llvm::Module * mod);

		/*
		 * convert all constant arguments of PHI-Nodes to instructions in the entry block
		 */

		llvm::Instruction * createAssignment(llvm::Value * val, llvm::Instruction * beforeInst);

		llvm::Instruction * createAssignment(llvm::Value * val, llvm::BasicBlock * block);

		void transformStoreConstants(llvm::StoreInst * store);

		struct AssignmentBridge
		{
			llvm::BasicBlock * bridge;
			llvm::Instruction * value;

			AssignmentBridge(llvm::BasicBlock * _bridge, llvm::Instruction * _value);
		};

		AssignmentBridge insertBridgeForBlock(llvm::Instruction * inst, llvm::BasicBlock * branchTarget);

		/*
		 * transforms PHINodes into a standardized format where
		 * no phi uses neither PHI-Nodes nor constants immediately
		 */
		void transformPHINode(llvm::PHINode * phi);

		/*
		 * calls specific instruction transform methods
		 * (for stores and PHIs so far)
		 */
		void transformInstruction(llvm::Instruction * inst);

		/*
		 * transforms all instructions occuring in a module if necessary
		 */
		void transformInstArguments(llvm::Module * mod);

		/*
		 * inserts all constituent struct types of @type into @types
		 */
		void insertIfStruct(TypeSet & types, const llvm::Type * type);

		/*
		 * give a generic name to all structs in @symTable
		 */
		void cleanStructNames(llvm::Module & M, ModuleInfo & modInfo);


		/*
		 * renames all globals with internal linkage
		 */
		void cleanInternalGlobalNames(llvm::Module & M);


		/*
		 * sort TypeSymbols
		 *
		 * (avoid forward declarations)
		 */
		//void sortTypeSymbolTable(llvm::TypeSymbolTable & symTable);

	public:
		static char ID;

                llvm::StringRef getPassName() const;

			virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

			Preparator() :
				ModulePass(ID)
			{}

			virtual ~Preparator();

			virtual bool runOnModule(llvm::Module& M);
		};
}


#endif /* PREPARATOR_HPP_ */
