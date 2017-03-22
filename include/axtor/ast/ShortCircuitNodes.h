/*
 * ShortCircuitNodes.h
 *
 *  Created on: 29.11.2010
 *      Author: Simon Moll
 */

#ifndef SHORTCIRCUITNODES_HPP_
#define SHORTCIRCUITNODES_HPP_

#include <axtor/config.h>

#ifdef ENABLE_SHORT_CIRCUIT_EXPRESSIONS
#include <vector>
#include <axtor/ast/ASTNode.h>


namespace axtor {

namespace ast {

/*
 * control-flow node for representing a short-circuit boolean expression
 */
struct ShortCircuitNode : public ConditionalNode
{
	struct CircuitElement
	{
		virtual bool isBlockWrapper() const;
	};

	/*
	 * wrapper for a single basic block
	 */
	struct CircuitBlock : public CircuitElement
	{
		CircuitBlock(llvm::BasicBlock * _block);
		bool isBlockWrapper() const
		{
			return true;
		}

		llvm::BasicBlock * getBlock() const { return block; }

	private:
		llvm::BasicBlock * block;
	};

	/*
	 * circuit expression element for expressing short-circuit evaluated expressions
	 */
	struct CircuitExpression : public CircuitElement
	{
		CircuitExpression(CircuitElement * _startElement, CircuitElement * _primaryExit, CircuitElement * _defaultExit);
		bool isBlockWrapper() const
		{
			return false;
		}

		CircuitElement * getStartElement() const { return startElement; }
		CircuitElement * getPrimaryExit() const { return primaryExit; }
		CircuitElement * getDefaultExit() const { return defaultExit; }

	private:
		CircuitElement * startElement;
		CircuitElement * primaryExit;
		CircuitElement * defaultExit;
	};

private:
	std::vector<CircuitElement*> elements;
};

}

}

#endif

#endif  /* SHORTCIRCUITNODES_HPP_ */
