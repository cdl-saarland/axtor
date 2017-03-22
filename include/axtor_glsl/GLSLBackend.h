/*
 * GLSLBackend.h
 *
 *  Created on: 03.05.2010
 *      Author: Simon Moll
 */

#ifndef GLSLBACKEND_H_
#define GLSLBACKEND_H_

#include <axtor/backend/AxtorBackend.h>
#include "InterpolantIntrinsic.h"
#include "GLSLWriter.h"

namespace axtor {

class GLSLWriter;

class GLSLBackend : public AxtorBackend
{

	static PlatformInfo * platform;

	static void init();

public:
	enum ADDR_SPACE
	{
		ConstantSpace = SPACE_CONSTANT,
		GlobalSpace = SPACE_GLOBAL,
		LocalSpace = SPACE_LOCAL,
		PrivateSpace = SPACE_PRIVATE,
		PointerSpace = SPACE_POINTER,
		NoPtrSpace = SPACE_NOPTR
	};

	GLSLBackend();

	virtual const std::string & getName();
	virtual const std::string & getLLVMDataLayout();

	static const std::string & getNameString();
	static const std::string & getLLVMDataLayoutString();

	//verifier
	virtual bool hasValidType(ModuleInfo * moduleInfo);

	//factory methods
	static GLSLWriter * createFittingMultiWriter(GLSLWriter & writer, std::ostream * first, std::ostream * second);
	static GLSLWriter * createWriterIfDefined(GLSLWriter & writer, std::ostream * stream);

	virtual SyntaxWriter * createModuleWriter(ModuleInfo & modInfo, const IdentifierScope & globals);
	virtual SyntaxWriter * createFunctionWriter(SyntaxWriter * modWriter, llvm::Function * func);
	virtual SyntaxWriter * createBlockWriter(SyntaxWriter * writer);

	virtual bool implementsFunction(llvm::Function * func);

	//interface for specifying passes specific to this backend
	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;
	virtual void addRequiredPasses(llvm::legacy::PassManager & pm) const;
};

}

#endif /* GLSLBACKEND_H_ */
