project("SkipList")
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads)

include_directories($(CMAKE_CURRENT_SOURCE_DIR)/source)
include_directories($(CMAKE_CURRENT_SOURCE_DIR)/source/skip_list)

add_executable(skip_list source/main.cpp)
target_link_libraries(skip_list Threads::Threads)
