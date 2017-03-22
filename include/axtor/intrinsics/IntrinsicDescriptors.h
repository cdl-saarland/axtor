/*
 * IntrinsicDescriptors.h
 *
 *  Created on: 19.03.2010
 *      Author: Simon Moll
 */

#ifndef INTRINSICDESCRIPTORS_HPP_
#define INTRINSICDESCRIPTORS_HPP_

#include <axtor/CommonTypes.h>
#include <axtor/console/CompilerLog.h>

namespace axtor
{
	/*
	 * Common interface for intrinsic descriptors
	 */
	class IntrinsicDescriptor
	{
	protected:
		uint getSize(StringVector::const_iterator start, StringVector::const_iterator end);
	public:
		virtual std::string build(StringVector::const_iterator start, StringVector::const_iterator end) = 0;
		virtual ~IntrinsicDescriptor() {}
	};

	typedef std::map<std::string, IntrinsicDescriptor*> IntrinsicsMap;



	/*
	 * class for function style intrinsics
	 */
	class IntrinsicFuncDesc : public IntrinsicDescriptor
	{
		std::string funcName;
		uint numArgs;

	public:
		IntrinsicFuncDesc(std::string _funcName, uint _numArgs);
		virtual ~IntrinsicFuncDesc() {}

		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	};

	/*
	 * class for call-site modifications
	 *
	 * e.g.
	 *
	 * SOMETHING(%,to_uint4(%),%) -> casts the second argument to an uint4
	 *
	 *  % : position of an argument string
	 *
	 *  Note, that ' ' characters are removed from the format string (future implementations may allow
	 *  escaping to deal with that)
	 */
	class IntrinsicComplexDesc : public IntrinsicDescriptor
	{
		StringVector chunks;

	public:
		IntrinsicComplexDesc(std::string formatString);
		virtual ~IntrinsicComplexDesc() {}
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	};

	/*
	 * intrinsic resulting in an assignment
	 */
	class IntrinsicAssignmentDesc : public IntrinsicDescriptor
	{
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
		virtual ~IntrinsicAssignmentDesc() {}
	};

	/*
	 * infix operator intrinsic
	 */
	class IntrinsicInfixDesc : public IntrinsicDescriptor
	{
		std::string op;
	public:
		IntrinsicInfixDesc(std::string _op);
		virtual ~IntrinsicInfixDesc() {}

		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	};

	/*
	 * unary operator intrinsic
	 */
	class IntrinsicUnaryDesc : public IntrinsicDescriptor
	{
		std::string op;
	public:
		IntrinsicUnaryDesc(std::string _op);
		virtual ~IntrinsicUnaryDesc() {}
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	};

	/*
	 * returns an empty string
	 */
	class IntrinsicNopDesc : public IntrinsicDescriptor
	{
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	public:
		virtual ~IntrinsicNopDesc() {}
	};

	/*
	 * returns the first operand
	 */
	class IntrinsicPassDesc : public IntrinsicDescriptor
	{
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	public:
		virtual ~IntrinsicPassDesc() {}
	};

	/*
	 * describes reserved global symbols
	 */
	class IntrinsicGlobalDesc : public IntrinsicDescriptor
	{
		std::string global;
	public:
		IntrinsicGlobalDesc(std::string _global);
		virtual ~IntrinsicGlobalDesc() {}
		std::string build (StringVector::const_iterator start, StringVector::const_iterator end);
	};

	/*
	 * generic component accessor intrinsic (e.g. for opaque matrix types)
	 */
	class IntrinsicGetterDesc : public IntrinsicDescriptor
	{
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	public:
		virtual ~IntrinsicGetterDesc() {}
	};

	/*
	 * generic component accessor intrinsic (e.g. for opaque matrix types)
	 */
	class IntrinsicSetterDesc : public IntrinsicDescriptor
	{
		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	public:
		virtual ~IntrinsicSetterDesc() {}
	};

	/*
	 * intrinsics with text replacement for calls
	 */
	class IntrinsicCallReplacementDesc : public IntrinsicDescriptor
	{
		std::string callText;

	public:
		IntrinsicCallReplacementDesc(std::string _callText);
		virtual ~IntrinsicCallReplacementDesc() {}

		std::string build(StringVector::const_iterator start, StringVector::const_iterator end);
	};

	/*
	 * special descriptor that returns an empty build string for this intrinsic
	 */
	class IntrinsicMuteDesc : public IntrinsicDescriptor
	{
	public:
		IntrinsicMuteDesc(std::string _callText) {}
		virtual ~IntrinsicMuteDesc() {}

		std::string build(StringVector::const_iterator start, StringVector::const_iterator end) { return ""; }
	};
}


#endif /* INTRINSICDESCRIPTORS_HPP_ */
