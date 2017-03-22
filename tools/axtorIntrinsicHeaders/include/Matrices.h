/*
 * Matrices.h
 *
 *  Created on: 15.05.2010
 *      Author: Simon Moll
 */

#ifndef MATRICES_HPP_
#define MATRICES_HPP_

#include "AddressSpaces.h"

#define OPAQUE(NAME) \
	class NAME; \

#define ALIAS(NEW,NAME) \
	typedef NAME NEW; \

extern "C" {
	OPAQUE(mat2x2)
	OPAQUE(mat2x3)
	OPAQUE(mat2x4)

	OPAQUE(mat3x2)
	OPAQUE(mat3x3)
	OPAQUE(mat3x4)

	OPAQUE(mat4x2)
	OPAQUE(mat4x3)
	OPAQUE(mat4x4)

	ALIAS(mat2,mat2x2)
	ALIAS(mat3,mat3x3)
	ALIAS(mat4,mat4x4)
}

#undef OPAQUE
#undef ALIAS

#endif /* MATRICES_HPP_ */
