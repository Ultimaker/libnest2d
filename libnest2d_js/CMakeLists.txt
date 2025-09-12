message(STATUS "Building for Emscripten")
cmake_minimum_required(VERSION 3.10)
project(libnest2d_js)

add_executable(libnest2d_js libnest2d_js.cpp)

# Emscripten bindings
set_target_properties(libnest2d_js PROPERTIES LINK_FLAGS "--bind")

# Find Clipper library (required for polyclipping/clipper.hpp)
find_package(clipper REQUIRED)

# Find Boost library (required for boost/geometry.hpp)
find_package(Boost REQUIRED)

find_package(NLopt REQUIRED)

# Include directories
target_include_directories(libnest2d_js PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${clipper_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Link Boost headers
target_link_libraries(libnest2d_js PRIVATE Boost::headers)

# Link Clipper library
target_link_libraries(libnest2d_js PRIVATE clipper)

target_link_libraries(libnest2d_js PRIVATE NLopt::nlopt)
# Define backend macro for Emscripten
target_compile_definitions(libnest2d_js PRIVATE LIBNEST2D_GEOMETRIES_clipper)
target_compile_definitions(libnest2d_js PRIVATE LIBNEST2D_OPTIMIZER_nlopt)



# Emscripten-specific options (optional, but recommended)
target_link_options(libnest2d_js
    PUBLIC
    "SHELL:-lembind"
    "SHELL:-sEXPORTED_RUNTIME_METHODS=[FS]"
    "SHELL:-sENVIRONMENT=web"
    "SHELL:-sMODULARIZE=1"
    "SHELL:-sEXPORT_ES6=1"
    "SHELL:-sWASMFS=1"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sERROR_ON_UNDEFINED_SYMBOLS=0"
)

# If you want to enable debug options, add them conditionally:
# $<$<CONFIG:Debug>:SHELL:-g3>
# $<$<CONFIG:Debug>:SHELL:-gsource-map>

