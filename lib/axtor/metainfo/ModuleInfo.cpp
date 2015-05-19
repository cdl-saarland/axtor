/*
 * ModuleInfo.cpp
 *
 *  Created on: 27.06.2010
 *      Author: gnarf
 */

#include <axtor/metainfo/ModuleInfo.h>
#include <axtor/console/CompilerLog.h>

namespace axtor
{
	ModuleInfo::ModuleInfo(llvm::Module & _M)  :
		M(_M)
	{
		std::string errInfo;
#if 0
		if (M.MaterializeAll(&errInfo)) {
			Log::fail("llvm::MaterializeAll error: " + errInfo);
		}
#endif
	}

	const llvm::Module * ModuleInfo::getModule() const
	{
		return & M;
	}

	std::string ModuleInfo::getTypeName(const llvm::Type * type) const
	{
		for (StringToType::const_iterator it = stringToType.begin(); it != stringToType.end(); ++it)
		{
			if (it->second == type)
				return it->first;
		}

		if (llvm::isa<const llvm::StructType>(type)) {
			std::string structName = type->getStructName();

			if (! structName.empty())
				return structName;
		}

		return ""; //unnamed
	}

	llvm::Type * ModuleInfo::setTypeName(llvm::Type * type, const std::string & name)
	{
		StringToType::iterator itType = stringToType.find(name);

		if (itType != stringToType.end()) {
			return itType->second;
		}
		stringToType[name] = type;
		return 0;
	}

	llvm::Type *  ModuleInfo::lookUpType(const std::string & name) const
	{
		StringToType::const_iterator it = stringToType.find(name);
		if (it != stringToType.end())
			return it->second;

		return M.getTypeByName(name);
	}
}
