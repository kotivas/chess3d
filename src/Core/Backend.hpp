#pragma once
#include <cstdint>

namespace Backend {
	void Init();
	// returns physical memory currently used by current process
	uint32_t VirtMemoryUsage();
	// returns percents of cpu currently used by current process
	float CpuUsage();
}
