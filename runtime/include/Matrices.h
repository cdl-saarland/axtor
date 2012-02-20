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
