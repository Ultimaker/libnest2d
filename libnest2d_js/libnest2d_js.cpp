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

        // Expose nest function from libnest2d
        function("nest",
            select_overload<std::size_t(std::vector<Item>::iterator,
                                        std::vector<Item>::iterator,
                                        const Box&,
                                        Coord,
                                        const NestConfig<NfpPlacer, FirstFitSelection>&,
                                        NestControl)>(
                &nest<NfpPlacer, FirstFitSelection, std::vector<Item>::iterator>
            )
        );

    // Expose StopCriteria as OptimizerConfig
    value_object<opt::StopCriteria>("OptimizerConfig")
        .field("max_iterations", &opt::StopCriteria::max_iterations)
        .field("absolute_score_difference", &opt::StopCriteria::absolute_score_difference)
        .field("relative_score_difference", &opt::StopCriteria::relative_score_difference)
        .field("stop_score", &opt::StopCriteria::stop_score);

    // Expose Optimizer (template, so use NloptOptimizer)
    class_<opt::Optimizer<opt::NloptOptimizer>>("Optimizer")
        .constructor<const opt::StopCriteria&>()
        .function("optimize_min",
            optional_override([](opt::Optimizer<opt::NloptOptimizer>& self,
                                std::function<double(opt::Input<double>)> func,
                                opt::Input<double> init,
                                opt::Bound<double> bound) {
                return self.optimize_min(func, init, bound);
            })
        );
}

#endif // LIBNEST2D_JS_H
