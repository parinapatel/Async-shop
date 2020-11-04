//
// Created by parin on 10/17/20.
//

#ifndef PROJECT3_THREADPOOL_H
#define PROJECT3_THREADPOOL_H

#define MAX_THREAD 128

#include <future>
#include <grpcpp/channel.h>
#include <queue>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include "VendorResponse.h"

// Reference : https://stackoverflow.com/questions/22030027/c11-dynamic-threadpool/32593766#32593766
// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
class threadpool {

public:
    // validate thread pool and initailze all required private variables
    explicit threadpool(int thread_count);

    // Start the workers and enroll them in the pool.
    void init();

    // Enqueue tasks with function signature and given arguments.
    template<typename Func, typename... Args>
    auto enqueue(Func &&f, Args &&... args);

private:
    int thread_count;
    std::vector<std::thread> thread_list;

    //  List to vars for managing race condition around queue
    std::queue<std::function<void()>> queue{};
    std::mutex work_queue_lock{};
    std::condition_variable wait_for_queue{};

    // Function which pops task from queue when possible and execute function.
    [[noreturn]] void do_work();
};

#endif // PROJECT3_THREADPOOL_H
