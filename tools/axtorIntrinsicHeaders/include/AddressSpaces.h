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
 * AddressSpaces.h
 *
 *  Created on: 13.04.2010
 *      Author: Simon Moll
 */

#ifndef ADDRESSSPACES_HPP_
#define ADDRESSSPACES_HPP_

#define SPACE_GLOBAL   1
#define SPACE_CONSTANT 2
#define SPACE_LOCAL    3
#define SPACE_PRIVATE  4
#define SPACE_POINTER  5 //used for shader_buffer_load/store extensions

#define SPACE_NOPTR 100

#define PRIVATE __attribute__((address_space(SPACE_PRIVATE)))

//shared among work group
#define LOCAL __attribute__((address_space(SPACE_LOCAL)))

//unique on device
#define GLOBAL __attribute__((address_space(SPACE_GLOBAL)))

//constant memory space
#define CONSTANT  __attribute__((address_space(SPACE_CONSTANT)))

//constant memory space
#define DEVICE_POINTER  __attribute__((address_space(SPACE_POINTER)))

//used to denote that this is in fact not a pointer type
#define NOPTR __attribute__((address_space(SPACE_NOPTR)))

#endif /* ADDRESSSPACES_HPP_ */
