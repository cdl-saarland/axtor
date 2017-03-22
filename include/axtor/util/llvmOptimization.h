/*
 * llvmOptimization.h
 *
 *  Created on: Aug 25, 2011
 *      Author: Simon Moll
 */

#ifndef LLVMOPTIMIZATION_HPP_
#define LLVMOPTIMIZATION_HPP_

namespace llvm {
	class Module;
}

namespace axtor {

	void optimizeModule(llvm::Module * module);


}

#endif /* LLVMOPTIMIZATION_HPP_ */
