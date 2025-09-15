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

}

#endif // LIBNEST2D_JS_H
