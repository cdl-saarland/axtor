#include "OpenCL_Intrinsics.h"

#define SPACE_GLOBAL   1
#define SPACE_CONSTANT 2
#define SPACE_LOCAL    3
#define SPACE_PRIVATE  4
#define SPACE_POINTER  5 //used for shader_buffer_load/store extensions

#define SPACE_NOPTR 100

#define PRIVATE __attribute__((address_space(SPACE_PRIVATE)))

//shared among work group
#define LOCAL __attribute__((address_space(SPACE_LOCAL)))

//unique on device
#define GLOBAL __attribute__((address_space(SPACE_GLOBAL)))

//constant memory space
#define CONSTANT  __attribute__((address_space(SPACE_CONSTANT)))

//constant memory space
#define DEVICE_POINTER  __attribute__((address_space(SPACE_POINTER)))

//used to denote that this is in fact not a pointer type
#define NOPTR __attribute__((address_space(SPACE_NOPTR)))


void llvm_memcpy_p2i8_p1i8_i32(char LOCAL* dest, char GLOBAL* src, int len, int align, bool isVolatile)
{
	for(int s = 0; s < len; ++s)
		*(dest++) = *(src++);
}

void llvm_memcpy_p1i8_p2i8_i32(char GLOBAL* dest,char LOCAL* src, int len, int align, bool isVolatile)
{
	for(int s = 0; s < len; ++s)
		*(dest++) = *(src++);
}

void llvm_memset(char GLOBAL* ptr, char p, int len, int align, bool isVolatile)
{
	for(int i = 0; i < len; ++i)
		ptr[i] = p;
}

