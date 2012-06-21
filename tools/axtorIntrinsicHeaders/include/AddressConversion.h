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
