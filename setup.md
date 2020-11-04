# Setup 

This document describes the steps for installing the dependencies.
You will be able to use the system as is for Project 4 as well.
We wll be primarily relying on `vcpkg` for all our dependencies.

## Dependencies
1. [`cmake`](https://cmake.org/download)                                                - For building C/C++ applications 3.10+
2. [`vcpkg`](https://github.com/microsoft/vcpkg)                                        - Package Manager for C/C++ libraries
3. [`protobuf`](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)  - Google Protocol Buffers
4. [`gRPC`](https://github.com/grpc/grpc/blob/master/src/cpp/README.md)                 - Google's RPC framework

## Steps
The steps below are for an Ubuntu 18.04 LTS system. 
You may need to modify these steps to work on other Linux distros.

`sudo apt-get install -y unzip build-essential`

`git clone https://github.com/microsoft/vcpkg`

`cd vcpkg/`

This next step will install cmake and ninja build system and configure the vcpkg binary. If you intend to use a version of cmake which is installed on your system, specify `-useSystemBinaries`

`./bootstrap-vcpkg.sh -disableMetrics` 

`./vcpkg search grpc` -> to search for the grpc package

`./vcpkg install grpc` -> this may take time for some.

### Skip this if you already have system cmake
`vcpkg` downloads cmake for you, but you will need to modify and export the `PATH` variable to point to it.

`export PATH=$PATH:$HOME/vcpkg/downloads/tools/cmake-3.14.0-linux/cmake-3.14.0-Linux-x86_64/bin`

## Build
**Important:** Modify `VCPKG_HOME` variable in the `project3/CMakeLists.txt`, to where your installation of the `vcpkg` exists.
    $ cd project3/
    $ rm -rf build; mkdir build
    $ cd build
    $ cmake -DVCPKG_TARGET_TRIPLET=x64-linux ..
    $ make

This will generate all the relevant binaries under the `build/bin` directory, namely `store`, `run_vendors` and `run_tests`.
