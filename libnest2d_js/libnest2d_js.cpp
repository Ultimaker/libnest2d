#ifndef LIBNEST2D_JS_H
#define LIBNEST2D_JS_H

#include <libnest2d/libnest2d.hpp>
#include <libnest2d/nester.hpp>
#include <libnest2d/optimizer.hpp>
#include <libnest2d/geometry_traits.hpp>
#include <emscripten/bind.h>
#include <vector>
#include <string>

using namespace libnest2d;
using namespace emscripten;

EMSCRIPTEN_BINDINGS(libnest2d_js) {
    // Register vector types for JS
    register_vector<Item>("ItemVector");
    register_vector<Box>("BoxVector");
    register_vector<PackGroup>("PackGroupVector");
    register_vector<Coord>("CoordVector");

    // Register main types
    emscripten::register_type<Box>("Box");
    emscripten::register_type<NestConfig<NfpPlacer, FirstFitSelection>>("NestConfig");
    emscripten::register_type<NestControl>("NestControl");

    emscripten::register_optional<Box>();
    emscripten::register_optional<NestConfig<NfpPlacer, FirstFitSelection>>();
    emscripten::register_optional<NestControl>();

    // Provide a wrapper for nest that takes a vector<Item> directly
    function("nestItems", optional_override([](std::vector<Item>& items,
                                               const Box& bin,
                                               Coord spacing,
                                               const NestConfig<NfpPlacer, FirstFitSelection>& config,
                                               NestControl control) {
        return nest<NfpPlacer, FirstFitSelection>(items.begin(), items.end(), bin, spacing, config, control);
    }));
}

#endif // LIBNEST2D_JS_H