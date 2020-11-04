//
// Created by parin on 10/17/20.
//

#ifndef PROJECT3_BIDCLIENT_H
#define PROJECT3_BIDCLIENT_H

#include "store.grpc.pb.h"
#include "threadpool.h"

class BidClient {

public:
    // init Bidclient and Assosiated variables and start client to procceed state
    BidClient(store::Store::AsyncService *service, grpc_impl::ServerCompletionQueue *pQueue);

    // Entrypoint for Client
    void Procceed();

    // Bind Thread pool class object
    void Assign_ThreadPool(threadpool *pool);

    threadpool *pool;

    // Bind Vendor Address to grpc channels to be used by bidclass
    void bind_address(std::vector<std::string> vendor_address);

private:
    store::Store::AsyncService *private_service;
    grpc::ServerCompletionQueue *private_queue;
    grpc::ServerContext private_context;

    store::ProductQuery client_request;
    store::ProductReply client_response;
    grpc::ServerAsyncResponseWriter<store::ProductReply> client;

    // Let's implement a tiny state machine with the following states.
    enum QueryStatus {
        CREATE,
        PROCESS,
        FINISH
    };
    QueryStatus status_; // The current serving state.

    bool Create();

    // create channels for communicting to vendors and create future task to query vendor
    // information/bid info (This part is creates object for Vendor Class to be used inside
    // threadpool) and pushes the task into threadpool list.
    bool Run();

    // Update state and cleanup
    void Done();

    // metavarible for race condition
    std::mutex vendor_lock{};
    std::condition_variable wait_for_vendor_lock{};

    std::vector<std::shared_ptr<grpc::Channel>> vendor_channels;

    // function which cordinates for Async request using VendorClass
    std::vector<VendorResponse::response> list_bid(std::string basicString);

    std::vector<VendorResponse *> connect_with_vendors();

    std::vector<VendorResponse::response> AsyncRequestQuery(std::string basicString);
};

#endif // PROJECT3_BIDCLIENT_H
