cmake_minimum_required(VERSION 3.15)
project(DHT_client)

set(CMAKE_CXX_STANDARD 14)

add_executable(DHT_new client.cpp messageType.h server.cpp)