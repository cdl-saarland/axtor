/*
 * AddressConversion.h
 *
 *  Created on: Aug 26, 2010
 *      Author: Simon Moll
 */

#ifndef ADDRESSCONVERSION_HPP_
#define ADDRESSCONVERSION_HPP_

#include "AddressSpaces.h"
#include "Slang_Api.h"

template<class T>
T NOPTR* localize(T GLOBAL* global);

template<class T>
T NOPTR* localize(T LOCAL* );

template<class T>
T NOPTR* localize(T CONSTANT* global);

template<class T>
T NOPTR* localize(T NOPTR* val)
{
	return val;
}

#define LOCALIZE_FUNCTIONS(TYPE,SUFFIX) \
	template<> \
	TYPE NOPTR* localize<TYPE>(TYPE GLOBAL * global) \
	{ return convGLOBALtoNOPTR_##SUFFIX(global); }


LOCALIZE_FUNCTIONS(mat4x4,mat4x4)
LOCALIZE_FUNCTIONS(mat3x3,mat3x3)
LOCALIZE_FUNCTIONS(mat2x2,mat2x2)

#endif /* ADDRESSCONVERSION_HPP_ */
