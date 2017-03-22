/*
 * CModuleInfo.h
 *
 *  Created on: 06.04.2010
 *      Author: Simon Moll
 */

#ifndef CMODULEINFO_HPP_
#define CMODULEINFO_HPP_

//#include <llvm/TypeSymbolTable.h>

#include <axtor/metainfo/ModuleInfo.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/InstructionIterator.h>

#include <vector>

namespace axtor {

/*
 * class for storing additional translation relevant information for the syntax-writer
 */
class CModuleInfo : public ModuleInfo
{

public:
/*	enum ArgumentModifier
	{
		IN, OUT, UNIFORM
	};*/

	//typedef std::map<std::string, ArgumentModifier> ArgumentMap;
private:

	//bitcode & arguments
	llvm::Module * mod;

	//output streams
	std::ostream & out;
public:
	llvm::Module * getModule();

	// ctor that reads kernels from opencl MDnode
	CModuleInfo(llvm::Module * mod, std::ostream & out);

	std::ostream & getStream();

	virtual void dump();

	virtual void dumpModule();

	virtual IdentifierScope createGlobalBindings();

	virtual void runPassManager(llvm::legacy::PassManager & pm);

	/*
	 * checks whether all types are supported and
	 */
	virtual void verifyModule();

	virtual bool isTargetModule(llvm::Module * other) const;
};

}

#endif /* CMODULEINFO_HPP_ */
