//
// Created by parin on 10/17/20.
//

#include "threadpool.h"

#include <grpc++/grpc++.h>

threadpool::threadpool(int thread_count)
        : queue(), work_queue_lock(), wait_for_queue() {
    this->thread_count = std::min(MAX_THREAD, thread_count);
    threadpool::init();
}

void threadpool::init() {
    int i = 0;
    while (i < thread_count) {
        thread_list.emplace_back([this, i] { do_work(); });
        std::cout << "Thread number " + std::to_string(i) + " is ready" << std::endl;
        i++;
    }
}

[[noreturn]] void threadpool::do_work() {

    std::function<void()> task;
    while (1) {
        std::unique_lock<std::mutex> queue_lock(threadpool::work_queue_lock);
        threadpool::wait_for_queue.wait(queue_lock, [this] { return !queue.empty(); });
        task = std::move(threadpool::queue.front());
        threadpool::queue.pop();
        queue_lock.unlock();
        task();
    }
}
