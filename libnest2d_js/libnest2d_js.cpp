#ifndef LIBNEST2D_JS_H
#define LIBNEST2D_JS_H
//Copyright (c) 2022 Ultimaker B.V.
//libnest2d_js is released under the terms of the LGPLv3 or higher.

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
        long x = jsPoint.call<long>("x");
        long y = jsPoint.call<long>("y");
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

// Helper function to convert std::vector<Radians> to JavaScript array
emscripten::val vectorRadiansToJsArray(const std::vector<Radians>& vec) {
    emscripten::val jsArray = emscripten::val::array();
    for (size_t i = 0; i < vec.size(); ++i) {
        jsArray.call<void>("push", static_cast<double>(vec[i]));
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

// Helper function to convert JavaScript array to std::vector<Radians>
std::vector<Radians> jsArrayToVectorRadians(const emscripten::val& jsArray) {
    std::vector<Radians> vec;
    unsigned length = jsArray["length"].as<unsigned>();
    vec.reserve(length);
    
    for (unsigned i = 0; i < length; i++) {
        vec.push_back(Radians(jsArray[i].as<double>()));
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

    // Point class
    emscripten::value_object<Point>("Point")
        .field("x", emscripten::optional_override([](const Point& self) { return self.X; }),
               emscripten::optional_override([](Point& self, long value) { self.X = value; }))
        .field("y", emscripten::optional_override([](const Point& self) { return self.Y; }),
               emscripten::optional_override([](Point& self, long value) { self.Y = value; }));

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
        .field("rotations", emscripten::optional_override(
            [](const NfpConfig& self) { return vectorRadiansToJsArray(self.rotations); }),
            emscripten::optional_override(
                [](NfpConfig& self, const emscripten::val& jsArray) { 
                    self.rotations = jsArrayToVectorRadians(jsArray); 
                }));

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
        .function("isFixed", &Item::isFixed)
        .function("isDisallowedArea", &Item::isDisallowedArea)
        .function("markAsFixedInBin", &Item::markAsFixedInBin)
        .function("markAsDisallowedAreaInBin", &Item::markAsDisallowedAreaInBin)
        .function("priority", select_overload<int() const>(&Item::priority))
        .function("setPriority", select_overload<void(int)>(&Item::priority))
        .function("toString", &Item::toString)
        .function("vertex", &Item::vertex)
        .function("setVertex", &Item::setVertex)
        .function("area", &Item::area)
        .function("isContourConvex", &Item::isContourConvex)
        .function("isHoleConvex", &Item::isHoleConvex)
        .function("areHolesConvex", &Item::areHolesConvex)
        .function("vertexCount", &Item::vertexCount)
        .function("holeCount", &Item::holeCount)
        .function("isInside", select_overload<bool(const Point&) const>(&Item::isInside))
        .function("isInsideItem", select_overload<bool(const Item&) const>(&Item::isInside))
        .function("isInsideBox", select_overload<bool(const Box&) const>(&Item::isInside))
        .function("isInsideCircle", select_overload<bool(const Circle&) const>(&Item::isInside))
        .function("boundingBox", &Item::boundingBox)
        .function("referenceVertex", &Item::referenceVertex)
        .function("rightmostTopVertex", &Item::rightmostTopVertex)
        .function("leftmostBottomVertex", &Item::leftmostBottomVertex)
        .function("translate", &Item::translate)
        .function("rotate", &Item::rotate)
        .function("transformedShape", &Item::transformedShape)
        .function("resetTransformation", &Item::resetTransformation)
        .class_function("intersects", &Item::intersects)
        .class_function("touches", &Item::touches)
        ;

    // Rectangle class (inherits from Item)
    class_<Rectangle, base<Item>>("Rectangle")
        .constructor<long, long>()
        .function("width", &Rectangle::width)
        .function("height", &Rectangle::height)
        // Inherited methods from Item are automatically available
        ;

    // register_vector for JavaScript array conversion
    register_vector<Item>("VectorItem");
    register_vector<Point>("VectorPoint");
    register_vector<double>("VectorDouble");

    // Main nest function
    function("nest", &nestWrapper);
}

#endif // LIBNEST2D_JS_H