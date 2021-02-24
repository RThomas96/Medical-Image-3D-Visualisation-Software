#ifndef NVIDIA_PROBLEM_TIME_H_
#define NVIDIA_PROBLEM_TIME_H_

/**
 * Shamelessly stolen from GLFW. Copied so no dependencies are necessary.
 */

#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#endif

#include <cstdint>

typedef struct platformTime {
#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
	bool hasPC; // has performance counter, or not ?
#endif
#if defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
	bool monotonic; // is monotonic (high-resolution), or not ?
#endif
	std::uint64_t frequency; // timer frequency
	std::uint64_t offset;
} platformTime;

extern platformTime pTime;

void initPlatformTimer(void);
std::uint64_t getPlatformTime(void);

double getTime(void);

#endif // NVIDIA_PROBLEM_TIME_H_
