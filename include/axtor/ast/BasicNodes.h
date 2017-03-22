/*
 * BasicNodes.h
 *
 *  Created on: 12.06.2010
 *      Author: Simon Moll
 */

#ifndef BASICNODES_HPP_
#define BASICNODES_HPP_

#include <map>

#include <llvm/IR/Function.h>

#include "ASTNode.h"
#include <axtor/CommonTypes.h>

namespace llvm {
	class SCEVAddRecExpr;
}
/*
 * the extractor exclusively uses nodes described in this file to rebuild the AST. More sophisticated control flow elements are created by enhancing a pre-existing AST
 */
namespace axtor
{

	class ForLoopInfo;

	namespace ast
	{

		/*
		 * represents functions
		 */
		class FunctionNode
		{
			llvm::Function * func;
			ControlNode * entryNode;

		public:
			llvm::Function * getFunction() const;
			ControlNode * getEntry() const;

			FunctionNode(llvm::Function * _func, ControlNode * _entryNode);
			void dump() const;
		};

		typedef std::map<llvm::Function*,FunctionNode*> ASTMap;

		/*
		 * conditional node (if / if..else)
		 *
		 * The concrete structure of the described conditional entity is determined by the values of @onTrueNode and @onFalseNode
		 *
		 * Note that in a textual representation it might be necessary to negate the controlling expression in cases like onTrueNode == NULL, onFalseNode != NULL
		 */
		class ConditionalNode : public ControlNode
		{
		public:
			enum ChildIndex
			{
				ON_TRUE = 0,
				ON_FALSE = 1
			};

			ConditionalNode(llvm::BasicBlock * _block, ControlNode * onTrueNode, ControlNode * onFalseNode);

			llvm::Value * getCondition() const;
			ControlNode * getOnTrue() const;
			ControlNode * getOnFalse() const;
			llvm::BasicBlock * getOnTrueBlock() const;
			llvm::BasicBlock * getOnFalseBlock() const;

			virtual std::string getTypeStr() const;
		};

		/*
		 * Infinitely looping node
		 */
		class LoopNode : public ControlNode
		{
		public:
			enum ChildIndex
			{
				BODY = 0
			};

			LoopNode(ControlNode * _body);
			virtual std::string getTypeStr() const;
		};


		/*
		 * for-loop with an induction variable
		 */
		class ForLoopNode : public ControlNode
		{
			ForLoopInfo * forInfo;

		public:
			enum ChildIndex
			{
				BODY = 0
			};

			ForLoopNode(ForLoopInfo * _forInfo, ControlNode * _body);
			~ForLoopNode();

			virtual std::string getTypeStr() const;
			ForLoopInfo * getForLoopInfo() const { return forInfo; };
		};

		/*
		 * break
		 */
		class BreakNode : public ControlNode
		{
		public:
			BreakNode(llvm::BasicBlock * _block);
			virtual std::string getTypeStr() const;
		};

		/*
		 * continue
		 */
		class ContinueNode : public ControlNode
		{
		public:
			ContinueNode(llvm::BasicBlock * _block);
			virtual std::string getTypeStr() const;
		};

		/*
		 * return
		 */
		class ReturnNode : public ControlNode
		{
		public:
			ReturnNode(llvm::BasicBlock * _block);
			llvm::ReturnInst * getReturn() const;
			virtual std::string getTypeStr() const;
		};

		/*
		 * unreachable
		 */
		class UnreachableNode : public ControlNode
		{
		public:
			virtual std::string getTypeStr() const;
			UnreachableNode(llvm::BasicBlock * _block);
		};


		/*
		 * sequence of ast nodes
		 */
		class ListNode : public ControlNode
		{
		public:
			/*
			 * constructor for single unconditionally branching basic blocks
			 */
			ListNode(const NodeVector & nodeVector);
			virtual std::string getTypeStr() const;
		};

		/*
		 * wrapper for a unconditionally branching basic block
		 */
		class BlockNode : public ControlNode
		{
		public:
			BlockNode(llvm::BasicBlock * _block);
			virtual std::string getTypeStr() const;
		};
	}
}

#endif /* BASICNODES_HPP_ */
