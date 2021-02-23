/*
 * llvmOptimization.cpp
 *
 *  Created on: Aug 25, 2011
 *      Author: Simon Moll
 */

#include <axtor/util/llvmOptimization.h>

#include <llvm/Analysis/Passes.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace axtor {

	void optimizeModule(llvm::Module * mod)
	{
              assert (mod);

    legacy::PassManager pm;
    // Using the supplied dataLayout crashes some kernels.

    PassManagerBuilder Builder;
    Builder.OptLevel = 3;
    //Builder.DisableUnitAtATime = true;
    Builder.LoopVectorize = false; // default: false
    Builder.SLPVectorize = false; // default: false
    Builder.DisableUnrollLoops = false; // default: false
    Builder.Inliner = nullptr;
    //outs() << "\nUsed module pass manager config:\n";
    //outs() << "  OptLevel     : " << Builder.OptLevel << "\n";
    //outs() << "  LoopVectorize: " << Builder.LoopVectorize << "\n";
    //outs() << "  BBVectorize  : " << Builder.BBVectorize << "\n";
    //outs() << "  SLPVectorize : " << Builder.SLPVectorize << "\n";
    //outs() << "  UnrollLoops  : " << !Builder.DisableUnrollLoops << "\n";

    pm.add(createLCSSAPass()); // simplify loops
    Builder.populateModulePassManager(pm);

    pm.run(*mod);
	}
}
