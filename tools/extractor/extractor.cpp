/*  Axtor - AST-Extractor for LLVM
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * extractor.cpp
 *
 *  Created on: 07.02.2010
 *      Author: Simon Moll
 */

#include <llvm/IR/LegacyPassManagers.h>

#include <iostream>
#include <stdio.h>
#include <fstream>

#include <axtor/config.h>

#include <axtor/metainfo/ModuleInfo.h>
#include <axtor/backend/AxtorBackend.h>

#ifdef ENABLE_GLSL
#include <axtor/GLSL/GLSLModuleInfo.h>
#include <axtor/GLSL/GLSLBackend.h>
#endif

#ifdef ENABLE_OPENCL
#include <axtor_ocl/OCLModuleInfo.h>
#include <axtor_ocl/OCLBackend.h>
#endif

#include <axtor/util/llvmTools.h>
#include <axtor/console/CompilerLog.h>
#include <axtor/Axtor.h>

#include "ArgumentReader.h"

static void dumpHelp();

#ifdef ENABLE_GLSL
static int run_GLSL(ArgumentReader args)
{
	std::ostream * vertOut = NULL;
	std::ostream * fragOut = NULL;

	//## input file
	if (args.getNumArgs() > 0)
	{
		std::string inputFile = args.get(0);
		llvm::Module * mod = axtor::createModuleFromFile(inputFile);

		if (!mod) {
			std::string name = args.get(0);
			std::cerr << "is not a bitcode file " << name << std::endl;
			exit(-1);
		}

		//## fragment shader file
		std::vector<std::string> params;
		if (args.readOption("-f", 1, params))
		{
			std::string outFileName = *( params.begin() );

			fragOut = new std::ofstream( outFileName.c_str(), std::ios::out);
		}

		//## vertex shader file
		if (args.readOption("-v", 1, params))
		{
			std::string outFileName = *( params.begin() );

			vertOut = new std::ofstream( outFileName.c_str(), std::ios::out);
		}

		//## generic file names
		if (args.readOption("-g", 1, params))
		{
			if (fragOut || vertOut)
			{
				std::cerr << "option -g must not be used in conjunction with -f or -v" << std::endl;
			}

			std::string outFileName = *( params.begin() );

			fragOut = new std::ofstream( (outFileName + ".frag").c_str(), std::ios::out);
			vertOut = new std::ofstream( (outFileName + ".vert").c_str(), std::ios::out);
		}

		//## argument bind file
		/* if (args.readOption("-b", 1, params))
		{
			std::string bindFileName = *( params.begin() );
			bindIn = new std::ifstream( bindFileName.c_str(), std::ios::in);
		} else {
			std::cerr << "missing mandatory option -b [bindfile]" << std::endl;
			exit(-1);
		} */

		//## dump to std out
		if (! vertOut) {
			vertOut = &std::cout;
		} else if (vertOut->bad()) {
			std::cerr << "invalid vertex shader output file" << std::endl;
			exit(-1);
		}

		if (! fragOut) {
			fragOut = &std::cout;
		} else if (fragOut->bad()) {
			std::cerr << "invalid fragment shader output file" << std::endl;
			exit(-1);
		}

		//## execute
#ifdef EVAL_DECOMPILE_TIME
		std::stringstream fragStream;
		std::stringstream vertStream;

		axtor::GLSLModuleInfo * modInfo = axtor::GLSLModuleInfo::createTestInfo(mod, vertStream, fragStream);
#else
		axtor::GLSLModuleInfo * modInfo = axtor::GLSLModuleInfo::createTestInfo(mod, *vertOut, *fragOut);
#endif
		axtor::GLSLBackend backend;
		axtor::translateModule(backend, *modInfo);
		delete modInfo;

		//release streams
		if (vertOut != &std::cout)
			delete vertOut;
		if (fragOut != &std::cout)
			delete fragOut;
		/* if (bindIn)
			delete bindIn; */

		return 0;
	} else {
		std::cerr << "no input file specified!" << std::endl;
		dumpHelp();
		return -1;
	}
}

static void dump_GLSL()
{
	std::cerr << "Options for the GLSL Backend (-m GLSL)"
			  << "\n"
			  << "<infile> [-v <vertShaderFile>] [-f <fragShaderFile>] [-g <fileName>]\n"
			  << "\n"
			  << "results will be written to std out if no output file is specified.\n"
			  << "-g <FILE>  same as \"-v <FILE>.vert -f <FILE>.frag\"\n"
			  << "           must not be used with -f or -v\n"
			  /* << "-b <FILE>  a bind file specifies the in-/output behaviour of global variables in the shader\n"
			  << "           bind file. format (per line):\n"
			  << "           [I|O|U]NAME\n"
			  << "           classifiers I : input buffer, O : output buffer, U : uniform\n" */
			  << std::endl;
}
#endif



#ifdef ENABLE_OPENCL
static int run_OpenCL(ArgumentReader args)
{
	std::ostream * outStream = nullptr;

	if (args.getNumArgs() > 0) {
		std::string inputFile = args.get(0);
		llvm::Module * mod = axtor::createModuleFromFile(inputFile);

		if (!mod) {
			axtor::Log::fail("no input module specified!");
		}


		axtor::StringVector params;
		if (args.readOption("-o", 1, params)) {
			std::string outFile = params.back();
			outStream = new std::ofstream(outFile.c_str(), std::ios::out);
		}

		axtor::OCLBackend backend;

		FunctionVector kernelVec;
		if (auto * kernelFunc= mod->getFunction("compute")) {
			kernelVec.push_back(kernelFunc);
		}

		if (outStream) {

			axtor::OCLModuleInfo modInfo(mod, kernelVec, *outStream);
			axtor::translateModule(backend, modInfo);
			delete outStream;

		} else {
			axtor::OCLModuleInfo modInfo(mod, kernelVec, std::cout);
			axtor::translateModule(backend, modInfo);
		}

		return 0;
	}

	std::cerr << "no input file specified!" << std::endl;
	dumpHelp();
	return -1;
}

static void dump_OpenCL()
{
	std::cerr << "Options for the OpenCL Backend (-m OCL)"
			  << "\n"
			  << "<infile> -o <outputFile>"
			  << "\n"
			  << "-o <FILE>  output file"
			  << std::endl;
}
#endif

static void dumpHelp()
{
#ifdef ENABLE_GLSL
	dump_GLSL();
#endif
#ifdef ENABLE_OPENCL
	dump_OpenCL();
#endif
}



int main(int argc, char ** argv)
{
	axtor::initialize(true);

	ArgumentReader args(argc, argv);
	axtor::StringVector backendVector;

	if (args.readOption("-m", 1, backendVector)) {
		std::string backendStr = backendVector.back();

		if (false) {} //just a dummy
#ifdef ENABLE_GLSL
		else if (backendStr == "GLSL")
			return run_GLSL(args);
#endif
#ifdef ENABLE_OPENCL
		else if (backendStr == "OCL")
			return run_OpenCL(args);
#endif
	}

	std::cerr << "no backend specified\n";
	dumpHelp();
	return -1;
}
