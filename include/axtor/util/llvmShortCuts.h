/*
 * llvmShortCuts.h
 *
 *  Created on: 08.02.2010
 *      Author: Simon Moll
 */

#ifndef LLVMSHORTCUTS_HPP_
#define LLVMSHORTCUTS_HPP_

#include <set>
#include <stack>
#include <algorithm>
#include <map>

#include <axtor/CommonTypes.h>

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
//#include <llvm/TypeSymbolTable.h>

#include <axtor/util/SharedContext.h>
#include <axtor/util/ExtractorRegion.h>

namespace axtor {

typedef std::pair<BlockSet, BlockSet> BlockSetPair;

/*
 * find functions with a certain prefix
 */
FunctionVector findFunctionsPrefixed(llvm::Module & M, std::string prefix);

std::string cleanDesignatorName(std::string name);

uint64_t generateTruncMask(uint width);

bool isType(llvm::Value * value, const llvm::Type::TypeID type);

inline bool isType(llvm::Value & value, const llvm::Type::TypeID type) { return isType(&value, type); }

bool isVoid(llvm::Value * value);

bool isType(const llvm::Type * type, const llvm::Type::TypeID id);

bool isVoid(const llvm::Type * type);

const llvm::FunctionType * getFunctionType(llvm::Function * func);
const llvm::Type * getReturnType(llvm::Function * func);

/*
 * checks if the CFG dominated by @start is only left using @expectedExits
 * returns .first  : set of exits that were not anticipated
 *         .second : expectedExits that were used
 */
BlockSetPair computeDominanceFrontierExt(llvm::BasicBlock * start, llvm::DominatorTree & domTree, BlockSet expectedExits);

/*
 * returns .first  : set of exits that were not anticipated
 *         .second : expectedExits that were used
 */
BlockSetPair computeExitSet(RegionVector regions, llvm::DominatorTree & domTree, BlockSet regularExits);

/*
 * checks whether @type ist made up of types from @typeSet
 */
bool containsType(const llvm::Type * type, const TypeSet & typeSet);

bool doesContainType(const llvm::Type * type, llvm::Type::TypeID id);


//bool getTypeSymbol(llvm::TypeSymbolTable & typeSymbolTable, const llvm::Type * type, std::string & out);

int getSuccessorIndex(llvm::Instruction * termInst, const llvm::BasicBlock * target);

/*
 * checks whether there is a non-anticipated path from A to B
 */
bool reaches (llvm::BasicBlock * A, llvm::BasicBlock * B, BlockSet regularExits);

template<class T>
std::vector<T> toVector(std::set<T> S);

/*
 * returns the Opcode of Instructions and ConstantExprs
 */
uint getOpcode(const llvm::Value * value);

/*
 * checks whether this is a GEP instruction or constant expression
 */
bool isGEP(llvm::Value * val);


/*
 *  Taken from LLVM
 */

// returns all a set of all occuring exit blocks
void getUniqueExitBlocks(llvm::Loop & loop, BlockSet & exits);


/// getExitEdges - Return all pairs of (_inside_block_,_outside_block_).
  void getExitEdges(llvm::Loop & loop, BlockPairVector & ExitEdges);

  void LazyRemapBlock(llvm::BasicBlock  & BB,
                                          ValueMap &ValueMap);

  //remaps instructions' operands if they are stored in the map
  void LazyRemapInstruction(llvm::Instruction  &I,
                                      ValueMap &ValueMap);

llvm::CmpInst * createNegation(llvm::Value * value, llvm::Instruction * before);

bool containsPHINodes(const llvm::BasicBlock * block);

BlockVector getAllPredecessors(llvm::BasicBlock * block);

template<class T>
bool contains(const std::vector<T> & vector, T element);

template<class T>
bool set_contains(const std::set<T> & set, T element);

bool usedInFunction(llvm::Function * func, llvm::Value * val);

template<class T>
void mergeInto(std::set<T> & A, std::set<T> & B);

template<class T>
std::set<T> getWithout(const std::set<T> & A, T object);

}


#endif /* LLVMSHORTCUTS_HPP_ */
