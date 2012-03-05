/*
 * kernelPacketizer.cpp
 *
 *  Created on: 1 Mar 2012
 *      Author: v1smoll
 */

#include <assert.h>
#include <vector>


#include <packetizerAPI.hpp>

#include "KernelPacketizer.h"

#include <llvm/Constants.h>
#include <llvm/Instructions.h>
#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Metadata.h>
#include <llvm/LLVMContext.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/system_error.h>
#include <llvm/Bitcode/ReaderWriter.h>

/*
 * Common Types
 */
typedef llvm::Function::ArgumentListType ArgList;
typedef std::vector<llvm::Type*> TypeVector;
typedef std::vector<llvm::Value*> ValueVector;
typedef std::vector<llvm::Constant*> ConstantVector;
typedef std::vector<std::string> StringVector;

/*
 * checks if this is a kernel function
 */
inline bool isKernelFunction(llvm::Module * mod, llvm::Function *function)
{
  if(!function)
    return false;

  llvm::NamedMDNode *openCLMetadata = mod->getNamedMetadata("opencl.kernels");
  if(!openCLMetadata)
    return false;

  for(unsigned K = 0, E = openCLMetadata->getNumOperands(); K != E; ++K) {
    llvm::MDNode &kernelMD = *openCLMetadata->getOperand(K);
    if(kernelMD.getOperand(0) == function)
      return true;
  }
  return false;
}

inline void updateKernelMD(llvm::Module * mod, ValueVector kernelVec)
{
	llvm::LLVMContext & context = mod->getContext();

	// remove stale Kernel info
	llvm::NamedMDNode *openCLMetadata = mod->getNamedMetadata("opencl.kernels");
	assert(openCLMetadata && "not an OpenCL module");
	mod->eraseNamedMetadata(openCLMetadata);
	delete openCLMetadata;

	// register all known kernels
	openCLMetadata = mod->getOrInsertNamedMetadata("opencl.kernels");
	for (ValueVector::const_iterator itKernel = kernelVec.begin(); itKernel != kernelVec.end(); ++itKernel)
	{
		llvm::MDNode * kernelMD = llvm::MDNode::get(context, llvm::ArrayRef<llvm::Value*>(*itKernel));
		openCLMetadata->addOperand(kernelMD);
	}
}

/*
 * utility functions
 */
llvm::Module * loadModule(llvm::LLVMContext & context, const char * fileName)
{
	llvm::SMDiagnostic diag;
	llvm::Module * mod = llvm::getLazyIRFileModule(fileName, diag, context);
	if (!mod) {
       llvm::errs() << diag.getMessage() << "\n";
	   return 0;
    }

	std::string errInfo;
	mod->MaterializeAll(&errInfo);
	if (errInfo == "")
		return mod;
	else
		return 0;
}

void writeModule(llvm::Module * mod, const char * fileName)
{
	assert (mod);
	std::string errorMessage = "";
	llvm::raw_fd_ostream file(fileName, errorMessage);
	llvm::WriteBitcodeToFile(mod, file);
	file.close();
}



/*
 * tie all thread-id dependent intrinsics to function arguments
 *
 * (enabled, globalId, localId, <ARGS>)
 */
