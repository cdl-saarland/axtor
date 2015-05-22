/*
 * GenericCSerializer.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: simoll
 */


#include <axtor/backend/generic/GenericCSerializer.h>
#include <llvm/IR/Module.h>
#include <axtor/writer/SyntaxWriter.h>
#include <axtor/metainfo/ModuleInfo.h>

#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/WrappedOperation.h>
#include <axtor/console/CompilerLog.h>

#include <llvm/IR/TypeFinder.h>
#include <llvm/IR/DerivedTypes.h>

#define INDENTATION_STRING "   "

using namespace llvm;

namespace axtor {

	void GenericCSerializer::spillStructTypeDeclarations(ModuleInfo & modInfo, GenericCWriter * stream)
	{ // FIXME rewrite type reordering
		TypeFinder finder;
		const llvm::Module * mod = modInfo.getModule();
		finder.run(*mod, false);

		TypeSet undeclaredStructTypes;

		for (Type * type : finder) {
			if (StructType * structTy = dyn_cast<StructType>(type)) {
				if (! structTy->isOpaque()) {
					undeclaredStructTypes.insert(type);
				}
			}
		}

		while (!undeclaredStructTypes.empty()) {
			for (Type * type : finder) {
				// print type declaration, if all prerequesites are made
				if (! containsType(type, undeclaredStructTypes)) {
					const std::string name = modInfo.getTypeName(type);
					std::string structStr = getStructTypeDeclaration(name, llvm::cast<const llvm::StructType>(type));
					stream->put( structStr );
					undeclaredStructTypes.erase(type);
				}
			}
		}
	}

	std::string GenericCSerializer::getStructTypeDeclaration(const std::string & structName, const llvm::StructType * structType)
	{
	   std::string res =  "typedef struct \n";
	   res +=  "{\n";

	   for(uint i = 0; i < structType->getNumElements(); ++i)
	   {
		   const llvm::Type * elementType = structType->getElementType(i);

		   std::string memberName = "x" + str<int>(i);
		   std::string memberStr = buildDeclaration(memberName, elementType);

		   res += INDENTATION_STRING + memberStr + ";\n";
	   }

	   res += "} " + structName + ";\n";
	   return res;
	}
}
