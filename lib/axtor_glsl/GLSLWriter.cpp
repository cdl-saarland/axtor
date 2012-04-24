/*
 * OCLWriter.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor_glsl/GLSLWriter.h>
#include <axtor/util/Assert.h>

namespace axtor {


 void GLSLWriter::put(std::string text)
{
	assert(false && "must not write with the root GLSL writer (use derived classes instead)");
}

inline void GLSLWriter::putLine(std::string text)
{
	put( text + '\n');
}

inline void GLSLWriter::putLineBreak()
{
	putLine("");
}

inline void GLSLWriter::put(char c)
{
	put( "" + c );
}

inline void GLSLWriter::putLine(char c)
{
	putLine( "" + c );
}

void GLSLWriter::dump()
{}

void GLSLWriter::print(std::ostream & out)
{}

/*
 *  ##### Type System #####
 */

/*
 * returns type symbols for default scalar types
 */
std::string GLSLWriter::getScalarType(const llvm::Type * type, bool signedInt)
{
#ifdef DEBUG
	std::cerr << "getScalarType("; type->dump(); std::cerr << ")\n";
#endif

	//### native platform type ###
   std::string nativeStr;
   std::string typeName = "";

   // getTypeSymbol(modInfo.getTypeSymbolTable(), type, typeName);

   /* NATIVE TYPES */
   if (platform.lookUp(type, typeName, nativeStr))
   {
	   return nativeStr;
   }

	//### C default types ###
  switch (type->getTypeID())
  {
     case llvm::Type::VoidTyID:  return "void";
     case llvm::Type::FloatTyID: return "float";
     case llvm::Type::DoubleTyID: return "double";
     case llvm::Type::IntegerTyID:
     {
        const llvm::IntegerType * intType = llvm::cast<llvm::IntegerType>(type);
        int width = intType->getBitWidth();

        if (width == 1)
           return "bool";
        else if (width <=8)
           return "char";
        else if (width <= 16)
           return "short";
        else if (width <= 32)
           return "int";
        else if (width <= 64)
           return "int"; //return "long";
        else
           assert(width <= 64 && "over-sized integer type");
     }

	case llvm::Type::OpaqueTyID:
	{
		Log::fail(type, "GLSL does not implement this opaque type");
	};

	case llvm::Type::PointerTyID:
	{
		Log::fail(type, "GLSL does not support pointers");
	}

	case llvm::Type::VectorTyID:
	{
		const llvm::VectorType * vectorType = llvm::cast<llvm::VectorType>(type);
		std::string baseStr;
		switch(vectorType->getElementType()->getTypeID())
		{
		case llvm::Type::FloatTyID:
			baseStr = "vec";
			break;

		case llvm::Type::IntegerTyID:
			if (signedInt) {
				baseStr = "ivec";
			} else {
				baseStr = "uvec";
			}
			break;

		default:
			assert(false && "unsupported element type");
		};

		int size = vectorType->getNumElements();

		if (size > 4) {
			Log::fail(type, "GLSL does not support vector types with more than 4 elements");
		}

		return baseStr + str<int>(size);
	}
	default: break;
  };
	Log::fail(type, "not a scalar type");
	return "";
}

/*
 * generates a type name for @type
 * if this is a pointer type, operate on its element type instead
 */
