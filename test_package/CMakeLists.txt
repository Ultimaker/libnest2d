cmake_minimum_required(VERSION 3.13)

project(Libnest2D_example)
set(CMAKE_CXX_STANDARD 17)
find_package(libnest2d REQUIRED)

add_executable(test main.cpp)

target_link_libraries(test PRIVATE libnest2d::libnest2d)
target_include_directories(test PRIVATE ${libnest2d_INCLUDE_DIRS})