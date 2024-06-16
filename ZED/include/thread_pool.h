#pragma once
#include "core.h"
#include <thread>
#include <mutex>
#include <queue>



namespace ZED
{
	class ThreadPool
	{
	public:
        ThreadPool() = delete;

        ThreadPool(size_t WorkerCount)
        {
            for (size_t i = 0; i < WorkerCount; ++i)
            {
                m_Threads.emplace_back([this]
                {
                    while (true)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(m_Task_Mutex);
                            m_Condition_Variable.wait(lock, [this] { return m_Stop || !m_Tasks.empty(); });
                            if (m_Stop && m_Tasks.empty())
                                return;

                            task = std::move(m_Tasks.front());
                            m_Tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(m_Task_Mutex);
                m_Stop = true;
            }

            m_Condition_Variable.notify_all();

            for (auto& thread : m_Threads)
                thread.join();
        }

        template <class T>
        void Enqueue(T&& NewTask)
        {
            {
                std::unique_lock<std::mutex> lock(m_Task_Mutex);
                m_Tasks.emplace(std::forward<T>(NewTask));
            }

            m_Condition_Variable.notify_one();
        }

	private:
        std::vector<std::thread> m_Threads;
        std::queue<std::function<void()>> m_Tasks;
        std::mutex m_Task_Mutex;
        std::condition_variable m_Condition_Variable;
        bool m_Stop = false;
	};
}