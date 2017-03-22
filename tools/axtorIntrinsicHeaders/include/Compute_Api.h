/*
 * Compute_Api.h
 *
 *  Created on: 07.04.2010
 *      Author: Simon Moll
 */

#ifndef COMPUTE_API_HPP_
#define COMPUTE_API_HPP_

#include "Types.h"
#include "AddressSpaces.h"

#include "IntrinsicMakros.h"

//#define REGISTER_FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC) \
//	intrinsics[#INTRINSIC] = new IntrinsicFunction(#ACTUAL,ARGC);

/*#define FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC) \
	RETURN INTRINSIC ARGS;

#define NATIVE(NAME) \
		class NAME;*/
inline int4 i4_ctor_i(int v)
{
	int4 r;
	r.x = v;
	r.y = v;
	r.z = v;
	r.w = v;
	return r;
}

inline int2 i2_ctor_ii(int x, int y)
{
	int2 r;
	r.x = x;
	r.y = y;
	return r;
}

inline int4 i4_ctor_iiii(int x, int y, int z, int w)
{
	int4 r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;
	return r;
}

inline uint4 u4_ctor_u(uint x)
{
	uint4 r;
	r.x = x;
	r.y = x;
	r.z = x;
	r.w = x;
	return r;
}

inline uint4 u4_ctor_uuuu(uint x, uint y, uint z, uint w)
{
	uint4 r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;
	return r;
}

inline float4 f4_ctor_f(float v)
{
	float4 r;
	r.x = v;
	r.y = v;
	r.z = v;
	r.w = v;
	return r;
}


inline float2 f2_ctor_ff(float x, float y)
{
	float2 r;
	r.x = x;
	r.y = y;
	return r;
}

extern "C"
{
	// generic types
	#include <axtor_ocl/OCL_Intrinsics.def>

	//FIXME sampler hack
	sampler_t NOPTR* fake_global_sampler(int config) __attribute__((const));
}

#include "UndefIntrinsicMakros.h"

#include "Attributes.h"

#endif /* COMPUTE_API_HPP_ */
