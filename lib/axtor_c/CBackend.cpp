/*
 * CBackend.cpp
 *
 *  Created on: 22.05.2015
 *      Author: Simon
 */

#include <axtor_c/CBackend.h>

namespace axtor {

PlatformInfo * CBackend::platform = NULL;

void CBackend::init()
{
	if (platform)
		return;

	StringSet nativeTypes;
	IntrinsicsMap intrinsics;

#ifndef DEBUG_INTRINSICS

#define ASSIGN(NAME) \
		intrinsics[NAME] = new IntrinsicAssignmentDesc();

#define FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC)\
		intrinsics[#INTRINSIC] = new IntrinsicFuncDesc(#ACTUAL,ARGC);

#define COMPLEX(INTRINSIC,RETURN,FORMAT,ARGS)\
		intrinsics[#INTRINSIC] = new IntrinsicComplexDesc(#FORMAT);

#define INFIX(NAME,OP) \
		intrinsics[NAME] = new IntrinsicInfixDesc(OP);

#define UNARY(NAME,OP) \
		intrinsics[NAME] = new IntrinsicUnaryDesc(OP);

#define NATIVE(NAME) \
		nativeTypes.insert(#NAME);

#define REPLACE_FUNC(NAME,RETURN,REPLACEMENT) \
		intrinsics[#NAME] = new IntrinsicCallReplacementDesc(#REPLACEMENT);

#define MUTE(NAME) \
		intrinsics[#NAME] = new IntrinsicMuteDesc(#NAME);

#define BUILTIN



#else
#define ASSIGN(NAME) \
		std::cerr << "ASSIGN : " NAME << '\n'; \
		intrinsics[NAME] = new IntrinsicAssignmentDesc();

#define FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC)\
		std::cerr << "FUNC : " #INTRINSIC << " -> " << #ACTUAL << '\n'; \
		intrinsics[#INTRINSIC] = new IntrinsicFuncDesc(#ACTUAL,ARGC);

#define COMPLEX(INTRINSIC,RETURN,FORMAT,ARGS)\
		std::cerr << "COMPLEX : " #INTRINSIC << " -> " << FORMATL << '\n'; \
		intrinsics[#INTRINSIC] = new IntrinsicComplexDesc(FORMAT);

#define INFIX(NAME,OP) \
		std::cerr << "INFIX : " NAME << " -> " << OP << '\n'; \
		intrinsics[NAME] = new IntrinsicInfixDesc(OP);

#define UNARY(NAME,OP) \
		std::cerr << "UNARY : " NAME << " -> " << OP << '\n';\
		intrinsics[NAME] = new IntrinsicUnaryDesc(OP);

#define NATIVE(NAME) \
		std::cerr << "NATIVE : " #NAME "\n";\
		nativeTypes.insert(#NAME);

#define REPLACE_FUNC(NAME,RETURN,REPLACEMENT) \
		std::cerr << "CALLREPLACEMENT : " #NAME "\n"; \
		intrinsics[#NAME] = new IntrinsicCallReplacementDesc(#REPLACEMENT);

#define MUTE(NAME) \
		std::cerr << "MUTE : " #NAME "\n"; \
		intrinsics[#NAME] = new IntrinsicMuteDesc(#NAME);

#define BUILTIN

#endif

#define CONST_FUNC FUNC
#define CONST_COMPLEX COMPLEX


#if 1
#define __AXTOR_INTERNAL
#include <axtor_c/C_Intrinsics.def>
#undef __AXTOR_INTERNAL
#endif

#undef ASSIGN
#undef FUNC
#undef COMPLEX
#undef INFIX
#undef UNARY
#undef NATIVE
#undef MUTE
#undef BUILTIN

	platform = new PlatformInfo(nativeTypes, intrinsics);
}


bool CBackend::requiresDesignator(llvm::Instruction * inst)
{
	return true;
}


CBackend::CBackend()
{
	init();
}

bool CBackend::hasValidType(ModuleInfo * modInfo)
{
	return true;//dynamic_cast<CModuleInfo*>(modInfo);
}

const std::string & CBackend::getName()
{
	return getNameString();
}

const std::string & CBackend::getLLVMDataLayout()
{
	return getLLVMDataLayoutString();
}

const std::string & CBackend::getNameString()
{
	static std::string name = "C Backend";
	return name;
}

const std::string & CBackend::getLLVMDataLayoutString()
{
	static std::string text =
		"E"
		"-p:64:64:64" /*- 64-bit pointers with 64-bit alignment*/
		"-i1:16:16"     /* i1 is 8-bit (byte) aligned */
		"-i8:16:16"     /* i8 is 8-bit (byte) aligned */
		"-i16:16:16"  /* i16 is 16-bit aligned */
		"-i32:32:32"  /* i32 is 32-bit aligned */
		"-i64:32:64"  /* i64 has ABI alignment of 32-bits but preferred alignment of 64-bits */
		"-f32:32:32"  /* float is 32-bit aligned */
		"-f64:32:64"  /* double is 64-bit aligned */
		"-f80:32:32"  /* some weird float format i found in a "-m32" module */
		"-v64:64:64"  /* (vec2) 64-bit vector is 64-bit aligned */
		"-v96:128:128" /* (vec3) 96-bit vector is 128-bit aligned */
		"-v128:128:128" /* (vec4) 128-bit vector is 128-bit aligned */
		"-a0:0:1"       /* aggregates are 8-bit aligned */
		"-s0:64:64" /* stack objects are 64-bit aligned */
		"-n16:32";   /* integer sizes*/

	return text;
}


//factory methods
SyntaxWriter * CBackend::createModuleWriter(ModuleInfo & _modInfo, const IdentifierScope & globals)
{
	return new CWriter(_modInfo, *platform);
}

SyntaxWriter * CBackend::createFunctionWriter(SyntaxWriter * modWriter, llvm::Function * func)
{
	assert(modWriter && "was NULL");
	// assert(dynamic_cast<CWriter*>(modWriter) && "not a C writer class");
	return new CPassThroughWriter(*static_cast<CWriter*>(modWriter));
}

SyntaxWriter * CBackend::createBlockWriter(SyntaxWriter * writer)
{
	return new CBlockWriter(*(static_cast<CWriter*>(writer)));
}

bool CBackend::implementsFunction(llvm::Function * func)
{
	return platform->implements(func);
}

//interface for specifying passes specific to this backend
void CBackend::getAnalysisUsage(llvm::AnalysisUsage & usage) const {}

void CBackend::addRequiredPasses(llvm::PassManager & pm) const
{
}

}

