/*
 * CWriter.h
 *
 *  Created on: 22.5.2014
 *      Author: Simon Moll
 */

#ifndef CWRITER_HPP
#define CWRITER_HPP

#include <set>
#include <string>
#include <vector>

#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
//#include <llvm/TypeSymbolTable.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstrTypes.h>

#include <axtor/CommonTypes.h>
#include <axtor/writer/SyntaxWriter.h>

#include <axtor/intrinsics/PlatformInfo.h>

#include <axtor/console/CompilerLog.h>

#include <axtor/intrinsics/AddressIterator.h>
#include <axtor/util/ResourceGuard.h>
#include <axtor/util/WrappedOperation.h>
#include <axtor/util/llvmConstant.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/stringutil.h>

#include <axtor/backend/generic/GenericCSerializer.h>
#include <axtor/backend/generic/GenericCWriter.h>

#include "CModuleInfo.h"

#define INDENTATION_STRING "   "
namespace llvm {
  class TerminatorInst;
};
namespace axtor {

class CBlockWriter;

/*
 * Generic SlangWriter interface
 */
class CWriter : public GenericCWriter, protected GenericCSerializer {
  friend class CBlockWriter;
  friend class CPassThroughWriter;
  friend class CMultiWriter;

private:
  CModuleInfo &modInfo;
  PlatformInfo &platform;

protected:
  virtual void put(const std::string &text);

public:
  virtual void dump();

  virtual void print(std::ostream &out);

  /*
   *  ##### Type System #####
   */

  /*
   * returns type symbols for default scalar types
   */
  std::string getScalarType(const llvm::Type *type,
                            bool asVectorElementType = false);
  std::string getLocalVariableName(const std::string &variableFullName);

  /*
   * generates a type name for @type
   * if this is a pointer type, operate on its element type instead
   */
  std::string getType(const llvm::Type *type);

protected:
  /*
   * build a C-style declaration for @root of type @type
   */
  std::string buildDeclaration(std::string root, const llvm::Type *type);
  std::string buildDeclarationRec(std::string root, const llvm::Type *type, bool firstType);

public:
  /*
   * Given by GenericCWriter
   */
  // virtual void writeAssignRaw(const std::string & dest, const std::string &
  // src) { return GenericCWriter::writeAssignRaw(dest, src); } virtual void
  // writeAssignRaw(const std::string & destName, llvm::Value * val,
  // IdentifierScope & locals) { GenericCWriter::writeAssignRaw(destName,
  // val,locals); }

  /*
   * ##### DECLARATIONS / OPERATORS & INSTRUCTIONS ######
   */

  /*
   * writes a generic function header and declares the arguments as mapped by
   * @locals
   */
  std::string getFunctionHeader(llvm::Function *func, IdentifierScope *locals);

  std::string getFunctionHeader(llvm::Function *func);

  virtual void writeVariableDeclaration(const VariableDesc &desc);

  virtual void writeFunctionDeclaration(llvm::Function *func,
                                        IdentifierScope *locals = NULL);

  // specialized memory access functions
  std::string getConsecutiveVectorAccess(llvm::Type &dataTy,
                                         llvm::Value &ptrVal,
                                         IdentifierScope &locals,
                                         llvm::Value *storedValue);
  std::string getRandomVectorAccess(llvm::Type &dataTy,
                                    llvm::Value &ptrVal,
                                    IdentifierScope &locals,
                                    llvm::Value *storedValue);
  std::string getLoadStore(llvm::Instruction &inst, IdentifierScope &locals);
  std::string getVectorAccess(llvm::Instruction & inst, IdentifierScope & locals);

  std::string getShuffle(llvm::Instruction & inst, IdentifierScope & locals);

  /*
   * returns the string representation of a operator using @operands as operand
   * literals
   */
  std::string getInstruction(llvm::Instruction *inst, StringVector operands);

  /*
   * returns the string representation of a operator using @operands as operand
   * literals
   */
  std::string getOperation(const WrappedOperation &operation,
                           std::vector<std::string> operands);

  std::string getVectorConvert(const WrappedOperation & op, StringVector operands);

  std::string getVectorTruncate(const WrappedOperation & op, StringVector operands);

  std::string getBroadcast(std::string laneText, llvm::Type & laneTy);

  std::string getVectorCompare(const WrappedOperation &op, StringVector operands);

  /*
   * returns the string referer to a value (designator for
   * instructions/serialisation for constants)
   */
  std::string getReferer(llvm::Value *value, IdentifierScope &locals);

  typedef std::vector<llvm::Value *> ValueVector;

  /*
   * return a dereferencing string for the next type node of the object using
   * address
   */
  std::string dereferenceContainer(std::string root, const llvm::Type *type,
                                   AddressIterator *&address,
                                   IdentifierScope &locals,
                                   const llvm::Type *&oElementType,
                                   uint addressSpace);

