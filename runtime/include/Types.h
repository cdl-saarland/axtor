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
 * Types.h
 *
 *  Created on: 07.03.2010
 *      Author: Simon Moll
 */

/*
 * Generic Shader Language Platform Types
 */

#ifndef TYPES_HPP_
#define TYPES_HPP_

//#include "Reflection.h"

extern "C" {

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long ulong;

	//vectors
	typedef double double16 __attribute__((ext_vector_type(16)));
	typedef double double8 __attribute__((ext_vector_type(8)));
	typedef double double4 __attribute__((ext_vector_type(4)));
	typedef double double3 __attribute__((ext_vector_type(3)));
	typedef double double2 __attribute__((ext_vector_type(2)));

	// float
	typedef float float16 __attribute__((ext_vector_type(16)));
	typedef float float8 __attribute__((ext_vector_type(8)));
	typedef float float4 __attribute__((ext_vector_type(4)));
	typedef float float3 __attribute__((ext_vector_type(3)));
	typedef float float2 __attribute__((ext_vector_type(2)));

	// int 32
	typedef int int16 __attribute__((ext_vector_type(16)));
	typedef int int8 __attribute__((ext_vector_type(8)));
	typedef int int4 __attribute__((ext_vector_type(4)));
	typedef int int3 __attribute__((ext_vector_type(3)));
	typedef int int2 __attribute__((ext_vector_type(2)));

	typedef uint uint16 __attribute__((ext_vector_type(16)));
	typedef uint uint8 __attribute__((ext_vector_type(8)));
	typedef uint uint4 __attribute__((ext_vector_type(4)));
	typedef uint uint3 __attribute__((ext_vector_type(3)));
	typedef uint uint2 __attribute__((ext_vector_type(2)));


	// int 16
	typedef short short16 __attribute__((ext_vector_type(16)));
	typedef short short8 __attribute__((ext_vector_type(8)));
	typedef short short4 __attribute__((ext_vector_type(4)));
	typedef short short3 __attribute__((ext_vector_type(3)));
	typedef short short2 __attribute__((ext_vector_type(2)));

	typedef ushort ushort16 __attribute__((ext_vector_type(16)));
	typedef ushort ushort8 __attribute__((ext_vector_type(8)));
	typedef ushort ushort4 __attribute__((ext_vector_type(4)));
	typedef ushort ushort3 __attribute__((ext_vector_type(3)));
	typedef ushort ushort2 __attribute__((ext_vector_type(2)));


	// int 8
	typedef char char16 __attribute__((ext_vector_type(16)));
	typedef char char8 __attribute__((ext_vector_type(8)));
	typedef char char4 __attribute__((ext_vector_type(4)));
	typedef char char3 __attribute__((ext_vector_type(3)));
	typedef char char2 __attribute__((ext_vector_type(2)));

	typedef uchar uchar16 __attribute__((ext_vector_type(16)));
	typedef uchar uchar8 __attribute__((ext_vector_type(8)));
	typedef uchar uchar4 __attribute__((ext_vector_type(4)));
	typedef uchar uchar3 __attribute__((ext_vector_type(3)));
	typedef uchar uchar2 __attribute__((ext_vector_type(2)));


	// int 1
	typedef bool bool4 __attribute__((ext_vector_type(4)));
	typedef bool bool3 __attribute__((ext_vector_type(3)));
	typedef bool bool2 __attribute__((ext_vector_type(2)));



	//convenience aliases for GLSL
	typedef float4 vec4;
	typedef float3 vec3;
	typedef float2 vec2;
}

#endif /* TYPES_HPP_ */
