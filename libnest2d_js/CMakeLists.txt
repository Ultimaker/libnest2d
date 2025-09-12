cmake_minimum_required(VERSION 3.10)
project(libnest2d_js)

add_library(libnest2d_js STATIC libnest2d_js.cpp)

# Emscripten bindings
set_target_properties(libnest2d_js PROPERTIES LINK_FLAGS "--bind")

# Include directories
target_include_directories(libnest2d_js PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Link dependencies if needed
# target_link_libraries(libnest2d_js <dependencies>)

