#ifndef LIBNEST2D_JS_H
#define LIBNEST2D_JS_H
//Copyright (c) 2022 Ultimaker B.V.
//libnest2d_js is released under the terms of the LGPLv3 or higher.

// Emscripten Embind bindings for libnest2d
#include <libnest2d/libnest2d.hpp>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <libnest2d/backends/clipper/geometries.hpp>
#include <libnest2d/placers/bottomleftplacer.hpp>

using namespace emscripten;
using namespace libnest2d;
using namespace placers;

// Type aliases to match Python bindings
using Point = PointImpl;
using Box = _Box<Point>;
using Circle = _Circle<Point>;
using Item = _Item<PolygonImpl>;
using NfpConfig = NfpPConfig<PolygonImpl>;
using Polygon = PolygonImpl;

// Add aliases for angle types
using Radians = libnest2d::Radians;
using Degrees = libnest2d::Degrees;

// Declare value types for TypeScript generation
EMSCRIPTEN_DECLARE_VAL_TYPE(PointList);
EMSCRIPTEN_DECLARE_VAL_TYPE(ItemList);
EMSCRIPTEN_DECLARE_VAL_TYPE(DoubleList);
EMSCRIPTEN_DECLARE_VAL_TYPE(ResultAndItem);

void testFunction() {
    std::vector<Item> input;

    auto volume = libnest2d::Box(1000, 1000);

    std::vector<Item> items;

    items.emplace_back(libnest2d::Item({
                                               Point(5, 10),
                                               Point(10, 10),
                                               Point(0, 0)
                                       }));
    auto& long_thin_triangle = items.back();

    items.emplace_back(libnest2d::Item({
                                               Point(0, 10),
                                               Point(10, 10),
                                               Point(10, 0),
                                               Point(0, 0),
                                       }));
    auto& square = items.back();

    items.emplace_back(libnest2d::Item({
                                               Point(5, 10),
                                               Point(10, 0),
                                               Point(0, 0)
                                       }));
    auto& equilateral_triangle = items.back();

    auto num_bins = libnest2d::nest(items, volume);
    std::cout << "Number of bins used: " << num_bins << std::endl;
}

// Helper function to convert a Point to a JavaScript object
emscripten::val pointToJSObject(const Point& point) {
    emscripten::val obj = emscripten::val::object();
    obj.set("x", long{ static_cast<long>(getX(point)) } );
    obj.set("y", long{ static_cast<long>(getY(point)) } );
    return obj;
}

ResultAndItem resultAndItems(const size_t result, const ItemList& items) {
    emscripten::val obj = emscripten::val::object();
    obj.set("result", result);
    obj.set("items", items);
    return ResultAndItem { obj };
}
// Helper function to convert a vector of Points to a JavaScript array
emscripten::val pointVectorToJSArray(const std::vector<Point>& points) {
    emscripten::val jsArray = emscripten::val::array();
    for (size_t i = 0; i < points.size(); ++i) {
        jsArray.set(i, pointToJSObject(points[i]));
    }
    return jsArray;
}
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

// Wrapper function for nest() to handle JavaScript arrays
ResultAndItem nestWrapper(const ItemList& jsItems, const Box& bin) {
    // Convert JavaScript array to std::vector<Item>
    std::vector<Item> items;
    auto length = jsItems["length"].as<unsigned>();
    items.reserve(length);

    for (unsigned i = 0; i < length; i++) {
        Item item = jsItems[i].as<Item>();
        items.push_back(item);
    }

    size_t result = nest(items, bin);

    emscripten::val jsItemsResult = emscripten::val::array();
    // Copy results back to original JavaScript items
    for (size_t i = 0; i < items.size() && i < length; ++i) {
        jsItemsResult.set(i, items[i]);
    }

    return resultAndItems(result, ItemList(jsItemsResult));
}

EMSCRIPTEN_BINDINGS(libnest2d_js) {
    // Register TypeScript types for arrays
    emscripten::register_type<PointList>("Point[]");
    emscripten::register_type<ItemList>("Item[]");
    emscripten::register_type<DoubleList>("number[]");
    emscripten::register_type<ResultAndItem>("{ result: number, items: Item[] }");

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
        .constructor<long, long, Point>()
        .class_function("infinite", &Box::infinite)
        .class_function("fromDimensions", optional_override([](long width, long height) -> Box {
            return Box(width, height);
        }))
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


    // Item class
    class_<Item>("Item")
        .constructor<const PolygonImpl&>()
        .class_function("createFromVertices", optional_override([](const emscripten::val& jsVertices) -> Item {
            std::vector<Point> vertices = jsArrayToPointVector(jsVertices);
            PolygonImpl polygon;
            polygon.Contour = vertices;
            return Item(polygon);
        }))
        .function("binId", select_overload<int() const>(&Item::binId))
        .function("setBinId", select_overload<void(int)>(&Item::binId))
        .function("area", &Item::area)
        .function("vertexCount", &Item::vertexCount)
        .function("boundingBox", &Item::boundingBox)
        .function("translate", &Item::translate)
        .function("rotate", &Item::rotate)
        .function("rotation", optional_override([](const Item& self) { return self.rotation(); }))
        .function("translation", optional_override([](const Item& self) { return self.translation(); }))
        .function("isFixed", &Item::isFixed)
        .function("isDisallowedArea", &Item::isDisallowedArea)
        .function("markAsFixedInBin", &Item::markAsFixedInBin)
        .function("markAsDisallowedAreaInBin", &Item::markAsDisallowedAreaInBin)
        .function("priority", select_overload<int() const>(&Item::priority))
        .function("setPriority", select_overload<void(int)>(&Item::priority))
        .function("transformedShape", optional_override([](const Item& self) {
            const auto& poly = self.transformedShape();
            return pointVectorToJSArray(poly.Contour);
        }));


    // Polygon class for internal type compatibility
    class_<Polygon>("Polygon");

    // Radians class for rotation angles
    class_<Radians>("Radians")
        .constructor<double>()
        .function("toDegrees", &Radians::toDegrees);

    // Degrees class for rotation angles  
    class_<Degrees>("Degrees")
        .constructor<double>()
        .function("toRadians", &Degrees::toRadians);

    // register_vector for JavaScript array conversion
    register_vector<Item>("VectorItem");
    register_vector<Point>("VectorPoint");
    register_vector<double>("VectorDouble");

    // Main nest function
    function("nest", &nestWrapper);

    function("testFunction", &testFunction);
}

#endif // LIBNEST2D_JS_H