std::string GLSLWriter::getType(const llvm::Type * type)
{
#ifdef DEBUG
	std::cerr << "getType("; type->dump(); std::cerr << ")\n";
#endif
	if (llvm::isa<llvm::ArrayType>(type)) {
		const llvm::ArrayType * arrType = llvm::cast<llvm::ArrayType>(type);
		return getType(arrType->getElementType()) + "[" + str<int>(arrType->getNumElements()) + "]";

	/*} else if (llvm::isa<llvm::VectorType>(type)) {
		const llvm::VectorType * vectorType = llvm::cast<llvm::VectorType>(type);
		return getScalarType(vectorType->getElementType()) + str<int>(vectorType->getNumElements()); */

	} else if (llvm::isa<llvm::PointerType>(type)) {
		const llvm::PointerType * ptrType = llvm::cast<llvm::PointerType>(type);
		const llvm::Type * elementType = ptrType->getElementType();
		//uint space = ptrType->getAddressSpace();

		return getType(elementType) + "*";

	} else if (llvm::isa<llvm::StructType>(type)) {
		std::string name;

		// FIXME
#if 0
		if (getTypeSymbol(modInfo.getTypeSymbolTable(), type, name))
		{
			return name;
		} else {
#endif
			Log::fail(type, "anonymous structs not implemented");
	// }
	}


	return getScalarType(type);
}

std::string GLSLWriter::getArgumentType(const llvm::Type * type, ArgumentQualifier access)
{
	if (llvm::isa<llvm::PointerType>(type))	{
		const llvm::PointerType * ptrType = llvm::cast<llvm::PointerType>(type);
		std::string typeStr = getType(ptrType->getContainedType(0));

		if (ptrType->getAddressSpace() == SPACE_NOPTR) {
			return "in " + typeStr;
		} else if (ptrType->getAddressSpace() == SPACE_POINTER) {
			return "" + typeStr + " *";
		}

		switch (access)
		{
		case IN:
			return "in " + typeStr;
		case OUT:
			return "out " + typeStr;
		case INOUT:
			return "inout " + typeStr;
		}
	}


	return getType(type); //by value
}

/*
 * build a C-style declaration for @root of type @type
 */
std::string GLSLWriter::buildDeclaration(std::string root, const llvm::Type * type)
{
	if (llvm::isa<llvm::ArrayType>(type)) {
		const llvm::ArrayType * arrType = llvm::cast<llvm::ArrayType>(type);
		return buildDeclaration(root + "[" + str<int>(arrType->getNumElements()) + "]", arrType->getElementType());

	/* } else if (llvm::isa<llvm::VectorType>(type)) {
		const llvm::VectorType * vectorType = llvm::cast<llvm::VectorType>(type);
		return getScalarType(vectvectorType->getElementType()) + str<int>(vectorType->getNumElements()) + " " + root; */

	} else if (llvm::isa<llvm::PointerType>(type)) {
		const llvm::PointerType * ptrType = llvm::cast<llvm::PointerType>(type);

		if (ptrType->getAddressSpace() == SPACE_NOPTR) {
			return buildDeclaration(root, ptrType->getElementType());
		} else if (ptrType->getAddressSpace() == SPACE_POINTER) {
			return buildDeclaration("* " + root + "", ptrType->getElementType());
		} else {
			Log::fail(type, "GLSL does not natively support pointer types");
		}

	} else if (llvm::isa<llvm::StructType>(type)) {
		std::string name;

		// FIXME
#if 0
		if (getTypeSymbol(modInfo.getTypeSymbolTable(), type, name))
		{
			return name + " " + root;
		} else {
		}
#endif
			Log::fail(type, "anonymous structs not implemented");
//		}

	}


	std::string scalarStr = getScalarType(type);
	return scalarStr + " " + root;
}

/*
 * ##### DECLARATIONS / OPERATORS & INSTRUCTIONS ######
 */

	/*
* writes a generic function header and declares the arguments as mapped by @locals
*/
std::string GLSLWriter::getFunctionHeader(llvm::Function * func, IdentifierScope * locals)
{
	std::string funcStr;
	const llvm::FunctionType * type = getFunctionType(func);
	const llvm::Type * returnType = type->getReturnType();

	//return type
	std::string typeStr = getType(returnType);
	funcStr += typeStr;

	//function name
	funcStr += ' ' + func->getName().str();

	//arguments
	ArgList & argList = func->getArgumentList();
	ArgList::iterator arg;


	//catch empty lists
	if (argList.empty())
	{
		return funcStr + "()";
	}

	//create generic argument list
	uint i;
	for(i = 0, arg = argList.begin();
		i < type->getNumParams() && arg != argList.end();
		++i, ++arg)
	{
		const llvm::Type * argType = type->getParamType(i);

		//std::string modifierStr = inferModifiers(arg);
		std::string typeStr = getArgumentType(argType);

		std::string argName = typeStr;

		if (locals) {
			const VariableDesc * desc = locals->lookUp(arg);
			argName += ' ' + desc->name;
		}

		 if (arg == argList.begin()) {
			 funcStr += '(';
		 } else {
			 funcStr += ", ";
		 }

		 funcStr += argName;
	}

	funcStr += ')';

	return funcStr;
}

/*
 * spills all function arguments of the specified function as globals with corresponding in/out modifier (depending on @isReceiving)
 * if @isVertToFragStage is set GLSLWriter::modInfo will be queried to get interpolation information (smooth,flat,etc)
 */
void GLSLWriter::writeShaderArgumentDeclarations(const IdentifierScope & globals, llvm::Function * func, bool isVertToFragStage, bool isReceiving)
{
	llvm::Function::ArgumentListType & argList = func->getArgumentList();
	typedef llvm::Function::ArgumentListType ArgList;
	typedef std::vector<int> IntVector;

	//spills global declarations for all function arguments
	uint idx = 0;
	for(ArgList::iterator itArg = argList.begin(); itArg != argList.end(); ++itArg, ++idx)
	{
		llvm::Argument * arg = itArg;
		const llvm::Type * argType = arg->getType();
		const VariableDesc * desc = globals.lookUp(arg);
		assert(desc && "shader function argument was not declared in global scope");

		std::string declareStr = buildDeclaration(desc->name, argType) + ";";

		if (isVertToFragStage) { //used for interpolation registers between vertex and fragment shader
			if (isReceiving) {
				putLine ("in " + declareStr);
			} else {
				std::string qualifier = modInfo.getAttributeInterpolantStr(idx);
				if (qualifier == "")
					putLine("out " + declareStr);
				else
					putLine(qualifier + " out " + declareStr);
			}

		} else { //not interpolanted shader argument to the vertex shader
			if (isReceiving) {
				putLine("in " + declareStr);
			} else {
				putLine("out " + declareStr);
			}
		}
	}
}


void GLSLWriter::writeShaderFunctionHeader(llvm::Function * func, IdentifierScope * locals)
{
	//write the function signature (shader type disambiguation is done with nested
	putLine("void main() ");
}

std::string GLSLWriter::getFunctionHeader(llvm::Function * func)
{
	return getFunctionHeader(func, NULL);
}

 void GLSLWriter::writeLineBreak()
{
	putLineBreak();
}

 void GLSLWriter::writeVariableDeclaration(const VariableDesc & desc)
{
	const llvm::Type * type = desc.type;
	if (desc.isAlloca) {
		type = desc.type->getContainedType(0);
	}

	putLine( buildDeclaration(desc.name, type) + ";");
}

 void GLSLWriter::writeFunctionDeclaration(llvm::Function * func, IdentifierScope * locals)
{
	putLine( getFunctionHeader(func, locals) + ';');
}

/*
 * default C Style operators
 */
std::string GLSLWriter::getBinaryOperator(llvm::Instruction * inst, std::string leftOp, std::string rightOp)
{
  bool isBool = inst->getType()->getPrimitiveSizeInBits() == 1;

  std::string token = "";
  bool intOps = //has integer operands
  	  inst->getOperand(0)->getType()->isIntegerTy(32);
  bool castUint = false; //cast operands to uint
  bool intResult = false; //cast result to int


  switch (inst->getOpcode())
  {
     //Arithmetic
	  case llvm::Instruction::Add: intResult = true;
     case llvm::Instruction::FAdd:
    	 token = "+"; break;

     case llvm::Instruction::Mul: intResult = true;
     case llvm::Instruction::FMul:
    	 token = "*"; break;

     case llvm::Instruction::Sub: intResult = true;
     case llvm::Instruction::FSub:
    	 token = "-"; break;

     case llvm::Instruction::UDiv: castUint = true;
     case llvm::Instruction::SDiv:
    	 intResult = true;
     case llvm::Instruction::FDiv:
        token = "/";
        break;

     case llvm::Instruction::URem: castUint = true;
     case llvm::Instruction::SRem:
    	 intResult = true;
     case llvm::Instruction::FRem:
    	 token = "%";
    	 break;

 //binary integer ops
     case llvm::Instruction::Shl:
    	 intResult = true;
    	 token = "<<";break;
     case llvm::Instruction::LShr:
    	 intResult = true;
    	 token= ">>"; break;

     case llvm::Instruction::AShr:
    	 intResult = true;
    	 token =">>>"; break;

     case llvm::Instruction::And:
    	 intResult = true;
    	 token = isBool ? "&&" : "&";break;

     case llvm::Instruction::Or:
    	 intResult = true;
    	 token = isBool ? "||" : "|";break;

     case llvm::Instruction::Xor:
    	 intResult = true;
    	 token = isBool ? "!=" : "^";break;

     //predicated CmpInsts
     case llvm::Instruction::FCmp:
     {
        llvm::CmpInst * cmpInst = llvm::cast<llvm::CmpInst>(inst);
        switch (cmpInst->getPredicate())
        {
           case llvm::CmpInst::FCMP_FALSE:
             assert(false && "why did nobody optimize that out?");

           case llvm::CmpInst::FCMP_OGT:
           case llvm::CmpInst::FCMP_UGT:
              token = ">";
              break;

           case llvm::CmpInst::FCMP_OGE:
           case llvm::CmpInst::FCMP_UGE:
              token = ">=";
              break;

           case llvm::CmpInst::FCMP_OLT:
           case llvm::CmpInst::FCMP_ULT:
              token = "<";
              break;

           case llvm::CmpInst::FCMP_OLE:
           case llvm::CmpInst::FCMP_ULE:
              token= "<=";
              break;

           case llvm::CmpInst::FCMP_OEQ:
              token = "=="; //overloaded operator in C
              break;

           default:
              assert(false && "unmapped cmp predicate");
        };
     }
     break;

     case llvm::Instruction::ICmp:
     {
        llvm::CmpInst * cmpInst = llvm::cast<llvm::CmpInst>(inst);
        switch (cmpInst->getPredicate())
        {
           case llvm::CmpInst::ICMP_UGT: castUint = true;
           case llvm::CmpInst::ICMP_SGT:
              token = ">";
              break;

           case llvm::CmpInst::ICMP_UGE: castUint = true;
           case llvm::CmpInst::ICMP_SGE:
              token = ">=";
              break;

           case llvm::CmpInst::ICMP_ULT: castUint = true;
           case llvm::CmpInst::ICMP_SLT:
              token= "<";
              break;

           case llvm::CmpInst::ICMP_ULE: castUint = true;
           case llvm::CmpInst::ICMP_SLE:
              token= "<=";
              break;

           case llvm::CmpInst::ICMP_EQ:
              token = "=="; //overloaded operator in C
              break;

           case llvm::CmpInst::ICMP_NE:
              token = "!=";
              break;

           default:
              assert(false && "unmapped cmp predicate");
        };
     }
	 break;

     default:
    	 Log::fail(inst, str<uint>(inst->getOpcode()) + " unsupported operator type");
	}
	intResult = intResult && !isBool;

	std::string leftStr = leftOp;
	std::string rightStr = rightOp;

	//add appropriate casts for integer operands
	if (castUint) {
		leftStr = "uint(" + leftOp + ")";
		rightStr = "uint(" + rightOp + ")";
	} else if (intOps) {
		leftStr = "int(" + leftOp + ")";
		rightStr = "int(" + rightOp + ")";
	}

	if (intResult)
		return "int(" + leftStr + " " + token + " " + rightStr + ")";
	else
		return "(" + leftStr + " " + token + " " + rightStr + ")";
}

/*
* returns the string representation of a operator using @operands as operand literals
*/
std::string GLSLWriter::getInstruction(llvm::Instruction * inst, std::vector<std::string> operands)
{
  std::string tmp;
  StringVector::const_iterator begin = operands.begin();


  //# function call
  if (llvm::isa<llvm::CallInst>(inst)) {
	  llvm::CallInst * caller = llvm::cast<llvm::CallInst>(inst);
	  llvm::Function * callee = caller->getCalledFunction();
	  if (platform.implements(callee))
	  {
		  return platform.build(callee->getNameStr(), operands.begin(), operands.end());// - 1);

	  } else {
		  tmp += callee->getNameStr() + '(';
		 for(StringVector::const_iterator itOp = begin; itOp != operands.end(); ++itOp)
		 {
			if (itOp != begin)
			   tmp +=", " + *itOp;
			else
			   tmp += *itOp;
		 }
		 tmp += ')';
		 return tmp;

	  }

  //# binary infix operator
  } else if (inst->isBinaryOp() || llvm::isa<llvm::CmpInst>(inst)) {
	  return getBinaryOperator(inst, operands[0], operands[1]);

  //# bitcast
  } else if (llvm::isa<llvm::BitCastInst>(inst)) {
     assert(operands.size() == 1 && "cast a single value . . non?");

     //fake assignment instruction (cast to same type)
     if (inst->getOperand(0)->getType() == inst->getType()) {
    	 return operands[0];

     } else {
		 Log::fail(inst, "GLSL does not support bitcasts");
     }

  //# value cast
  } else if (llvm::isa<llvm::CastInst>(inst)) {
	  const llvm::Type * sourceType = inst->getOperand(0)->getType();
	  const llvm::Type * targetType = inst->getType();

	  if (llvm::isa<llvm::UIToFPInst>(inst)) { //all integers are declared as signed
		  std::string intCast = "(unsigned " + getType(sourceType) + ")";
		  return getType(targetType) + "(" + intCast + "(" + operands[0] + "))";
	  } else {
		  return getType(targetType) + "(" + operands[0] + ")";
	  }

  //# select
  } else if (llvm::isa<llvm::SelectInst>(inst)) {
	  return "(" + operands[0] + " ? " + operands[1] + " : " + operands[2] + ")";

  //# generic opcode based scheme
  }

  Log::fail(inst, "unimplemented instruction type");
  return "<unreachable in GLSLWriter::getInstruction>";
}


/*
* return a dereferencing string for the next type node of the object using address
*/
std::string GLSLWriter::dereferenceContainer(std::string root, const llvm::Type * type, AddressIterator *& address, IdentifierScope & locals, const llvm::Type *& oElementType)
{
	if (llvm::isa<llvm::StructType>(type)) {
		uint64_t index;
		llvm::Value * indexVal = address->getValue();
		if (!evaluateInt(indexVal, index)) {
			Log::fail(indexVal, "can not dynamically access struct members");
		}

		address = address->getNext();
		oElementType = type->getContainedType(index);


		return root + ".x" + str<int>(index);

	} else if (llvm::isa<llvm::ArrayType>(type)) {
		llvm::Value * indexVal = address->getValue();
		oElementType = type->getContainedType(0);
		const VariableDesc * desc = locals.lookUp(indexVal);
		assert(desc && "undefined index value");

		address = address->getNext();


		return root + "[" + desc->name + "]";


	} else if (llvm::isa<llvm::PointerType>(type)) {
		const llvm::PointerType * ptrType = llvm::cast<llvm::PointerType>(type);
		llvm::Value * indexVal = address->getValue();
		oElementType = type->getContainedType(0);

		address = address->getNext();

		uint64_t index;

		//NV_shader_buffer_* - extensions support
		if (ptrType->getAddressSpace() == SPACE_POINTER) {
			if (evaluateInt(indexVal, index)) {
				if (index == 0) {
					return "(*" + root + ")";
				} else {
					if (index > 0 && address) {
						address = address->getNext();
					}
					return "(" + root + ")[" + str<int>(index) + "]";
				}
			} else {
				const VariableDesc * desc = locals.lookUp(indexVal);
				assert(desc && "undefined index value");

				return "(" + root + ")[" + desc->name + "]";
			}

		} else if (evaluateInt(indexVal, index) && index == 0) {
			return root;
		} else {
			Log::fail(type, "GLSL does not support dynamic arrays");
		}
	}

	Log::fail(type, "can not dereference this type");
}

/*
 * return a name representing a dereferenced pointer
 *if noImplicitDeref is false, the exact address of the value is returned
 */
std::string GLSLWriter::getDereffedPointer(llvm::Value * val, IdentifierScope & locals)
{
	AddressIterator::AddressResult result = AddressIterator::createAddress(val, platform.getDerefFuncs());
	ResourceGuard<AddressIterator> __guardAddress(result.iterator);

	AddressIterator * address = result.iterator;
	llvm::Value * rootValue = result.rootValue;

	const llvm::Type * rootType = rootValue->getType();

#ifdef DEBUG
	val->dump();
	rootValue->dump();
	rootType->dump();
	if (address) address->dump();
#endif

	//allocas are syntactically dereferenced by their name string
	if (llvm::isa<llvm::AllocaInst>(rootValue)) {
		address = address->getNext();
		rootType = rootType->getContainedType(0);
	}

	//dereff the initial pointer (if it points to a more complex structure)

	const VariableDesc * desc = locals.lookUp(rootValue);
	assert(desc && "root value was not mapped");

	std::string tmp = desc->name;

	while (address)
	{
#ifdef DEBUG
		std::cerr << "deref : " << tmp << "\n";
		assert(rootType && "was not set");
		rootType->dump();
#endif
		const llvm::Type * elementType = NULL;
		tmp = dereferenceContainer(tmp, rootType, address, locals, elementType);
		//assert(elementType && "derefContainer did not set element type");
		rootType = elementType;
#ifdef DEBUG
		std::cerr << "dereferenced to " << tmp << "\n";
#endif

	}

	return tmp;
}

	std::string GLSLWriter::getAllNullLiteral(const llvm::Type * type)
	{
		switch(type->getTypeID())
		{
			case llvm::Type::VectorTyID:
			{
				const llvm::VectorType * arrType = llvm::cast<llvm::VectorType>(type);
				//uint size = arrType->getNumElements();
				std::string elementStr = getAllNullLiteral(arrType->getElementType());
				return getType(type) + "(" + elementStr + ")";
			}



			//case llvm::Type::StructTyID:
			//case llvm::Type::ArrayTyID:

/*					const llvm::ArrayType * arrType = llvm::cast<llvm::ArrayType>(type);
				uint size = arrType->getNumElements();
				std::string elementStr = getAllNullLiteral(arrType->getElementType());

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

			case llvm::Type::IntegerTyID:
				return "0";

			default:
				Log::fail(type, "null literals of this kind are currently unsupported");
		}
	}

	/*
    * return the string representation of a constant
    */
   std::string GLSLWriter::getLiteral(llvm::Constant * val)
   {
      //## Constant integer (and Bool)
      if (llvm::isa<llvm::ConstantInt>(val))
      {
    	  const llvm::IntegerType * intType = llvm::cast<llvm::IntegerType>(val->getType());
    	  if (intType->getBitWidth() == 1) {
    		  if (val->isNullValue()) {
    			  return "false";
    		  } else {
    			  return "true";
    		  }
    	  } else if (intType->getBitWidth() <= 64) {
    		  llvm::ConstantInt * constInt = llvm::cast<llvm::ConstantInt>(val);
    		  uint64_t data = constInt->getLimitedValue();
    		  std::string hexStr = convertToHex(data, intType->getBitWidth() / 4);
    		  return "0x" + hexStr;
    	  } else {
			  Log::fail(val, "value exceeds size limit, expected bit size <= 64, was " + str<int>(intType->getBitWidth()));
    	  }

      //## Constant Float
      } else if (llvm::isa<llvm::ConstantFP>(val)) {
         llvm::ConstantFP * constFP = llvm::cast<llvm::ConstantFP>(val);
         llvm::APFloat apFloat = constFP->getValueAPF();
         float num = apFloat.convertToFloat();
         return str<float>(num);

      //## Function
      } else if (llvm::isa<llvm::Function>(val)) {
         return val->getName();

      //## Constant Array
      } else if (llvm::isa<llvm::ConstantArray>(val)) {
         std::string buffer = "{";
         llvm::ConstantArray * arr = llvm::cast<llvm::ConstantArray>(val);
         const llvm::ArrayType * arrType = arr->getType();
         for(uint i = 0; i < arrType->getNumElements(); ++i) {
            llvm::Constant * elem = arr->getOperand(i);
            if (i > 0)
               buffer += "," + getLiteral(elem);
            else
               buffer += getLiteral(elem);
         }
         buffer += "}";
         return buffer;

      } else if (llvm::isa<llvm::ConstantVector>(val)) {
    	  llvm::ConstantVector * vector = llvm::cast<llvm::ConstantVector>(val);
    	  const llvm::VectorType * vectorType = vector->getType();

    	  std::string buffer = "";
    	  for(uint i = 0; i < vectorType->getNumElements(); ++i)
    	  {
    		  llvm::Value * opVal = vector->getOperand(i);
    		  std::string opStr = getLiteral(llvm::cast<llvm::Constant>(opVal));

    		  if (i > 0) {
    			  buffer += ", ";
    		  }
    		  buffer += opStr;
    	  }

    	  return getType(vectorType) + "(" + buffer + ")";

      //default undefined values to zero initializers
      } else if (val->isNullValue() || llvm::isa<llvm::UndefValue>(val)) {
    	  const llvm::Type * type = val->getType();
    	  return getAllNullLiteral(type);
      }

      //## unsupported literal
      Log::fail(val, "unsupported literal");
      assert(false);
   }

   /*
    * tries to create a literal string it @val does not have a variable
    */
   std::string GLSLWriter::getValueToken(llvm::Value * val, IdentifierScope & locals)
   {
	   const VariableDesc * desc = locals.lookUp(val);
	   if (desc) {
		   return desc->name;
	   } else {
		   assert(llvm::isa<llvm::Constant>(val) && "undeclared value is not a constant");
		   return getLiteral(llvm::cast<llvm::Constant>(val));
	   }
   }

   /*
   * returns the string representation of a non-instruction value
   */
   std::string GLSLWriter::getNonInstruction(llvm::Value * value, IdentifierScope & locals)
   {

	   if (llvm::isa<llvm::GlobalValue>(value)) { // global variable
		   const VariableDesc * desc = locals.getParent()->lookUp(value);
		   return desc->name;

	   } else if (llvm::isa<llvm::PHINode>(value)) { // PHI-Node
		   const VariableDesc * phiDesc = locals.lookUp(value);
		   assert(phiDesc && "unmapped PHI-Node");
		   return phiDesc->name;

	   } else if (llvm::isa<llvm::ConstantExpr>(value)) { //ConstantExpr -> probably deref
		   return getDereffedPointer(value, locals);

      } else if (llvm::isa<llvm::Constant>(value)) { //arbitrary constant
         return getLiteral(llvm::cast<llvm::Constant>(value));

	  }

	 Log::fail(value, "failure : could not translate nonInstruction");
	 return "";
   }

  /*
   * returns the string representation of a ShuffleInstruction
   */
std::string GLSLWriter::getShuffleInstruction(llvm::ShuffleVectorInst * shuffle, IdentifierScope & locals)
{
	llvm::Value * firstVector = shuffle->getOperand(0);
	const llvm::VectorType * firstType = llvm::cast<const llvm::VectorType>(firstVector->getType());
	llvm::Value * secondVector = shuffle->getOperand(1);
	//const llvm::VectorType * secondType = llvm::cast<const llvm::VectorType>(secondVector->getType());
	llvm::Value * indexVector = shuffle->getOperand(2);
	const llvm::VectorType * indexType =  llvm::cast<const llvm::VectorType>(indexVector->getType());

	const llvm::Type * elementType = firstType->getElementType();

	int secondBase = firstType->getNumElements();
	int numIndices = indexType->getNumElements();

	std::string firstStr = getValueToken(firstVector, locals);

	const VariableDesc * secondDesc = locals.lookUp(secondVector);
	bool hasSecond = ! llvm::isa<llvm::UndefValue>(secondVector);

	std::string secondStr;
	if (hasSecond) {
		secondStr = secondDesc ? secondDesc->name : getLiteral(llvm::cast<llvm::Constant>(secondVector));
	}

#ifdef DEBUG
	std::cerr << "SHUFFLE:\n"
			<< "first=" << firstStr << "\n"
			<< "second=" << secondStr << "\n"
			<< "\n";
#endif

	//get the target types name
	std::string typeStr = getType(shuffle->getType());

	//build a string extracting values from one of the two vectors
	std::string accu = "";
	std::string elements = firstStr + ".s";

	int firstMask = shuffle->getMaskValue(0);
	bool wasLiteral = false;
	bool useFirst = firstMask < secondBase;
	bool isMultiDraw = false; //shuffle is composed of multiple draw sequences

	if (firstMask >= 0) {
		if (useFirst) {
			elements = firstStr + ".";
		} else {
			elements = secondStr + ".";
		}
		wasLiteral = false;
	} else {
		elements = getLiteral(llvm::Constant::getNullValue(elementType));
		wasLiteral = true;
	}

	for(int i = 0; i < numIndices; ++i)
	{
		int mask = shuffle->getMaskValue(i);

		//set up the element source (last was literal, current is literal unlike the last OR change of source vector)
		if (wasLiteral || (mask < 0) || (useFirst != (mask < secondBase))) {
			accu += elements;
			wasLiteral = false;

			if (mask >= 0) {
				useFirst = mask < secondBase;
				if (useFirst) {
					elements = ", " + firstStr + ".";
				} else {
					assert(hasSecond && "trying to access elements from undef vector");
					elements = ", " + secondStr + ".";
				}
				isMultiDraw = true;
			}
		}

		//pick elements
		if (mask < 0) {
			wasLiteral = true;
			elements = ", " + getLiteral(llvm::Constant::getNullValue(elementType));
		} else {
			wasLiteral = false;
			if (useFirst) {
				elements += getComponentDesignator(mask);
			} else {
				elements += getComponentDesignator(mask - secondBase);
			}
		}
	}

	//add last element
	accu += elements;

	if (isMultiDraw) {
		return typeStr + "(" + accu + ")";
	} else {
		return accu;
	}
}

std::string GLSLWriter::getComponentDesignator(int index)
{
	assert(index >= 0 && "can not access at neg indices");
	switch (index)
	{
	case 0:
		return "x";
	case 1:
		return "y";
	case 2:
		return "z";
	case 3:
		return "w";

	default:
		Log::fail("can not operate on vector width > 4 in GLSL\n");
	}
}

std::string getVectorElementString(int idx)
{
	switch (idx)
	{
		case 0: return "x";
		case 1: return "y";
		case 2: return "z";
		case 3: return "w";
		default: Log::fail(str<int>(idx) + " index for vector element access is out of range");
	}
}

/*
 * returns the string representation of an ExtractElementInstruction
 */
std::string GLSLWriter::getExtractElementInstruction(llvm::ExtractElementInst * extract, IdentifierScope & locals)
{
	llvm::Value * indexVal= extract->getIndexOperand();
	llvm::Value * vectorVal = extract->getVectorOperand();
	std::string vectorStr = getValueToken(vectorVal, locals);

	uint64_t index;

	if (evaluateInt(indexVal, index)) {
		return vectorStr + "." + getVectorElementString(index);
	}

	Log::fail(extract, "can not randomly extract values from a vector");
	return "";
}

/*
 * returns the string representation of an InsertElementInstruction
 * if the vector value is defined this creates two instructions
 */
void GLSLWriter::writeInsertElementInstruction(llvm::InsertElementInst * insert, IdentifierScope & locals)
{
	llvm::Value * vec = insert->getOperand(0);
	llvm::Value * value = insert->getOperand(1);
	llvm::Value * idxVal = insert->getOperand(2);

	const VariableDesc * desc = locals.lookUp(insert);
	assert(desc && "undeclared instruction");
	std::string descStr = desc->name;

	//const llvm::VectorType * vecType = llvm::cast<llvm::VectorType>(vec->getType());
	//const llvm::Type * elemType = vecType->getElementType();


	const VariableDesc * vecDesc = locals.lookUp(vec);
	std::string vecStr;
	if (vecDesc) {
		vecStr = vecDesc->name;
	} else if (! llvm::isa<llvm::UndefValue>(vec)) {
		assert(llvm::isa<llvm::Constant>(vec) && "non constant was not a declared variable");
		vecStr = getLiteral(llvm::cast<llvm::Constant>(vec));
	}

	uint64_t index;
	if (!evaluateInt(idxVal, index)) {
		Log::fail(idxVal, "was not a constant");
	}

	const VariableDesc * valueDesc = locals.lookUp(value);
	std::string valueStr;
	if (valueDesc) {
		valueStr = valueDesc->name;
	} else {
		assert(llvm::isa<llvm::Constant>(value) && "non constant was not a declared variable");
		valueStr = getLiteral(llvm::cast<llvm::Constant>(value));
	}

	if (! llvm::isa<llvm::UndefValue>(vec)) {
		putLine( descStr + " = " + vecStr + ";" );
	}
	putLine( descStr + "." + getVectorElementString(index) + " = " + valueStr + ";" );

	/*int width = vecType->getNumElements();

	//build the string

	std::string result = "(" + getScalarType(elemType) + str<int>(width) + ")(";

	result += vecStr + ".s";
	for(int i = 0; i < index; ++i) {
		result += hexstr(i);
	}

	result += ", " + valueStr + ", " + vecStr + ".s";

	for(int i = index + 1; i < width; ++i) {
		result += hexstr(i);
	}

	result += ")";

	return result;*/
}

/*
 *
 */
void GLSLWriter::writeFragmentCall(llvm::CallInst * call, IdentifierScope & locals)
{
	typedef llvm::Function::ArgumentListType ArgList;

	llvm::Function * fragFunc = modInfo.getFragFunc();
	ArgList & argList = fragFunc->getArgumentList();

	llvm::Argument * fragArg = argList.begin();

	for(uint i = 0; i < call->getNumArgOperands(); ++i, fragArg = ArgList::getNext(fragArg))
	{
		llvm::Value * operand = call->getArgOperand(i);

		//determine the interpolated value
		llvm::Value * interpValue = operand;

		if (llvm::isa<llvm::CallInst>(operand)) {
			llvm::CallInst * operandCall = llvm::cast<llvm::CallInst>(operand);

			if (platform.isIntrinsic<InterpolantDescriptor>(operandCall->getCalledFunction())) {
				interpValue = operandCall->getOperand(1);
			}
		}

		const VariableDesc * interpDesc = locals.getParent()->lookUp(fragArg); //frag shader parameters are treated as global variables in the scope hierachie

		if (llvm::isa<llvm::UndefValue>(operand)) {
			if (operand->getType()->isPointerTy()) {
				Log::fail(operand, "does not allow undefined or constant pointer values in stage calls");
			} else {
				Log::warn(operand, "passing undefined value to subsequent stage");
			}
		} else {

			const VariableDesc * valueDesc = locals.lookUp(interpValue);

			assert(valueDesc && "unnamed operand");
			assert(interpDesc && "unnamed operand");

			putLine(interpDesc->name + " = " + valueDesc->name + ";");
		}
	}
}

/*
* write a single instruction or atomic value as isolated expression
*/
std::string GLSLWriter::getInstructionAsExpression(llvm::Instruction * inst, IdentifierScope & locals)
{
   //catch loads as they need a dereferenced operand
	if (llvm::isa<llvm::LoadInst>(inst)) {
		llvm::LoadInst * load = llvm::cast<llvm::LoadInst>(inst);
		llvm::Value * pointer = load->getOperand(0);

		return getDereffedPointer(pointer, locals);

	//interpret this PHINode as a variable assignment
	} else if (llvm::isa<llvm::PHINode>(inst)) {
		llvm::PHINode * phi = llvm::cast<llvm::PHINode>(inst);
		const VariableDesc * commonDesc = locals.lookUp(phi->getIncomingValue(0));
		return commonDesc->name;

	} else if (llvm::isa<llvm::ShuffleVectorInst>(inst)) {
		return getShuffleInstruction(llvm::cast<llvm::ShuffleVectorInst>(inst), locals);

	} else if (llvm::isa<llvm::ExtractElementInst>(inst)) {
		return getExtractElementInstruction(llvm::cast<llvm::ExtractElementInst>(inst), locals);

	//check if this is an intrinsic
	} else if (llvm::isa<llvm::CallInst>(inst)) {
		llvm::CallInst * call = llvm::cast<llvm::CallInst>(inst);
		llvm::Function * callee = call->getCalledFunction();

		{
#ifdef DEBUG
			std::cerr << "intrinsic call " << callee->getName().str() << "\n";
#endif
			std::vector<std::string> operands;
		  for(uint opIdx = 0; opIdx < call->getNumArgOperands(); ++opIdx)
		  {
			 llvm::Value * op = call->getArgOperand(opIdx);

			 //#warning "x86_mmx workaround"
			 if (op->getType()->getTypeID() == llvm::Type::X86_MMXTyID)
			 {
				//Log::warn(op, "MMX operand");
				if (llvm::isa<llvm::BitCastInst>(op))
				{
					llvm::BitCastInst * bc = llvm::cast<llvm::BitCastInst>(op);
					op = bc->getOperand(0);
				}
			 }
			 const VariableDesc * opDesc = locals.lookUp(op);

			 if (opDesc) {
				 operands.push_back(opDesc->name);
			 } else {
				operands.push_back(getNonInstruction(op, locals));
			 }
		  }

		  return getInstruction(inst, operands);
		}
	}

	assert(! llvm::isa<llvm::InsertElementInst>(inst) && "insert element instructions is translated into multiple statements (use writeInsertElementInstruction)");

	/*
	 * generic call-style instruction scheme
	 */
  std::vector<std::string> operands;
  for(uint opIdx = 0; opIdx < inst->getNumOperands(); ++opIdx)
  {
     llvm::Value * op = inst->getOperand(opIdx);

     const VariableDesc * opDesc = locals.lookUp(op);

     if (opDesc) {
		 operands.push_back(opDesc->name);
     } else {
        operands.push_back(getNonInstruction(op, locals));
     }
  }

  return getInstruction(inst, operands);
}

/*
* write a complex expression made up of elements from valueBlock, starting from root, writing all included insts to @oExpressionInsts
*/
std::string GLSLWriter::getComplexExpression(llvm::BasicBlock * valueBlock, llvm::Value * root, IdentifierScope & locals, InstructionSet * oExpressionInsts)
{
 // PHI-Nodes cant be part of a single expression
if (llvm::isa<llvm::Instruction>(root) &&
        ! llvm::isa<llvm::PHINode>(root) &&
        ! llvm::isa<llvm::GetElementPtrInst>(root))
  {
     llvm::Instruction * inst = llvm::cast<llvm::Instruction>(root);
     if (inst->getParent() == valueBlock) { //recursively decompose into operator queries

    	 if (oExpressionInsts)
    		 oExpressionInsts->insert(inst);

        std::vector<std::string> operands;
        for(uint opIdx = 0; opIdx < inst->getNumOperands(); ++opIdx)
        {
           llvm::Value * op = inst->getOperand(opIdx);

           operands.push_back(getComplexExpression(valueBlock, op, locals, oExpressionInsts));
        }

        return getInstruction(inst, operands);
     } else {
        const VariableDesc * desc = locals.lookUp(inst);
        return desc->name;
     }
  } else { //non-instruction value
	 return getNonInstruction(root, locals);
  }
}


/*
* writes a generic function header for utility functions and the default signature for the shade func
*/
void GLSLWriter::writeFunctionHeader(llvm::Function * func, IdentifierScope * locals)
{
  if ((modInfo.getFragFunc() == func) || (modInfo.getVertFunc() == func)) {
	  writeShaderFunctionHeader(func, locals);
  } else {
	  putLine( getFunctionHeader(func, locals));
	}
}

void GLSLWriter::writeInstruction(const VariableDesc * desc, llvm::Instruction * inst, IdentifierScope & locals)
{
#ifdef DEBUG
		std::cerr << "writeInstruction:: var=" << (desc ? desc->name : "NULL") << std::endl;
		std::cerr << '\t';
		inst->dump();
#endif
	//dont write instructions consumed their value users
	if (llvm::isa<llvm::GetElementPtrInst>(inst) ||
                llvm::isa<llvm::AllocaInst>(inst))
          return;

//#warning "x86_mmx workaround"
	if (llvm::isa<llvm::BitCastInst>(inst)) {
		if (inst->getType()->getTypeID() == llvm::Type::X86_MMXTyID)
		{
			return;
		}
	}

	if (llvm::isa<llvm::PHINode>(inst))
	{
		const VariableDesc * phiDesc = locals.lookUp(inst);
		writeAssignRaw(phiDesc->name, phiDesc->name + "_in");
		return;
	}

	//fragment shader call
	if (llvm::isa<llvm::CallInst>(inst)) {
		llvm::CallInst * call = llvm::cast<llvm::CallInst>(inst);
		llvm::Function * callee = call->getCalledFunction();
		if(callee->getNameStr() == "shadeFragment") {
			writeFragmentCall(call, locals);
			return;
		}
	}

	if (llvm::isa<llvm::StoreInst>(inst)) {
		llvm::StoreInst * store = llvm::cast<llvm::StoreInst>(inst);
		llvm::Value * pointer = store->getOperand(1);
		const VariableDesc * valueDesc = locals.lookUp(store->getOperand(0));

		std::string srcString;
		if (valueDesc) {
			srcString = valueDesc->name;
		} else {
			srcString = getLiteral(llvm::cast<llvm::Constant>(store->getOperand(0)));
		}

		//decode the GEP and store the value
		std::string name = getDereffedPointer(pointer, locals);
		putLine(name + " = " + srcString + ";" );

	} else if (llvm::isa<llvm::InsertElementInst>(inst)) {
		writeInsertElementInstruction(llvm::cast<llvm::InsertElementInst>(inst), locals);

	} else {
		std::string intrinsicStr = getInstructionAsExpression(inst, locals);
		if (intrinsicStr == "")
			return;

		if (desc) {
			putLine( desc->name + " = " + intrinsicStr + ';');

		} else {
			putLine( intrinsicStr + ';');
		}
	}
}

void GLSLWriter::writeIf(const llvm::Value * condition, bool negate, IdentifierScope & locals)
{
	const VariableDesc * condVar = locals.lookUp(condition);
	std::string condStr = condVar ? condVar->name : "";

	if (! negate)
		putLine( "if (" + condStr + ")" );
	else
		putLine( "if (! " + condStr + ")" );
}

void GLSLWriter::writeElse()
{
	putLine( "else" );
}

void GLSLWriter::writeLoopContinue()
{
	putLine("continue;");
}

void GLSLWriter::writeLoopBreak()
{
	putLine( "break;" );
}

void GLSLWriter::writeDo()
{
  putLine( "do" );
}

void GLSLWriter::writeAssign(const VariableDesc & dest, const VariableDesc & src)
{
#ifdef DEBUG
	std::cerr << "writeAssign:: enforcing assignment to " << dest.name << std::endl;
#endif
	writeAssignRaw( dest.name, src.name);
}

void GLSLWriter::writeAssignRaw(const std::string & dest, const std::string & src)
{
	putLine(dest + " = " + src + ";");
}

/*
 * write a while for a post-checked loop
 */
void GLSLWriter::writePostcheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate)
{
   llvm::Value * loopCond = branchInst->getCondition();

   const VariableDesc * desc = locals.lookUp(loopCond);

   assert(desc && "was not mapped");

    std::string expr = desc->name;

    if (negate)
    	putLine( "while (! " + expr + ");" );
    else
    	putLine( "while (" + expr + ");" );
}

/*
 * write a while for a postchecked loop, if oExpressionInsts != NULL dont write, but put all consumed instructions in the set
 */
void GLSLWriter::writePrecheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate, InstructionSet * oExpressionInsts)
{
  llvm::Value * loopCond = branchInst->getCondition();

  std::string expr = getComplexExpression(branchInst->getParent(), loopCond, locals, oExpressionInsts);

  if (! oExpressionInsts) {
	  if (negate)
		  putLine( "while (! " + expr + ")" );
	  else
		  putLine( "while (" + expr + ")" );
  }
}

void GLSLWriter::writeInfiniteLoopBegin()
{
   putLine ( "while(true)" );
}

void GLSLWriter::writeInfiniteLoopEnd()
{
   putLine( "" );
}

void GLSLWriter::writeReturnInst(llvm::ReturnInst * retInst, IdentifierScope & locals)
{
#ifdef DEBUG
	std::cerr << "GLSLWriter::writeReturnInst(..)\n";
#endif
	if (retInst->getNumOperands() > 0) {
		const VariableDesc * desc = locals.lookUp(retInst->getOperand(0));

		if (desc) {
			putLine( "return " +  desc->name + ";" );
		} else {
			llvm::Value * retVal = retInst->getReturnValue();

			if (! llvm::isa<llvm::UndefValue>(retVal))
			{
				putLine( "return " + getNonInstruction(retInst->getReturnValue(), locals) + ";" );
			} else {
				Log::warn(retInst, "skipping return for the returned value is undefined!");
			}
		}

	} else { ///void return
		putLine( "return;" );
	}
}

/*
* writes a generic struct type declaration to the fragment shader
*/
std::string GLSLWriter::getStructTypeDeclaration(const llvm::Type * type)
{
   const llvm::StructType * structType = llvm::cast<const llvm::StructType>(type);

   std::string structName;
   bool structHasName = getTypeSymbol(modInfo.getTypeSymbolTable(), type, structName);
   AXTOR_ASSERT(structHasName && "struct should have a name");

   std::string res =  "struct " + structName + "\n{\n";

   for(uint i = 0; i < structType->getNumElements(); ++i)
   {
	   const llvm::Type * elementType = structType->getElementType(i);

	   std::string memberName = "x" + str<int>(i);
	   std::string memberStr = buildDeclaration(memberName, elementType);

	   res += INDENTATION_STRING + memberStr + ";\n";
   }

   res += "};\n";
   return res;
}

void GLSLWriter::writeFunctionPrologue(llvm::Function * func, IdentifierScope & locals)
{
   typedef llvm::Function::ArgumentListType ArgList;
   const ArgList & argList = func->getArgumentList();

   ConstValueSet arguments;
   for(ArgList::const_iterator arg = argList.begin(); arg != argList.end(); ++arg)
   {
	   arguments.insert(arg);
   }

   for(ConstVariableMap::const_iterator itVar = locals.begin();
	   itVar != locals.end();
	   ++itVar)
   {
	   const VariableDesc & desc = itVar->second;

#ifdef DEBUG
	   itVar->first->print(llvm::outs()); std::cout << " - > " << itVar->second.name << std::endl;
#endif

	   if (arguments.find(itVar->first) == arguments.end()) {
		   if (llvm::isa<llvm::PHINode>(itVar->first))
		   {
			   VariableDesc inputDesc;
			   inputDesc.name = desc.name + "_in";
			   inputDesc.type = desc.type;
			   writeVariableDeclaration(inputDesc);
		   }

//#warning "x86_mmx workaround"
		   if (desc.type->getTypeID() != llvm::Type::X86_MMXTyID) {
			   writeVariableDeclaration(desc);
		   }
	   }
   }


   // pass through attributes pointers

   //## spill globals
   llvm::Module * mod = modInfo.getModule();
	for (llvm::Module::global_iterator global = mod->global_begin(); global != mod->global_end(); ++global)
	{
		if (llvm::isa<llvm::GlobalVariable>(global))
		{
			llvm::GlobalVariable * var = llvm::cast<llvm::GlobalVariable>(global);
			std::string varName = var->getName().str();
			const llvm::PointerType * varType = var->getType();
			const llvm::Type * contentType = varType->getElementType();
			if (! llvm::isa<llvm::PointerType>(contentType)) {
				continue;
			}

			const llvm::PointerType * contentPtrType = llvm::cast<const llvm::PointerType>(contentType);
			if (contentPtrType->getAddressSpace() != GLSLBackend::PointerSpace) {
				continue;
			}

			if (func == modInfo.getVertFunc()) {
				writeAssignRaw("frag_" + varName, varName);

			} else if (func == modInfo.getFragFunc()) {
				VariableDesc desc;
				desc.name = varName;
				desc.type = contentType;
				desc.isAlloca = false;
				writeVariableDeclaration(desc);
				writeAssignRaw(varName, "frag_" + varName);
			}
		}
	}

   writeLineBreak();
}

/*
* dumps all global symbols
*/
GLSLWriter::GLSLWriter(ModuleInfo & _modInfo, const IdentifierScope & globals, PlatformInfo & _platform) :
		modInfo(static_cast<GLSLModuleInfo&>(_modInfo)),
		platform(_platform)
	{
		GLSLWriter * both = GLSLBackend::createFittingMultiWriter(*this, modInfo.getFragStream(), modInfo.getVertStream());
		GLSLWriter * frag = GLSLBackend::createWriterIfDefined(*this, modInfo.getFragStream());
		GLSLWriter * vert = GLSLBackend::createWriterIfDefined(*this, modInfo.getVertStream());

		ResourceGuard<SyntaxWriter> __guardBoth(both);
		ResourceGuard<SyntaxWriter> __guardFrag(frag);
		ResourceGuard<SyntaxWriter> __guardVert(vert);

		llvm::Module * mod = modInfo.getModule();

		//### version header ###
		both->putLine("#version " GLSL_VERSION);

		//TODO only add these if the shader program uses DEVICE_POINTERS pointers
		both->putLine("#extension GL_NV_shader_buffer_load : require");
		both->putLine("#extension GL_EXT_bindable_uniform : require");
		both->putLine("");

		//### write struct defs ###
		llvm::TypeSymbolTable & types = modInfo.getTypeSymbolTable();
		for (llvm::TypeSymbolTable::const_iterator itType = types.begin(); itType != types.end(); ++itType)
		{
			std::string name = itType->first;
			const llvm::Type * type = itType->second;

			if (type && llvm::isa<const llvm::StructType>(type))
			{
				std::string structStr = getStructTypeDeclaration(type);
				both->put( structStr );
			}
		}

		//## spill globals
		for (llvm::Module::global_iterator global = mod->global_begin(); global != mod->global_end(); ++global)
		{
			if (llvm::isa<llvm::GlobalVariable>(global))
			{
				llvm::GlobalVariable * var = llvm::cast<llvm::GlobalVariable>(global);
				const llvm::PointerType * varType = var->getType();
				const llvm::Type * contentType = varType->getElementType();
				std::string varName = var->getName().str();
				uint space = varType->getAddressSpace();

#ifdef DEBUG
				std::cerr << "global variable :" << varName << "\n";
				std::cerr << "space : " << space << "\n";
				std::cerr << "content type : "; contentType->dump(); std::cerr << '\n';
#endif
				std::string declarePart = buildDeclaration(varName, contentType);

				if (platform.implements(var))
					continue;

				if (var->isConstant()) { // global constant
					std::string initStr = getLiteral(var->getInitializer());
					std::string declareStr =  declarePart + " = " + initStr + ";";
					both->putLine("const " + declareStr);
				} else if (space == SPACE_GLOBAL) { // uniform (global constant) OR opaque sampler type
					std::string declareStr = declarePart + ";";
					both->putLine( "uniform " + declareStr );

				} else if (space == SPACE_PRIVATE) { // private (fragment output)
					std::string declareStr = declarePart + ";";
					frag->putLine("out " + declareStr );

				} else  { //possibly a device pointer
					if (llvm::isa<llvm::PointerType>(contentType)) {
						const llvm::PointerType * contentPtrType = llvm::cast<const llvm::PointerType>(contentType);

						if (contentPtrType->getAddressSpace() == GLSLBackend::PointerSpace)
						{
							std::string passVarStr = buildDeclaration("frag_" + varName, contentPtrType) + ";";
							std::string declareStr = declarePart + ";";

							vert->putLine("in " + declareStr );
							vert->putLine("out " + passVarStr );
							frag->putLine("in " + passVarStr );
						}
					}
				}
			}
	}

	both->writeLineBreak();


	//## spill function prototypes
	for(llvm::Module::iterator func = mod->begin(); func != mod->end(); ++func)
	{
		if (!platform.implements(func) && modInfo.getFragFunc() != func && modInfo.getVertFunc() != func)
		{
			both->writeFunctionDeclaration(func, NULL);
		}
	}

	both->writeLineBreak();

	//## spill buffers
	//program to vertex shader (no interpolation, vert shader is receiving)
	vert->writeShaderArgumentDeclarations(globals, modInfo.getVertFunc(), false, true);  //vertex input

	//vertex shader to fragment shader (interpolation qualifiers,frag shader receives input from vert shader)
	vert->writeShaderArgumentDeclarations(globals, modInfo.getFragFunc(), true, false);//vertex output
	frag->writeShaderArgumentDeclarations(globals, modInfo.getFragFunc(), true, true); //fragment input

		//## generate generic vertex shader
#ifdef DEBUG
		std::cerr << "completed GLSLWriter ctor\n";
#endif
		both->writeLineBreak();
}

GLSLModuleInfo & GLSLWriter::getModuleInfo()
{
	return modInfo;
}

void GLSLBlockWriter::put(std::string text)
{
	parent.put(INDENTATION_STRING + text);
}

GLSLBlockWriter::GLSLBlockWriter(GLSLWriter & _parent) :
	GLSLWriter(_parent),
	parent(_parent)
{
	parent.putLine("{");
}

void GLSLBlockWriter::writeFunctionPrologue(llvm::Function * func, IdentifierScope & funcContext)
{
	parent.writeFunctionPrologue(func, funcContext);
}

void GLSLBlockWriter::writeReturnInst(llvm::ReturnInst * retInst, IdentifierScope & locals)
{
	parent.writeReturnInst(retInst, locals);
}


GLSLBlockWriter::~GLSLBlockWriter()
{
	parent.putLine("}");
}

void GLSLRedirectedWriter::put(std::string text)
{
	stream << text;
}

GLSLRedirectedWriter::GLSLRedirectedWriter(GLSLWriter & _parent, std::ostream & _stream) :
		GLSLWriter(_parent),
		stream(_stream)
{}

GLSLRedirectedWriter::~GLSLRedirectedWriter()
{}

void GLSLMultiStageWriter::put(std::string text)
{
	if (vert)
		vert->put(text);
	if (frag)
		frag->put(text);
}

GLSLMultiStageWriter::GLSLMultiStageWriter(GLSLWriter & _parent, GLSLWriter * _vertWriter, GLSLWriter * _fragWriter) :
		GLSLWriter(_parent),
		vert(_vertWriter), frag(_fragWriter)
{
	std::cerr << "multi writer created!\n";
}

GLSLMultiStageWriter::~GLSLMultiStageWriter()
{
	delete vert;
	delete frag;
}

void GLSLMultiStageWriter::writeFunctionPrologue(llvm::Function * func, IdentifierScope & locals)
{
   typedef llvm::Function::ArgumentListType ArgList;
   const ArgList & argList = func->getArgumentList();

   std::cerr << "Multi::prologue func=" << func->getNameStr() << "\n";

   ConstValueSet arguments;
   for(ArgList::const_iterator arg = argList.begin(); arg != argList.end(); ++arg)
   {
	   arguments.insert(arg);
   }

   for(ConstVariableMap::const_iterator itVar = locals.begin();
	   itVar != locals.end();
	   ++itVar)
   {
	   const VariableDesc & desc = itVar->second;

#ifdef DEBUG
	   itVar->first->print(llvm::outs()); std::cout << " - > " << itVar->second.name << std::endl;
#endif

	   if (arguments.find(itVar->first) == arguments.end()) {
		   if (llvm::isa<llvm::PHINode>(itVar->first))
		   {
			   VariableDesc inputDesc;
			   inputDesc.name = desc.name + "_in";
			   inputDesc.type = desc.type;
			   writeVariableDeclaration(inputDesc);
		   }

//#warning "x86_mmx workaround"
		   if (desc.type->getTypeID() != llvm::Type::X86_MMXTyID) {
			   writeVariableDeclaration(desc);
		   }
	   }
   }


   // pass through attributes pointers

   //## spill globals
   llvm::Module * mod = modInfo.getModule();
	for (llvm::Module::global_iterator global = mod->global_begin(); global != mod->global_end(); ++global)
	{
		if (llvm::isa<llvm::GlobalVariable>(global))
		{
			llvm::GlobalVariable * var = llvm::cast<llvm::GlobalVariable>(global);
			std::string varName = var->getName().str();
			const llvm::PointerType * varType = var->getType();
			const llvm::Type * contentType = varType->getElementType();
			if (! llvm::isa<llvm::PointerType>(contentType)) {
				continue;
			}

			const llvm::PointerType * contentPtrType = llvm::cast<const llvm::PointerType>(contentType);
			if (contentPtrType->getAddressSpace() != GLSLBackend::PointerSpace) {
				continue;
			}

			VariableDesc desc;
			desc.name = varName;
			desc.type = contentType;
			desc.isAlloca = false;
			frag->writeVariableDeclaration(desc);
			frag->writeAssignRaw(varName, "frag_" + varName);
		}
	}

   writeLineBreak();
}

GLSLDummyWriter::GLSLDummyWriter(GLSLWriter & parent) :
	GLSLWriter(parent)
{}

void GLSLDummyWriter::put(std::string text)
{}

GLSLFragWriter::GLSLFragWriter(axtor::GLSLWriter & parent, std::ostream & fragStream) :
	GLSLRedirectedWriter(parent, fragStream)
{}

void GLSLFragWriter::writeReturnInst(llvm::ReturnInst * retInst, IdentifierScope & locals)
{
#ifdef DEBUG
	std::cerr << "GLSLFragWriter::writeReturnInst(..)\n";
#endif
	/*
	* in the fragment shader function the return value determines if the fragment is discarded or kept
	*/
	llvm::Value * keepFragVal = retInst->getOperand(0);
	uint64_t keepFrag;

	if (evaluateInt(keepFragVal, keepFrag)) {
		if (keepFrag == 0) {
			putLine( "discard;" );
		} else {
			putLine( "return;" );
		}

	} else {
		const VariableDesc * desc = locals.lookUp(keepFragVal);
		assert(desc && "was not defined");

		putLine("if (" + desc->name + ")");
		{
			GLSLBlockWriter nested(*this);
			nested.putLine("return;");
		}
		putLine("else");
		{
			GLSLBlockWriter nested(*this);
			nested.putLine("discard;");
		}
	}
 /*
  * generic treatment of ReturnInsts
  */
}

}
