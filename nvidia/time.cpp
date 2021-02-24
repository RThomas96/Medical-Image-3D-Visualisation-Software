#include "./time.hpp"

#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
#endif

#if  defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#	include <unistd.h>
#	include <sys/time.h>
#	include <time.h>
#endif

platformTime pTime{};

void initPlatformTimer(void) {
#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
	std::uint64_t freq;
	if (QueryPerformanceFrequency((LARGE_INTEGER*) &freq)) {
		pTime.hasPC = true;
		pTime.frequency = freq;
	} else {
		pTime.hasPC = false;
		pTime.frequency = 1000;
	}
#endif
#if  defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#	if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		pTime.monotonic = true;
		pTime.frequency = 1000000000;
	} else
#	endif
	{
		pTime.monotonic = false;
		pTime.frequency = 1000000;
	}
#endif
	pTime.offset = getPlatformTime();
}

std::uint64_t getPlatformTime(void) {
#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
	if (pTime.hasPC) {
		uint64_t val;
		QueryPerformanceCounter((LARGE_INTEGER*) &value);
		return value;
	} else {
		return static_cast<std::uint64_t>(timeGetTime());
	}
#endif

#if  defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#	if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
	if (pTime.monotonic) {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
	} else
#	endif
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
	}
#endif
}

double getTime(void) {
	return static_cast<double>(getPlatformTime() - pTime.offset) /
		static_cast<double>(pTime.frequency);
}
