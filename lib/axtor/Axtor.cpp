/*
 * Axtor.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/Axtor.h>

#include <axtor/config.h>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/IR/LegacyPassManager.h>


#include <axtor/util/stringutil.h>
#include <axtor/util/Timer.h>
#include <axtor/util/llvmTools.h>
#include <axtor/util/ResourceGuard.h>


#include <axtor/pass/CGIPass.h>
#include <axtor/pass/CNSPass.h>
#include <axtor/pass/ExitUnificationPass.h>
#include <axtor/pass/LoopBranchSeparationPass.h>
#include <axtor/pass/Normalizer.h>
#include <axtor/pass/Preparator.h>
#include <axtor/pass/RestructuringPass.h>
#include <axtor/pass/Serializer.h>
#include <axtor/pass/SimpleUnswitchPass.h>
#include <axtor/pass/TargetProvider.h>
#include <axtor/pass/TrivialReturnSplitter.h>

#include <axtor/util/llvmInit.h>

#include <axtor/pass/ReForPass.h>


namespace axtor {
	static bool axtorInitialized = false;

	void initialize(bool alsoLLVM)
	{
		if (axtorInitialized) return;

		CompilerLog::init(llvm::errs());
		if (alsoLLVM) {
			initLLVM();
		}

		axtorInitialized = true;
	}

	void translateModule(AxtorBackend & backend, ModuleInfo & modInfo)
	{
		initialize(false);

#ifdef DEBUG
		modInfo.verifyModule();
#endif

		modInfo.dumpModule();

		llvm::legacy::PassManager pm;

#ifdef EVAL_DECOMPILE_TIME
		Timer timer;
		timer.start();
#endif

		addBackendPasses(backend, modInfo, pm);

		modInfo.runPassManager(pm);

#ifdef EVAL_DECOMPILE_TIME
		timer.end();
		long ms = timer.getTotalMS();
		std::cerr << "## Evaluation ##\n"
				  << "\tdecompile time = " << str<long>(ms) << "ms\n";
#endif

	}

	void addBackendPasses(AxtorBackend & backend, ModuleInfo & modInfo, llvm::legacy::PassManager & pm)
	{
		initialize(false);

		llvm::Pass * loopSimplify = llvm::createLoopSimplifyPass();

		axtor::SimpleUnswitchPass * simpleUnswitch = new axtor::SimpleUnswitchPass();
		axtor::Preparator * preperator = new axtor::Preparator();
		axtor::ExitUnificationPass * loopExitEnum = new axtor::ExitUnificationPass();
		axtor::TrivialReturnSplitter * returnSplitter = new axtor::TrivialReturnSplitter();

	#ifdef ENABLE_CNS
		axtor::CNSPass * regularizer = new axtor::CNSPass();
	#endif

		//axtor::OpaqueTypeRenamer * renamer = new axtor::OpaqueTypeRenamer();
		axtor::Serializer * serializer = new axtor::Serializer();



		//target info
		pm.add(new axtor::TargetProvider(backend, modInfo));

		// ## REQUIRED TRANSFORMATIONS ##

		//recover registers
		pm.add(llvm::createPromoteMemoryToRegisterPass());


		//LLVM transformations
		//pm.add(loopUnswitch);
		pm.add(loopSimplify);
		// pm.add(llvm::createIndVarSimplifyPass()); // (DON'T)

		pm.add(returnSplitter);

		//LLVM-Analysis
		//pm.add(loopInfo);
		//pm.add(domTree);
		//pm.add(postDomTree);


		// ## CONTROL-FLOW RESTRUCTURING ##
		/*
		 * important : don't change the "add" order of the passes (!)
		 */
	#ifdef ENABLE_CNS
		pm.add(regularizer);
	#endif


		//Required by restruct pass
		// llvm::Pass * regMemPass = llvm::createDemoteRegisterToMemoryPass();
		// pm.add(regMemPass);

		//axtor transformations building on LLVM analysis
		pm.add(loopExitEnum);

		//get rid of switches early on
    	pm.add(simpleUnswitch);

    	if (! getenv("AXTOR_NO_FOR")) {
    		pm.add(new ReForPass);
        }

        axtor::RestructuringPass * restruct = new axtor::RestructuringPass();
        pm.add(restruct);

        //opaque type renamer
        //pm.add(renamer);

        //backend specific passes
        backend.addRequiredPasses(pm);

        // sanitize variable names
        pm.add(preperator);

        // ## BUILTIN TRANSFORMATION ##
        //remove optimization artifacts
        pm.add(new CGIPass());

        // clear out artifacts
        pm.add(llvm::createDeadCodeEliminationPass());

        //read-only serialization pass
        pm.add(serializer);

        //axtor passes

	}

}
