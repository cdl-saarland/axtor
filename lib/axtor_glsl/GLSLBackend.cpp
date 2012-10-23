/*
 * GLSLBackend.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor_glsl/GLSLBackend.h>

#include <axtor/pass/CGIPass.h>

namespace axtor {

PlatformInfo * GLSLBackend::platform = NULL;

void GLSLBackend::init()
{
	if (platform)
		return;

	StringSet nativeTypes;
	IntrinsicsMap intrinsics;

#ifndef DEBUG_INTRINSICS

#define BUILTIN(TYPE,INTRINSIC,ACTUAL)\
		intrinsics[#INTRINSIC] = new IntrinsicGlobalDesc(#ACTUAL);

#define FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC)\
		intrinsics[#INTRINSIC] = new IntrinsicFuncDesc(#ACTUAL,ARGC);

#define INFIX(OP,RETURN,INTRINSIC,ARGS)\
		intrinsics[#INTRINSIC] = new IntrinsicInfixDesc(#OP);

#define INTERP(INTRINSIC,RETURN,ARGS)\
		intrinsics[#INTRINSIC] = new InterpolantDescriptor();

#define NATIVE(NAME)\
		nativeTypes.insert(#NAME);

#define PASS(RET,INTRINSIC,ARG) \
		intrinsics[#INTRINSIC] = new IntrinsicPassDesc();

#define ACCESSOR(NAME,TYPE,ARGUMENT) \
		intrinsics[#NAME] = new IntrinsicAccessorDesc();

#define GETTER(NAME,TYPE,ARGUMENT) \
		intrinsics[#NAME] = new IntrinsicGetterDesc();

#define SETTER(NAME,ARGUMENT) \
		intrinsics[#NAME] = new IntrinsicSetterDesc();

#define REPLACE_FUNC(NAME,RETURN,REPLACEMENT) \
		intrinsics[#NAME] = new IntrinsicCallReplacementDesc(#REPLACEMENT);

#else

#define BUILTIN(TYPE,INTRINSIC,ACTUAL)\
		std::cerr << "GLOBAL : " #INTRINSIC " -> " #ACTUAL "\n";\
		intrinsics[#INTRINSIC] = new IntrinsicGlobalDesc(#ACTUAL);

#define FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC)\
		std::cerr << "FUNC : " #INTRINSIC " -> " #ACTUAL "\n"; \
		intrinsics[#INTRINSIC] = new IntrinsicFuncDesc(#ACTUAL,ARGC);

#define INFIX(OP,RETURN,INTRINSIC,ARGS)\
		std::cerr << "INFIX : " #INTRINSIC " -> " #OP "\n"; \
		intrinsics[#INTRINSIC] = new IntrinsicInfixDesc(#OP);

#define INTERP(INTRINSIC,RETURN,ARGS)\
		std::cerr << "NOP : " #INTRINSIC "\n"; \
		intrinsics[#INTRINSIC] = new InterpolantDescriptor();

#define NATIVE(NAME)\
		std::cerr << "NATIVE : " #NAME "\n";\
		nativeTypes.insert(#NAME);

#define PASS(RET,INTRINSIC,ARG) \
		std::cerr << "PASS : " #INTRINSIC "\n"; \
		intrinsics[#INTRINSIC] = new IntrinsicPassDesc();

#define GETTER(NAME,TYPE,ARGUMENT) \
		std::cerr << "GETTER : " #NAME "\n"; \
		intrinsics[#NAME] = new IntrinsicGetterDesc();

#define SETTER(NAME,ARGUMENT) \
		std::cerr << "SETTER : " #NAME "\n"; \
		intrinsics[#NAME] = new IntrinsicSetterDesc();

#define REPLACE_FUNC(NAME,RETURN,REPLACEMENT) \
		std::cerr << "CALLREPLACEMENT : " #NAME "\n"; \
		intrinsics[#NAME] = new IntrinsicCallReplacementDesc(#REPLACEMENT);

#endif

#define CONST_FUNC FUNC
#define CONST_COMPLEX COMPLEX

#include <axtor_glsl/GLSL_Intrinsics.def>

BUILTIN(gl_MaxDrawBuffers,uint,gl_MaxDrawBuffers)
BUILTIN(gl_MaxClipDistances,uint,gl_MaxClipDistances)

#undef FUNC
#undef INTERP
#undef BUILTIN
#undef INFIX
#undef NATIVE

	platform = new PlatformInfo(nativeTypes, intrinsics);
}

GLSLBackend::GLSLBackend()
{
	init();
}

bool GLSLBackend::hasValidType(ModuleInfo * modInfo)
{
	return true;//dynamic_cast<OCLModuleInfo*>(modInfo);
}

const std::string & GLSLBackend::getName()
{
	return getNameString();
}

const std::string & GLSLBackend::getLLVMDataLayout()
{
	return getLLVMDataLayoutString();
}


const std::string & GLSLBackend::getNameString()
{
	static std::string text = "GLSL Backend";;
	return text;
}

const std::string & GLSLBackend::getLLVMDataLayoutString()
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


SyntaxWriter * GLSLBackend::createModuleWriter(ModuleInfo & _modInfo, const IdentifierScope & globals)
{
	return new GLSLWriter(_modInfo, globals, *platform);
}

GLSLWriter * GLSLBackend::createWriterIfDefined(GLSLWriter & writer, std::ostream * stream)
{
	if (stream) {
		return new GLSLRedirectedWriter(writer, *stream);
	} else {
		return new GLSLDummyWriter(writer);
	}
}

GLSLWriter * GLSLBackend::createFittingMultiWriter(GLSLWriter & writer, std::ostream * vertStream, std::ostream * fragStream)
{
	assert(dynamic_cast<GLSLWriter*>(&writer) && "not a GLSL writer object");

	GLSLWriter * vertWriter = new GLSLRedirectedWriter(writer, *vertStream);
	GLSLWriter * fragWriter = new GLSLRedirectedWriter(writer, *fragStream);
	return new GLSLMultiStageWriter(writer, vertWriter, fragWriter);
}

SyntaxWriter * GLSLBackend::createFunctionWriter(SyntaxWriter * modWriter, llvm::Function * func)
{
	assert(func && "was NULL");
	assert(modWriter && "invalid module writer");
	assert(dynamic_cast<GLSLWriter*>(modWriter) && "not a GLSL writer object");
	GLSLWriter & writer = *static_cast<GLSLWriter*>(modWriter);

	GLSLModuleInfo & modInfo = writer.getModuleInfo();

	if (func == modInfo.getFragFunc()) {
#ifdef DEBUG
		std::cerr << "created fragment writer for func : " << func->getName().str() << "\n";
#endif
		return new GLSLFragWriter(writer, *modInfo.getFragStream());
	} else if (func == modInfo.getVertFunc()){
		return new GLSLRedirectedWriter(writer, *modInfo.getVertStream());
	} else {
		return createFittingMultiWriter(writer, modInfo.getVertStream(), modInfo.getFragStream());
	}
}

SyntaxWriter * GLSLBackend::createBlockWriter(SyntaxWriter * writer)
{
	assert(dynamic_cast<GLSLWriter*>(writer) && "not a GLSL writer object");
	return new GLSLBlockWriter(*(static_cast<GLSLWriter*>(writer)));
}

bool GLSLBackend::implementsFunction(llvm::Function * func)
{
	return platform->implements(func);
}

//interface for specifying passes specific to this backend
void GLSLBackend::getAnalysisUsage(llvm::AnalysisUsage & usage) const {}

void GLSLBackend::addRequiredPasses(llvm::PassManager & pm) const
{
}

}

