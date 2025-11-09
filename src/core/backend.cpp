#include "backend.hpp"

#ifdef WIN32
#include <windows.h>
#include "psapi.h"

namespace Backend {
	static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	static int numProcessors;
	static HANDLE self;

	void Init() {
		SYSTEM_INFO sysInfo;
		FILETIME ftime, fsys, fuser;

		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(FILETIME));

		self = GetCurrentProcess();
		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
	}

	float CpuUsage() {
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (sys.QuadPart - lastSysCPU.QuadPart) +
			(user.QuadPart - lastUserCPU.QuadPart);
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;

		return percent * 100;
	}

	uint32_t VirtMemoryUsage() {
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.PrivateUsage;
	}
}
#endif

#ifdef POSIX

uint32_t MemoryUsage() {
	return 0;
}

float CpuUsage() {
	return 0;
}

#endif
