/*
 * GenericCWriter.h
 *
 *  Created on: Apr 24, 2012
 *      Author: simoll
 */

#ifndef GENERICCWRITER_H_
#define GENERICCWRITER_H_

#include <axtor/writer/SyntaxWriter.h>
#include <vector>

typedef std::vector<int> IntVector;

/*
 * This class implements basic serialization capabilities in C-syntax
 * Other syntax backends may extend and customize it to implement
 * target specific language features
 */

namespace axtor {

	class GenericCWriter : public SyntaxWriter
	{

	private: //helper methods
		// consumes the first and all following cumulative values in a string to build an array dereferencing sum of indices "DATA[add0+add1+add2+add3]"
		std::string buildArraySubscript(std::string root, AddressIterator *& address, IdentifierScope & locals);

	public:

		// custom vector element access
		virtual std::string getVectorElementString (int index)=0;
		virtual std::string getVectorElementString (IntVector indices)=0;


		// TODO list of customised functions
		// customised for kernel spilling
		virtual void writeFunctionPrologue(llvm::Function * func, IdentifierScope & locals);


		// queried to implement custom behaviour for instructions (after PHI-resolution)
		virtual bool writeInstructionCustomized(const VariableDesc * desc, llvm::Instruction * inst, IdentifierScope & locals)=0;

		/*
		 * returns type symbols for default scalar types
		 */
		virtual std::string getScalarType(const llvm::Type * type, bool asVectorElementType = false)=0;

		/*
		 *  Default to unnamed pointer space if the result is the empty string
		 */
		virtual std::string getAddressSpaceName(uint space)=0;

	  	/*
	    * writes a generic function header and declares the arguments as mapped by @locals
	    */
		// customizable function header writing
		virtual std::string getFunctionHeader(llvm::Function * func, IdentifierScope * locals)=0;

	   /*
	    * returns the string representation of a operator using @operands as operand literals
	    */
	   virtual std::string getOperation(const WrappedOperation & operation, std::vector<std::string> operands)=0;





		virtual void dump();

		virtual void print(std::ostream & out);

		/*
		 *  ##### Type System #####
		 */


	  std::string getLocalVariableName(const std::string &variableFullName);
	  std::string getLocalVariableKernel(const std::string &variableFullName);

		/*
		 * generates a type name for @type
		 * if this is a pointer type, operate on its element type instead
		 */
		std::string getType(const llvm::Type * type);

		/*
		 * build a C-style declaration for @root of type @type
		 */
		std::string buildDeclaration(std::string root, const llvm::Type * type);


		/*
		* ##### DECLARATIONS / OPERATORS & INSTRUCTIONS ######
		 */
		std::string getFunctionHeader(llvm::Function * func);

		void writeLineBreak();

		void writeVariableDeclaration(const VariableDesc & desc);

		virtual void writeFunctionDeclaration(llvm::Function * func, IdentifierScope * locals = NULL);

		/*
		 * default C Style operators (returns if operand casting is required)
		 */
	   std::string getOperatorToken(const WrappedOperation & op, bool & isSigned);

	   /*
	   * returns the string representation of a operator using @operands as operand literals
	   */
	   std::string getInstruction(llvm::Instruction * inst, std::vector<std::string> operands);

	   /*
	    * returns the string representation of a constant
	    */
	   std::string getConstant(llvm::Constant * constant, IdentifierScope & locals);


	   /*
	    * returns the string referer to a value (designator for instructions/serialisation for constants)
	    */
	   std::string getReferer(llvm::Value * value, IdentifierScope & locals);

	   typedef std::vector<llvm::Value*> ValueVector;

	   /*
	    * return a dereferencing string for the next type node of the object using address
	    */
		std::string dereferenceContainer(std::string root, const llvm::Type * type, AddressIterator *& address, IdentifierScope & locals, const llvm::Type *& oElementType, uint addressSpace);

