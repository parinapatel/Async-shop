# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
set (VCPKG_HOME "$ENV{HOME}/vcpkg") #IMPORTANT: change this to where your vcpkg exists.
set (CMAKE_TOOLCHAIN_FILE "/home/parin/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake")
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Debug)

project(project3)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(test)
