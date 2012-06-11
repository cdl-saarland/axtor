/*
 * GenericCWriter.cpp
 *
 *  Created on: Jun 8, 2012
 *      Author: simoll
 */


#include <axtor/backend/generic/GenericCWriter.h>
#include <llvm/Instructions.h>

namespace axtor {

void GenericCWriter::writeElse()
{
	putLine( "else" );
}

void GenericCWriter::writeLoopContinue()
{
	putLine("continue;");
}

void GenericCWriter::writeLoopBreak()
{
	putLine( "break;" );
}

void GenericCWriter::writeDo()
{
	putLine( "do" );
}

/*
	  * default C Style operators
	  */
	 std::string GenericCWriter::getOperatorToken(const WrappedOperation & op, bool & isSigned)
	 {
	   isSigned = true;
	   switch (op.getOpcode())
	   {
	      //Arithmetic
	   	 case llvm::Instruction::FAdd:
	      case llvm::Instruction::Add: return "+";
	      case llvm::Instruction::FMul:
	      case llvm::Instruction::Mul: return "*";
	      case llvm::Instruction::FSub:
	      case llvm::Instruction::Sub: return "-";
	      case llvm::Instruction::UDiv:
	       isSigned = false;
	      case llvm::Instruction::SDiv:
	      case llvm::Instruction::FDiv: return "/";

	      case llvm::Instruction::URem:
	     	 isSigned = false;
	      case llvm::Instruction::SRem:
	      case llvm::Instruction::FRem: return "%";

	  //binary integer ops
	      case llvm::Instruction::Shl: return "<<";
	      case llvm::Instruction::LShr: return ">>";
	      case llvm::Instruction::AShr: return ">>";

	      //the distinction between logic and bitwise operators is based on the instruction type (if its a "bool" aka "i1" return the logic operator)
	      case llvm::Instruction::And:
	      case llvm::Instruction::Or:
	      case llvm::Instruction::Xor:
	      {
	     	 if (op.getType()->isIntegerTy(1)) {
	     		 switch (op.getOpcode()) {
	 				 case llvm::Instruction::And: return "&&";
	 				  case llvm::Instruction::Or: return "||";
	 				  case llvm::Instruction::Xor: return "!=";
	 			 };

	     	 } else {
	     		 switch (op.getOpcode()) {
	 				 case llvm::Instruction::And: return "&";
	 				  case llvm::Instruction::Or: return "|";
	 				  case llvm::Instruction::Xor: return "^";
	 			 };
	     	 }
	      }

	      //predicated CmpInsts
	      case llvm::Instruction::FCmp:
	      {
	         switch (op.getPredicate())
	         {
	            case llvm::CmpInst::FCMP_FALSE:
	              assert(false && "why did nobody optimize that out?");

	            case llvm::CmpInst::FCMP_OGT:
	            case llvm::CmpInst::FCMP_UGT:
	               return ">";

	            case llvm::CmpInst::FCMP_OGE:
	            case llvm::CmpInst::FCMP_UGE:
	               return ">=";

	            case llvm::CmpInst::FCMP_OLT:
	            case llvm::CmpInst::FCMP_ULT:
	               return "<";

	            case llvm::CmpInst::FCMP_OLE:
	            case llvm::CmpInst::FCMP_ULE:
	               return "<=";

	            case llvm::CmpInst::FCMP_OEQ:
	               return "=="; //overloaded operator in C

	            default:
	               assert(false && "unmapped cmp predicate");
	         };
	      }

	      case llvm::Instruction::ICmp:
	      {
	         switch (op.getPredicate())
	         {
	            case llvm::CmpInst::ICMP_UGT:
	         	   isSigned = false;
	            case llvm::CmpInst::ICMP_SGT:
	               return ">";

	            case llvm::CmpInst::ICMP_UGE:
	         	   isSigned = false;
	            case llvm::CmpInst::ICMP_SGE:
	               return ">=";

	            case llvm::CmpInst::ICMP_ULT:
	         	   isSigned = false;
	            case llvm::CmpInst::ICMP_SLT:
	               return "<";

	            case llvm::CmpInst::ICMP_ULE:
	         	   isSigned = false;
	            case llvm::CmpInst::ICMP_SLE:
	               return "<=";

	            case llvm::CmpInst::ICMP_EQ:
	               return "=="; //overloaded operator in C

	            case llvm::CmpInst::ICMP_NE:
	               return "!=";

	            default:
	               assert(false && "unmapped cmp predicate");
	         };
	      }

	      default:
	     	 Log::fail(op.getValue(), str<uint>(op.getOpcode()) + " unsupported operator type");
	   }
	   Log::fail("failure : internal error in getOperator()");
	   return "UNREACHABLE";
	 }

	 std::string GenericCWriter::buildArraySubscript(std::string root, AddressIterator *& address, IdentifierScope & locals)
	 {
	 	uint64_t index;

	 	std::stringstream out;
	 	bool isFirst = true;

	 	do
	 	{
	 		llvm::Value * indexVal = address->getValue();

	 		if (!isFirst) {
	 			out << " + ";
	 		}

	 		//dynamic subscript
	 		if (evaluateInt(indexVal, index)) {
	 			out << str<uint64_t>(index);

	 		//static subscript
	 		} else {
	 			const VariableDesc * desc = locals.lookUp(indexVal);
	 			assert(desc && "undefined index value");
	 			out << desc->name;
	 		}

	 		isFirst = false;
	 		address = address->getNext();
	 	} while (address && address->isCumulative());

	 	return root + "[" + out.str() + "]";
	 }

	 void GenericCWriter::writeAssignRaw(const std::string & dest, const std::string & src)
	 {
	 	putLine(dest + " = " + src + ";");
	 }

	 void GenericCWriter::writeAssignRaw(const std::string & destStr, llvm::Value * val, IdentifierScope & locals)
	 {
	 	const VariableDesc * srcDesc = locals.lookUp(val);
	 	std::string srcText;

	 	if (srcDesc) {
	 		srcText = srcDesc->name;
	 	} else if (llvm::isa<llvm::GetElementPtrInst>(val)) {
	 		srcText = getPointerTo(val, locals);
	 	} else if (llvm::isa<llvm::Constant>(val)) {
	 		srcText = getConstant(llvm::cast<llvm::Constant>(val), locals);
	 	} else {
	 		Log::fail(val, "source values of this kind are not covered in writeAssignRaw (TODO)");
	 	}

	 	putLine(destStr + " = " + srcText + ";");
	 }

}
