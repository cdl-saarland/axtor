/*
 * OCLModuleInfo.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor_glsl/GLSLModuleInfo.h>

#include <axtor/util/llvmDebug.h>

namespace axtor {

llvm::Module * GLSLModuleInfo::getModule()
{
	return mod;
}

GLSLModuleInfo::GLSLModuleInfo(llvm::Module * _mod, llvm::Function * _vertFunc, llvm::Function * _fragFunc, std::ostream * _vertOut, std::ostream * _fragOut) :
		ModuleInfo(*_mod),
		mod(_mod), vertFunc(_vertFunc), fragFunc(_fragFunc), vertOut(_vertOut), fragOut(_fragOut)
{
	assert(fragFunc && vertFunc && "turns out we need both because the fragment shader inputs interpolation depends on the call of it in the vertex shader");
	verifyModule();
	//assert((!vertFunc || vertOut) && "need an ostream to write the vertex shader to");
	//assert((!fragFunc || fragOut) && "need an ostream to write the fragment shader to");

	//create interpolation info
	interpolant = determineFragmentInputModifiers(vertFunc, fragFunc);
}

int GLSLModuleInfo::getAttributeInterpolant(uint index) const
{
	return interpolant[index];
}

std::string GLSLModuleInfo::getInterpolantName(int type) const
{
	switch (type)
	{
		case FRAG_ARG_SMOOTH: return "smooth";
		case FRAG_ARG_FLAT:   return "flat";
		case FRAG_ARG_NOPERSPECTIVE: return "noperspective";
		case FRAG_ARG_DEFAULT: return "";//"default";
		default: return "";
	}
}

std::string GLSLModuleInfo::getAttributeInterpolantStr(uint idx) const
{
	int type = interpolant[idx];
	return getInterpolantName(type);
}


/*
 * returns the call which specifies the interpolant options for the fragment shaders input parameters
 */
llvm::CallInst * GLSLModuleInfo::getStageCall(llvm::Function * vertFunc, llvm::Function * fragFunc)
{
	if (!fragFunc)
		return NULL;

	for(llvm::Value::use_iterator use = fragFunc->use_begin(); use != fragFunc->use_end(); ++use)
	{
		llvm::User * user = *use;

		if (llvm::isa<llvm::CallInst>(user)) {
			llvm::CallInst * call = llvm::cast<llvm::CallInst>(user);
			if ((call->getParent()->getParent() == vertFunc) && (call->getCalledFunction() == fragFunc))
			{
				return call;
			}
		}
	}

	return NULL;
}

int GLSLModuleInfo::getInterpolantType(std::string interpolantName)
{
	if (interpolantName.find("smooth", 0) != std::string::npos) {
		return FRAG_ARG_SMOOTH;
	} else if (interpolantName.find("flat", 0) != std::string::npos) {
		return FRAG_ARG_FLAT;
	} else if (interpolantName.find("noperspective", 0) != std::string::npos) {
		return FRAG_ARG_NOPERSPECTIVE;
	} else if (interpolantName.find("default", 0) != std::string::npos) {
		return FRAG_ARG_DEFAULT;
	} else {
		return -1;
	}
}

std::vector<int> GLSLModuleInfo::determineFragmentInputModifiers(llvm::Function * vertFunc, llvm::Function * fragFunc)
{
	assert(vertFunc && fragFunc && "should be caught by verifyModule");
	llvm::CallInst * stageCall = getStageCall(vertFunc, fragFunc);
	assert(stageCall);

	uint numParams = getFunctionType(fragFunc)->getNumParams();
	std::vector<int> flags(numParams, 0);

	for(uint index = 0; index < numParams; ++index)
	{
		uint flag = 0;
		uint operandIndex = index + 1;
		llvm::Value * operand = stageCall->getArgOperand(operandIndex);

		if (llvm::isa<llvm::CallInst>(operand)) {
#ifdef DEBUG
			std::cerr << "is a call:"; llvm::errs() << *operand;
#endif
			llvm::CallInst * operandCall = llvm::cast<llvm::CallInst>(operand);
			llvm::Function * calledFunc = operandCall->getCalledFunction();

			std::string interpolantName = calledFunc->getName().str();
#ifdef DEBUG
			std::cerr <<"interpolant name : " << interpolantName << "\n";
#endif
			int interpFlag = getInterpolantType(interpolantName);
			if (interpFlag > -1) {
				flag = interpFlag;
			}
		} else {
#ifdef DEBUG
			std::cerr << "not a call : "; llvm::errs() << *operand;
#endif
		}

		flags[index] = flag;
	}

	return flags;
}

