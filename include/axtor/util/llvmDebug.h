/*
 * llvmDebug.h
 *
 *  Created on: 16.03.2010
 *      Author: Simon Moll
 */

#ifndef LLVMDEBUG_HPP_
#define LLVMDEBUG_HPP_

#include <stdio.h>
#include <iostream>

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Verifier.h>

#include <axtor/CommonTypes.h>

#ifdef _DEBUG
#	include <signal.h>
#	include <sys/types.h>
#	include <unistd.h>
#endif

namespace axtor {

	void dumpBlockVector(const BlockVector & blocks);

	std::string toString(const BlockSet & blocks);
	void dumpBlockSet(const BlockSet & blocks);

	void dumpTypeSet(const TypeSet & types);

	void verifyModule(llvm::Module & mod);

	void dumpUses(llvm::Value * val);

#ifdef _DEBUG
	inline void debugBreak() {
		kill(getpid(), SIGINT);
	}
#else
	inline void debugBreak() {}
#endif

}

#endif /* LLVMDEBUG_HPP_ */
