cmake_minimum_required(VERSION 3.25)
project(3170104535Lab7)

set(CMAKE_CXX_STANDARD 17)

add_executable(3170104535Lab7 client/client_main.cpp cmake-build-debug/client/func.cpp
        client/func.cpp
        server/server_main.cpp)

add_executable(client/client_main.cpp)