/*
 * AddressSpaces.h
 *
 *  Created on: 03.05.2010
 *      Author: Simon Moll
 */

#ifndef ADDRESSSPACES_HPP_
#define ADDRESSSPACES_HPP_

//### supported address spaces
// PTX-compatible address spaces (do not work)
#define SPACE_CONSTANT 1
#define SPACE_LOCAL    2
#define SPACE_GLOBAL   3

// other address spaces
#define SPACE_PRIVATE  4
#define SPACE_POINTER  5

#define SPACE_NOPTR 100


#endif /* ADDRESSSPACES_HPP_ */
