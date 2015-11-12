/*  Axtor - AST-Extractor for LLVM
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * GLSLModuleInfo.h
 *
 *  Created on: 06.04.2010
 *      Author: Simon Moll
 */

#ifndef GLSLMODULEINFO_H_
#define GLSLMODULEINFO_H_

#include <axtor/console/CompilerLog.h>
#include <axtor/metainfo/ModuleInfo.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/InstructionIterator.h>

#include "GLSLCommon.h"

namespace axtor {

/*
 * class for storing additional translation relevant information for the syntax-writer
 */
class GLSLModuleInfo : public ModuleInfo
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
	llvm::Function * vertFunc;
	llvm::Function * fragFunc;

	//output streams
	std::ostream * vertOut;
	std::ostream * fragOut;

	std::vector<int> interpolant;

	static llvm::CallInst * getStageCall(llvm::Function * vertFunc, llvm::Function * fragFunc);

	int getInterpolantType(std::string interpolantName);
	/*
	 * maps each fragment shader argument to a FRAG_ARG_* value from GLSLCommon.hpp (interpolation behaviour between stages)
	 */
	std::vector<int> determineFragmentInputModifiers(llvm::Function * vertFunc, llvm::Function * fragFunc);

	std::string getInterpolantName(int type) const;

public:
	llvm::Module * getModule();

	llvm::Function * getFragFunc() const;
	llvm::Function * getVertFunc() const;

	std::ostream * getFragStream() const;
	std::ostream * getVertStream() const;

	static GLSLModuleInfo * createTestInfo(llvm::Module * mod, std::ostream & vertOut, std::ostream & fragOut);
	inline static GLSLModuleInfo * create(llvm::Module * mod, std::ostream & vertOut, std::ostream & fragOut)
	{
		return createTestInfo(mod, vertOut, fragOut);
	}

	GLSLModuleInfo(llvm::Module * _mod, llvm::Function * _vertShaderFunc, llvm::Function * _fragShaderFunc, std::ostream * _vertOut, std::ostream * _fragOut);

	//dump module info
	virtual void dump();

	//dump the module encapsulated by this module info object
	virtual void dumpModule();

	virtual IdentifierScope createGlobalBindings();

	virtual void runPassManager(llvm::legacy::PassManager & pm);

	/*
	 * checks whether all types are supported and
	 */
	virtual void verifyModule();

	virtual bool isTargetModule(llvm::Module * other) const;

	int getAttributeInterpolant(uint index) const;

	std::string getAttributeInterpolantStr(uint idx) const;
};

}

#endif /* GLSLMODULEINFO_H_ */
