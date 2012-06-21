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
 * IntrinsicMakros.h
 *
 *  Created on: Oct 18, 2010
 *      Author: Simon Moll
 */

#ifndef INTRINSICMAKROS_HPP_
#define INTRINSICMAKROS_HPP_

#define FUNC(INTRINSIC,RETURN,ACTUAL,ARGS,ARGC) \
	const RETURN INTRINSIC ARGS __attribute__((const));

#define COMPLEX(INTRINSIC,RETURN,FORMAT,ARGS) \
	const RETURN INTRINSIC ARGS __attribute__((const));

#define INFIX(OP,RETURN,INTRINSIC,ARGS) \
	const RETURN INTRINSIC ARGS;

#define NATIVE(NAME) \
		class NAME;

#define INTERP(NAME,RETURN,ARGS)\
		const RETURN NAME ARGS;

#define PASS(RETURN,INTRINSIC,ARG)\
		const RETURN INTRINSIC ARG;

#define BUILTIN(TYPE,INTRINSIC,ACTUAL) \
		extern TYPE INTRINSIC;

#define GETTER(NAME,TYPE,ARGUMENT) \
		const TYPE NAME ARGUMENT;

#define SETTER(NAME,ARGUMENT) \
		void NAME ARGUMENT;

#define REPLACE_FUNC(NAME,RETURN,REPLACEMENT) \
		const RETURN NAME();

#define MUTE(X)


#endif /* INTRINSICMAKROS_HPP_ */
