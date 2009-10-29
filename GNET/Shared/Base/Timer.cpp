#include "Timer.h"

using namespace GNET;

Timer::Timer(int ms_time) {
	timer_max = ms_time;
}

void Timer::Reset() {
	Reset(timer_max);
}
void Timer::Reset(int ms_time) {
	timer_max = ms_time;
	clock_tic = clock() + (ms_time * CLOCKS_PER_SEC) / 1000;
}

bool Timer::Finished() {
	return clock() > clock_tic;
}

void Timer::WaitTillFinished() {
	int time_to_wait = clock_tic - clock();
	if (time_to_wait < 0)
		return;
	
	Sleep(time_to_wait * 1000 / CLOCKS_PER_SEC);
}
