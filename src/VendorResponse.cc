//
// Created by parin on 10/17/20.
//

#include "VendorResponse.h"

struct VendorResponse::response VendorResponse::Async_vendor_response_bid() {
    vendor::BidReply r;
    struct VendorResponse::response Vendor_Response_Obj
            {
            };
    void *got_tag;
    bool ok = false;

    grpc_client_queue.Next(&got_tag, &ok);
    auto *call = static_cast<AsyncClientCall *>(got_tag);

    GPR_ASSERT(ok);

    if (call->status.ok()) {

        r = call->reply;
        Vendor_Response_Obj.v_id = r.vendor_id();
        Vendor_Response_Obj.price = r.price();
        Vendor_Response_Obj.product = call->query.product_name().c_str();
    } else {
        std::cerr << "something failed in RPC" << std::endl;
    }
    delete call;

    return Vendor_Response_Obj;
}

void VendorResponse::Async_Vendor_assemble_bid(std::string bidQuery) {
    vendor::BidQuery r;
    r.set_product_name(bidQuery.c_str());

    auto call = new AsyncClientCall;
    call->query = r;
//    std::cout << "making async query to vendors" << r.product_name() << std::endl;

    call->response_reader = vendor_stub->AsyncgetProductBid(&call->context, r, &grpc_client_queue);

    call->response_reader->Finish(&call->reply, &call->status, (void *) call);
}
