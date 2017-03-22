/*
 * Timer.h
 *
 *  Created on: Feb 24, 2011
 *      Author: Simon Moll
 */

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <sys/time.h>

namespace axtor {

	class Timer
	{
		timeval startTime;
		timeval endTime;

		void dumpTV(timeval v) const
		{
			std::cerr << "secs " << v.tv_sec << "\n"
					  << "ms   " << v.tv_usec << "\n";
		}

	public:
		inline void start()
		{
			timespec start;
			clock_gettime(CLOCK_REALTIME, &start);
			TIMESPEC_TO_TIMEVAL(&startTime, &start)
		}

		inline void end()
		{
			timespec end;
			clock_gettime(CLOCK_REALTIME, &end);
			TIMESPEC_TO_TIMEVAL(&endTime, &end)
		}

		long getTotalMS() const
		{
			long start = startTime.tv_sec * 1000 + (startTime.tv_usec / 1000);
			long end = endTime.tv_sec * 1000 + (endTime.tv_usec / 1000);

			return end - start;
		}
	};

}

#endif /* TIMER_HPP_ */
