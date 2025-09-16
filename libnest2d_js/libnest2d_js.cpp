#ifndef LIBNEST2D_JS_H
#define LIBNEST2D_JS_H
//Copyright (c) 2022 Ultimaker B.V.
//libnest2d_js is released und        .function("setY", optional_override([](Point& self, long value) { setY(self, value); }))r the terms of the LGPLv3 or higher.

// Emscripten Embind bindings for libnest2d
#include <libnest2d/libnest2d.hpp>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <libnest2d/backends/clipper/geometries.hpp>
#include <libnest2d/optimizers/nlopt/subplex.hpp>

using namespace emscripten;
using namespace libnest2d;
using namespace placers;

// Type aliases to match Python bindings
using Point = PointImpl;
using Box = _Box<Point>;
using Circle = _Circle<Point>;
using Item = _Item<PolygonImpl>;
using NfpConfig = NfpPConfig<PolygonImpl>;
using BottomLeftConfig = BLConfig<PolygonImpl>;
using DJDHeuristicConfig = DJDHeuristic::Config;

// Declare value types for TypeScript generation
EMSCRIPTEN_DECLARE_VAL_TYPE(PointList);
EMSCRIPTEN_DECLARE_VAL_TYPE(ItemList);
EMSCRIPTEN_DECLARE_VAL_TYPE(DoubleList);

// Helper function to convert JavaScript arrays to std::vector<Point>
std::vector<Point> jsArrayToPointVector(const emscripten::val& jsArray) {
    std::vector<Point> vertices;
    unsigned length = jsArray["length"].as<unsigned>();
    vertices.reserve(length);
    
    for (unsigned i = 0; i < length; i++) {
        emscripten::val jsPoint = jsArray[i];
        // Use property access instead of method calls for better compatibility
        long x = jsPoint["x"].as<long>();
        long y = jsPoint["y"].as<long>();
        vertices.emplace_back(x, y);
    }
    
    return vertices;
}

// Helper function to convert std::vector<double> to JavaScript array
emscripten::val vectorDoubleToJsArray(const std::vector<double>& vec) {
    emscripten::val jsArray = emscripten::val::array();
    for (size_t i = 0; i < vec.size(); ++i) {
        jsArray.call<void>("push", vec[i]);
    }
    return jsArray;
}

// Helper function to convert JavaScript array to std::vector<double>
std::vector<double> jsArrayToVectorDouble(const emscripten::val& jsArray) {
    std::vector<double> vec;
    unsigned length = jsArray["length"].as<unsigned>();
    vec.reserve(length);
    
    for (unsigned i = 0; i < length; i++) {
        vec.push_back(jsArray[i].as<double>());
    }
    
    return vec;
}

// Wrapper function for nest() to handle JavaScript arrays
long nestWrapper(emscripten::val jsItems, const Box& bin, long distance = 1, const NfpConfig& config = NfpConfig()) {
    // Convert JavaScript array to std::vector<Item>
    std::vector<Item> items;
    unsigned length = jsItems["length"].as<unsigned>();
    items.reserve(length);
    
    for (unsigned i = 0; i < length; i++) {
        Item item = jsItems[i].as<Item>();
        items.push_back(item);
    }
    
    // Pre-process distance
    if (distance <= 0) {
        distance = 1;
    }
    
    // Create nest config
    NestConfig<> nestConfig(config);
    
    // Call the nest function
    long result = nest(items, bin, distance, nestConfig);
    
    // Copy results back to original JavaScript items
    for (size_t i = 0; i < items.size() && i < length; ++i) {
        jsItems.set(i, val(items[i]));
    }
    
    return result;
}