		// auxiliary functions for obtaining dereferencing or pointer to strings
		std::string getPointerTo(llvm::Value * val, IdentifierScope & locals, const std::string * rootName = 0);
		std::string getVolatilePointerTo(llvm::Value * val, IdentifierScope & locals, const std::string * rootName = 0);
		std::string getReferenceTo(llvm::Value * val, IdentifierScope & locals, const std::string * rootName = 0);
		/*
		 * return a name representing a dereferenced pointer
		*/
		std::string unwindPointer(llvm::Value * val, IdentifierScope & locals, bool & oDereferenced, const std::string * rootName = 0);

		std::string getAllNullLiteral(const llvm::Type * type);
		/*
		* return the string representation of a constant
		*/
	   std::string getLiteral(llvm::Constant * val);

	   /*
		* tries to create a literal string it @val does not have a variable
		*/
	   std::string getValueToken(llvm::Value * val, IdentifierScope & locals);

	   /*
	   * returns the string representation of a non-instruction value
	   */
	   std::string getNonInstruction(llvm::Value * value, IdentifierScope & locals);

	   /*
	   * returns the string representation of a ShuffleInstruction
	   */
		std::string getShuffleInstruction(llvm::ShuffleVectorInst * shuffle, IdentifierScope & locals);

		/*
		 * returns the string representation of an ExtractElementInstruction
		 */
		std::string getExtractElementInstruction(llvm::ExtractElementInst * extract, IdentifierScope & locals);

		/*
		 * returns the string representation of an InsertElement/ValueInstruction
		 * if the vector/compound value is defined this creates two instructions
		 */
		void writeInsertElementInstruction(llvm::InsertElementInst * insert, IdentifierScope & locals);
		void writeInsertValueInstruction(llvm::InsertValueInst * insert, IdentifierScope & locals);

		/*
	   * write a single instruction or atomic value as isolated expression
	   */
	   std::string getInstructionAsExpression(llvm::Instruction * inst, IdentifierScope & locals);

	   /*
	   * write a complex expression made up of elements from valueBlock, starting from root, writing all included insts to @oExpressionInsts
	   */
	   // generic
	   std::string getComplexExpression(llvm::BasicBlock * valueBlock, llvm::Value * root, IdentifierScope & locals, InstructionSet * oExpressionInsts = NULL);

	   /*
	    * writes a generic function header for utility functions and the default signature for the shade func
	    */
	   // customized
	   virtual void writeFunctionHeader(llvm::Function * func, IdentifierScope * locals = NULL);





	   // ### GENERIC ###
		virtual void writeInstruction(const VariableDesc * desc, llvm::Instruction * inst, IdentifierScope & locals);

		virtual void writeIf(const llvm::Value * condition, bool negateCondition, IdentifierScope & locals);

		virtual void writeElse();

		virtual void writeLoopContinue();

		virtual void writeLoopBreak();

		virtual void writeDo();

		//half-unchecked assign
		virtual void writeAssignRaw(const std::string & destName, llvm::Value * val, IdentifierScope & locals);

		virtual void writeAssign(const VariableDesc & desc, const VariableDesc & src);

		virtual void writeAssignRaw(const std::string & dest, const std::string & src);

		/*
		 * write a while for a post<checked loop
		 */
		void writePostcheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate);

		/*
		 * write a while for a postchecked loop, if oExpressionInsts != NULL dont write, but put all consumed instructions in the set
		 */
	   virtual void writePrecheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate, InstructionSet * oExpressionInsts);

	   virtual void writeInfiniteLoopBegin();

	   virtual void writeInfiniteLoopEnd();






	   // customized
	   virtual void writeReturnInst(llvm::ReturnInst * retInst, IdentifierScope & locals);



	   /*
	    * writes a generic struct type declaration to the module
	    */
	   // generic
	   virtual std::string getStructTypeDeclaration(const std::string & structName, const llvm::StructType * structType);

	   virtual void writeFunctionPrologue(llvm::Function * func, IdentifierScope & locals);

	   /*
	    * spills all global declarations (variables && types)
	    */
	   GenericCWriter(ModuleInfo & _modInfo, PlatformInfo & _platform);

	protected:
	   /*
	    * used for nested writer creation
	    */
	   GenericCWriter(GenericCWriter & writer);

	};

}



#endif /* GENERICCWRITER_H_ */
