#include <axtor/util/llvmDebug.h>

std::string axtor::toString(const BlockSet & blocks)
{
	std::stringstream out;
	out << "BlockSet {\n";
	for(BlockSet::const_iterator itBlock = blocks.begin(); itBlock != blocks.end(); ++itBlock)
	{
		llvm::BasicBlock * block = *itBlock;
		out << "\t" << (block ? block->getName().str() : "null") << ",\n";
	}
	out << "}\n";
	return out.str();
}

void axtor::dumpBlockSet(const BlockSet & blocks)
{
	std::cerr << toString(blocks) << "\n";
}

void axtor::dumpTypeSet(const TypeSet & types)
{
	std::cerr << "TypeSet {\n";
	for(TypeSet::const_iterator itType = types.begin(); itType != types.end(); ++itType)
	{
		const llvm::Type * type = *itType;
		std::cerr << "\t"; llvm::errs() << *type;
	}
	std::cerr << "}\n";
}

void axtor::dumpBlockVector(const BlockVector & blocks)
{
	std::cerr << "BlockVector [\n";
	for(BlockVector::const_iterator itBlock = blocks.begin(); itBlock != blocks.end(); ++itBlock)
	{
		llvm::BasicBlock * block = *itBlock;
		std::cerr << "\t" << (block ? block->getName().str() : "null") << ",\n";
	}
	std::cerr << "]\n";
}



void axtor::verifyModule(llvm::Module & mod)
{
	if (llvm::verifyModule(mod, &llvm::errs())) {
        llvm::errs() << "\n\n\n##### BROKEN MODULE #####\n";
	    llvm::errs() << mod;
	    llvm::errs() << "##### END OF DUMP ##### \n\n\n";
	    abort();
       }
}

void axtor::dumpUses(llvm::Value * val)
{
	std::cerr << "dumping value uses for "; llvm::errs() << *val;
	std::cerr << "----- begin of user list ----\n";
	for(llvm::Value::use_iterator use = val->use_begin(); use != val->use_end(); ++use)
	{
		use->getUser()->print(llvm::errs());
	}
	std::cerr << "----- end of user list -----\n";
}

