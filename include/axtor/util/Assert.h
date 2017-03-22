/*
 * Assert.h
 *
 *  Created on: 08.09.2011
 *      Author: Dmitri Rubinstein
 */

#ifndef ASSERT_HPP_
#define ASSERT_HPP_

#include <assert.h>

#define AXTOR_UNUSED(expr) (void)sizeof(expr)

#ifndef NDEBUG
#define AXTOR_ASSERT(cond) assert(cond)
#else
#define AXTOR_ASSERT(cond) AXTOR_UNUSED(cond)
#endif

#endif /* ASSERT_HPP_ */
