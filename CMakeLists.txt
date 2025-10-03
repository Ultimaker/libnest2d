cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)
project(libnest2d)
find_package(standardprojectsettings REQUIRED)
find_package(spdlog REQUIRED)

option(BUILD_SHARED_LIBS "Build shared libs instead of static (applies for dependencies as well)" OFF)
option(HEADER_ONLY "If enabled static library will not be built." ON)
option(ENABLE_TESTING "Build with Google unittest" OFF)
option(WITH_JS_BINDINGS "Build JS/WASM bindings for npm package" OFF)
set(GEOMETRIES clipper CACHE STRING "Geometry backend, available options: 'clipper' (default), 'boost'")
set(OPTIMIZER nlopt CACHE STRING "Optimization backend, available options: 'nlopt' (default), 'optimlib'")
set(THREADING std CACHE STRING "Multithreading, available options: 'std' (default), 'tbb', 'omp', 'none'")

add_library(project_options INTERFACE)
set_project_warnings(project_options)

set(nest2d_HDRS
        include/libnest2d/libnest2d.hpp
        include/libnest2d/nester.hpp
        include/libnest2d/geometry_traits.hpp
        include/libnest2d/geometry_traits_nfp.hpp
        include/libnest2d/common.hpp
        include/libnest2d/optimizer.hpp
        include/libnest2d/parallel.hpp
        include/libnest2d/utils/metaloop.hpp
        include/libnest2d/utils/rotfinder.hpp
        include/libnest2d/utils/rotcalipers.hpp
        include/libnest2d/utils/bigint.hpp
        include/libnest2d/utils/rational.hpp
        include/libnest2d/utils/boost_alg.hpp
        include/libnest2d/placers/placer_boilerplate.hpp
        include/libnest2d/placers/bottomleftplacer.hpp
        include/libnest2d/placers/nfpplacer.hpp
        include/libnest2d/selections/selection_boilerplate.hpp
        include/libnest2d/selections/filler.hpp
        include/libnest2d/selections/firstfit.hpp
        include/libnest2d/selections/djd_heuristic.hpp
        )

if("${GEOMETRIES}" STREQUAL "clipper")
    find_package(clipper REQUIRED)
    find_package(Boost REQUIRED)
    target_link_libraries(project_options INTERFACE clipper::clipper boost::boost)
    list(APPEND nest2d_HDRS
            include/libnest2d/clipper/clipper_polygon.hpp
            include/libnest2d/clipper/geometries.hpp
            )
elseif("${GEOMETRIES}" STREQUAL "boost")
    find_package(Boost REQUIRED)
    target_link_libraries(project_options INTERFACE boost::boost)
else()
    message(FATAL_ERROR "Unknown GEOMETRIES: ${GEOMETRIES} specified; use one of the following: 'clipper' (default), 'boost'")
endif()
target_compile_definitions(project_options INTERFACE LIBNEST2D_GEOMETRIES_${GEOMETRIES})

if("${OPTIMIZER}" STREQUAL "nlopt")
    find_package(NLopt REQUIRED)
    target_link_libraries(project_options INTERFACE NLopt::nlopt spdlog::spdlog)
    list(APPEND nest2d_HDRS
            include/libnest2d/optimizers/nlopt/simplex.hpp
            include/libnest2d/optimizers/nlopt/subplex.hpp
            include/libnest2d/optimizers/nlopt/genetic.hpp
            include/libnest2d/optimizers/nlopt/nlopt_boilerplate.hpp
            )
elseif("${OPTIMIZER}" STREQUAL "optimlib")
    find_package(armadillo REQUIRED)
    target_link_libraries(project_options INTERFACE armadillo::armadillo)
    list(APPEND nest2d_HDRS
            include/libnest2d/optimizers/optimlib/particleswarm.hpp
            )
else()
    message(FATAL_ERROR "Unknown OPTIMIZER: ${OPTIMIZER} specified; use one of the following: 'nlopt' (default), 'optimlib'")
endif()
target_compile_definitions(project_options INTERFACE LIBNEST2D_OPTIMIZER_${OPTIMIZER})

if("${THREADING}" STREQUAL "std")
    use_threads(project_options)
elseif("${THREADING}" STREQUAL "tbb")
    find_package(TBB REQUIRED)
    target_link_libraries(project_options INTERFACE tbb::tbb)
    target_compile_definitions(project_options INTERFACE TBB_USE_CAPTURED_EXCEPTION)
elseif("${THREADING}" STREQUAL "omp")
    find_package(OpenMP REQUIRED)
    target_link_libraries(project_options INTERFACE OpenMP::OpenMP)
elseif("${THREADING}" STREQUAL "none")
    message(WARNING "Compiling without threading support")
else()
    message(FATAL_ERROR "Unknown OPTIMIZER: ${OPTIMIZER} specified; use one of the following: 'nlopt' (default), 'optimlib'")
endif()
target_compile_definitions(project_options INTERFACE LIBNEST2D_THREADING_${THREADING})

set(libnest2d_SRCS
    src/libnest2d.cpp
)

if(WITH_JS_BINDINGS OR EMSCRIPTEN)
    add_subdirectory(libnest2d_js)
endif()


if(HEADER_ONLY)
    add_library(nest2d INTERFACE ${libnest2d_HDRS})
    target_link_libraries(nest2d INTERFACE project_options)
    target_include_directories(nest2d
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
else()
    if(BUILD_SHARED_LIBS)
        add_library(nest2d SHARED ${libnest2d_SRCS} ${libnest2d_HDRS})
        if(WIN32)
            set_target_properties(nest2d PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
        endif()
    else()
        add_library(nest2d STATIC ${libnest2d_SRCS} ${libnest2d_HDRS})
    endif()
    target_link_libraries(nest2d PUBLIC project_options)
    target_include_directories(nest2d
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    )
endif()

if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
