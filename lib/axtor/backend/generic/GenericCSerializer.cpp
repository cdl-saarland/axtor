/*
 * GenericCSerializer.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: simoll
 */


#include <axtor/backend/generic/GenericCSerializer.h>
#include <llvm/Module.h>
#include <axtor/writer/SyntaxWriter.h>
#include <axtor/metainfo/ModuleInfo.h>

#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/WrappedOperation.h>
#include <axtor/console/CompilerLog.h>

#define INDENTATION_STRING "   "

namespace axtor {

	void GenericCSerializer::spillStructTypeDeclarations(ModuleInfo & modInfo, GenericCWriter * stream)
	{
		const llvm::Module * mod = modInfo.getModule();
		StructTypeVector types;
		mod->findUsedStructTypes(types);

		//### print type declarations ###
		{
			//### sort dependent types for forward declaration ###
			{
				TypeSet undeclaredTypes;

				for (StructTypeVector::iterator itType = types.begin(); itType != types.end(); ++itType) {
					if ((*itType)->getStructNumElements() > 0) //opqaue types don't need to be declared
					undeclaredTypes.insert(*itType);
				}

				for (StructTypeVector::iterator itType = types.begin(); itType != types.end();) {

					const llvm::StructType * type = *itType;
					const std::string name = modInfo.getTypeName(*itType);

					if (
							(type->getStructNumElements() == 0) || // don't declare opaque types
							undeclaredTypes.find(type) == undeclaredTypes.end() || //already declared
							containsType(type, undeclaredTypes))
					{
						++itType;
					} else {

						//write declaration
						std::string structStr = getStructTypeDeclaration(name, llvm::cast<const llvm::StructType>(type));
						stream->put( structStr );

						undeclaredTypes.erase(type);
						itType = types.begin();
					}
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
