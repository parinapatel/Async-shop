//
// Created by parin on 10/17/20.
//

#ifndef PROJECT3_VENDORRESPONSE_H
#define PROJECT3_VENDORRESPONSE_H

// https://chromium.googlesource.com/external/github.com/grpc/grpc/+/chromium-deps/2016-07-19/examples/cpp/helloworld/greeter_async_client2.cc
#include <grpcpp/grpcpp.h>
#include <memory>
#include <protos/vendor.grpc.pb.h>
#include <string>

class VendorResponse {
public:
    // Data structure to hold incoming vendor data.
    struct response {
        double price{};
        std::string v_id;
        std::string product;
    };

    explicit VendorResponse(std::shared_ptr<grpc::Channel> ptr)
            : vendor_stub(vendor::Vendor::NewStub(ptr)) {};

    // Ask vendor for bid and move forward without waiting for response
    void Async_Vendor_assemble_bid(std::string bidQuery);

    //  Wait for response to come and then return request.
    response Async_vendor_response_bid();

private:
    std::unique_ptr<vendor::Vendor::Stub> vendor_stub;
    grpc::CompletionQueue grpc_client_queue;

    // struct for keeping state and data information
    struct AsyncClientCall {

        vendor::BidQuery query;
        // Container for the data we expect from the server.
        vendor::BidReply reply;
        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;
        // Storage for the status of the RPC upon completion.
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<vendor::BidReply>> response_reader;
    };
};

#endif // PROJECT3_VENDORRESPONSE_H
