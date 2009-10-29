#include <windows.h>
#include <time.h>

#pragma once

namespace GNET {
	class Timer {
	private:
		int timer_max;
		clock_t clock_tic;
	public:
		Timer(int ms_time = 500);
		void Timer::Reset();
		void Timer::Reset(int ms_time);		
		void WaitTillFinished();
		bool Timer::Finished();
		void Stop();
	};
}