llvm::Function * wrapKernel(llvm::LLVMContext & context, llvm::Function * scalarFunc, const uint smdWidth)
{
	llvm::Type * voidType = llvm::Type::getVoidTy(context);
	llvm::Type * boolType = llvm::Type::getInt1Ty(context);
	llvm::Type * intType = llvm::Type::getInt32Ty(context);

	llvm::Constant * smdConst = llvm::ConstantInt::get(intType, static_cast<uint64_t>(smdWidth), false);

	llvm::Module * mod = scalarFunc->getParent();

	/*
	 * Initial sanity checks
	 */
	assert(scalarFunc->getReturnType() == voidType && "not a kernel function");


	/*
	 * Retrieve OpenCL intrinsics
	 */
	llvm::Function * getGlobalIdFunc = mod->getFunction("get_global_id");
	llvm::Function * getLocalIdFunc = mod->getFunction("get_local_id");
	llvm::Function * getLocalSizeFunc = mod->getFunction("get_local_size");


	/*
	 * Create threaded function stub {threadedFunc}
	 *
	 * (localId, globalId, <scalarKernel arguments>)
	 */
	llvm::FunctionType * scalarFuncType = scalarFunc->getFunctionType();
	TypeVector modifiedTypeVec;

	modifiedTypeVec.push_back(intType); //  int localId
	modifiedTypeVec.push_back(intType); //  int globalId

	uint numParams = scalarFuncType->getFunctionNumParams();
	for (uint paramIdx = 0; paramIdx < numParams; ++paramIdx)
	{
		llvm::Type * paramType = scalarFuncType->getParamType(paramIdx);
		modifiedTypeVec.push_back(paramType);
	}

	ArgList & scalarArgList = scalarFunc->getArgumentList();

	llvm::FunctionType * threadedFuncType = llvm::FunctionType::get(scalarFuncType->getReturnType(), llvm::ArrayRef<llvm::Type*>(modifiedTypeVec), false);
	llvm::Function * threadedFunc = llvm::Function::Create(threadedFuncType, scalarFunc->getLinkage(), "threadedKernelWrapper", mod);



	/*
	 * Retrieve argument values (put all others on a vector for later use)
	 */
	ArgList::iterator itArg = threadedFunc->arg_begin();
	llvm::Argument * localIdArg = itArg++;
	llvm::Argument * globalIdArg = itArg++;

        assert(localIdArg->getType() == intType && "invalid type");
        assert(globalIdArg->getType() == intType && "invalid type");

	ValueVector restArgVec; restArgVec.reserve(numParams);
	for (;itArg != threadedFunc->arg_end(); ++itArg)
		restArgVec.push_back(itArg);



	/*
	 * Generate the wrapper code
	 *
	 * threadedKernelWrapper(localId, globalId, <ARGS>)
	 * { scalarFunc(<ARGS>)  } //scalarFuncCall
	 */
	llvm::BasicBlock * entryBlock = llvm::BasicBlock::Create(context, "entry", threadedFunc);

	//entry
	llvm::CallInst * scalarFuncCall =
			llvm::CallInst::Create(scalarFunc, restArgVec, "", entryBlock);
	llvm::ReturnInst::Create(context, entryBlock);



	/*
	 * Inline scalarFunccall into the threadedKernelWrapper
	 */
	llvm::InlineFunctionInfo  IFI;
	llvm::InlineFunction(scalarFuncCall, IFI, false); //TODO true?


	/*
	 * Remap function calls to parameters (just loop)
	 *
	 * get_global_id() -> globalId
	 * get_local_id() -> localId
	 */
	for (llvm::Function::iterator itBlock = threadedFunc->begin(); itBlock != threadedFunc->end(); ++itBlock)
		for (llvm::BasicBlock::iterator itInst = itBlock->begin(); itInst != itBlock->end(); ++itInst)
			if (llvm::isa<llvm::CallInst>(itInst)) {
				llvm::CallInst * someCall = llvm::cast<llvm::CallInst>(itInst);
				llvm::Function * callee = someCall->getCalledFunction();

				// the real values need to be provided from the outside
                                // we only vectorize over the first dimension (leave all other calls alone as their values will stay valid)
				if (callee == getGlobalIdFunc || callee == getLocalIdFunc || callee == getLocalSizeFunc) {
                                        llvm::Value * dimVal = someCall->getArgOperand(0);
                                        assert(llvm::isa<llvm::ConstantInt>(dimVal));
                                        llvm::ConstantInt * dimIntConst= llvm::cast<llvm::ConstantInt>(dimVal);
                                        uint64_t dim = dimIntConst->getValue().getLimitedValue();
                                        if (dim > 0)
                                           continue;

                                        if (callee == getGlobalIdFunc)                        
					    someCall->replaceAllUsesWith(globalIdArg);                                        
                                        else if (callee == getLocalIdFunc)
					    someCall->replaceAllUsesWith(localIdArg);
				        // can be reconstructed given the smdSize, then realWorkSize = packetizedWorkSize * smdSize
                                        // as packetizedWorkSize == workSize / smdWize
				        else if (callee == getLocalSizeFunc) {
					    llvm::Instruction * realLocalSize = llvm::BinaryOperator::Create(llvm::Instruction::Mul, smdConst, someCall);
					    someCall->replaceAllUsesWith(realLocalSize);
                                        }
				}
			}

	return threadedFunc;
}



/*
 * insert something like
 * 		idFunc() * smdWidth + <0, ..., smdWidth>
 */
