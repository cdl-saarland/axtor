/*
 * GenericCSerializer.h
 *
 *  Created on: Apr 24, 2012
 *      Author: simoll
 */

#ifndef GENERICCSERIALIZER_H_
#define GENERICCSERIALIZER_H_

#include <vector>
#include <axtor/writer/SyntaxWriter.h>
#include <axtor/util/WrappedOperation.h>

#include "GenericCWriter.h"

typedef std::vector<llvm::StructType*> StructTypeVector;
typedef std::vector<int> IntVector;

/*
 * This class implements basic serialization capabilities in C-syntax
 * Other syntax backends may extend and customize it to implement
 * target specific language features
 */

namespace axtor {

	class GenericCSerializer
	{
	public:
	   // GENERIC:
		void spillStructTypeDeclarations(ModuleInfo & modInfo, GenericCWriter * stream);
		virtual std::string getStructTypeDeclaration(const std::string & structName, const llvm::StructType * structType);


	   // custom function for serialization of declarations
		virtual std::string buildDeclaration(std::string root, const llvm::Type * type) = 0;

		virtual std::string getLiteral(llvm::Constant * val) = 0;
	};
}



#endif /* GENERICCSERIALIZER_H_ */
