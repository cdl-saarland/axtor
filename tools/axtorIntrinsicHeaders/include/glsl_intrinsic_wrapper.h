/*
 * intrinsic_wrapper.h
 *
 *  Created on: Aug 2, 2010
 *      Author: Simon Moll
 */

#ifndef INTRINSIC_WRAPPER_HPP_
#define INTRINSIC_WRAPPER_HPP_

#include "Slang_Api.h"

static inline float floorf(float f)
{
	return floor_f(f);
}

#endif /* INTRINSIC_WRAPPER_HPP_ */
