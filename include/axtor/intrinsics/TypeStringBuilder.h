/*
 * TypeStringBuilder.h
 *
 *  Created on: 20.03.2010
 *      Author: Simon Moll
 */

#ifndef TYPESTRINGBUILDER_HPP_
#define TYPESTRINGBUILDER_HPP_

#include <util/stringutil.h>
#include <iostream>

#warning "deprecated!!"
/*
 * objects of this class are used as a building site to derive Backend specific type names from LLVM-types
 * This is necessary because type names can be made up of type names for its element types
 */
class TypeStringBuilder
{
public:
	typedef std::vector<TypeStringBuilder*> TypeBuilderVector;

private:
	const llvm::Type * type;
	std::string formatString;
	TypeBuilderVector children;

public:
	TypeStringBuilder(const llvm::Type * _type) :
		type(_type)
	{}

	~TypeStringBuilder()
	{
		for(TypeBuilderVector::iterator itChild = children.begin(); itChild != children.end(); ++itChild)
		{
			delete *itChild;
		}
	}

	const llvm::Type * getType()
	{
		return type;
	}

	std::string getFormatString()
	{
		return formatString;
	}

	TypeStringBuilder * getChild(uint idx)
	{
		assert(idx < children.size() && "invalid idx");
		return children[idx];
	}

	uint getNumChildren() const
	{
		return children.size();
	}

	void setFormatString(std::string format)
	{
		formatString = format;
	}

	void pushType(const llvm::Type * child)
	{
		children.push_back(new TypeStringBuilder(type));
	}

	std::string toString()
	{
		if (children.empty())
			return formatString;

		std::string * childStrings = new std::string[children.size()];

		// build childStrings
		for (uint i = 0; i < children.size(); ++i)
		{
			TypeStringBuilder * childBuilder = children[i];
			childStrings[i] = childBuilder->toString();
		}

		//insert childString into formatString
		std::string result;
		StringVector tokens = tokenizeIsolated(formatString, "%");

		assert(tokens.size() == children.size() + 1 && "there must be a %s for each child:invalid formatString");

		for(uint i = 0; i < children.size(); ++i)
		{
			result += tokens[i] + childStrings[i];
		}

		result += tokens[children.size()];

		delete [] childStrings;

		return result;
	}
};

#endif /* TYPESTRINGBUILDER_HPP_ */
