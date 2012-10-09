/*
 * OCLEnum.cpp
 *
 *  Created on: 25 Jan 2012
 *      Author: v1smoll
 */

#include <axtor_ocl/OCLEnum.h>
#include <axtor_ocl/OCLEnumValues.h>
#include <axtor/util/llvmConstant.h>

#define SAMPLER_CASE(STR,NAME)  case NAME:STR=#NAME; break;

bool axtor::evaluateEnum_MemFence(llvm::Value * val, std::string & result)
{
	uint64_t enumIndex;
	if (! evaluateInt(val, enumIndex))
		return false;

	switch(enumIndex)
	{
		SAMPLER_CASE(result, CLK_LOCAL_MEM_FENCE)
		SAMPLER_CASE(result, CLK_GLOBAL_MEM_FENCE)
	default:
		return false;
	}

	return true;
}


bool axtor::evaluateEnum_Sampler(size_t cfg, std::string & result)
{
	std::string normStr;
	switch (cfg & 1) //0x..00001
	{
		SAMPLER_CASE(normStr, CLK_NORMALIZED_COORDS_FALSE)
		SAMPLER_CASE(normStr, CLK_NORMALIZED_COORDS_TRUE)
	}

	std::string addStr;
	switch (cfg & 6) //0x..00110
	{
		SAMPLER_CASE(addStr, CLK_ADDRESS_NONE)
		SAMPLER_CASE(addStr, CLK_ADDRESS_REPEAT)
		SAMPLER_CASE(addStr, CLK_ADDRESS_CLAMP_TO_EDGE)
		SAMPLER_CASE(addStr, CLK_ADDRESS_CLAMP32)
	default:
		return false;
	}

	std::string filterStr;
	switch (cfg & 8) //0X..01000
	{
		SAMPLER_CASE(filterStr, CLK_FILTER_NEAREST)
		SAMPLER_CASE(filterStr, CLK_FILTER_LINEAR)
	default:
			return false;
	}

	result = normStr + " | " + addStr + " | " + filterStr;

	return true;
}

#undef SAMPLER_CASE

