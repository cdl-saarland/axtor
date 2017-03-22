/*
 * AxtorBackend.h
 *
 *  Created on: 19.03.2010
 *      Author: Simon Moll
 */

#ifndef AXTORBACKEND_HPP_
#define AXTORBACKEND_HPP_

#include <llvm/IR/Function.h>

#include <axtor/metainfo/ModuleInfo.h>
#include <axtor/intrinsics/PlatformInfo.h>
#include <axtor/writer/SyntaxWriter.h>
#include <axtor/CommonTypes.h>

namespace llvm {
	namespace legacy {
		class PassManager;
	}
}

namespace axtor {

struct AxtorBackend
{
	virtual ~AxtorBackend() {}
	virtual const std::string & getName()=0;
	virtual const std::string & getLLVMDataLayout()=0;

	//verifier
	virtual bool hasValidType(ModuleInfo * moduleInfo)=0;

	// returns if Serializer should emit a variable binding for this instruction
	virtual bool requiresDesignator(llvm::Instruction * inst) { return true; };

	//factory methods
	virtual SyntaxWriter * createModuleWriter(ModuleInfo & modInfo, const IdentifierScope & globals)=0;
	virtual SyntaxWriter * createFunctionWriter(SyntaxWriter * modWriter, llvm::Function * func)=0;
	virtual SyntaxWriter * createBlockWriter(SyntaxWriter * writer)=0;

	virtual bool implementsFunction(llvm::Function * func)=0;

	//interface for specifying passes specific to this backend
	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const=0;
	virtual void addRequiredPasses(llvm::legacy::PassManager & pm) const=0;
};

}

#endif /* AXTORBACKEND_HPP_ */