llvm::Function * GLSLModuleInfo::getFragFunc() const
{
	return fragFunc;
}

llvm::Function * GLSLModuleInfo::getVertFunc() const
{
	return vertFunc;
}

std::ostream * GLSLModuleInfo::getFragStream() const
{
	return fragOut;
}

std::ostream * GLSLModuleInfo::getVertStream() const
{
	return vertOut;
}

void GLSLModuleInfo::dump()
{
	std::cerr
		  << "GLSL shader module descriptor\n"
		  << "vertex shader function:\n"
		  << (vertFunc ? " * " + vertFunc->getName().str() : "NONE") << "\n"
		  << "fragment shader function:\n"
		  << (fragFunc ? " * " + fragFunc->getName().str() : "NONE") << "\n";

	std::cerr << "vertex -> fragment shader variables\n";
	uint i  = 0;
	for(std::vector<int>::iterator itArg = interpolant.begin(); itArg != interpolant.end(); ++itArg, ++i)
	{
		int interpolant = *itArg;
		std::string typeStr = getInterpolantName(interpolant);

		std::cerr << "arg[" << i << "] : " << typeStr << "\n";
	}
}

void GLSLModuleInfo::dumpModule()
{
	llvm::errs() << *mod;
}


IdentifierScope GLSLModuleInfo::createGlobalBindings()
{
	typedef llvm::Function::ArgumentListType ArgList;

	ConstVariableMap globals;
	llvm::Module * mod = getModule();

	//bind global variables
	for(llvm::Module::const_global_iterator it = mod->global_begin(); it != mod->global_end(); ++it)
	{
		if (llvm::isa<llvm::GlobalValue>(it))
		{
			std::string name = it->getName().str();
			globals[it] = VariableDesc(it, name);
		}
	}

	//bind non-shade functions
	for(llvm::Module::iterator func = mod->begin(); func != mod->end(); ++func)
	{
		if ((llvm::cast<llvm::Function>(func) == getFragFunc()) ||
				(llvm::cast<llvm::Function>(func) == getVertFunc()))
			continue;

		globals[func] = VariableDesc(func, func->getName().str());
	}

	//add bindings for the shader registers
	ArgList & fragArgs = getFragFunc()->getArgumentList();
	ArgList & vertArgs = getVertFunc()->getArgumentList();

	//declare function arguments in global scope
	for(ArgList::iterator fragArg = fragArgs.begin();fragArg != fragArgs.end(); ++fragArg)
	{
		std::string name = fragArg->getName().str();
		globals[fragArg] = VariableDesc(fragArg, name);
	}

	for(ArgList::iterator vertArg = vertArgs.begin();vertArg != vertArgs.end(); ++vertArg)
	{
		std::string name = vertArg->getName().str();
		globals[vertArg] = VariableDesc(vertArg, name);
	}


	#ifdef DEBUG
		std::cerr << "ModuleContext created\n";
	#endif

	return IdentifierScope(globals);
}

/*
* checks whether all types are supported and
*/
void GLSLModuleInfo::verifyModule()
{
	if (fragFunc)
	{
		if (!getReturnType(fragFunc)->isIntegerTy(1))
			Log::fail(fragFunc, "fragment shader function must return bool");
	} else {
			Log::fail(fragFunc, "at least a fragment shader declaration is required");
	}

	if (vertFunc)
	{
		if (vertFunc->isDeclaration() && fragFunc->isDeclaration())
			Log::fail(vertFunc, "module does not contain any defined shader stages");
	} else {
		Log::fail(fragFunc, "module does not contain any shader stages");
	}

	axtor::verifyModule(*mod);
}

bool GLSLModuleInfo::isTargetModule(llvm::Module * other) const
{
	return mod == other;
}

GLSLModuleInfo * GLSLModuleInfo::createTestInfo(llvm::Module * mod, std::ostream & vertOut, std::ostream & fragOut)
{
	assert(mod && "not specified");
	llvm::Function * vertFunc = mod->getFunction("shadeVertex");
	llvm::Function * fragFunc = mod->getFunction("shadeFragment");

	return new GLSLModuleInfo(mod, vertFunc, fragFunc, &vertOut, &fragOut);
}

void GLSLModuleInfo::runPassManager(llvm::legacy::PassManager & pm)
{
	pm.run(*getModule());
}

}
