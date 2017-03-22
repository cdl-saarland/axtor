/*
 * ResourceGuard.h
 *
 *  Created on: 19.03.2010
 *      Author: Simon Moll
 */

#ifndef RESOURCEGUARD_HPP_
#define RESOURCEGUARD_HPP_

namespace axtor {
/*
 * required for creating local scopes for heap objects
 */
template<class T>
class ResourceGuard
{
	T * obj;
public:

	ResourceGuard(T * _obj) :
		obj(_obj)
	{
	#ifdef DEBUG
		std::cerr << "ResourceGuard ctor()" << std::endl;
	#endif
	}

	~ResourceGuard()
	{
	#ifdef DEBUG
		std::cerr << "ResourceGuard dtor()" << std::endl;
	#endif
		if (obj) {
			delete obj;
		}
	}

	inline T * get()
	{
		return obj;
	}

	inline T & operator*()
	{
		return *obj;
	}

	inline T * operator ->()
	{
		return obj;
	}
};

}


#endif /* RESOURCEGUARD_HPP_ */
