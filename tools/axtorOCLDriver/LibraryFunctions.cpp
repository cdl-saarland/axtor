#include <CL/cl.h>

#include <sys/types.h>
#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <vector>

#ifdef ENABLE_PACKETIZER
#include "KernelPacketizer.h"
#endif

// OpenCL 1.0 API-Functions
#define CL_CREATE_KERNEL_NAME "clCreateKernel"
typedef cl_kernel (*clCreateKernel_Function)
			(cl_program      /* program */,
               const char *    /* kernel_name */,
               cl_int *        /* errcode_ret */);

#define CL_RELEASE_PROGRAM_NAME "clReleaseProgram"
typedef cl_int (*clReleaseProgram_Function)(cl_program /* program */);

#define CL_CREATE_PROGRAM_WITH_SOURCE_NAME "clCreateProgramWithSource"
typedef cl_program (*clCreateProgramWithSource_Function)
		(cl_context, cl_uint, const char **, const size_t *, cl_int *);

#define CL_BUILD_PROGRAM_NAME "clBuildProgram"
typedef cl_int (*clBuildProgram_Function)
		(cl_program, cl_uint, const cl_device_id *, const char *, void (*)(cl_program, void *), void *);


#define CL_RETAIN_PROGRAM_NAME "clRetainProgram"
typedef cl_int (*clRetainProgram_Function)
		(cl_program);

#define CL_GET_PROGRAM_INFO_NAME "clGetProgramInfo"
typedef cl_int (*clGetProgramInfo_Function)
		(cl_program         /* program */,
                 cl_program_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */);

#define CL_GET_PROGRAM_BUILD_INFO_NAME "clGetProgramBuildInfo"
typedef cl_int (*clGetProgramBuildInfo_Function)
		(cl_program            /* program */,
                      cl_device_id          /* device */,
                      cl_program_build_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */);

#define CL_CREATE_PROGRAM_WITH_BINARY_NAME "clCreateProgramWithBinary"
typedef cl_program (*clCreateProgramWithBinary_Function)
		(cl_context                     /* context */,
                          cl_uint                        /* num_devices */,
                          const cl_device_id *           /* device_list */,
                          const size_t *                 /* lengths */,
                          const unsigned char **         /* binaries */,
                          cl_int *                       /* binary_status */,
                          cl_int *                       /* errcode_ret */);












// Temporary files
const std::string inputOCLFile = "/tmp/OCL_in.cl";
const  std::string tmpBCFile = "/tmp/OCL_tmp.bc";
const  std::string tmpOCLFile = "/tmp/OCL_gen.cl";


// Wrapper state
struct ProgramDesc
{
	cl_context context;
	std::string sourceStr;
	cl_program handle;


	ProgramDesc(cl_context _context, std::string _sourceStr) :
		context(_context), sourceStr(_sourceStr), handle(0) {}

	ProgramDesc(cl_context _context, cl_program _handle) :
		context(_context), sourceStr(""), handle(_handle) {}

	bool isValid() const { return handle != 0; }
	bool isFromBinary() const { return sourceStr.empty(); }
};

typedef std::vector<ProgramDesc*> ProgramDescVec;


// Library state
static ProgramDescVec programs;

static void writeFile(const char * filePath, const std::string & data)
{
	std::ofstream out(filePath);
	out << data;
	out.flush();
}

static char* readFile(const char* filePath, size_t & size) {
  std::ifstream fileStream(filePath);
  if(fileStream.is_open()) {
    fileStream.seekg(0, std::ios::end);
    int fileSize = fileStream.tellg();
    char* source = new char [fileSize + 1];
    fileStream.seekg(0, std::ios::beg);
    fileStream.read(source, fileSize);
    fileStream.close();
    source[fileSize] = '\0';
    size = fileSize + 1;
    return source;
  }
  else {
    std::stringstream ss;
    ss << "Cannot open: " << filePath;
    return 0;
  }
}

cl_int dumpError(cl_int errcode)
{
	if (errcode == CL_SUCCESS)
		return errcode;

	std::string name;

	switch (errcode)
	{
#define NAMED_ERROR(CODE) \
	case CODE: name=#CODE; break;


		NAMED_ERROR(CL_INVALID_CONTEXT)

		NAMED_ERROR(CL_INVALID_COMMAND_QUEUE)

		NAMED_ERROR(CL_BUILD_ERROR)
		NAMED_ERROR(CL_BUILD_PROGRAM_FAILURE)

		NAMED_ERROR(CL_INVALID_ARG_INDEX)
		NAMED_ERROR(CL_INVALID_BINARY)
		NAMED_ERROR(CL_INVALID_BUILD_OPTIONS)
		NAMED_ERROR(CL_INVALID_PROGRAM)
		NAMED_ERROR(CL_INVALID_PROGRAM_EXECUTABLE)

		NAMED_ERROR(CL_INVALID_KERNEL)
		NAMED_ERROR(CL_INVALID_KERNEL_DEFINITION)
		NAMED_ERROR(CL_INVALID_KERNEL_NAME)


#undef NAMED_ERROR
	default:
		name = "<unnamed>"; break;
	};

	std::cerr << "Failed with " << name << "!!!\n";
	return errcode;
}

