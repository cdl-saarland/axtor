/*
 * OCL_FrontendWrapper.h
 *
 *  Created on: 26 Jan 2012
 *      Author: v1smoll
 */

#ifndef OCL_FRONTENDWRAPPER_H_
#define OCL_FRONTENDWRAPPER_H_

#include <float.h>
#include <stddef.h>
#include "OpenCL_Intrinsics.h"


#define CONVERT_FUNCTION(to, from) \
	  __attribute__((pure, always_inline)) \
	  to convert_##to (from)

	CONVERT_FUNCTION(float4, uint4);
	CONVERT_FUNCTION(float4, uchar4);
	CONVERT_FUNCTION(int4, uint4);
	CONVERT_FUNCTION(int4, uchar4);
	CONVERT_FUNCTION(uint4, uchar4);
	CONVERT_FUNCTION(float, uint);
	CONVERT_FUNCTION(uint, float);
	CONVERT_FUNCTION(uchar4, float4);
	CONVERT_FUNCTION(uchar4, int4);
	CONVERT_FUNCTION(char4, uchar4);
	CONVERT_FUNCTION(uchar4, char4);
	CONVERT_FUNCTION(double4, uint4);

	__attribute__((pure, always_inline))
	  uchar4 convert_uchar4_sat(float4);


#undef CONVERT_FUNCTION

#endif /* OCL_FRONTENDWRAPPER_H_ */