EMSCRIPTEN_BINDINGS(libnest2d_js) {
    // Register TypeScript types for arrays
    emscripten::register_type<PointList>("Point[]");
    emscripten::register_type<ItemList>("Item[]");
    emscripten::register_type<DoubleList>("number[]");

    // Point class - fix the getter/setter type issue
    class_<Point>("Point")
        .constructor<>()
        .constructor<long, long>()
        .function("x", optional_override([](const Point& self) -> long { return getX(self); }))
        .function("y", optional_override([](const Point& self) -> long { return getY(self); }))
        .function("setX", optional_override([](Point& self, long value) { setX(self, value); }))
        .function("setY", optional_override([](Point& self, long value) { setY(self, value); }))
        ;

    // Box class
    class_<Box>("Box")
        .constructor<Point>()
        .constructor<Point, Point>()
        .constructor<long, long>()
        .constructor<long, long, Point>()
        .class_function("infinite", &Box::infinite)
        .function("minCorner", select_overload<const Point&() const>(&Box::minCorner))
        .function("maxCorner", select_overload<const Point&() const>(&Box::maxCorner))
        .function("width", &Box::width)
        .function("height", &Box::height)
        .function("area", &Box::area<double>)
        .function("center", select_overload<Point() const>(&Box::center))
        ;

    // Circle class
    class_<Circle>("Circle")
        .constructor<>()
        .constructor<Point, double>()
        .function("center", select_overload<const Point&() const>(&Circle::center))
        .function("setCenter", select_overload<void(const Point&)>(&Circle::center))
        .function("radius", select_overload<double() const>(&Circle::radius))
        .function("setRadius", select_overload<void(double)>(&Circle::radius))
        .function("area", &Circle::area)
        ;

    // NfpConfig::Alignment enum
    enum_<NfpConfig::Alignment>("Alignment")
        .value("CENTER", NfpConfig::Alignment::CENTER)
        .value("BOTTOM_LEFT", NfpConfig::Alignment::BOTTOM_LEFT)
        .value("BOTTOM_RIGHT", NfpConfig::Alignment::BOTTOM_RIGHT)
        .value("TOP_LEFT", NfpConfig::Alignment::TOP_LEFT)
        .value("TOP_RIGHT", NfpConfig::Alignment::TOP_RIGHT)
        .value("DONT_ALIGN", NfpConfig::Alignment::DONT_ALIGN)
        ;

    // NfpConfig class
    emscripten::value_object<NfpConfig>("NfpConfig")
        .field("alignment", &NfpConfig::alignment)
        .field("starting_point", &NfpConfig::starting_point)
        .field("accuracy", &NfpConfig::accuracy)
        .field("explore_holes", &NfpConfig::explore_holes)
        .field("parallel", &NfpConfig::parallel)
        ;

    // BottomLeftConfig class
    emscripten::value_object<BottomLeftConfig>("BottomLeftConfig")
        .field("min_obj_distance", &BottomLeftConfig::min_obj_distance)
        .field("epsilon", &BottomLeftConfig::epsilon)
        .field("allow_rotations", &BottomLeftConfig::allow_rotations)
        ;

    // DJDHeuristicConfig class
    emscripten::value_object<DJDHeuristicConfig>("DJDHeuristicConfig")
        .field("try_reverse_order", &DJDHeuristicConfig::try_reverse_order)
        .field("try_pairs", &DJDHeuristicConfig::try_pairs)
        .field("try_triplets", &DJDHeuristicConfig::try_triplets)
        .field("initial_fill_proportion", &DJDHeuristicConfig::initial_fill_proportion)
        .field("waste_increment", &DJDHeuristicConfig::waste_increment)
        .field("allow_parallel", &DJDHeuristicConfig::allow_parallel)
        .field("force_parallel", &DJDHeuristicConfig::force_parallel)
        ;

    // Item class
    class_<Item>("Item")
        .constructor<const PolygonImpl&>()
        .class_function("createFromVertices", optional_override([](const emscripten::val& jsVertices) -> Item* {
            std::vector<Point> vertices = jsArrayToPointVector(jsVertices);
            PolygonImpl polygon;
            polygon.Contour = vertices;
            return new Item(polygon);
        }), allow_raw_pointers())
        .function("binId", select_overload<int() const>(&Item::binId))
        .function("setBinId", select_overload<void(int)>(&Item::binId))
        .function("area", &Item::area)
        .function("vertexCount", &Item::vertexCount)
        .function("boundingBox", &Item::boundingBox)
        .function("translate", &Item::translate)
        .function("rotate", &Item::rotate)
        ;

    // register_vector for JavaScript array conversion
    register_vector<Item>("VectorItem");
    register_vector<Point>("VectorPoint");
    register_vector<double>("VectorDouble");

    // Main nest function
    function("nest", &nestWrapper);
}

#endif // LIBNEST2D_JS_H