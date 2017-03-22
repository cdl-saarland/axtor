/*
 * stringutil.h
 *
 *  Created on: 08.02.2010
 *      Author: Simon Moll
 */

#ifndef STRINGUTIL_HPP_
#define STRINGUTIL_HPP_

#include <string>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <stdint.h>

typedef std::vector<std::string> StringVector;

namespace axtor {

template<typename T>
std::string str(T number);

template<>
std::string str(float number);

bool isNum(char c);

bool isABC(char c);

bool isAlphaNum(char c);

int parseInt(char * cursor, char ** out);

/*
 * returns the hex string representation for this integer (so far only for 0 <= i <= 15)
 */
std::string hexstr(int i);

/*
 * returns the hex string representation of an integer variable
 */
std::string convertToHex(uint64_t num, unsigned int bytes);

/*
 * returns a StringVector of tokens from @src delimited by @delimiter
 * such that there is a string before and after each occurrence of @delimiter (even if it's an empty string)
 */
std::vector<std::string> tokenizeIsolated(std::string src, std::string delimiter);

}

#endif /* STRINGUTIL_HPP_ */
