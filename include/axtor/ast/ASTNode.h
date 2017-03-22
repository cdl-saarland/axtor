/*
 * ASTNode.h
 *
 *  Created on: 12.06.2010
 *      Author: Simon Moll
 */

#ifndef ASTNODE_HPP_
#define ASTNODE_HPP_

#include <llvm/IR/BasicBlock.h>

#include <vector>
#include <map>

namespace axtor
{
	namespace ast
	{
		class ControlNode;
		typedef std::vector<ControlNode*> NodeVector;
		typedef std::map<llvm::BasicBlock*, ControlNode*> NodeMap;

		/*
		 * nodes representing control flow structure
		 */
		class ControlNode
		{
		public:
			typedef std::vector<ControlNode*> NodeVector;
			enum NodeType
			{
				IF,          //if or if..else
				LOOP,        //infinite loop
				FOR,		 //for loop with induction variable
				BREAK,       //break
				CONTINUE,    //continue
				RETURN,      //return
				UNREACHABLE, //unreachable terminator
				BLOCK,       //unconditionally branching basic block (wrapper)
				LIST         //list of nodes
			};

		private:
			NodeType type;
			llvm::BasicBlock * block;
			NodeVector children;

		public:

			ControlNode(NodeType _type, llvm::BasicBlock * _block, uint numChildren);
			ControlNode(NodeType _type, llvm::BasicBlock * _block, const NodeVector & destChildren);
			virtual ~ControlNode();

			// returns the block this node represents
			llvm::BasicBlock * getBlock() const;
			ControlNode * getNode(int idx) const;

			void setNode(int idx, ControlNode * node);
			NodeType getType() const;

			llvm::TerminatorInst * getTerminator() const;

			NodeVector::const_iterator begin() const;
			NodeVector::const_iterator end() const;

			/*
			 * determines the first executed block on evaluation
			 */
			virtual llvm::BasicBlock * getEntryBlock() const;

			virtual void dump(std::string prefix) const;
			void dump() const;

			virtual std::string getTypeStr() const=0;
		};
	}
}

#endif /* ASTNODE_HPP_ */
