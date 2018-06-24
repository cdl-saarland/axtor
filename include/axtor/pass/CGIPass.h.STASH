/*
 * CGIPass.h
 *
 *  Created on: Apr 12, 2011
 *      Author: Simon Moll
 */

#ifndef CGIPASS_HPP_
#define CGIPASS_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/Analysis/CallGraph.h>

/*
 * (generic) Code Generator Intrinsics Pass
 *
 * provides implementations for the code-generator intrinsics
 */


namespace axtor {

class CGIPass : public llvm::ModulePass
{

	class Session
	{
		llvm::Module & M;
		bool removeIfUsed(const std::string & funcName);

	public:
		Session(llvm::Module & _M) :
			M(_M)
		{}

		bool run();

		/*
		 * implements llvm.memcpy for GPUs
		 */
		void lowerMemCpy();

		void lowerMemSet();

                static llvm::StringRef getPassName() { return "CGIPass - OCL Code Generator Intrinsics"; }
	};

	public:
		static char ID;

		CGIPass() :
			llvm::ModulePass(ID)
		{}

		virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

		bool runOnModule(llvm::Module & M);

		virtual llvm::StringRef getPassName() const
		{
			return Session::getPassName();
		}
};

}

#endif /* CGIPASS_HPP_ */
