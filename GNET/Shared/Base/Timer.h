#include <windows.h>
#include <time.h>

#pragma once

namespace GNET {
	///A class used to get the timer functionality.
	class Timer {
	private:
		int timer_max;
		clock_t clock_tic;
	public:
		///Constructor
		Timer(int ms_time = 500);

		///A function which resets the timer by the default time.
		void Timer::Reset();

		///A function which resets the timer according to a particular time.
		void Timer::Reset(int ms_time);		

		///A function which sleeps till the timer is finished.
		void WaitTillFinished();

		///A function which returns true is the timer is finished.
		bool Timer::Finished();

		///A function which stops the timer.
		void Stop();
	};
}