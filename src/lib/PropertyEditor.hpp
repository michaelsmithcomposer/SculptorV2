#pragma once

#include "../lib/NumberInput.hpp"
#include "../lib/Interface.hpp"

class LayerProperty;
enum class ModSource;

class PropertyEditor : public CCNode {
public:

    static PropertyEditor* create(LayerProperty* property);
    bool init(LayerProperty* property);
    
    LayerProperty* property;

    void setModSource(ModSource modSource);

private:   

    NumberInput* valueInput;
    NumberInput* modInput;
    NineSlice* base;

    ModSource modSource;

    void createEditor();

};