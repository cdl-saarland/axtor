/*
 * TextStream.h
 *
 *  Created on: 03.05.2010
 *      Author: Simon Moll
 */

#ifndef TEXTSTREAM_HPP_
#define TEXTSTREAM_HPP_

#include <string>
#include <iostream>

namespace axtor {

class TextStream
{
public:
	virtual void put(std::string text)=0;
	virtual void putLine(std::string text)=0;
};

class TextFileStream : public TextStream
{
	std::ostream & out;

public:
	TextFileStream(std::ostream & _out);
	virtual void put(std::string text);
	virtual void putLine(std::string text);
};

class MultiStream : public TextStream
{
	TextStream & first;
	TextStream & second;

public:
	MultiStream(TextStream & _first, TextStream & _second);
	virtual void put(std::string);
	virtual void putLine(std::string);
};

}
#endif /* TEXTSTREAM_HPP_ */
