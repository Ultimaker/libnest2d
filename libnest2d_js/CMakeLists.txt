message(STATUS "Building for Emscripten")
cmake_minimum_required(VERSION 3.10)
project(libnest2d_js)

add_executable(libnest2d_js libnest2d_js.cpp)

if (NOT CMAKE_CXX_PLATFORM_ID STREQUAL "emscripten")
    use_threads(libnest2d_js)
endif ()

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
target_link_libraries(libnest2d_js PRIVATE clipper::clipper)

target_link_libraries(libnest2d_js PRIVATE NLopt::nlopt)

target_link_libraries(libnest2d_js PUBLIC spdlog::spdlog)
# Define backend macro for Emscripten
target_compile_definitions(libnest2d_js PRIVATE LIBNEST2D_GEOMETRIES_clipper)
target_compile_definitions(libnest2d_js PRIVATE LIBNEST2D_OPTIMIZER_nlopt)

# Emscripten-specific options (optional, but recommended)
target_link_options(libnest2d_js
        PUBLIC
        "SHELL:-s USE_ES6_IMPORT_META=1"
        "SHELL:-s FORCE_FILESYSTEM=1"
        "SHELL:-s EXPORT_NAME=libnest2d_js"
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_ES6=1"
        "SHELL:-s SINGLE_FILE=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=0"
        "SHELL:--bind"
        "SHELL:-l embind"
        "SHELL: --emit-tsd libnest2d_js.d.ts"
        "SHELL:-sWASM_BIGINT=1"
        "SHELL:-sASSERTIONS=1"

)

# If you want to enable debug options, add them conditionally:
# $<$<CONFIG:Debug>:SHELL:-g3>
# $<$<CONFIG:Debug>:SHELL:-gsource-map>
