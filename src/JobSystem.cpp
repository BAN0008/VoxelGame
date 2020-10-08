#include "JobSystem.hpp"
#include "Log.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

namespace JobSystem
{

	std::queue<Job> jobQueue;
	std::mutex queueLock;
	std::vector<std::thread> threads;
	std::atomic<bool> stop;

	void _ThreadLoop()
	{
		// while (!jobQueue.empty()) {
		while (!stop) {
			queueLock.lock();
			if (!jobQueue.empty()) {
				const auto job = jobQueue.front();
				jobQueue.pop();
				queueLock.unlock();
				job();
			}
			else {
				queueLock.unlock();
				std::this_thread::sleep_for(std::chrono::seconds(2));
			}
		}
	}

	void StartThreads(int threadCount)
	{
		stop = false;
		threadCount = threadCount > 0 ? threadCount : std::thread::hardware_concurrency();
		threadCount = threadCount > 0 ? threadCount : 1;
		for (int i = 0; i < threadCount; i++) {
			threads.emplace_back(_ThreadLoop);
		}
	}

	void StopThreads()
	{
		stop = true;
		queueLock.lock();
		while (!jobQueue.empty()) {
			jobQueue.pop();
		}

		queueLock.unlock();
		for (std::thread &thread : threads) {
			thread.join();
		}
	}

	void AddJob(const Job &job)
	{
		jobQueue.push(job);
	}
}