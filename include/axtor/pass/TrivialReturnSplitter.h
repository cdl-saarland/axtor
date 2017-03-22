/*
 * TrivialReturnSplitter.h
 *
 *  Created on: 08.02.2011
 *      Author: Simon Moll
 */

#ifndef TRIVIALRETURNSPLITTER_HPP_
#define TRIVIALRETURNSPLITTER_HPP_

#include <llvm/Pass.h>

namespace llvm {
	class Function;
}


/*
 * this pass assigns names to all types and instructions
 */
namespace axtor
{
	class TrivialReturnSplitter : public llvm::FunctionPass
	{
	private:
	public:
		static char ID;

			virtual llvm::StringRef getPassName() const;

			virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

			TrivialReturnSplitter() :
				llvm::FunctionPass(ID)
			{}

			virtual ~TrivialReturnSplitter();

			virtual bool runOnFunction(llvm::Function& F);
		};
}


#endif /* TRIVIALRETURNSPLITTER_HPP_ */
