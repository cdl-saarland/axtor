/*
 * GenericCSerializer.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: simoll
 */


#include <axtor/backend/generic/GenericCSerializer.h>
#include <llvm/Module.h>
#include <axtor/writer/SyntaxWriter.h>

#define INDENTATION_STRING "   "

namespace axtor {

	void GenericCSerializer::spillStructTypeDeclarations(llvm::Module * mod, SyntaxWriter * stream)
	{
		StructTypeVector types;
		mod->findUsedStructTypes(types);

		//### print type declarations ###
		{
			//### sort dependent types for forward declaration ###
			{
				TypeSet undeclaredTypes;

				for (StructTypeVector::iterator itType = types.begin(); itType != types.end(); ++itType) {
					undeclaredTypes.insert(*itType);
				}

				for (StructTypeVector::iterator itType = types.begin(); itType != types.end();) {

					const llvm::StructType * type = *itType;
					const std::string name = (*itType)->getName().str();

					if (
							undeclaredTypes.find(type) == undeclaredTypes.end() || //already declared
							containsType(type, undeclaredTypes))
					{
						++itType;
					} else {

						//write declaration
						std::string structStr = getStructTypeDeclaration(name, llvm::cast<const llvm::StructType>(type));
						put( structStr );

						undeclaredTypes.erase(type);
						itType = types.begin();
					}
				}
			}
		}
	}

	 std::string GenericCSerializer::getStructTypeDeclaration(const std::string & structName, const llvm::StructType * structType)
	{
	   std::string res =  "struct " + structName + "\n";
	   res +=  "{\n";

	   for(uint i = 0; i < structType->getNumElements(); ++i)
	   {
		   const llvm::Type * elementType = structType->getElementType(i);

		   std::string memberName = "x" + str<int>(i);
		   std::string memberStr = buildDeclaration(memberName, elementType);

		   res += INDENTATION_STRING + memberStr + ";\n";
	   }

	   res += "};\n";
	   return res;
	}


}
