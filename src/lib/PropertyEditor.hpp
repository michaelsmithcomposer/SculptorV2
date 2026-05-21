#pragma once

#include "../lib/NumberInput.hpp"
#include "../lib/Interface.hpp"
//#include "../lib/Modulator.hpp"

namespace Sculptor {

    class LayerProperty;
    enum class ModSource;
    class Modulator;

    class Property {
    public:

        std::string label;
        CommonFilter filter;

        Property(std::string label, CommonFilter filter, float value) : label(label), filter(filter) { setValue(value); };
        Property(const Property&) = delete;
        Property& operator=(const Property&) = delete;
        Property(Property&&) = default;
        Property& operator=(Property&&) = default;

        float getValue() const { return value; }
        void setValue(float value) {
            bool changed = (this->value != value);
            this->value = value;
            if (changed && onChanged) {
                onChanged();
            }
        }

        void setCallback(std::function<void()> onChanged) { this->onChanged = onChanged; }

    protected:

        float value;
        std::function<void()> onChanged;
    };

    class ModulatableProperty : public Property {
    public:

        using Property::Property;

        float getModValue(Modulator* mod) { return modValues.contains(mod) ? modValues.at(mod) : 0; }
        void setModValue(Modulator* mod, float value) {
            bool changed = (this->modValues[mod] != value);
            this->modValues[mod] = value;
            if (changed && onChanged) {
                onChanged();
            }
        }
        void removeModulator(Modulator* modulator);

        float evaluate(GDProperties& objProps, Layer* layer);

        std::unordered_map<Modulator*, float> modValues;

    private:



    };


    class ModulatablePropertyEditor : public CCNode {
    public:

        static ModulatablePropertyEditor* create(ModulatableProperty* property, CCSize size);
        bool init(ModulatableProperty* property, CCSize size);

        void updateLabel();

    private:

        ModulatableProperty* property;
        NumberInput* valueInput;
        NumberInput* modInput;

    };

    class SinglePropertyEditor : public CCNode {
    public:

        static SinglePropertyEditor* create(Property* property, CCSize size);
        bool init(Property* property, CCSize size);

    private:

        Property* property;
        NumberInput* valueInput;

    };

}