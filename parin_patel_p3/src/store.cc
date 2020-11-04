#include <cmath>
#include <fstream>
#include <grpc++/grpc++.h>
#include <iostream>
#include <string>
#include <vector>

#include "BidClient.h"
#include "store.grpc.pb.h"
#include "threadpool.h"

#define USAGE                                                                                      \
    "./store <filepath for vendor addresses> \
        <port to listen on for clients> \
        <maximum number of threads in threadpool>"

class AsyncStore {
public:
    ~AsyncStore() {
        this->shutdown();
    }

    AsyncStore(const std::string &path, int port) {
        read_vender_address(path.c_str());
        if (port > pow(2, 16) || port < 1) {
            std::cerr << "Port , " << port << " is not between 1 and " << pow(2, 16) << std::endl;
            exit(EXIT_FAILURE);
        } else
            server_address = "0.0.0.0:" + std::to_string(port);
    }

    void run(threadpool *pthreadpool) {
        this->pool = pthreadpool;
        if (this->pool == nullptr) {
            std::cerr << "NULLLLLLL pool" << std::endl;
        }
        start_server();
    }

private:
    std::vector<std::string> vendor_address;
    std::string server_address;
    store::Store::AsyncService service_;
    std::unique_ptr<grpc::ServerCompletionQueue> queue;
    std::unique_ptr<grpc::Server> server_;
    threadpool *pool;

    /**
     * Read Provided Vendor Address file
     * */
    void read_vender_address(const char *const vendor_file_path) {
        std::fstream vector_file;
        vector_file.open(vendor_file_path, std::ios::in);
        if (vector_file.is_open()) {
            std::string l;
            while (std::getline(vector_file, l)) {
                vendor_address.push_back(l);
            }
            vector_file.close();
        } else {
            perror("given vector file is not available. open failed");
            exit(EXIT_FAILURE);
        }
    }

    void start_server() {
        grpc::ServerBuilder builder;

        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        queue = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        HandleRpcs();
    }

    void shutdown() {
        server_->Shutdown();
        queue->Shutdown();
    }

    void HandleRpcs() {
        new BidClient(&service_, queue.get());
        void *tag;
        bool ok;
        while (true) {
            GPR_ASSERT(queue->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<BidClient *>(tag)->Assign_ThreadPool(pool);
            static_cast<BidClient *>(tag)->bind_address(vendor_address);
            static_cast<BidClient *>(tag)->Procceed();
        }
    }
};

int main(int argc, char **argv) {

    if (argc != 4 || (std::string) "-h" == argv[1] || (std::string) "--help" == argv[1]) {
        std::cout << USAGE << std::endl;
        exit(EXIT_SUCCESS);
    }
    std::string file_path;
    int listen_port, max_threads;
    if (argc == 4) {
        file_path = argv[1];
        listen_port = atoi(argv[2]);
        max_threads = atoi(argv[3]);
    }

    AsyncStore asyncStore{file_path, listen_port};
    threadpool pool{max_threads};

    //    asyncStore.bind_threadpool(&pool);
    asyncStore.run(&pool);
    return EXIT_SUCCESS;
}
