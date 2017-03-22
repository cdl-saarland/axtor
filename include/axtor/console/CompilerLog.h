/*
 * CompilerLog.h
 *
 *  Created on: 01.03.2010
 *      Author: Simon Moll
 */

#ifndef COMPILERLOG_HPP_
#define COMPILERLOG_HPP_

#include <stdlib.h>
#include <iostream>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

#include <axtor/Annotations.h>

namespace axtor {

/*
 * default interface for communicating compiler warnings/errors
 */
class CompilerLog
{
private:
	static llvm::raw_ostream * msgStream;
	static uint numWarnings;

	static void assertStream();

	static void terminate() ANNOT_NORETURN;

	static void warning();

public:

	static void init(llvm::raw_ostream & _msgStream);

	/*
	 * print the @msg-warning and continue
	 */
	static void warn(const llvm::Value * value, const std::string & msg);

	static void warn(const llvm::Type * type, const std::string & msg);

	static void warn(const std::string & msg);

	/*
	 * print the error message @msg, dump the object and quit
	 */
	static void fail(const llvm::Function * Function, const std::string & msg) ANNOT_NORETURN;

	static void fail(const llvm::Value * value, const std::string & msg) ANNOT_NORETURN;

	static void fail(const llvm::Type * type, const std::string & msg) ANNOT_NORETURN;

	static void fail(const std::string & msg) ANNOT_NORETURN;
};

typedef CompilerLog Log;
}


#endif /* MESSAGEWRITER_HPP_ */