enum PipelineResult
{
	CLANG_FAILURE,
	OPT_FAILURE,
	PACKETIZER_FAILURE,
	AXTOR_FAILURE,
	PIPELINE_OK
};

std::string getEnvString(const char * name, const char * defValue="")
{
	char * val = getenv(name);
	return std::string(val ?: defValue);
}

PipelineResult runPipeline(const char * options)
{
// tools
	std::string sedCmdOne = "sed \'s/__inline/inline/\' -i " + inputOCLFile;
	std::string sedCmdTwo = "sed \'s/inline/static inline/\' -i " + inputOCLFile;

	std::string llvmPath = getEnvString("LLVM_PATH");
	std::string axtorBuildOptions = getEnvString("AXTOR_CLANG_OPTIONS", "-O0");
	std::string axtorOptOptions = getEnvString("AXTOR_OPT_OPTIONS");

	std::stringstream clangCmdOut;

	std::string oclangBin = llvmPath + "oclang.sh";
	std::string optBin = llvmPath + "opt";
	std::string axtorBin = llvmPath + "axtor";

	clangCmdOut     << "LD_PRELOAD=\"\" "
			<< oclangBin << " "

			// options
			<< (options ?: "") << " "   // passed through the OpenCL - API
			<< axtorBuildOptions << " " // specified by  AXTOR_CLANG_OPTIONS

			//target
			<< inputOCLFile << " -o " << tmpBCFile;
	std::string clangCmd = clangCmdOut.str();

	std::stringstream optCmdOut;
	optCmdOut       << "LD_PRELOAD=\"\" "
			<< optBin << " "
			<< axtorOptOptions         // specified by AXTOR_OPT_OPTIONS
			<< " " << tmpBCFile << " -o " << tmpBCFile;
	std::string optCmd = optCmdOut.str();


	std::string axtorCmd = "LD_PRELOAD=\"\" "
			+ axtorBin + " "
			+ tmpBCFile
			+ " -m OCL "
			+ "-o " + tmpOCLFile;

	std::cerr << "CLANG:" << clangCmd << "\n\n\nAXTOR:" << axtorCmd << "\n";

	// inline -> static inline
	assert(! system(sedCmdOne.c_str()));
	assert(! system(sedCmdTwo.c_str()));

	// invoke Clang ( OpenCL -> BC )
	if (system(clangCmd.c_str())) {
		std::cerr << "&&&&& FRONTEND_FAILURE!";
		return CLANG_FAILURE;
	}

	if (system(optCmd.c_str())) {
		std::cerr << "&&&&& OPT_FAILURE!";
		return OPT_FAILURE;
	}

	// run the packetizer
#ifdef ENABLE_PACKETIZER
	packetizeAllKernelsInModule(tmpBCFile.c_str(), 4);
#endif



	// invoke Axtor ( BC -> OpenCL)
	if (system(axtorCmd.c_str())) {
		std::cerr << "&&&&& AXTOR_FAILURE!";
		return AXTOR_FAILURE;
	}

	return PIPELINE_OK;
}






/*
 * Implementations
 */
extern "C" cl_kernel clCreateKernel(
		cl_program      program,
        const char *    kernel_name,
        cl_int *        errcode_ret)
{
	ProgramDesc * desc = reinterpret_cast<ProgramDesc*>(program);

	clCreateKernel_Function originalCreateKernel;
	*(void **)(&originalCreateKernel) = dlsym(RTLD_NEXT, CL_CREATE_KERNEL_NAME);

	cl_int err;
	cl_kernel kernel = originalCreateKernel(desc->handle, kernel_name, &err);
	dumpError(err);
	if (errcode_ret)
		*errcode_ret = err;

	return kernel;
}

extern "C" cl_int clReleaseProgram(
		cl_program program)
{
	ProgramDesc * desc = reinterpret_cast<ProgramDesc*>(program);

	clReleaseProgram_Function originalReleaseProgram;
	*(void **)(&originalReleaseProgram) = dlsym(RTLD_NEXT, CL_RELEASE_PROGRAM_NAME);

	return dumpError(originalReleaseProgram(desc->handle));
}

