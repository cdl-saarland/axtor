/*
 * CBackend.h
 *
 *  Created on: 22.05.2015
 *      Author: Simon
 */

#ifndef CBACKEND_HPP_
#define CBACKEND_HPP_

#include <axtor/backend/AxtorBackend.h>
#include <axtor/intrinsics/PlatformInfo.h>

#include "CWriter.h"
#include "CModuleInfo.h"

namespace llvm {
	namespace legacy {
		class PassManager;
	}
}

namespace axtor {

class CBackend : public AxtorBackend
{
	static PlatformInfo * platform;

	static void init();

public:
	CBackend();
	~CBackend() {}

	virtual bool hasValidType(ModuleInfo * modInfo);

	virtual const std::string & getName();
	virtual const std::string & getLLVMDataLayout();

	static const std::string & getNameString();
	static const std::string & getLLVMDataLayoutString();

	//factory methods
	virtual SyntaxWriter * createModuleWriter(ModuleInfo & modInfo, const IdentifierScope & globals);

	virtual SyntaxWriter * createFunctionWriter(SyntaxWriter * modWriter, llvm::Function * func);

	virtual SyntaxWriter * createBlockWriter(SyntaxWriter * writer);

	virtual bool implementsFunction(llvm::Function * func);

	//does suppress binding for the global_fake_sampler call
	virtual bool requiresDesignator(llvm::Instruction * inst);

	//interface for specifying passes specific to this backend
	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;
	virtual void addRequiredPasses(llvm::legacy::PassManager & pm) const;
};

}

#endif /* CBACKEND_HPP_ */
