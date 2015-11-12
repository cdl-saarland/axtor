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

			virtual const char * getPassName() const;

			virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

			TrivialReturnSplitter() :
				llvm::FunctionPass(ID)
			{}

			virtual ~TrivialReturnSplitter();

			virtual bool runOnFunction(llvm::Function& F);
		};
}


#endif /* TRIVIALRETURNSPLITTER_HPP_ */