inline llvm::Instruction * insertPlusThreadOffset(
		llvm::LLVMContext & context,
		llvm::Function * idFunc,
		const char * name, llvm::BasicBlock * block,

		llvm::Constant * smdConst,    // i32 smdWidth
		llvm::Constant * zeroVec,     // <i32 x smdWidth> ( 0 )
		llvm::Constant * offsetVec    // <0, 1, ..., smdWidth>
) {

	llvm::Type * voidType = llvm::Type::getVoidTy(context);
	llvm::IntegerType * boolType = llvm::Type::getInt1Ty(context);
	llvm::IntegerType * intType = llvm::Type::getInt32Ty(context);
	llvm::VectorType * intVecType = llvm::cast<llvm::VectorType>(smdConst->getType());



	// fetch thread local id
	llvm::CallInst * localIdVal = llvm::CallInst::Create(idFunc, "smdLocalId", block);

	// LocalIdBase = smdWidth * smdLocalId
	llvm::Instruction * localIdBaseVal = llvm::BinaryOperator::Create(llvm::Instruction::Mul, smdConst, localIdVal, "localIDBase", block);

	// replLocalIdBase = int<smdWidth>(localIdBase)
	llvm::Instruction * replBaseVal = new llvm::ShuffleVectorInst(localIdBaseVal, llvm::UndefValue::get(intVecType), zeroVec, "replLocalIdBase", block);

	// replLocalIdBase + (0, 1, ..., <smdWidth>)
	return llvm::BinaryOperator::Create(llvm::Instruction::Add, replBaseVal, offsetVec, "localIDVec", block);
}


/*
 * Take a packetized function generated from a threaded scalarFunc
 *
 * (<4 x i1> enabled, <4 x i32> localId, <4 x i32> globalId, <REST>)
 *
 * and build a wrapper around it with the same signature as scalarFunc and (hopefully) equivalent semantics
 */
llvm::Function * kernelizePackedFunction(llvm::LLVMContext & context, llvm::Function * scalarFunc, llvm::Function * packedFunc, const uint smdWidth)
{
	llvm::Module * mod = scalarFunc->getParent();

	llvm::Type * voidType = llvm::Type::getVoidTy(context);
	llvm::IntegerType * intType = llvm::Type::getInt32Ty(context);
	llvm::VectorType * intVecType = llvm::VectorType::get(intType, smdWidth);


	/*
	 * declare some constants
	 */
	// constant (0,1,2,3,.. smdWidth - 1) of type <smdWidth x i32>
	uint32_t * offsetArray = new uint32_t[smdWidth];
	for(uint i = 0; i < smdWidth; ++i)
		offsetArray[i] = i;

	llvm::Constant * offsetVec = llvm::ConstantDataVector::get(context, llvm::ArrayRef<uint32_t>(offsetArray, smdWidth));

	llvm::Constant * zeroVec = llvm::ConstantAggregateZero::get(intVecType);

	llvm::Constant * smdConst = llvm::ConstantInt::get(intType, static_cast<uint64_t>(smdWidth), false);


	/*
	 * Initial sanity checks
	 */
	assert(scalarFunc->getReturnType() == voidType && "not a kernel function");
	const llvm::FunctionType * packedFuncType = packedFunc->getFunctionType();

	assert(packedFuncType->getParamType(0) == intVecType); // localId
	assert(packedFuncType->getParamType(1) == intVecType); // globalId


	/*
	 * Retrieve OpenCL intrinsics
	 */
	llvm::Function * getGlobalIdFunc = mod->getFunction("get_global_id");
	llvm::Function * getLocalIdFunc = mod->getFunction("get_local_id");


	/*
	 * Create threaded function stub {wrapperFunc}
	 *
	 * (<scalarKernel arguments>)
	 */
	llvm::FunctionType * scalarFuncType = scalarFunc->getFunctionType();
	llvm::Function * wrapperFunc = llvm::Function::Create(scalarFuncType, scalarFunc->getLinkage(), scalarFunc->getName(), mod);
	llvm::BasicBlock * header = llvm::BasicBlock::Create(context, "entry", wrapperFunc);


	// from now on start building the calls arguments
	ValueVector callArgs;

	/*
	 * compute get local id
	 *
	 * entry : l = get_local_id()
	 *         localIdVec = vec<smdWidth>(l) //realised by a shuffle inst and an add
	 */
	llvm::Instruction * localIdVec = insertPlusThreadOffset(context, getLocalIdFunc, "local", header, smdConst, zeroVec, offsetVec);
	llvm::Instruction * globalIdVec = insertPlusThreadOffset(context, getGlobalIdFunc, "global", header, smdConst, zeroVec, offsetVec);
	callArgs.push_back(localIdVec);
	callArgs.push_back(globalIdVec);



	/*
	 * Cast all remaining arguments to their vector types (retrieved from threaded signature)
	 */
	ArgList & wrapperArgs = wrapperFunc->getArgumentList();

	for (ArgList::iterator itArg = wrapperArgs.begin(); itArg != wrapperArgs.end(); ++itArg) {
		llvm::Type * argType = itArg->getType();

		// cast T* to <smdWidth x T>*
		/*if (llvm::isa<llvm::PointerType>(argType)) {
			llvm::PointerType * dataPtr = llvm::cast<llvm::PointerType>(argType);
			llvm::Type * elemType = dataPtr->getElementType();
			llvm::VectorType * dataVecType = llvm::VectorType::get(elemType, smdWidth);
			llvm::PointerType * vecDataPtr = dataVecType->getPointerTo(dataPtr->getAddressSpace());

			llvm::CastInst * vecData = llvm::BitCastInst::CreatePointerCast(itArg, vecDataPtr, "castTopacket", header);
			callArgs.push_back(vecData);

		// scalar value case
		} else */{
			callArgs.push_back(itArg);
		}

	}

	/*
	 *  put everything together
	 *  wrapperFunc(<scalar>)
	 *  {
	 *     packedFunc(localIdVec, globalIdVec, PACKETZED(<scalar args>)
	 *     return;
	 *  }
	 *
	 */
    llvm::CallInst * packedCall = llvm::CallInst::Create(packedFunc, callArgs, "", header);
    llvm::ReturnInst::Create(context, header);

	/*
	 * Inline packedFunc into the wrapperFunc
	 */
	llvm::InlineFunctionInfo  IFI;
	llvm::InlineFunction(packedCall, IFI, false); //TODO true?

	return wrapperFunc;
}


