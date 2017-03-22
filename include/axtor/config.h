/*
 * config.h
 *
 *  Created on: 27.03.2010
 *      Author: Simon Moll
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

/*
 * Enables CNS for node splitting
 */
#define ENABLE_CNS

/*
 * enables short-circuit expression recovery
 */
// #define ENABLE_SHORT_CIRCUIT_EXPRESSIONS

/*
 * enables evaluation output
 */
//#define EVAL_DECOMPILE_TIME

/*
 * enables the native algorithm in the NodeSplitting restructuring procedure
 */
//#define NS_NAIVE

/*
 * enables the debug mode for the restructuring procedure
 */
//#define DEBUG_RESTRUCT

/*
 * enables debug mode for loop exit unification
 */
#define DEBUG_LEU

/*
 * enables verbose debug
 */
//#define DEBUG

/*
 * enables intrinsic dump for all initialized backends
 */
//#define DEBUG_INTRINSICS

/*
 * enables expensive integrity checks and debug output
 */
#define AXTOR_EXPENSIVE_CHECKS

//Debug flags
#ifdef _DEBUG
/*
 * makes each axtor pass place a notification when its invoked
 */
#define DEBUG_PASSRUN

/*
 * show CFGs after CFG operations
 */
//#define DEBUG_VIEW_CFGS

//#define DEBUG_DOMFRONT

/*
 * dumps module after pass runs in separate files
 */
#define DEBUG_DUMP_MODULES

#endif


/*
 * Convenience macros
 */

#ifdef AXTOR_EXPENSIVE_CHECKS
	#define EXPENSIVE_TEST if (true)
#else
	#define EXPENSIVE_TEST if (false)
#endif

#ifdef _DEBUG
	#define DEBUG_STREAM llvm::errs()
	#define IF_DEBUG if (true)

#else
	#define DEBUG_STREAM std::stringstream __dummy; __dummy
	#define IF_DEBUG if (false)
#endif

#endif /* CONFIG_HPP_ */
