// Expose libnest2d API to JavaScript via Emscripten
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
    // Expose ClipperLib::IntPoint as Point
    value_object<ClipperLib::IntPoint>("Point")
        .field("x", &ClipperLib::IntPoint::X)
        .field("y", &ClipperLib::IntPoint::Y);

    // Expose ClipperLib::Polygon
    value_object<ClipperLib::Polygon>("Polygon")
        .field("contour", &ClipperLib::Polygon::Contour)
        .field("holes", &ClipperLib::Polygon::Holes);

    // Expose NestConfig (template, so use default types)
    value_object<NestConfig<>>("NestConfig")
        .field("placer_config", &NestConfig<>::placer_config);

    // Expose _Nester (template, so use default types)
    class_<_Nester<NfpPlacer, FirstFitSelection>>("Nester")
        .constructor<>()
        .function("nest", &_Nester<NfpPlacer, FirstFitSelection>::nest);

    // Expose StopCriteria as OptimizerConfig
    value_object<opt::StopCriteria>("OptimizerConfig")
        .field("max_iterations", &opt::StopCriteria::max_iterations)
        .field("absolute_score_difference", &opt::StopCriteria::absolute_score_difference)
        .field("relative_score_difference", &opt::StopCriteria::relative_score_difference)
        .field("stop_score", &opt::StopCriteria::stop_score);

    // Expose Optimizer (template, so use NloptOptimizer)
    class_<opt::Optimizer<opt::NloptOptimizer>>("Optimizer")
        .constructor<const opt::StopCriteria&>()
        .function("optimize_min", &opt::Optimizer<opt::NloptOptimizer>::optimize_min<double>);
}

#endif // LIBNEST2D_JS_H
