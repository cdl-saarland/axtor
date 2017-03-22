/*
 * Slang_Api.h
 *
 *  Created on: 26.02.2010
 *      Author: Simon Moll
 */

/*
 * Generic Shader language Platform API
 */

#ifndef GLSL_API_H_
#define GLSL_API_H_

#include "Attributes.h"
#include "AddressSpaces.h"
#include "Types.h"
#include "Matrices.h"

#include "IntrinsicMakros.h"

// name alias for specifying uniform globals
#define UNIFORM(T) \
	extern "C" T GLOBAL

#define POINTER(T) \
	(T DEVICE_POINTER*)

#define UNUSED(expr) (void)sizeof(expr)

extern "C"
{
	#include <axtor_glsl/GLSL_Intrinsics.def>


/*
 * basic declarations for sub-shader form BC shaders
 */
	//lighting API (implemented by X-Lighting modules)
	int getNumLights();
	vec3 getLightPosition(int lightId);
	vec3 getLightContribution(int lightId, vec3 position, vec3 normal);
	inline vec3 getLightDir(int lightId, vec3 position)
	{
		return normalize_f3(getLightPosition(lightId) - position);
	}

	vec4 sampleTexture(const char * name, vec2 texCoord);
}

/*
 * utility functions
 */
inline vec2 v2_ctor_ff(float x, float y)
{
	vec2 res;
	res.x = x;
	res.y = y;
	return res;
}

inline int2 v2_to_i2(vec2 v)
{
	int2 o;
	o.x = (int) v.x;
	o.y = (int) v.y;
	return o;
}

inline vec2 v2_ctor_f(float v)
{
	vec2 res;
	res.x = v;
	res.y = v;
	return res;
}

inline vec3 v3_ctor_fff(float x, float y, float z)
{
	vec3 res;
	res.x = x;
	res.y = y;
	res.z = z;
	return res;
}

inline vec3 v3_ctor_v2f(vec2 v, float z)
{
	vec3 res;
	res.x = v.x;
	res.y = v.y;
	res.z = z;
	return res;
}

inline vec3 v3_ctor_f(float x)
{
	return v3_ctor_fff(x, x, x);
}

inline vec4 v4_ctor_f(float x)
{
	vec4 res;
	res.x = x;
	res.y = x;
	res.z = x;
	res.w = x;
	return res;
}


inline vec4 v4_ctor_ffff(float x, float y, float z, float w)
{
	vec4 res;
	res.x = x;
	res.y = y;
	res.z = z;
	res.w = w;
	return res;
}

inline vec4 v4_ctor_v2ff(vec2 v, float z, float w)
{
	vec4 res;
	res.x = v.x;
	res.y = v.y;
	res.z = z;
	res.w = w;
	return res;
}

inline vec4 v4_ctor_v3f(vec3 v, float w)
{
	return v4_ctor_ffff(v.x, v.y, v.z, w);
}

//#include "AddressConversion.h" //must not be an extern "C"

#include "UndefIntrinsicMakros.h"

#endif /* GLSL_API_H_ */
