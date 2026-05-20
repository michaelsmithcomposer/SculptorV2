#pragma once

#include <Geode/Geode.hpp>
#include "../lib/Geometry.hpp"


using namespace geode::prelude;
using namespace Sculptor;

const float gdUnit = 30.f;

class Layer;

enum class GDProperty {
    ID,
    X,
    Y,
    Rotation,
    ScaleX,
    ScaleY,
    ZLayer,
    Color,
    SecondaryColor,
    Hue,
    Saturation,
    Value,
    Groups
};

class GDProperties {
public:

    std::unordered_map<GDProperty, std::any> properties;

    GDProperties(std::unordered_map<GDProperty, std::any> properties) {
        this->properties = properties;
    }    

    GameObject* applyTo(GameObject* object) const;

    void applyGenericLayerProperties(Layer* layer);

    static GDProperties fromRightTriangle(const RightTriangle& triangle, bool inflateEpsilon = false);

    static std::vector<GDProperties> fromRightTriangleInterior(const RightTriangle& triangle);

    static std::vector<GDProperties> fromRightTriangleExterior(const RightTriangle& triangle);

    static GDProperties fromLine(const Line& line, float width);

    static GDProperties fromCircle(const Circle& circle);

    
};






inline std::unordered_map<std::string, int> nameID = {
    {"block", 1},
    {"square-unit", 211},
    {"square-half", 916},
    {"square-quarter", 917},
    {"circle-unit", 3621},
    {"circle-big", 3637},
    {"circle-quarter", 1764},
    {"line", 1753},
    {"line-thick", 1756},
    {"glow-unit", 1888},
    {"glow-half", 1886},
    {"glow-quarter", 1887},
    {"glow-line-quarter", 1292},
    {"glow-corner-quarter", 1009},
    {"triangle-unit", 693},
    {"triangle-unit-stripe", 691},
    {"triangle-unit-grid", 695},
    {"triangle-unit-bevel", 699},
    {"triangle-unit-tile", 713},
    {"triangle-unit-brick", 701},
};