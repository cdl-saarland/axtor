/*
 * CWriter.cpp
 *
 *  Created on: 22.04.2015
 *      Author: Simon
 */

#include <axtor/config.h>
#include <axtor_c/CWriter.h>

#include <axtor/util/WrappedLiteral.h>
#include <vector>

#include <llvm/IR/Instructions.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

// def ForLoopInfo
#include <axtor/util/llvmLoop.h>

using namespace llvm;

// return the Aurora-SX mangled suffix for this vector type (to be used in builtins)
static
std::string
GetVectorSuffix(const llvm::Type & elemTy) {
  if (elemTy.isIntegerTy()) {
    const int numBits = elemTy.getPrimitiveSizeInBits();
    switch (numBits){
      case 1 : return "m";
      case 32: return "si";
      case 64: return "sl";
      default: abort(); // unsupported vector bit width
    }
  } else if (elemTy.isFloatTy()) {
    return "sf";
  } else if (elemTy.isDoubleTy()) {
    return "df";
  }
  abort(); // unepxected type suffix
}

namespace axtor {

void CWriter::put(const std::string &text) { modInfo.getStream() << text; }

void CWriter::dump() {}

void CWriter::print(std::ostream &out) {}

/*
 *  ##### Type System #####
 */

/*
 * returns type symbols for default scalar types
 */
std::string CWriter::getScalarType(const llvm::Type *type,
                                   bool asVectorElementType) {
#ifdef DEBUG
  std::cerr << "getScalarType(";
  type->dump();
  std::cerr << ")\n";
#endif

  //### native platform type ###
  std::string nativeStr;
  std::string typeName = "";

  /* NATIVE TYPES */
  if (platform.lookUp(type, typeName, nativeStr)) {
    return nativeStr;
  }

  //### C default types ###
  switch (type->getTypeID()) {
  case llvm::Type::VoidTyID:
    return "void";
  case llvm::Type::FloatTyID:
    return "float";
  case llvm::Type::DoubleTyID:
    return "double";
  case llvm::Type::IntegerTyID: {
    const llvm::IntegerType *intType = llvm::cast<llvm::IntegerType>(type);
    int width = intType->getBitWidth();

    if (width == 1)
      return "bool";
    else if (width <= 8)
      return "char";
    else if (width <= 16)
      return "short";
    else if (width <= 32)
      return "int";
    else if (width <= 64) {
      return "long";
    } else
      Log::fail(type, "(width > 64) over-sized integer type");
  }

    // FIXME
    /*case llvm::Type::OpaqueTyID:
    {
            Log::fail(type, "OpenCL does not implement this opaque type");
    };*/

  case llvm::Type::PointerTyID: {
    Log::fail(type, "OpenCL does not support nested pointers");
  }

  case llvm::Type::VectorTyID: {
    const llvm::VectorType *vectorType = llvm::cast<llvm::VectorType>(type);
    int size = vectorType->getNumElements();
    if (vectorType->getElementType()->isPointerTy()) {
      assert(size == 256);
      return "vr256p";
    } else {
      std::string elementStr = getScalarType(vectorType->getElementType());
      return elementStr + str<int>(size);
    }
  }

  default:
    Log::fail(type, "not a scalar type");
  };
  abort();
}

std::string CWriter::getLocalVariableName(const std::string &variableFullName) {
  return variableFullName.substr(variableFullName.find_first_of('.') + 1,
                                 variableFullName.length() -
                                     variableFullName.find_first_of('.'));
}

/*
 * generates a type name for @type
 * if this is a pointer type, operate on its element type instead
 */
std::string CWriter::getType(const llvm::Type *type) {
#ifdef DEBUG
  std::cerr << "getType(";
  type->dump();
  std::cerr << ")\n";
#endif
  if (llvm::isa<llvm::ArrayType>(type)) {
    const llvm::ArrayType *arrType = llvm::cast<llvm::ArrayType>(type);
    return getType(arrType->getElementType()) + "[" +
           str<int>(arrType->getNumElements()) + "]";

  } else if (llvm::isa<llvm::VectorType>(type)) {
    const llvm::VectorType *vectorType = llvm::cast<llvm::VectorType>(type);
    if (vectorType->getVectorElementType()->isPointerTy()) {
      return "vr" + str(vectorType->getNumElements()) + "p";
    }

    return getScalarType(vectorType->getElementType(), true) +
           str<int>(vectorType->getNumElements());

  } else if (llvm::isa<llvm::PointerType>(type)) {
    const llvm::PointerType *ptrType = llvm::cast<llvm::PointerType>(type);
    const llvm::Type *elementType = ptrType->getElementType();
    {
      return getType(elementType) + "*";
    }

  } else if (llvm::isa<llvm::StructType>(type)) {

    // Get all of the struct types used in the module.
    std::string structName = modInfo.getTypeName(type);
    if (!structName.empty())
      return structName;
    else
      Log::fail(type, "anonymous structs not implemented");
  }

  return getScalarType(type);
}

/*
 * build a C-style declaration for @root of type @type
 */
std::string CWriter::buildDeclaration(std::string root,
                                      const llvm::Type *type) {
  if (llvm::isa<llvm::ArrayType>(type)) {
    const llvm::ArrayType *arrType = llvm::cast<llvm::ArrayType>(type);
    return buildDeclaration(root + "[" + str<int>(arrType->getNumElements()) +
                                "]",
                            arrType->getElementType());

  } else if (llvm::isa<llvm::VectorType>(type)) {
    const llvm::VectorType *vectorType = llvm::cast<llvm::VectorType>(type);
    if (vectorType->getVectorElementType()->isPointerTy()) {
      return "vr" + str(vectorType->getNumElements()) + "p " + root;
    }
    return getScalarType(vectorType->getElementType(), true) +
           str<int>(vectorType->getNumElements()) + " " + root;

  } else if (llvm::isa<llvm::PointerType>(type)) {
    const llvm::PointerType *ptrType = llvm::cast<llvm::PointerType>(type);
    const llvm::Type *elementType = ptrType->getElementType();
    {
      return buildDeclaration("(*" + root + ")", elementType);
    }

  } else if (llvm::isa<llvm::StructType>(type)) {
    std::string name;
    // Get all of the struct types used in the module.

    std::string structName = modInfo.getTypeName(type);
    if (!structName.empty())
      // return "struct " + structName + " " + root;
      return structName + " " + root;
    else
      Log::fail(type, "anonymous structs not implemented");
  }

  std::string scalarStr = getScalarType(type);
  return scalarStr + " " + root;
}

/*
 * ##### DECLARATIONS / OPERATORS & INSTRUCTIONS ######
 */

// returns true if pure (only used in read_image or write_image)

inline bool checkImageAccess(llvm::Value *imageVal, bool &oReadOnly) {
  typedef llvm::Value::use_iterator UseIt;
  UseIt itStart = imageVal->use_begin();
  UseIt itEnd = imageVal->use_end();

  bool hasRead = false;
  bool hasWrite = false;
  for (UseIt it = itStart; it != itEnd; ++it) {
    llvm::CallInst *callInst = llvm::dyn_cast<llvm::CallInst>(*it);
    if (callInst) {
      std::string calledName = callInst->getCalledFunction()->getName();
      hasWrite |= calledName.substr(0, 11) == "write_image";
      hasRead |= calledName.substr(0, 10) == "read_image";

      if (hasRead && hasWrite) {
        return false;
      }
    }
  }

  oReadOnly = hasRead;
  return hasRead != hasWrite; // do not allow specialization if no uses occur
}

/*
 * writes a generic function header and declares the arguments as mapped by
 * @locals
 */
std::string CWriter::getFunctionHeader(llvm::Function *func,
                                       IdentifierScope *locals) {
  std::stringstream builder;
  const llvm::FunctionType *type = getFunctionType(func);
  const llvm::Type *returnType = type->getReturnType();

  // attributes
#if 0
	if (modInfo.isKernelFunction(func)) {
		builder << "__kernel ";
	} else
#endif

  if (func->hasFnAttribute(llvm::Attribute::InlineHint) ||
      func->hasFnAttribute(llvm::Attribute::AlwaysInline)) {
    builder << "inline ";
  }

  // return type
  std::string typeStr = getType(returnType);
  builder << typeStr;

  // function name
  builder << ' ' << func->getName().str();

  // catch empty lists
  if (func->arg_empty()) {
    builder << "()";
    return builder.str();
  }

  // create generic argument list
  bool firstArg = true;
  for (auto &arg : func->args())
  // for(i = 0, arg = argList.begin();
  // 	i < type->getNumParams() && arg != argList.end();
  // 	++i, ++arg)
  {
    const llvm::Type *argType = arg.getType();

    // dereference byVal pointers
    if (arg.hasByValAttr()) {
      argType = llvm::cast<llvm::PointerType>(argType)->getElementType();
    }

    // std::string modifierStr = inferModifiers(arg);

    std::string argName;
    if (locals) {
      const VariableDesc *desc = locals->lookUp(&arg);
      argName = desc->name;
    }

    if (firstArg) {
      builder << '(';
      firstArg = false;
    } else {
      builder << ", ";
    }

    builder << buildDeclaration(argName, argType);
  }

  builder << ')';
  return builder.str();
}

std::string CWriter::getFunctionHeader(llvm::Function *func) {
  return getFunctionHeader(func, NULL);
}

void CWriter::writeVariableDeclaration(const VariableDesc &desc) {
  const llvm::Type *type = desc.type;
  if (desc.isAlloca) {
    type = desc.type->getContainedType(0);
  }

  putLine(buildDeclaration(desc.name, type) + ";");
}

void CWriter::writeFunctionDeclaration(llvm::Function *func,
                                       IdentifierScope *locals) {
  putLine(getFunctionHeader(func, locals) + ';');
}

std::string CWriter::getInstruction(llvm::Instruction *inst,
                                    std::vector<std::string> operands) {
  return getOperation(WrappedInstruction(inst), operands);
}

// serializer representation of a SCEV
std::string CWriter::getSCEV(const llvm::SCEV *scev, IdentifierScope &locals) {
  std::string opToken;
  bool infix = true;
  bool signedOp = true;

  switch (static_cast<SCEVTypes>(scev->getSCEVType())) {
  case scConstant: {
    auto *scevConst = dyn_cast<SCEVConstant>(scev);
    return getConstant(scevConst->getValue(), locals);
  }

  case scAddExpr:
    opToken = "+";
    break;
  case scMulExpr:
    opToken = "*";
    break;
  case scUDivExpr:
    opToken = "/";
    signedOp = false;
    break;

  case scUMaxExpr:
    opToken = "max";
    infix = false;
    signedOp = false;
    break;
  case scSMaxExpr:
    opToken = "max";
    infix = false;
    break;
  case scAddRecExpr: {
    assert(false && "implement");
  }

  case scTruncate:
  case scZeroExtend:
  case scSignExtend:
    assert(false && "implement");

    // use variable references
  case scUnknown: {
    auto *scevUnknown = dyn_cast<SCEVUnknown>(scev);
    scevUnknown->getValue();
  }
  case scCouldNotCompute:
  default:
    abort();
  }

  std::vector<std::string> operands;
  auto *narySCEV = dyn_cast<SCEVNAryExpr>(scev);
  for (uint i = 0; i < narySCEV->getNumOperands(); ++i) {
    const SCEV *opSCEV = narySCEV->getOperand(i);
    std::string scevText = getSCEV(opSCEV, locals);
    if (!signedOp) {
      auto *scevType = opSCEV->getType();
      std::string castStr = "(unsigned " + getType(scevType) + ")";
      operands.push_back(castStr + "(" + scevText + ")");
    } else {
      operands.push_back(scevText);
    }
  }

  // infix operators
  assert(!infix || operands.size() == 2);
  if (infix) {
    return "(" + operands[0] + opToken + operands[1] + ")";
  }

  // function call operators
  std::stringstream ss;
  ss << opToken << "(";
  for (uint i = 0; i < operands.size(); ++i) {
    if (i > 0)
      ss << ", ";
    ss << operands[i];
  }
  ss << ")";

  return ss.str();
}

// TODO move increment/initialization here and suppress all consumed
// instructions
void CWriter::writeForLoopBegin(ForLoopInfo &forInfo, IdentifierScope &locals) {

  std::string ivStr = locals.lookUp(forInfo.phi)->name;
  // std::string beginStr = getValueToken(forInfo.beginValue, locals);
  std::string beginStr = getValueToken(
      forInfo.beginValue, locals); // locals.lookUp(forInfo.phi)->name + "_in";

  std::string exitConditionStr =
      getComplexExpression(forInfo.headerBlock, forInfo.exitCond, locals);

  Instruction *incInst = cast<Instruction>(forInfo.ivIncrement);
  std::string ivIncrementStr = getInstructionAsExpression(incInst, locals);

  if (!forInfo.exitOnFalse) {
    exitConditionStr = "!(" + exitConditionStr + ")";
  }
#if 1
  if (forInfo.ivParallelLoop) {
    putLine("#pragma simd");
  }
#endif

  putLine("for (" + ivStr + "=" + beginStr + ";" + exitConditionStr + ";" +
          ivStr + "=" + ivIncrementStr + ")");
}

std::string
CWriter::getVectorConvert(const WrappedOperation & op, StringVector operands) {
  abort(); // TODO implement
}

std::string
CWriter::getVectorTruncate(const WrappedOperation & op, StringVector operands) {
  abort(); // TODO implement
}

std::string
CWriter::getVectorCompare(const WrappedOperation &op,
                          StringVector operands) {
  abort(); // missing vfmk builtin
}

std::string CWriter::getOperation(const WrappedOperation &op,
                                  StringVector operands) {
  std::string tmp;

  //# binary infix operator
  if (op.isBinaryOp() || op.isCompare()) {
    bool signedOps = true;
    if (op.isCompare() && op.getValue()->getType()->isVectorTy()) {
      return getVectorCompare(op, operands);
    }

    std::string token = getOperatorToken(op, signedOps);

    if (token == "ord") {
      // NaN test
      return "(" + operands[0] + " == " + operands[0] + ")";
    }

    if (signedOps) {
      return "(" + operands[0] + token + operands[1] + ")";

    } else {
      const llvm::Type *operandType = op.getOperand(0)->getType();
      std::string opTypeStr = getType(operandType);
      std::string convUnsignedStr = "(unsigned " + opTypeStr + ")";

      return "(" + convUnsignedStr + "(" + operands[0] + ") " + token + " " +
             convUnsignedStr + "(" + operands[1] + "))";
    }

    //# bitcast
  } else if (op.isa(llvm::Instruction::BitCast)) {
    const llvm::Type *srcType = op.getOperand(0)->getType();
    const llvm::Type *destType = op.getType();

    assert(operands.size() == 1 && "cast a single value . . non?");

    // fake assignment instruction (cast to same type)
    if (srcType == destType) {
      return operands[0];

    } else {
      std::string typeStr = getType(destType);
      /*
               if (srcType->isPointerTy() && destType->isPointerTy()) {
                       return "(" + typeStr + ")(" + operands[0] + ")";
               } else { */
      return "(" + typeStr + ")(" + operands[0] + ")";
      //    	 }
    }

    //# reinterpreting cast
  } else if (op.isCast()) {
    const llvm::Type *sourceType = op.getOperand(0)->getType();
    const llvm::Type *targetType = op.getType();

    if (op.isa(llvm::Instruction::UIToFP)) { // integers are declared signed
      std::string intCast = "(unsigned " + getType(sourceType) + ")";
      return "(" + getType(targetType) + ")" + intCast + "(" + operands[0] +
             ")";

      // truncation: mask out bits and cast to smaller type
    } else if (op.isa(llvm::Instruction::Trunc)) {
      size_t destWidth = targetType->getScalarSizeInBits();
      uint64_t maskInt = generateTruncMask(destWidth);

      std::string fittedStr =
          "(" + operands[0] + ") & 0x" +
          convertToHex(maskInt, std::max<int>(1, destWidth / 4));

      // convert_bool is not supported
      return (destWidth == 1 ? "(bool)" : "(" + getType(targetType) + ")") +
             "(" + fittedStr + ")";

    } else if (!targetType->isIntegerTy(1)) { // use ints for bools
      bool isUnsigned = op.isa(llvm::Instruction::ZExt);
      std::string targetTypeStr = getType(targetType);
      std::string srcTypeStr = getType(sourceType);

      // special cast bool to int case
      if (sourceType->isIntegerTy(1)) {
        std::string targetCastStr;
        std::string suffixStr;
        if (isUnsigned) {
          targetCastStr =
              "(" + targetTypeStr + ")((unsigned " + targetTypeStr + ")(";
          suffixStr += "))";
        } else {
          targetCastStr = "(" + targetTypeStr + ")(";
          suffixStr += ")";
        }
        return targetCastStr + operands[0] + suffixStr;
      }

      // we need to operate on unsigned data types to get a zero extension
      if (isUnsigned) {
        std::string srcCastStr = "(unsigned " + srcTypeStr;
        std::string targetCastStr = "(" + targetTypeStr + ")";
        return targetCastStr + srcCastStr + "(" + operands[0] + "))";

        // bool conversions and sign/float extension will do without casts
        // (hopefully)
      } else {
        return "(" + getType(targetType) + ")(" + operands[0] + ")";
      }
    }

    //# select
  } else if (op.isa(llvm::Instruction::Select)) {
    return operands[0] + "? " + operands[1] + ": " + operands[2];

    //# function call
  } else if (op.isa(llvm::Instruction::Call)) {
    assert(!llvm::isa<llvm::ConstantExpr>(op.getValue()) &&
           "ConstantExpr call not implemented");

    llvm::CallInst *caller = llvm::cast<llvm::CallInst>(op.getValue());
    llvm::Function *callee = caller->getCalledFunction();

    StringVector::const_iterator beginParams = operands.begin();
    StringVector::const_iterator endParams = operands.end();
    StringRef calleeName = callee->getName();

    if (platform.implements(callee)) {
      return platform.build(callee->getName().str(), beginParams, endParams);

    } else if (calleeName == "rv_any_v256") {
      return "(0 != __builtin_ve_mpcnt(" + *beginParams  + ")";

    } else {
      tmp = callee->getName().str(); // add function name
    }

    tmp += '(';
    for (StringVector::const_iterator itOp = beginParams; itOp != endParams;
         ++itOp) {
      if (itOp != beginParams)
        tmp += ", " + *itOp;
      else
        tmp += *itOp;
    }
    tmp += ')';
    return tmp;

    //# ERROR handling (foremost generic opcode based scheme)
  }

  Log::fail(std::string(llvm::Instruction::getOpcodeName(op.getOpcode())) +
            "unimplemented instruction type");
  abort();
}

typedef std::vector<llvm::Value *> ValueVector;

std::string CWriter::getReferer(llvm::Value *value, IdentifierScope &locals) {
  return getValueToken(value, locals);
}

/*
 * return a dereferencing string for the next type node of the object using
 * address
 */
std::string CWriter::dereferenceContainer(std::string root,
                                          const llvm::Type *type,
                                          AddressIterator *&address,
                                          IdentifierScope &locals,
                                          const llvm::Type *&oElementType,
                                          uint addressSpace) {
  if (llvm::isa<llvm::StructType>(type)) {
    uint64_t index;
    llvm::Value *indexVal = address->getValue();

    if (!evaluateInt(indexVal, index)) {
      Log::fail(type, "can not dynamically access struct members");
    }

    address = address->getNext();
    oElementType = type->getContainedType(index);

    return "(" + root + ").x" + str<int>(index);

  } else if (llvm::isa<llvm::ArrayType>(type)) {
    oElementType = type->getContainedType(0);
    return buildArraySubscript(root, address, locals);

  } else if (llvm::isa<llvm::PointerType>(type)) {
    // address = address->getNext();
    oElementType = type->getPointerElementType();
    return buildArraySubscript(root, address, locals);
  // cast to pointer and dereference from there (slightly hacky)
  } else if (llvm::isa<llvm::VectorType>(type)) {
    const llvm::VectorType *vecType = llvm::cast<llvm::VectorType>(type);
    llvm::Type *elemType = type->getContainedType(0);
    uint width = vecType->getNumElements();
    llvm::ArrayType *arrType = llvm::ArrayType::get(elemType, width);
    llvm::PointerType *ptrToElemType =
        llvm::PointerType::get(elemType, addressSpace);

    std::string castRootStr = "((" + getType(ptrToElemType) + ")&(" + root + "))";

    return dereferenceContainer(castRootStr, arrType, address, locals,
                                oElementType, addressSpace);
  }

  Log::fail(type, "can not dereference this type");
} // namespace axtor

std::string CWriter::getVolatilePointerTo(llvm::Value *val,
                                          IdentifierScope &locals,
                                          const std::string *rootName) {
  std::string ptrStr = getPointerTo(val, locals, rootName);
  const llvm::Type *type = val->getType();
  std::string castStr = "volatile " + getType(type);
  return "(" + castStr + ")(" + ptrStr + ")";
}

/*
 * return a name representing a dereferenced pointer
 *if noImplicitDeref is false, the exact address of the value is returned
 *
 *@param rootName  if set, the rootName is used as base-string for
 *dereferencing, otw the detected base value is look-up in @locals
 *@param oDereferenced returns if the pointer is already dereferenced
 */

std::string CWriter::unwindPointer(llvm::Value *val, IdentifierScope &locals,
                                   bool &oDereferenced,
                                   const std::string *rootName) {
  uint addressSpace = 0;
  // uncover the address space, if this is a pointer
  if (llvm::isa<llvm::PointerType>(val->getType())) {
    const llvm::PointerType *ptrType =
        llvm::cast<llvm::PointerType>(val->getType());
    addressSpace = ptrType->getAddressSpace();
  }

  AddressIterator::AddressResult result =
      AddressIterator::createAddress(val, platform.getDerefFuncs());
  ResourceGuard<AddressIterator> __guardAddress(result.iterator);

  AddressIterator *address = result.iterator;
  llvm::Value *rootValue = result.rootValue;

  const llvm::Type *rootType = rootValue->getType();

#ifdef DEBUG
  std::cerr << "dereferencing value:\n";
  val->dump();
  std::cerr << "root value:\n";
  rootValue->dump();
  std::cerr << "root type:\n";
  rootType->dump();
  std::cerr << "address:\n";
  if (address)
    address->dump();
  else
    std::cerr << "NONE\n";
#endif

  // allocas are dereffed by their name string
  bool hasImplicitPtrDeref = llvm::isa<llvm::AllocaInst>(rootValue);

  // byval function arguments don't need be dereferenced
  if (llvm::isa<llvm::Argument>(rootValue)) {
#ifdef DEBUG
    std::cerr << "DEREF: argument case\n";
#endif
    llvm::Argument *arg = llvm::cast<llvm::Argument>(rootValue);
    hasImplicitPtrDeref |= arg->hasByValAttr();
  }

  // local variables are initialised in the program, so assume implicit deref
  if (llvm::isa<llvm::GlobalVariable>(rootValue)) {
#ifdef DEBUG
    std::cerr << "DEREF: Global case\n";
#endif

    // llvm::GlobalVariable * gv = llvm::cast<llvm::GlobalVariable>(rootValue);
    hasImplicitPtrDeref = true; // FIXME
#if 0
		hasImplicitPtrDeref |= gv->isConstant() || gv->getType()->getAddressSpace() == SPACE_LOCAL || gv->getType()->getAddressSpace() == SPACE_CONSTANT;
#endif
  }

  // this variable is implicitly dereferenced by its designator
  if (hasImplicitPtrDeref) {
    if (address) {
      uint64_t test;
      assert(evaluateInt(address->getValue(), test) && test == 0 &&
             "skipped non-trivial dereferencing value");
      address = address->getNext();
    }
    rootType = rootType->getContainedType(0);
  }

  // dereff the initial pointer (if it points to a more complex structure)
  std::string tmp;
  if (rootName) {
    tmp = *rootName;

  } else if (llvm::isa<llvm::Constant>(rootValue)) {
    tmp = getConstant(llvm::cast<llvm::Constant>(rootValue), locals);

  } else {
    const VariableDesc *desc = locals.lookUp(rootValue);
    assert(desc && "root value was not mapped");
    tmp = desc->name;
  }

  // this is a pointer
  if (!address) {
#ifdef DEBUG
    std::cerr << "DEREF: real pointer case. deref: " << hasImplicitPtrDeref
              << "\n";
#endif

    oDereferenced = hasImplicitPtrDeref;
    return tmp;
  }

  while (address) {
#ifdef DEBUG
    std::cerr << "deref : " << tmp << "\n";
    assert(rootType && "was not set");
    rootType->dump();
#endif
    const llvm::Type *elementType = 0;
    tmp = dereferenceContainer(tmp, rootType, address, locals, elementType,
                               addressSpace);
    // assert(elementType && "derefContainer did not set element type");
    rootType = elementType;
#ifdef DEBUG
    std::cerr << "dereferenced to " << tmp << "\n";
#endif
  }

  oDereferenced = true;

  return tmp;
}

std::string CWriter::getAllNullLiteral(const llvm::Type *type) {
  switch (type->getTypeID()) {
  case llvm::Type::VectorTyID: {
    const llvm::VectorType *arrType = llvm::cast<llvm::VectorType>(type);
    // uint size = arrType->getNumElements();
    std::string elementStr = getAllNullLiteral(arrType->getElementType());
    auto * zeroConst = Constant::getNullValue(arrType->getElementType());
    if (type->isVectorTy()) {
      return getBroadcast(elementStr, *type->getVectorElementType());
    } else {
      return "(" + getType(type) + ")(" + elementStr + ")";
    }
  }

    // case llvm::Type::StructTyID:
    // case llvm::Type::ArrayTyID:

    /*					const llvm::ArrayType * arrType =
       llvm::cast<llvm::ArrayType>(type); uint size = arrType->getNumElements();
                                    std::string elementStr =
       getAllNullLiteral(arrType->getElementType());

                                    std:string accu = "{";
                                    for(int i = 0; i < size; ++i)
                                    {
                                            if (i > 0) accu += ", ";
                                            accu += elementStr;
                                    }
                                    accu += "}";
                                    return accu;*/

  case llvm::Type::DoubleTyID:
    return "0.0f";

  case llvm::Type::FloatTyID:
    return "0.0f";

  case llvm::Type::PointerTyID:
    Log::warn(type, "Using null pointer literal");
  case llvm::Type::IntegerTyID:
    return "0";

  default:
    Log::fail(type,
              "OpenCL does not natively support null literals of this kind");
  }
}

static bool
IsUniformVector(llvm::Constant & val) {
  ResourceGuard<WrappedLiteral> vector(CreateLiteralWrapper(&val));
  auto * elem = vector->getOperand(0);
  for (int i = 1; i < vector->getNumOperands(); ++i) {
    if (elem != vector->getOperand(i)) return false;
  }
  return true;
}


std::string
CWriter::getBroadcast(std::string laneText, llvm::Type & laneType) {
  auto suffix = GetVectorSuffix(laneType);
#if 0
  std::string token;
  if (isa<Constant>(val)) {
    token = getLiteral(&cast<Constant>(val));
  } else {
    token = getValueToken(&val, *scope);
  }
#endif
  return "__builtin_ve_vbrd" + suffix + "(" + laneText + ")";
}

/*
 * return the string representation of a constant
 */
std::string CWriter::getLiteral(llvm::Constant *val) {
  //## Constant integer (and Bool)
  if (llvm::isa<llvm::ConstantInt>(val)) {
    const llvm::IntegerType *intType =
        llvm::cast<llvm::IntegerType>(val->getType());
    if (intType->getBitWidth() == 1) {
      if (val->isNullValue()) {
        return "false";
      } else {
        return "true";
      }
    } else if (intType->getBitWidth() <= 64) {
      llvm::ConstantInt *constInt = llvm::cast<llvm::ConstantInt>(val);
      uint64_t data = constInt->getLimitedValue();
      std::string hexStr = convertToHex(data, intType->getBitWidth() / 4);
      std::string typeStr = getType(intType);

      return "(" + typeStr + ")(0x" + hexStr + ")";
    } else {
      Log::fail(val, "value exceeds size limit, expected bit size <= 64, was " +
                         str<int>(intType->getBitWidth()));
    }

    //## Constant Float
  } else if (llvm::isa<llvm::ConstantFP>(val)) {
    llvm::ConstantFP *constFP = llvm::cast<llvm::ConstantFP>(val);
    llvm::APFloat apFloat = constFP->getValueAPF();
    if (constFP->getType()->isFloatTy()) {
      float num = apFloat.convertToFloat();
      return "(float)(" + str<float>(num) + ")";
    } else if (constFP->getType()->isDoubleTy()) {
      double num = apFloat.convertToDouble();
      return "(double)(" + str<double>(num) + ")";
    } else {
      Log::fail(val, "Unsupported constant float");
    }

    //## Function
  } else if (llvm::isa<llvm::Function>(val)) {
    return val->getName();

    //## Constant Array
  } else if (llvm::isa<llvm::ConstantArray>(val) ||
             llvm::isa<llvm::ConstantDataArray>(val)) {

    ResourceGuard<WrappedLiteral> arr(CreateLiteralWrapper(val));

    std::string buffer = "{";
    const llvm::ArrayType *arrType =
        llvm::cast<llvm::ArrayType>(val->getType());
    for (uint i = 0; i < arrType->getNumElements(); ++i) {
      llvm::Constant *elem = llvm::cast<llvm::Constant>(arr->getOperand(i));
      if (i > 0)
        buffer += "," + getLiteral(elem);
      else
        buffer += getLiteral(elem);
    }
    buffer += "}";
    return buffer;

  } else if (llvm::isa<llvm::ConstantVector>(val) ||
             llvm::isa<llvm::ConstantDataVector>(val)) {

    ResourceGuard<WrappedLiteral> vector(CreateLiteralWrapper(val));

    EXPENSIVE_TEST if (!vector.get()) {
      Log::fail(val, "unrecognized constant type");
    }

    const llvm::VectorType *vectorType =
        llvm::cast<llvm::VectorType>(val->getType());

    if (IsUniformVector(*val)) {
      auto * elem = vector->getOperand(0);
      ConstVariableMap varMap;
      IdentifierScope emptyScope(varMap);
      return getBroadcast(getValueToken(elem, emptyScope), *elem->getType());
    }

    abort(); // not implements (below code does not work for NCC)
    std::string buffer = "";
    for (uint i = 0; i < vectorType->getNumElements(); ++i) {
      llvm::Value *opVal = vector->getOperand(i);
      std::string opStr = getLiteral(llvm::cast<llvm::Constant>(opVal));

      if (i > 0) {
        buffer += ", ";
      }
      buffer += opStr;
    }

    return "(" + getType(vectorType) + ")(" + buffer + ")";

    // default undefined values to zero initializers
  } else if (llvm::isa<llvm::UndefValue>(val) || val->isNullValue()) {
    const llvm::Type *type = val->getType();
    return getAllNullLiteral(type);
  }

  //## unsupported literal
  Log::fail(val, "unsupported literal");
  assert(false);
}

/*
 * tries to create a literal string it @val does not have a variable
 */
std::string CWriter::getValueToken(llvm::Value *val, IdentifierScope &locals) {
  const VariableDesc *desc = locals.lookUp(val);
  if (desc) {
    return desc->name;
  } else {
    assert(llvm::isa<llvm::Constant>(val) &&
           "undeclared value is not a constant");
    return getLiteral(llvm::cast<llvm::Constant>(val));
  }
}

/*
 * returns the string representation of a non-instruction value
 */
std::string CWriter::getNonInstruction(llvm::Value *value,
                                       IdentifierScope &locals) {
  assert(!llvm::isa<llvm::Instruction>(value) &&
         "should only be called for non-instruction values");

  const VariableDesc *desc = NULL;

  if (llvm::isa<llvm::GlobalValue>(value) &&
      (desc = locals.getParent()->lookUp(value))) {
    return desc->name;

  } else if (llvm::isa<llvm::PHINode>(value)) { // PHI-Node
    const VariableDesc *phiDesc = locals.lookUp(value);
    assert(phiDesc && "unmapped PHI-Node");
    return phiDesc->name;

  } else if (llvm::isa<llvm::Constant>(value)) {
    return getConstant(llvm::cast<llvm::Constant>(value), locals);
  }

  Log::fail(value, "failure : could not translate nonInstruction");
  return "";
}

/*
 * returns the string representation of a ShuffleInstruction
 */
std::string CWriter::getShuffleInstruction(llvm::ShuffleVectorInst *shuffle,
                                           IdentifierScope &locals) {
  llvm::Value *firstVector = shuffle->getOperand(0);
  const llvm::VectorType *firstType =
      llvm::cast<const llvm::VectorType>(firstVector->getType());
  llvm::Value *secondVector = shuffle->getOperand(1);
  // const llvm::VectorType * secondType = llvm::cast<const
  // llvm::VectorType>(secondVector->getType());
  llvm::Value *indexVector = shuffle->getOperand(2);
  const llvm::VectorType *indexType =
      llvm::cast<const llvm::VectorType>(indexVector->getType());

  llvm::Type *elementType = firstType->getElementType();

  int secondBase = firstType->getNumElements();
  int numIndices = indexType->getNumElements();

  std::string firstStr = getValueToken(firstVector, locals);

  const VariableDesc *secondDesc = locals.lookUp(secondVector);
  bool hasSecond = !llvm::isa<llvm::UndefValue>(secondVector);

  std::string secondStr;
  if (hasSecond) {
    secondStr = secondDesc
                    ? secondDesc->name
                    : getLiteral(llvm::cast<llvm::Constant>(secondVector));
  }

  // broadcast from idiom
  // vFirst = insert undef, 0, % elem
  // vBroadcast = shuffle vFirst, undef, <0, .., 0>
  auto cData = dyn_cast<ConstantData>(indexVector);
  if (cData && cData->isNullValue()) {
    auto * insertInst = dyn_cast<InsertElementInst>(firstVector);
    if (insertInst) {
      llvm::Value *vec = insertInst->getOperand(0);
      llvm::Value *elem = insertInst->getOperand(1);
      llvm::Value *idxVal = insertInst->getOperand(2);
      if (isa<UndefValue>(vec)) {
       return getBroadcast(getValueToken(elem, locals), *elem->getType());
      }
    }
  } else {
    abort(); // implement shuffle
  }

#ifdef DEBUG
  std::cerr << "SHUFFLE:\n"
            << "first=" << firstStr << "\n"
            << "second=" << secondStr << "\n"
            << "\n";
#endif

  // get the target types name
  std::string typeStr = getType(shuffle->getType());

  // build a string extracting values from one of the two vectors
  std::string accu = "";
  std::string elements = firstStr + ".s";

  int firstMask = shuffle->getMaskValue(0);
  bool wasLiteral = false;
  bool useFirst = firstMask < secondBase;

  if (firstMask >= 0) {
    if (useFirst) {
      elements = firstStr + ".s";
    } else {
      elements = secondStr + ".s";
    }
    wasLiteral = false;
  } else {
    elements = getLiteral(llvm::Constant::getNullValue(elementType));
    wasLiteral = true;
  }

  for (int i = 0; i < numIndices; ++i) {
    int mask = shuffle->getMaskValue(i);

    // set up the element source (last was literal, current is literal unlike
    // the last OR change of source vector)
    if (wasLiteral || (mask < 0) || (useFirst != (mask < secondBase))) {
      accu += elements;
      wasLiteral = false;

      if (mask >= 0) {
        useFirst = mask < secondBase;
        if (useFirst) {
          elements = ", " + firstStr + ".s";
        } else {
          assert(hasSecond && "trying to access elements from undef vector");
          elements = ", " + secondStr + ".s";
        }
      }
    }

    // pick elements
    if (mask < 0) {
      wasLiteral = true;
      elements = ", " + getLiteral(llvm::Constant::getNullValue(elementType));
    } else {
      wasLiteral = false;
      if (useFirst) {
        elements += hexstr(mask);
      } else {
        elements += hexstr(mask - secondBase);
      }
    }
  }

  // add last element
  accu += elements;

  return "(" + typeStr + ")(" + accu + ")";
}

/*
 * returns the string representation of an ExtractElementInstruction
 */
std::string
CWriter::getExtractElementInstruction(llvm::ExtractElementInst *extract,
                                      IdentifierScope &locals) {
  llvm::Value *indexVal = extract->getIndexOperand();
  llvm::Value *vectorVal = extract->getVectorOperand();
  std::string vectorStr = getValueToken(vectorVal, locals);

  uint64_t index;

  if (evaluateInt(indexVal, index)) {
    return vectorStr + ".s" + hexstr(index);
  }

  Log::fail(extract, "can not randomly extract values from a vector");
  abort();
}

void CWriter::writeInsertValueInstruction(llvm::InsertValueInst *insert,
                                          IdentifierScope &locals) {
  llvm::Value *derefValue = insert->getOperand(0);
  llvm::Value *insertValue = insert->getOperand(1);

  const VariableDesc *insertDesc = locals.lookUp(insert);
  assert(insertDesc && "instruction not bound");
  const VariableDesc *targetDesc = locals.lookUp(derefValue);

  // assign initial value
  if (targetDesc) {
    writeAssign(*insertDesc, *targetDesc);

  } else if (!llvm::isa<llvm::UndefValue>(derefValue)) {
    assert(llvm::isa<llvm::Constant>(derefValue));
    llvm::Constant *derefConst = llvm::cast<llvm::Constant>(derefValue);
    writeAssignRaw(insertDesc->name, getLiteral(derefConst));
  }

  std::string derefStr = getReferenceTo(
      insert, locals, &insertDesc->name); // the original value was written to
                                          // the InsertValueInstructions variable

  // insert value
  std::string valueStr;
  const VariableDesc *valueDesc = locals.lookUp(insertValue);

  if (valueDesc) {
    valueStr = valueDesc->name;
  } else {
    assert(llvm::isa<llvm::Constant>(insertValue));
    llvm::Constant *insertConst = llvm::cast<llvm::Constant>(insertValue);
    valueStr = getLiteral(insertConst);
  }

  writeAssignRaw(derefStr, valueStr);
}

/*
 * returns the string representation of an InsertElementInstruction
 * if the vector value is defined this creates two instructions
 */
void CWriter::writeInsertElementInstruction(llvm::InsertElementInst *insert,
                                            IdentifierScope &locals) {
  llvm::Value *vec = insert->getOperand(0);
  llvm::Value *value = insert->getOperand(1);
  llvm::Value *idxVal = insert->getOperand(2);

  const VariableDesc *desc = locals.lookUp(insert);
  assert(desc && "undeclared instruction");
  std::string descStr = desc->name;

  // const llvm::VectorType * vecType =
  // llvm::cast<llvm::VectorType>(vec->getType()); const llvm::Type * elemType =
  // vecType->getElementType();

  const VariableDesc *vecDesc = locals.lookUp(vec);
  std::string vecStr;
  if (vecDesc) {
    vecStr = vecDesc->name;
  } else if (!llvm::isa<llvm::UndefValue>(vec)) {
    assert(llvm::isa<llvm::Constant>(vec) &&
           "non constant was not a declared variable");
    vecStr = getLiteral(llvm::cast<llvm::Constant>(vec));
  } else {
    vecStr = descStr; // modify undef
  }

  const VariableDesc *valueDesc = locals.lookUp(value);
  std::string valueStr;
  if (valueDesc) {
    valueStr = valueDesc->name;
  } else {
    assert(llvm::isa<llvm::Constant>(value) &&
           "non constant was not a declared variable");
    valueStr = getLiteral(llvm::cast<llvm::Constant>(value));
  }

  uint64_t index;
  if (!evaluateInt(idxVal, index)) {
    Log::fail(insert, "non-static parameter access");
  }
  auto suffixText = GetVectorSuffix(*value->getType());
  putLine(descStr + " = __builtin_ve_lsv" + suffixText + "(" + vecStr + ", " + str(index) + ", " + valueStr + ");");

#if 0
  if (!llvm::isa<llvm::UndefValue>(vec)) {
    putLine(descStr + " = " + vecStr + ";");
  }

  putLine(descStr + ".s" + hexstr(index) + " = " + valueStr + ";");
#endif
}

Value&
GetAccessedPointer(llvm::Instruction & memInst) {
  auto * store = dyn_cast<llvm::StoreInst>(&memInst);
  auto * load = dyn_cast<llvm::LoadInst>(&memInst);
  if (store) return *store->getPointerOperand();
  else return *load->getPointerOperand();
}

Value*
GetStoredValue(llvm::Instruction & inst) {
  auto * store = dyn_cast<StoreInst>(&inst);
  if (!store) return nullptr;
  return store->getValueOperand();
}

std::string
CWriter::getRandomVectorAccess(llvm::Type & dataTy, llvm::Value & ptrVal, IdentifierScope & locals, llvm::Value * storedValue) {
  abort(); // TODO implement
}

std::string
CWriter::getConsecutiveVectorAccess(llvm::Type & dataTy, llvm::Value & ptrVal, IdentifierScope & locals, llvm::Value * storedValue) {
  auto suffix = GetVectorSuffix(dataTy);

  auto ptrText = getPointerTo(&ptrVal, locals);
  const std::string strideText = str<>(dataTy.getPrimitiveSizeInBits() / 8); // FIXME use DataLayout store size instead

  if (storedValue) {
    auto storedTxt = getValueToken(storedValue, locals);
    return "__builtin_ve_vst" + suffix + "("
      + storedTxt + ", "
      + ptrText + ", "
      + strideText + ")";
  } else {
    return "__builtin_ve_vld" + suffix + "("
      + ptrText + ", "
      + strideText + ")";
  }
}

std::string
CWriter::getVectorAccess(llvm::Instruction & inst, IdentifierScope & locals) {
  auto & ptr = GetAccessedPointer(inst);
  auto * ptrTy = ptr.getType();
  auto * storedVal = GetStoredValue(inst);
  if (ptrTy->isPointerTy()) {
    return getConsecutiveVectorAccess(*ptrTy->getPointerElementType()->getVectorElementType(), ptr, locals, storedVal);
  } else {
    return getRandomVectorAccess(*ptrTy->getVectorElementType()->getPointerElementType()->getVectorElementType(), ptr, locals, storedVal);
  }
}

std::string
CWriter::getLoadStore(llvm::Instruction & inst, IdentifierScope & locals) {
  auto * storedVal = GetStoredValue(inst);
  auto * load = dyn_cast<llvm::LoadInst>(&inst);
  auto & pointer = GetAccessedPointer(inst);

  if (inst.getType()->isVectorTy() ||
     (storedVal && storedVal->getType()->isVectorTy()))
  {
    // builtin memory access
    return getVectorAccess(inst, locals);

  } else if (load) {
    // generic load
    if (load->isVolatile()) {
      return "*(" + getVolatilePointerTo(&pointer, locals) + ")";
    } else {
      return getReferenceTo(&pointer, locals);
    }

  } else {
    // generic store
    llvm::StoreInst &store = llvm::cast<llvm::StoreInst>(inst);

    const VariableDesc *valueDesc = locals.lookUp(store.getOperand(0));

    llvm::Value *op = storedVal;

    std::string srcString;
    // fetch reference of pointer target to obtain the address value
    if (llvm::isa<llvm::PointerType>(op->getType())) {
      srcString = getPointerTo(op, locals);

      // pass values by identifier
    } else if (valueDesc) {
      srcString = valueDesc->name;

    } else { // otw
      assert(llvm::isa<llvm::Constant>(op));
      srcString = getLiteral(llvm::cast<llvm::Constant>(op));
    }

    // decode the GEP and store the value
    if (store.isVolatile()) {
      std::string ptrText = getVolatilePointerTo(&pointer, locals);
      std::string name = "*(" + ptrText + ")";
      writeAssignRaw(name, srcString);
    } else {
      std::string name = getReferenceTo(&pointer, locals);
      IF_DEBUG std::cerr << "store to " << name << "\n";
      writeAssignRaw(name, srcString);
    }
    return "<written>";
  }
}

/*
 * write a single instruction or atomic value as isolated expression
 */
std::string CWriter::getInstructionAsExpression(llvm::Instruction *inst,
                                                IdentifierScope &locals) {
  // catch loads as they need a dereferenced operand
  if (llvm::isa<llvm::LoadInst>(inst)) {
    return getLoadStore(*inst, locals);

    // interpret this PHINode as a variable assignment
  } else if (llvm::isa<llvm::PHINode>(inst)) {
    llvm::PHINode *phi = llvm::cast<llvm::PHINode>(inst);
    const VariableDesc *commonDesc = locals.lookUp(phi->getIncomingValue(0));
    return commonDesc->name;

    // function call
  } else if (llvm::isa<llvm::CallInst>(inst)) {
    llvm::CallInst *call = llvm::cast<llvm::CallInst>(inst);
    llvm::Function *callee = call->getCalledFunction();

#if 0
		// OpenCL special treatment of barrier/memfence instructions
		if (callee->getName() == "barrier" ||
			callee->getName() == "mem_fence")
		{
			std::string calleeName = callee->getName();

			llvm::Value * arg = call->getArgOperand(0);
			std::string enumStr;
			if (! evaluateEnum_MemFence(arg, enumStr)) {
				Log::fail(call, "expected enum value");
			}

			std::string result = calleeName + "(" + enumStr + ")";
			return result;

		} else
#endif
    { // regular intrinsic
#ifdef DEBUG
      std::cerr << "intrinsic call " << callee->getName().str() << "\n";
#endif
      std::vector<std::string> operands;

      int argIdx = 0;
      for (auto &arg : callee->args()) {
        llvm::Value *op = call->getArgOperand(argIdx++);

        const VariableDesc *opDesc = locals.lookUp(op);

        bool isByValue = arg.hasByValAttr();

        if (opDesc) {
          // global variables and allocas are dereferenced by their name
          if (opDesc->isAlloca || llvm::isa<llvm::GlobalVariable>(op)) {
            if (isByValue)
              operands.push_back(opDesc->name);
            else
              operands.push_back("&" + opDesc->name);

            // all other pointer values need explicit dereferencing
          } else {
            if (isByValue)
              operands.push_back("*(" + opDesc->name + ")");
            else
              operands.push_back(opDesc->name);
          }
        } else {
          std::string operandStr;

          //### operand instruction without parent block
          if (llvm::isa<llvm::Instruction>(op))
            operandStr = getInstructionAsExpression(
                llvm::cast<llvm::Instruction>(op), locals);
          else //### non-runtime dependent value
            operandStr = getValueToken(op, locals);

          operands.push_back(operandStr);
        }
      }

      return getInstruction(inst, operands);
    }

    //# shuffle vector expression
  } else if (llvm::isa<llvm::ShuffleVectorInst>(inst)) {
    return getShuffleInstruction(llvm::cast<llvm::ShuffleVectorInst>(inst),
                                 locals);

    //# extract element expression
  } else if (llvm::isa<llvm::ExtractElementInst>(inst)) {
    return getExtractElementInstruction(
        llvm::cast<llvm::ExtractElementInst>(inst), locals);

    //# dynamic expression (use deref and then obtain element ptr)
  } else if (llvm::isa<llvm::GetElementPtrInst>(inst)) {
    return getPointerTo(inst, locals);

  } else if (llvm::isa<llvm::ExtractValueInst>(inst)) {
#ifdef DEBUG
    std::cerr << "is extract value(!)\n";
#endif
    return getReferenceTo(inst, locals);
  }

  assert(!llvm::isa<llvm::InsertElementInst>(inst) &&
         "insert element instructions is translated into multiple statements "
         "(use writeInsertElementInstruction)");

  /*
   * generic call-style instruction
   */
  std::vector<std::string> operands;
  for (uint opIdx = 0; opIdx < inst->getNumOperands(); ++opIdx) {
    llvm::Value *op = inst->getOperand(opIdx);
    const VariableDesc *opDesc = locals.lookUp(op);

    if (opDesc) {
      // assigned value
      if (opDesc->isAlloca) {
        operands.push_back("&" + opDesc->name);
      } else {
        operands.push_back(opDesc->name);
      }

    } else { // inline entity
      std::string operandStr;

      //### operand instruction without parent block
      if (llvm::isa<llvm::Instruction>(op))
        operandStr = getInstructionAsExpression(
            llvm::cast<llvm::Instruction>(op), locals);
      else //### non-runtime dependent value
        operandStr = getValueToken(op, locals);

      operands.push_back(operandStr);
    }
  }

  return getInstruction(inst, operands);
}

/*
 * write a complex expression made up of elements from valueBlock, starting from
 * root, writing all included insts to @oExpressionInsts
 */
std::string CWriter::getComplexExpression(llvm::BasicBlock *valueBlock,
                                          llvm::Value *root,
                                          IdentifierScope &locals,
                                          InstructionSet *oExpressionInsts) {
  // PHI-Nodes cant be part of a single expression
  if (llvm::isa<llvm::Instruction>(root) && !llvm::isa<llvm::PHINode>(root) &&
      !llvm::isa<llvm::GetElementPtrInst>(root)) {
    llvm::Instruction *inst = llvm::cast<llvm::Instruction>(root);
    if (inst->getParent() ==
        valueBlock) { // recursively decompose into operator queries

      if (oExpressionInsts)
        oExpressionInsts->insert(inst);

      std::vector<std::string> operands;
      for (uint opIdx = 0; opIdx < inst->getNumOperands(); ++opIdx) {
        llvm::Value *op = inst->getOperand(opIdx);

        operands.push_back(
            getComplexExpression(valueBlock, op, locals, oExpressionInsts));
      }

      return getInstruction(inst, operands);
    }
  }

  return getValueToken(root, locals);
}

/*
 * writes a generic function header for utility functions and the default
 * signature for the shade func
 */
void CWriter::writeFunctionHeader(llvm::Function *func,
                                  IdentifierScope *locals) {
  putLine(getFunctionHeader(func, locals));
}

void CWriter::writeInstruction(const VariableDesc *desc,
                               llvm::Instruction *inst,
                               IdentifierScope &locals) {
#ifdef DEBUG
  std::cerr << "writeInstruction:: var=" << (desc ? desc->name : "NULL")
            << std::endl;
  std::cerr << '\t';
  inst->dump();
#endif
  // dont write instructions consumed by their value users
  if (llvm::isa<llvm::GetElementPtrInst>(inst) ||
      llvm::isa<llvm::AllocaInst>(inst))
    return;

  // ### PHI-Node
  if (llvm::isa<llvm::PHINode>(inst)) {
    const VariableDesc *phiDesc = locals.lookUp(inst);
    writeAssignRaw(phiDesc->name, phiDesc->name + "_in");
    return;
  }

  //### Store instruction
  if (llvm::isa<llvm::StoreInst>(inst)) {
    putLine(getLoadStore(*inst, locals) + ";");

    //### InsertElement Instruction
  } else if (llvm::isa<llvm::InsertElementInst>(inst)) {
    writeInsertElementInstruction(llvm::cast<llvm::InsertElementInst>(inst),
                                  locals);

    //### InsertValue Instruction
  } else if (llvm::isa<llvm::InsertValueInst>(inst)) {
    writeInsertValueInstruction(llvm::cast<llvm::InsertValueInst>(inst),
                                locals);

    //### assigning instruction
  } else if (desc) {
    std::string instStr = getInstructionAsExpression(inst, locals);
    if (!instStr.empty())
      writeAssignRaw(desc->name, instStr);

    //### void/discarded result instruction
  } else {
    std::string instStr = getInstructionAsExpression(inst, locals);
    if (instStr.size() > 0) {
      putLine(instStr + ';');
    }
  }
}

void CWriter::writeIf(const llvm::Value *condition, bool negate,
                      IdentifierScope &locals) {
  const VariableDesc *condVar = locals.lookUp(condition);

  std::string condStr;

  if (condVar) {
    condStr = condVar->name;

  } else if (llvm::isa<llvm::ConstantInt>(condition)) {
    const llvm::ConstantInt *constInt =
        llvm::cast<llvm::ConstantInt>(condition);
    condStr = constInt->isZero() ? "false" : "true";

  } else {
    assert(
        false &&
        "controlling expression must be a literal or have a bound identifier");
  }

  if (!negate)
    putLine("if (" + condStr + ")");
  else
    putLine("if (! " + condStr + ")");
}

/*
 * write a while for a post<checked loop
 */
void CWriter::writePostcheckedWhile(llvm::BranchInst *branchInst,
                                    IdentifierScope &locals, bool negate) {
  llvm::Value *loopCond = branchInst->getCondition();

  const VariableDesc *desc = locals.lookUp(loopCond);

  assert(desc && "was not mapped");

  std::string expr = desc->name;

  if (negate)
    putLine("while (! " + expr + ");");
  else
    putLine("while (" + expr + ");");
}

/*
 * write a while for a postchecked loop, if oExpressionInsts != NULL dont write,
 * but put all consumed instructions in the set
 */
void CWriter::writePrecheckedWhile(llvm::BranchInst *branchInst,
                                   IdentifierScope &locals, bool negate,
                                   InstructionSet *oExpressionInsts) {
  llvm::Value *loopCond = branchInst->getCondition();

  std::string expr = getComplexExpression(branchInst->getParent(), loopCond,
                                          locals, oExpressionInsts);

  if (!oExpressionInsts) {
    if (negate)
      putLine("while (! " + expr + ")");
    else
      putLine("while (" + expr + ")");
  }
}

void CWriter::writeInfiniteLoopBegin() { putLine("while(true)"); }

void CWriter::writeInfiniteLoopEnd() { putLine(""); }

void CWriter::writeReturnInst(llvm::ReturnInst *retInst,
                              IdentifierScope &locals) {
  if (retInst->getNumOperands() > 0) // return value
  {
    const VariableDesc *desc = locals.lookUp(retInst->getOperand(0));

    if (desc) {
      putLine("return " + desc->name + ";");
    } else {
      llvm::Value *retVal = retInst->getReturnValue();

      if (!llvm::isa<llvm::UndefValue>(retVal)) {
        putLine("return " + getValueToken(retInst->getReturnValue(), locals) +
                ";");
      } else {
        Log::warn(retInst,
                  "skipping return for the returned value is undefined!");
      }
    }

  } else { /// void return
    putLine("return;");
  }
}

void CWriter::writeFunctionPrologue(llvm::Function *func,
                                    IdentifierScope &locals) {
  ConstValueSet arguments;
  for (auto &arg : func->args()) {
    arguments.insert(&arg);
  }

  for (ConstVariableMap::iterator itVar = locals.identifiers.begin();
       itVar != locals.identifiers.end(); ++itVar) {
    VariableDesc &desc = itVar->second;

#ifdef DEBUG
    itVar->first->print(llvm::outs());
    std::cout << " - > " << itVar->second.name << std::endl;
#endif

    if (arguments.find(itVar->first) == arguments.end()) {
      if (llvm::isa<llvm::PHINode>(itVar->first)) {
        VariableDesc inputDesc;
        inputDesc.name = desc.name + "_in";
        inputDesc.type = desc.type;
        writeVariableDeclaration(inputDesc);
      }
      writeVariableDeclaration(desc);
    }
  }

  // dump local storage declarations to kernel functions
#if 0
   if (modInfo.isKernelFunction(func))
   {
		for (llvm::Module::global_iterator global = func->getParent()->global_begin(),
         end = func->getParent()->global_end();
         global != end; ++global) {
			std::string varName = global->getName().str();
			const llvm::PointerType * gvType = global->getType();
			const llvm::Type * contentType = gvType->getElementType();

			//spill dynamic local globals in kernel space
			if (gvType->getAddressSpace() == SPACE_LOCAL && !global->isConstant()) {
				std::string spaceName = getAddressSpaceName(SPACE_LOCAL);

        // Check kernel name agains current function.
        if (! usedInFunction(func, &*global))
          continue;

				std::string declareStr = buildDeclaration(getLocalVariableName(varName),
                                                  contentType) + ";";
				putLine(spaceName + " " + declareStr );
			}
		}
   }
#endif
  putLineBreak();
}

static const char * ModuleHeader ="\
typedef vr256si int256;\n\
typedef vr256sl long256;\n\
typedef vr256sf float256;\n\
typedef vr256df double256;\n\
typedef vm256 bool256;\n";

CWriter::CWriter(ModuleInfo &_modInfo, PlatformInfo &_platform)
    : modInfo(reinterpret_cast<CModuleInfo &>(_modInfo)), platform(_platform) {
  putLine("#include <cmath>");

  put(ModuleHeader);

  putLine("extern \"C\" {");

  llvm::Module *mod = modInfo.getModule();

  spillStructTypeDeclarations(modInfo, this);
  putLine("");

  //## spill globals
  for (llvm::Module::global_iterator global = mod->global_begin();
       global != mod->global_end(); ++global) {
    if (llvm::isa<llvm::GlobalVariable>(global)) {
      llvm::GlobalVariable *var = llvm::cast<llvm::GlobalVariable>(global);
      const llvm::PointerType *varType = var->getType();
      std::string varName = var->getName().str();
#if 0
				const llvm::Type * contentType = varType->getElementType();
				uint space = varType->getAddressSpace();

				if (var->isConstant()) { // __constant
					std::string initStr = getLiteral(var->getInitializer());
					std::string declareStr = buildDeclaration(varName, contentType) + " = " + initStr + ";";
					putLine( "__constant " + declareStr);

				} else if (space == SPACE_GLOBAL) { //global variable with external initialization (keep the pointer)
#endif
      std::string declareStr = buildDeclaration(varName, varType) + ";";

      // std::string spaceName = getAddressSpaceName(space);
      // putLine(spaceName + " " + declareStr );
      putLine(declareStr);
#if 0
				} else if (space == SPACE_LOCAL) { //work group global variable with initialization (declare for content type)
				/*	std::string spaceName = getAddressSpaceName(space);
					std::string declareStr = buildDeclaration(varName, contentType) + ";";

					putLine(spaceName + " " + declareStr );*/
					//will declare this variable in any using kernel function

				} else {
					Log::warn(var, "discarding global variable declaration");
				}
#endif
    }
  }

  putLine("");

  //## spill function declarations
  for (Function &func : *mod) {
    if (!platform.implements(&func))
      putLine(getFunctionHeader(&func) + ";");
  }

  putLine("}"); /* extern C */

#ifdef DEBUG
  std::cerr << "completed CWriter ctor\n";
#endif
}

CWriter::CWriter(CWriter &writer)
    : modInfo(writer.modInfo), platform(writer.platform) {}

template class ResourceGuard<AddressIterator>;

/*
 * BlockWriter (indents all writes and embraces them in curly brackets)
 */
void CBlockWriter::put(const std::string &text) {
  parent.put(INDENTATION_STRING + text);
}

CBlockWriter::CBlockWriter(CWriter &_parent)
    : CWriter(_parent), parent(_parent) {
  parent.putLine("{");
}

CBlockWriter::~CBlockWriter() { parent.putLine("}"); }

/*
 * PassThrough writer
 */
CPassThroughWriter::CPassThroughWriter(CWriter &_parent)
    : CWriter(_parent), parent(_parent) {}

void CPassThroughWriter::put(const std::string &msg) { parent.put(msg); }

/*
 * Multi writer
 */
CMultiWriter::CMultiWriter(CWriter &_first, CWriter &_second)
    : CWriter(_first), first(_first), second(_second) {}

void CMultiWriter::put(const std::string &msg) {
  first.put(msg);
  second.put(msg);
}

} // namespace axtor