extern "C" cl_int clRetainProgram(cl_program program)
{
	ProgramDesc * desc = reinterpret_cast<ProgramDesc*>(program);

	clRetainProgram_Function originalRetainProgram;
	*(void **)(&originalRetainProgram) = dlsym(RTLD_NEXT, CL_RETAIN_PROGRAM_NAME);
	return dumpError(originalRetainProgram(desc->handle));
}

extern "C" cl_int clGetProgramInfo
		(cl_program         program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret)
{
	ProgramDesc * desc = reinterpret_cast<ProgramDesc*>(program);

	clGetProgramInfo_Function originalGetProgramInfo;
	*(void **)(&originalGetProgramInfo) = dlsym(RTLD_NEXT, CL_GET_PROGRAM_INFO_NAME);
	return originalGetProgramInfo(desc->handle, param_name, param_value_size, param_value, param_value_size_ret);
}

extern "C" cl_int clGetProgramBuildInfo
		(cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret)
{
	ProgramDesc * desc = reinterpret_cast<ProgramDesc*>(program);

	clGetProgramBuildInfo_Function originalGetProgramBuildInfo;
	*(void **)(&originalGetProgramBuildInfo) = dlsym(RTLD_NEXT, CL_GET_PROGRAM_BUILD_INFO_NAME);
	return dumpError(originalGetProgramBuildInfo(desc->handle, device, param_name, param_value_size, param_value, param_value_size_ret));
}





// main functions

extern "C" cl_program clCreateProgramWithBinary
		(cl_context                     context,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const size_t *                 lengths,
                          const unsigned char **         binaries,
                          cl_int *                       binary_status,
                          cl_int *                       errcode_ret)
{
	clCreateProgramWithBinary_Function originalCreateProgramWithBinary;
	*(void **)(&originalCreateProgramWithBinary) = dlsym(RTLD_NEXT, CL_CREATE_PROGRAM_WITH_BINARY_NAME);
	cl_program realHandle = originalCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);

	ProgramDesc * desc = new ProgramDesc(context, realHandle);
	programs.push_back(desc);
	return reinterpret_cast<cl_program>(desc);
}

extern "C" cl_int clBuildProgram(
		cl_program           program,
		cl_uint              num_devices,
		const cl_device_id * device_list,
		const char *         options,
		void (*pfn_notify)(cl_program /* program */, void * /* user_data */),
		void *               user_data)
{
	// Get the original functions
	clCreateProgramWithSource_Function originalCreateProgramWithSource;
	*(void **)(&originalCreateProgramWithSource) = dlsym(RTLD_NEXT, CL_CREATE_PROGRAM_WITH_SOURCE_NAME);

	clBuildProgram_Function originalBuildProgram;
	*(void **)(&originalBuildProgram) = dlsym(RTLD_NEXT, CL_BUILD_PROGRAM_NAME);

	ProgramDesc * desc = reinterpret_cast<ProgramDesc*>(program);

	if (! desc->isFromBinary()) {

		writeFile(inputOCLFile.c_str(), desc->sourceStr);

		// do the loop
		switch (runPipeline(options))
		{
		case CLANG_FAILURE:
			std::cerr << "&&&&& CLANG_FAILURE!\n"; return CL_BUILD_ERROR;
		case AXTOR_FAILURE:
			std::cerr << "&&&&& AXTOR_FAILURE!\n"; return CL_BUILD_ERROR;
		default:
			std::cerr << "&&&&& PIPELINE_OK!\n"; break;
		}

		// read from disk
		size_t dataLen;
		char * axtorBuffer = readFile(tmpOCLFile.c_str(), dataLen);
		// std::cerr << axtorBuffer << "\n";

		// call create program with source late
		cl_int errcode;
		desc->handle = originalCreateProgramWithSource(desc->context, 1, (const char**) &axtorBuffer, &dataLen, &errcode);
	}

	// do the actual build
	cl_int errCode = originalBuildProgram(desc->handle, num_devices, device_list, options, pfn_notify, user_data);
	return dumpError(errCode);
}


extern "C" cl_program clCreateProgramWithSource(
		cl_context context,
		cl_uint count,
		const char **strings,
		const size_t *lengths,
		cl_int *errcode_ret)
{

	  // Get the original function.

	// store temporarily
	std::stringstream buffer;
	for (uint i = 0; i < count; ++i)
	{
		buffer << strings[i]  << "\n";
	}

	ProgramDesc * desc = new ProgramDesc(context, buffer.str());
	programs.push_back(desc);
	cl_program fakeHandle = reinterpret_cast<cl_program>(desc);

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

	return fakeHandle;
}
