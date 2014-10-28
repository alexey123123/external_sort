#include "platform.h"
#include "clock_ms.h"



#if defined(WindowsPlatform)
#include <windows.h> // for Sleep, GetTickCount
#endif // Win32Platform

#if defined(LinuxPlatform)
#include <time.h> // for CLK_TCK
#include <unistd.h> // for usleep
// #include <sys/times.h> // for times
#endif // PosixPlatform



unsigned int clock_ms() {
#if defined(LinuxPlatform)

	//tms tinfo;
	//return uint64(times(&tinfo))*1000/CLK_TCK;

	timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
		return 0;

	return ts.tv_sec * 1000 +
		ts.tv_nsec / 1000000;

#elif defined(WindowsPlatform)

	return ::GetTickCount();

#endif
}

unsigned int diff_clock_ms(unsigned int from,unsigned int to) {
	if (to>=from) return to-from;
	return 0xffffffff-from+to;
}

void sleep_ms(int timeout) {
	if (timeout<0)
		return;
#if defined(LinuxPlatform)
	sleep(timeout/1000); // sec
	usleep((timeout%1000)*1000); // microsec

#elif defined(WindowsPlatform)
	::Sleep(timeout);
#endif
}