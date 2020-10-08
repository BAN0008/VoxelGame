#pragma once
#include <functional>

namespace JobSystem
{
	using Job = std::function<void(void)>;

	void StartThreads(int threadCount = 0);
	void StopThreads();
	void AddJob(const Job &Job);
}