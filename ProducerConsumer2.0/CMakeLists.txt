project("ProducerConsumer2.0")
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads)

include_directories($(CMAKE_CURRENT_SOURCE_DIR)/source)

add_executable(producer_consumer2 source/main.cpp)
target_link_libraries(producer_consumer2 Threads::Threads)
