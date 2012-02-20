/*
 * OCL_FrontendWrapper.cpp
 *
 *  Created on: 26 Jan 2012
 *      Author: v1smoll
 */


#include "OCL_FrontendWrapper.hpp"

#define CONVERT_FUNCTION(to, from) \
  __attribute__((always_inline)) \
  to convert_##to (from in)

// REGULAR
CONVERT_FUNCTION(float4, uint4)   { return convert_f4_u4(in);  }
CONVERT_FUNCTION(float4, uchar4)  { return convert_f4_uc4(in); }
CONVERT_FUNCTION(int4, uint4)     { return convert_i4_u4(in);  }
CONVERT_FUNCTION(int4, uchar4)    { return convert_i4_uc4(in); }
CONVERT_FUNCTION(float, uint)     { return convert_f_u(in);    }
CONVERT_FUNCTION(double4, uint4)  { return convert_d4_u4(in);  }
CONVERT_FUNCTION(uint4, uchar4)   { return reinterpret_cast<uint4>(convert_u4_uc4(in)); }
CONVERT_FUNCTION(uint, float)     { return convert_u_f(in); }
CONVERT_FUNCTION(uchar4, float4)  { return reinterpret_cast<uchar4>(convert_uc4_f4(in)); }
CONVERT_FUNCTION(uchar4, int4)    { return reinterpret_cast<uchar4>(convert_uc4_i4(in)); }
CONVERT_FUNCTION(uchar4, char4)   { return reinterpret_cast<uchar4>(convert_uc4_c4(in)); }
CONVERT_FUNCTION(char4, uchar4)   { return convert_c4_uc4(in); }

// SAT
uchar4 convert_uchar4_sat(float4 in) { return reinterpret_cast<uchar4>(convert_sat_uc4_f4(in)); }


#undef CONVERT_FUNCTION

