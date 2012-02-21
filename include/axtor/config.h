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
 * config.h
 *
 *  Created on: 27.03.2010
 *      Author: Simon Moll
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

// trigger the axtor debug mode with the LLVM debug build flag
#ifdef _DEBUG
#define DEBUG
#endif

/*
 * Enables CNS for node splitting
 */
#define ENABLE_CNS

/*
 * enables short-circuit expression recovery
 */
// #define ENABLE_SHORT_CIRCUIT_EXPRESSIONS

/*
 * enables restructuring AST extraction
 */
#define USE_RESTRUCT_PASS

/*
 * show CFGs after CFG operations
 */
//#define DEBUG_VIEW_CFGS

/*
 * enables evaluation output
 */
//#define EVAL_DECOMPILE_TIME

/*
 * enables the native algorithm in the NodeSplitting restructuring procedure
 */
//#define NS_NAIVE


/*
 * enables verbose debug
 */
//#define DEBUG

/*
 * enables intrinsic dump for all initialized backends
 */
//#define DEBUG_INTRINSICS

//Debug flags
#ifdef AXTOR_DEBUG
/*
 * makes each axtor pass place a notification when its invoked
 */
#define DEBUG_PASSRUN

//#define DEBUG_DOMFRONT

#endif

#endif /* CONFIG_HPP_ */
