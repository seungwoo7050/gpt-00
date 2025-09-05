// [SEQUENCE: MVP2-9]
#include "ThreadPool.h"

// [SEQUENCE: MVP2-10]
// 생성자: 작업자 스레드를 생성하고 실행 시작
ThreadPool::ThreadPool(size_t numThreads) : stop_(false) {
    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] { workerThread(); });
    }
}

// [SEQUENCE: MVP2-11]
// 소멸자: 모든 스레드가 안전하게 종료되도록 보장
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all(); // 모든 대기 중인 스레드를 깨움
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// [SEQUENCE: MVP2-12]
// 작업자 스레드의 메인 루프
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // [SEQUENCE: MVP2-13]
            // 작업이 있거나, 종료 신호가 올 때까지 대기
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
                return; // 종료
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(); // 작업 실행
    }
}
