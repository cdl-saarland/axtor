/*
 * ModuleInfo.h
 *
 *  Created on: 25.02.2010
 *      Author: Simon Moll
 */

#ifndef MODULEINFO_HPP_
#define MODULEINFO_HPP_

#include <llvm/IR/Module.h>

#include <llvm/Support/FormattedStream.h>
//#include <llvm/TypeSymbolTable.h>

#include <axtor/CommonTypes.h>
#include <llvm/IR/Type.h>

#include <string>

namespace llvm {
	namespace legacy {
		class PassManager;
	}
}

namespace axtor {

/*
 * base class for Backend specific ModuleInfo classes that
 * provides additional information about the target llvm::Module
 */
class ModuleInfo
{
	llvm::Module & M;

	typedef std::map<std::string, llvm::Type*> StringToType;

	StringToType stringToType;


public:
	ModuleInfo(llvm::Module & _M);

	virtual ~ModuleInfo() {}

	const llvm::Module * getModule() const;

	/*
	 * returns whether this the module info object for that module
	 */
	virtual bool isTargetModule(llvm::Module*) const = 0;

	/*
	 * create global scope identifier bindings
	 */
	virtual IdentifierScope createGlobalBindings() = 0;

	/*
	 * verify the module integrity with respect to target language limitations
	 */
	virtual void verifyModule() = 0;

	/*
	 * dump information about this module descriptor
	 */
	virtual void dump()=0;

	/*
	 * dump all information contained in this module info object
	 */
	virtual void dumpModule()=0;

	virtual void runPassManager(llvm::legacy::PassManager & pm)=0;

	/*
	 * returns the type bound to that name
	 *
	 * (ModuleInfo typenames override llvm::Module struct type names)
	 */

	std::string getTypeName(const llvm::Type * type) const;
	// will return a conflicting type, or 0 iff succeeded
	llvm::Type * setTypeName(llvm::Type * type, const std::string & _typeName);
	llvm::Type * lookUpType(const std::string & name) const;

	/*
	 * write all extracted data to the given output stream
	 */
	virtual void writeToLLVMStream(llvm::formatted_raw_ostream & out)
	{ assert(false && "not implemented"); }

};

}

#endif /* MODULEINFO_HPP_ */
