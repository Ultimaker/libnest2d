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
    // Expose Point type
    value_object<Point>("Point")
        .field("x", &Point::x)
        .field("y", &Point::y);

    // Expose Polygon type
    value_object<Polygon>("Polygon")
        .field("points", &Polygon::points);

    // Expose NesterConfig
    value_object<NesterConfig>("NesterConfig")
        .field("iterations", &NesterConfig::iterations)
        .field("time_limit", &NesterConfig::time_limit);

    // Expose Nester class
    class_<Nester>("Nester")
        .constructor<>()
        .function("nest", &Nester::nest);

    // Expose OptimizerConfig
    value_object<OptimizerConfig>("OptimizerConfig")
        .field("max_evals", &OptimizerConfig::max_evals)
        .field("tolerance", &OptimizerConfig::tolerance);

    // Expose Optimizer class
    class_<Optimizer>("Optimizer")
        .constructor<>()
        .function("optimize", &Optimizer::optimize);

    // Expose utility functions (example)
    function("polygonArea", &polygon_area);
}

#endif // LIBNEST2D_JS_H