  // auxiliary functions for obtaining dereferencing or pointer to strings
  std::string getVolatilePointerTo(llvm::Value *val, IdentifierScope &locals,
                                   const std::string *rootName = 0);
  /*
   * return a name representing a dereferenced pointer
   */
  std::string unwindPointer(llvm::Value *val, IdentifierScope &locals,
                            bool &oDereferenced,
                            const std::string *rootName = 0);

  std::string getAllNullLiteral(const llvm::Type *type);
  /*
   * return the string representation of a constant
   */
  std::string getLiteral(llvm::Constant *val);

  /*
   * tries to create a literal string it @val does not have a variable
   */
  std::string getValueToken(llvm::Value *val, IdentifierScope &locals);

  /*
   * returns the string representation of a non-instruction value
   */
  std::string getNonInstruction(llvm::Value *value, IdentifierScope &locals);

  /*
   * returns the string representation of a ShuffleInstruction
   */
  std::string getShuffleInstruction(llvm::ShuffleVectorInst *shuffle,
                                    IdentifierScope &locals);

  /*
   * returns the string representation of an ExtractElementInstruction
   */
  std::string getExtractElementInstruction(llvm::ExtractElementInst *extract,
                                           IdentifierScope &locals);

  /*
   * returns the string representation of an InsertElement/ValueInstruction
   * if the vector/compound value is defined this creates two instructions
   */
  void writeInsertElementInstruction(llvm::InsertElementInst *insert,
                                     IdentifierScope &locals);
  void writeInsertValueInstruction(llvm::InsertValueInst *insert,
                                   IdentifierScope &locals);

  /*
   * write a single instruction or atomic value as isolated expression
   */
  std::string getInstructionAsExpression(llvm::Instruction *inst,
                                         IdentifierScope &locals);

  /*
   * write a complex expression made up of elements from valueBlock, starting
   * from root, writing all included insts to @oExpressionInsts
   */
  std::string getComplexExpression(llvm::BasicBlock *valueBlock,
                                   llvm::Value *root, IdentifierScope &locals,
                                   InstructionSet *oExpressionInsts = NULL);

  /*
   * writes a generic function header for utility functions and the default
   * signature for the shade func
   */
  virtual void writeFunctionHeader(llvm::Function *func,
                                   IdentifierScope *locals = NULL);

  virtual void writeInstruction(const VariableDesc *desc,
                                llvm::Instruction *inst,
                                IdentifierScope &locals);

  virtual void writeIf(const llvm::Value *condition, bool negateCondition,
                       IdentifierScope &locals);

  /*
   * write a while for a post<checked loop
   */
  void writePostcheckedWhile(llvm::BranchInst *branchInst,
                             IdentifierScope &locals, bool negate);

  /*
   * write a while for a postchecked loop, if oExpressionInsts != NULL dont
   * write, but put all consumed instructions in the set
   */
  virtual void writePrecheckedWhile(llvm::BranchInst *branchInst,
                                    IdentifierScope &locals, bool negate,
                                    InstructionSet *oExpressionInsts);

  virtual void writeInfiniteLoopBegin();

  virtual void writeInfiniteLoopEnd();

  virtual void writeReturnInst(llvm::ReturnInst *retInst,
                               IdentifierScope &locals);

  // For loop
  virtual void writeForLoopBegin(ForLoopInfo &forInfo, IdentifierScope &locals);

  virtual void writeFunctionPrologue(llvm::Function *func,
                                     IdentifierScope &locals);

  CWriter(ModuleInfo &_modInfo, PlatformInfo &_platform);

  // serializer representation of a SCEV
  // FIXME unused
  std::string getSCEV(const llvm::SCEV *scev, IdentifierScope &locals);

protected:
  /*
   * used for nested writer creation
   */
  CWriter(CWriter &writer);
};
#if 0
class CModuleWriter : public CWriter {
	   /*
	    * spills all global declarations (variables && types)
	    */
	   CModuleWriter(ModuleInfo & _modInfo, PlatformInfo & _platform);
};
#endif
/*
 * forwards all writes to the parent object
 */
class CPassThroughWriter : public CWriter {
  CWriter &parent;

public:
  CPassThroughWriter(CWriter &_parent);

protected:
  virtual void put(const std::string &text);
};

/*
 * writes the output to two streams
 */
class CMultiWriter : public CWriter {
  CWriter first;
  CWriter second;

public:
  CMultiWriter(CWriter &_first, CWriter &_second);

protected:
  virtual void put(const std::string &text);
};

/*
 * BlockWriter (indents all writes and embraces them in curly brackets)
 */
class CBlockWriter : public CWriter {
  CWriter &parent;

protected:
  virtual void put(const std::string &text);

public:
  CBlockWriter(CWriter &_parent);

  virtual ~CBlockWriter();
};

} // namespace axtor

#endif
