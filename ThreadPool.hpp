//
// Created by Steven Roddan on 2/16/2026.
//

#ifndef CUDAHISTOGRAMS_THREADPOOL_HPP
#define CUDAHISTOGRAMS_THREADPOOL_HPP

#include <any>
#include <future>
#include <thread>
#include <queue>

class ThreadPool {
    const size_t numThreads;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::condition_variable cv;
    std::mutex mutex;

    bool isShutdown = false;
public:
    void work(size_t threadIdx) {
        std::unique_lock lock(mutex);

        while (!isShutdown) {
            cv.wait(lock, [this]() {
                return !tasks.empty() || isShutdown;
            });

            if (isShutdown && tasks.empty()) return;

            auto task = tasks.front();
            tasks.pop();    // pops front of queue.
            lock.unlock();
            task();
            lock.lock();
        }
    }

    explicit ThreadPool(const size_t numThreads) : numThreads(numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back(&ThreadPool::work, this, i);
        }
    }

    template<typename F>
    auto queue(F&& f) {
        using R = std::invoke_result_t<F>;

        auto p = std::make_shared<std::promise<R>>();
        auto future = p->get_future();

        {
            std::unique_lock lock(mutex);
            tasks.push([p, f = std::forward<F>(f)]() mutable {
                try {
                    if constexpr (std::is_void_v<R>) {
                        f();
                        p->set_value();
                    } else {
                        p->set_value(f());
                    }
                } catch (...) {
                    p->set_exception(std::current_exception());
                }
            });
        }
        cv.notify_one();
        return future;
    }

    ~ThreadPool() {
        {
            // ensures that isShutdown is seen by
            std::unique_lock lock(mutex);
            isShutdown = true;
        }
        cv.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }
};

#endif //CUDAHISTOGRAMS_THREADPOOL_HPP