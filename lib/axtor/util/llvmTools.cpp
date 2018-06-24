/*
 * llvmTools.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/util/llvmTools.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <system_error>

#include <iostream>

using namespace llvm;

namespace axtor {

	llvm::Module* createModuleFromFile(std::string fileName)
	{
		llvm::LLVMContext & context = SharedContext::get();
		return createModuleFromFile(fileName, context);
	}


	llvm::Module* createModuleFromFile(std::string fileName, llvm::LLVMContext & context)
	{
		SMDiagnostic smDiag;
		std::unique_ptr<Module> modPtr = parseIRFile(fileName, smDiag, context);
		if (!modPtr) {
			smDiag.print("axtor", llvm::errs(), true);

		}
		return modPtr.release();
	}

	void writeModuleToFile(llvm::Module * M, const std::string & fileName)
	{
		assert (M);
		std::error_code EC;
		llvm::raw_fd_ostream file(fileName.c_str(), EC);
                M->print(file, nullptr);
                file.close();
                if (EC) {
                  errs() << "ERROR: printing module to file failed: " << EC.message() << "\n";
                  abort();
                }
	}

}

