/*
 * CGIPass.cpp
 *
 *  Created on: Apr 12, 2011
 *      Author: Simon Moll
 */

#include <axtor/console/CompilerLog.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/pass/CGIPass.h>
#include <axtor/util/llvmBuiltins.h>

#include <llvm/Analysis/CallGraph.h>

#include <iostream>

namespace axtor {

	char CGIPass::ID = 0;

	llvm::RegisterPass<CGIPass> __regCGIPass(
				"generic-cgi", "Generic Code Generator Intrinsics",
				false, /* mutates the CFG */
				true); /* transformation */


	CGIPass::CGIPass() :
		llvm::ModulePass(ID)
	{}

	void CGIPass::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
		usage.addRequired<llvm::CallGraph>();
	}

	bool CGIPass::runOnModule(llvm::Module & M)
	{
		//session set-up
		Session session(M);

#ifdef DEBUG
		std::cerr << " ### module before Pass : \"" << getPassName() << "\"\n";
		M.dump();
		std::cerr << "[EOF]\n";
#endif
		bool retVal = session.run();
#ifdef DEBUG
		std::cerr << " ### module after Pass : \"" << getPassName() << "\"\n";
		M.dump();
		std::cerr << "[EOF]\n";
#endif
		return retVal;
	}

	/*
	 * ### actual pass implementation ###
	 */
	bool CGIPass::Session::run()
	{
		// Remove superfluous functions (ordered by data dependency)
		removeIfUsed("llvm.stackrestore");
		removeIfUsed("llvm.stacksave");

		// lower memcpy for GPUs
		lowerMemCpy();

		lowerMemSet();

		return true;
	}

	bool CGIPass::Session::removeIfUsed(const std::string & funcName)
	{
		llvm::Function * llvmFunc = M.getFunction(funcName);

		if (llvmFunc) {
#ifdef DEBUG
			std::cerr << "uses function " << funcName << "\n";
#endif
			for (llvm::Function::use_iterator itCall = llvmFunc->use_begin(); itCall != llvmFunc->use_end(); itCall = llvmFunc->use_begin())
			{
				llvm::User* user = *itCall;
				llvm::Instruction * call = llvm::cast<llvm::Instruction>(user);
#ifdef DEBUG
				std::cerr << "removing user " << call->getName().str() << "\n";
#endif
				call->eraseFromParent();
			}
			assert(llvmFunc->getNumUses() == 0);
			//llvmFunc->eraseFromParent();
		} else {
#ifdef DEBUG
			std::cerr << "does not exist: " << funcName << "\n";
#endif
		}
		return llvmFunc;
	}

	void CGIPass::Session::lowerMemCpy()
	{
		FunctionVector memCpyVec = findFunctionsPrefixed(M, "llvm.memcpy");
		for (FunctionVector::const_iterator itMemCpy = memCpyVec.begin(); itMemCpy != memCpyVec.end(); ++itMemCpy)
		{
			llvm::Function * memCpyFunc = *itMemCpy;
			std::string memCpyName = memCpyFunc->getNameStr();
			llvm::Function::arg_iterator itArg = memCpyFunc->arg_begin();
			llvm::Function::arg_iterator destPtrArg = itArg++;
			llvm::Function::arg_iterator srcPtrArg = itArg;

			if (!llvm::isa<llvm::PointerType>(destPtrArg->getType()) ||
				!llvm::isa<llvm::PointerType>(srcPtrArg->getType()))
			{
				Log::fail(memCpyFunc, "invalid memcpy declaration!");
			}

			llvm::PointerType * destPtrType = llvm::cast<llvm::PointerType>(destPtrArg->getType());
			llvm::PointerType *  srcPtrType = llvm::cast<llvm::PointerType>( srcPtrArg->getType());

#ifdef DEBUG
			std::cerr << "supplementing memcpy: " << memCpyFunc->getNameStr() << "\n";
#endif
			llvm::Function * definedMemCpy =
					create_memcpy(M, memCpyName, destPtrType->getAddressSpace(), srcPtrType->getAddressSpace());
			definedMemCpy->setLinkage(llvm::Function::InternalLinkage); // hide internally
			definedMemCpy->setName("gpu_memcpy"); // mangle name
		}
	}

	void CGIPass::Session::lowerMemSet()
	{
		FunctionVector memSetVec = findFunctionsPrefixed(M, "llvm.memset");
		for (FunctionVector::const_iterator itMemSet = memSetVec.begin(); itMemSet != memSetVec.end(); ++itMemSet)
		{
			llvm::Function * memSetFunc = *itMemSet;
			std::string memSetName = memSetFunc->getNameStr();
			llvm::Function::arg_iterator ptrArg = *(memSetFunc->arg_begin());

			if (!llvm::isa<llvm::PointerType>(ptrArg->getType()))
			{
				Log::fail(memSetFunc, "invalid memset declaration!");
			}

			llvm::PointerType * ptrType = llvm::cast<llvm::PointerType>(ptrArg->getType());

#ifdef DEBUG
			std::cerr << "supplementing memset: " << memSetFunc->getNameStr() << "\n";
#endif
			llvm::Function * definedMemSet =
					create_memset(M, memSetName, ptrType->getAddressSpace());
			definedMemSet->setLinkage(llvm::Function::InternalLinkage); // hide internally
			definedMemSet->setName("gpu_memset"); // mangle name
		}
	}
}
