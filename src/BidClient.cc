//
// Created by parin on 10/17/20.
//

#include "BidClient.h"

#include <future>

#include "VendorResponse.h"
#include "threadpool.h"
#include "unistd.h"

thread_local std::vector<VendorResponse *> local_channels;

// https://stackoverflow.com/questions/14335161/anonymous-stdpackaged-task
template<typename Func, typename... Args>
auto threadpool::enqueue(Func &&f, Args &&... args) {
    // get return type of the function
    using RetType = std::invoke_result_t<Func, Args...>;

    auto task = std::make_shared<std::packaged_task<RetType()>>(
            [&f, &args...]() { return f(std::forward<Args>(args)...); });

    {

        // lock jobQueue mutex, add job to the job queue
        std::lock_guard<std::mutex> lock(threadpool::work_queue_lock);

        // place the job into the queue
        threadpool::queue.emplace([task]() { (*task)(); });
    }
    threadpool::wait_for_queue.notify_one();
    return task->get_future();
}

BidClient::BidClient(store::Store::AsyncService *service, grpc_impl::ServerCompletionQueue *pQueue)
        : private_service(service), private_queue(pQueue), client(&private_context), status_(CREATE) {
    BidClient::Procceed();
}

void BidClient::Assign_ThreadPool(threadpool *pool) {
    this->pool = pool;
    if (this->pool == nullptr) {
        std::cerr << "please assigned pool" << std::endl;
    }
}

void BidClient::Procceed() {

    if (this->status_ == CREATE) {
        if (!BidClient::Create()) {
            std::cerr << "Client Creation Failed" << std::endl;
        }
    } else if (this->status_ == PROCESS) {
        if (!BidClient::Run()) {
            std::cerr << "Client Run Failed" << std::endl;
        }
    } else {
        BidClient::Done();
    }
}

bool BidClient::Create() {
    private_service->RequestgetProducts(
            &private_context, &client_request, &client, private_queue, private_queue, this);
    this->status_ = PROCESS;
    return true;
}

void BidClient::bind_address(std::vector<std::string> vendor_address) {
    {
        std::unique_lock<std::mutex> vendor_l(vendor_lock);
        for (std::string address : vendor_address) {
            auto c = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
            this->vendor_channels.push_back(c);
        }
    }
    wait_for_vendor_lock.notify_all();
}

std::vector<VendorResponse *> BidClient::connect_with_vendors() {
    std::vector<VendorResponse *> t_c;

    for (std::shared_ptr<grpc::Channel> v : this->vendor_channels) {
        VendorResponse *d = new VendorResponse(v);
        t_c.push_back(d);
    }
    return t_c;
}

std::vector<VendorResponse::response> BidClient::list_bid(std::string basicString) {
    {
        std::unique_lock<std::mutex> vendor_l(vendor_lock);
        wait_for_vendor_lock.wait(vendor_l, [this] { return this->vendor_channels.size() != 0; });
        local_channels = BidClient::connect_with_vendors();
    }
    std::vector<VendorResponse::response> r{};
    VendorResponse::response d{};

    for (auto v : local_channels) {
        v->Async_Vendor_assemble_bid(basicString);
    }

    for (auto v : local_channels) {
        VendorResponse::response r1 = v->Async_vendor_response_bid();
        r.push_back(r1);
    }

    return r;
}

std::vector<VendorResponse::response> BidClient::AsyncRequestQuery(std::string basicString) {

    auto future_function = [this](std::string basicString) {
        return BidClient::list_bid(basicString);
    };
    auto d = this->pool->enqueue(future_function, basicString);

    std::vector<VendorResponse::response> r = d.get();
    return r;
}

bool BidClient::Run() {
    new BidClient(private_service, private_queue);
    if (this->pool == nullptr) {
        std::cerr << "Please Assgin Threadpool using Assign_ThreadPool(threadpool *pool)"
                  << std::endl;
    }
    std::string vender_query = client_request.product_name();
    //    auto future_response = this->pool->append_request(vender_query);

    std::vector<VendorResponse::response> vendor_responses;
    vendor_responses = BidClient::AsyncRequestQuery(vender_query);

    for (auto r : vendor_responses) {
        auto client_response_ = client_response.add_products();
        client_response_->set_price(r.price);
        client_response_->set_vendor_id(r.v_id);
    }
    this->status_ = FINISH;
    client.Finish(client_response, grpc::Status::OK, this);
    return true;
}

void BidClient::Done() {
    GPR_ASSERT(status_ == FINISH);
    delete this;
}