llvm::Function * generateSMDStub(llvm::LLVMContext & context, llvm::Function * threadedFun, std::string packedName, uint smdWidth)
{
	llvm::Type * voidType = llvm::Type::getVoidTy(context);
	llvm::IntegerType * intType = llvm::Type::getInt32Ty(context);
	llvm::VectorType * intVecType = llvm::VectorType::get(intType, smdWidth);

	TypeVector signature;
	llvm::FunctionType * funType = threadedFun->getFunctionType();

	signature.push_back(intVecType); // globalIdVec
	signature.push_back(intVecType); // localIdVec

	for (uint i = 2; i < funType->getFunctionNumParams(); ++i)
	{
		signature.push_back(funType->getFunctionParamType(i)); //preserve scalars
	}

	llvm::FunctionType * smdType = llvm::FunctionType::get(voidType, llvm::ArrayRef<llvm::Type*>(signature), false);
	return llvm::Function::Create(smdType, threadedFun->getLinkage(), packedName, threadedFun->getParent());
}



void packetizeAllKernelsInModule(const char * fileName, uint smdWidth)
{
	llvm::LLVMContext & context = llvm::getGlobalContext();

	// load Module
	llvm::Module * mod = loadModule(context, fileName);

	assert(mod);

	// iterate over all kernel functions
	Packetizer::Packetizer packetizer(*mod, smdWidth, smdWidth, false, false, true);

	StringVector kernelNames; kernelNames.reserve(4);


	/*
	 * wrap-up all scalar kernel functions and add them to the packetizer
	 */
	for (llvm::Module::iterator itFun = mod->begin(); itFun != mod->end(); ++itFun)
	{
		if (isKernelFunction(mod, itFun)) {
			llvm::Function * threadedFun = wrapKernel(context, itFun, smdWidth);

			std::string scalarName = itFun->getName().str();
			std::string threadedName = threadedFun->getName().str();
			std::string packedName = scalarName + "_smd";
                        generateSMDStub(context, threadedFun, packedName, smdWidth);

			kernelNames.push_back(scalarName);
			packetizer.addFunction(threadedName, packedName);
		}
	}

	/*
	 * packetize..
	 */
	packetizer.run();

	/*
	 * Retrieve packetized functions and wrap them again in the scalar signature
	 */
	ValueVector kernelFunVec; kernelFunVec.reserve(kernelNames.size());

	for (StringVector::const_iterator itKernelName = kernelNames.begin(); itKernelName != kernelNames.end(); ++itKernelName)
	{
		std::string scalarName = *itKernelName;
		std::string packedName = scalarName + "_smd";

		llvm::Function * scalarFun = mod->getFunction(scalarName);
		assert(scalarFun && "disappeared after packetizer run");

		llvm::Function * packedFun = mod->getFunction(packedName);
		assert(packedFun && "was not generated by packetizer");

		// kernelize and replace
		llvm::Function * smdKernelFun = kernelizePackedFunction(context, scalarFun, packedFun, smdWidth);
		smdKernelFun->takeName(scalarFun); // steal the scalar function's name

		// clean-up
		scalarFun->removeFromParent();
		packedFun->removeFromParent();

		kernelFunVec.push_back(smdKernelFun);
	}

	/*
	 * update OpenCL Meta-data
	 */
	updateKernelMD(mod, kernelFunVec);


	/*
	 * Write module to disk
	 */
	writeModule(mod, fileName);
}
