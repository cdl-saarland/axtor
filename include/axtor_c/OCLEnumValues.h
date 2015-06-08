/*
 * OCLEnumValues.h
 *
 *  Created on: Aug 17, 2012
 *      Author: simoll
 */

#ifndef OCLENUMVALUES_H_
#define OCLENUMVALUES_H_


namespace axtor {

	enum OCLMemFence
	{
		CLK_LOCAL_MEM_FENCE = 0,
		CLK_GLOBAL_MEM_FENCE = 1
	};

	enum OCLSamplerConfig
	{
		// 1. bit
		CLK_NORMALIZED_COORDS_FALSE = (0 << 0),
		CLK_NORMALIZED_COORDS_TRUE =  (1 << 0),

		// 2 & 3 bit
		CLK_ADDRESS_NONE =            (0 << 1),
		CLK_ADDRESS_REPEAT =          (1 << 1),
		CLK_ADDRESS_CLAMP_TO_EDGE =   (2 << 1),
		CLK_ADDRESS_CLAMP32 =         (3 << 1),

		// 4. bit
		CLK_FILTER_NEAREST =          (0 << 3),
		CLK_FILTER_LINEAR =           (1 << 3)
	};
}


#endif /* OCLENUMVALUES_H_ */
