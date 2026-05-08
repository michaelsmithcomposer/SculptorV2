#pragma once

#include <Geode/Geode.hpp>


using namespace geode::prelude;

class Layer;

class GDProperties {
public:

    std::unordered_map<std::string, std::any> properties;

    GDProperties(std::unordered_map<std::string, std::any> properties) {
        this->properties = properties;
    }    

    GameObject* applyTo(GameObject* object);

    void applyLayerPropertyOffsets(Layer* layer);

  

    
};




inline std::unordered_map<std::string, std::string> GDPropertyID = {
    {"ID", "1"},
    {"x", "2"},
    {"y", "3"},
    {"flip-horizontal", "4"},
    {"flip-vertical", "5"},
    {"rotation", "6"},
    {"trigger-time", "10"},
    {"editor-layer", "20"},
    {"color", "21"},
    {"color-secondary", "22"},
    {"z-layer", "24"},
    {"z-layer-sub", "25"},
    {"scale", "32"},
    {"hsv-enable", "41"},
    {"hsv", "43"},
    {"groups", "57"},
    {"scale-x", "128"},
    {"scale-y", "129"}
};

inline std::unordered_map<std::string, float> GDPropertyDefault = {    
    {"x", 0},
    {"y", 0},    
    {"rotation", 0},     
    {"scale-x", 1},
    {"scale-y", 1}
};

inline std::unordered_map<std::string, float> layerPropertyDefault = {
    {"test", 0},
    {"offset-x", 0},
    {"offset-y", 0},
    {"offset-rotation", 0},
    {"offset-scale-x", 0},
    {"offset-scale-y", 0},
    {"offset-hue", 0},
    {"offset-saturation", 0},
    {"offset-value", 0},

    {"color", 1},
    {"z-layer", 1},
    {"width", 1.5},
    {"glow-width", 0.5},
    {"triangle-texture", 0},
    {"ID", 1},
    {"spacing", 25},    
    {"normal-rotation", 0},
    {"strip-width", 10},
    {"strip-angle", 0}
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