#pragma once

#include <array>
#include <string>
#include <iostream>
#include <chrono>

// required headers
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <shared_mutex>

#include "Core/CommonDefinitions.h"

namespace engi
{

	class ParallelExecutor
	{
	public:
		using execution_func_type = std::function<void(uint32_t, uint32_t)>;
		using executor_func_type = std::function<void(uint32_t)>;

	public:
		ParallelExecutor(uint32_t numThreads);
		~ParallelExecutor();

		void wait();

		// Executes a function in parallel blocking the caller thread.
		void execute(const execution_func_type& func, uint32_t numTasks, uint32_t tasksPerBatch);

		// Executes a function in parallel asynchronously.
		void executeAsync(const execution_func_type& func, uint32_t numTasks, uint32_t tasksPerBatch);

		inline constexpr size_t numThreads() const { return m_threads.size(); }
		inline bool isWorking() const { return m_finishedThreadNum < m_threads.size(); }

	public:
		// 100% CPU occupation, it may cause OS hitches.
		// No point to have more threads than the number of CPU logical cores.
		static inline constexpr uint32_t getMaxThreads() { return s_maxThreads; }

		// 50-100% CPU occupation
		static inline constexpr uint32_t getHalfThreads() { return s_maxThreads / 2; }

	protected:
		inline void awake() { m_workCV.notify_all(); }

		void workLoop(uint32_t threadIndex);

	protected:
		bool m_isLooping;

		std::atomic<uint32_t> m_finishedThreadNum;
		std::atomic<uint32_t> m_completedBatchNum;
		executor_func_type m_executeTasks;

		std::shared_mutex m_mutex;
		std::condition_variable_any m_waitCV;
		std::condition_variable_any m_workCV;

		std::vector<std::thread> m_threads;

	private:
		static const uint32_t s_maxThreads;
	}; // ParallelExecutor class

}; // engi namespace
