#include <Geode/Geode.hpp>
#include "../lib/PropertyEditor.hpp"
#include "../sculptor/Layer.hpp"
#include "../lib/Manager.hpp"
#include "../lib/Modulator.hpp"

using namespace geode::prelude;

namespace Sculptor {

    float ModulatableProperty::evaluate(GDProperties& objProps, Layer* layer) {
        float result = value;
        for (const auto& [modulator, amount] : modValues) {
            result += amount * modulator->evaluate(objProps, layer);
        }
        return result;
    }

    void ModulatableProperty::removeModulator(Modulator* modulator) {
        auto it = modValues.find(modulator);
        if (it != modValues.end()) {
            modValues.erase(it);
            if (onChanged) {
                onChanged();
            }
        }
    }

    ModulatablePropertyEditor* ModulatablePropertyEditor::create(ModulatableProperty* property, CCSize size) {
        auto node = new ModulatablePropertyEditor();
        if (node && node->init(property, size)) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    bool ModulatablePropertyEditor::init(ModulatableProperty* property, CCSize size) {
        if (!CCNode::init()) return false;

        this->property = property;

        //

        setScale(0.5);
        float padding = 10;

        setContentSize(size);

        auto base = NineSlice::create("square02b_001.png", { 0, 0, 80, 80 });
        base->setScale(0.5);
        base->setColor({ 0, 0, 0 });
        base->setOpacity(45);
        base->setContentSize({ size.width * 2 + padding, size.height * 2 + padding });
        addChildAtPosition(base, Anchor::Center);

        auto node = CCNode::create();
        node->setContentSize(size);
        node->setLayout(RowLayout::create());

        valueInput = NumberInput::create(property->filter);
        valueInput->setValue(property->getValue());
        valueInput->valueCallback = [this](float value) { this->property->setValue(value); };
        node->addChild(valueInput);

        modInput = NumberInput::create(property->filter);
        modInput->setValue(property->getModValue(Manager::get()->selectedModulator));
        modInput->setBaseColor(Modulator::info[Manager::get()->selectedModulator->type].color);
        modInput->setHighlightColor({ 255, 255, 255 });
        modInput->valueCallback = [this](float value) {
            this->property->setModValue(Manager::get()->selectedModulator, value);
            updateLabel();
            };
        node->addChild(modInput);
        node->updateLayout();

        addChild(node);

        auto labelProperty = CCLabelBMFont::create(property->label.c_str(), "chatFont.fnt");
        labelProperty->setID("label"_spr);
        labelProperty->setScale(0.75);
        addChildAtPosition(labelProperty, Anchor::Top, { 0, -4 });

        updateLabel();

        return true;

    }

    void ModulatablePropertyEditor::updateLabel() {
        auto label = static_cast<CCLabelBMFont*>(getChildByID("label"_spr));
        auto str = std::string(label->getString());
        str = str.substr(0, str.find("*"));
        std::vector <std::pair<int, ccColor3B>> colors;
        for (const auto& [modulator, amount] : property->modValues) {
            if (amount != 0) {
                colors.push_back(std::make_pair(str.size(), Modulator::info[modulator->type].color));
                str += "*";
            }
        }
        label->setString(str.c_str());
        for (const auto& [i, color] : colors) {
            static_cast<CCFontSprite*>(label->getChildByIndex(i))->setColor(color);
        }

    }

    SinglePropertyEditor* SinglePropertyEditor::create(Property* property, CCSize size) {
        auto node = new SinglePropertyEditor();
        if (node && node->init(property, size)) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    bool SinglePropertyEditor::init(Property* property, CCSize size) {
        if (!CCNode::init()) return false;

        this->property = property;

        //

        setScale(0.5);
        float padding = 10;

        setContentSize(size);

        auto base = NineSlice::create("square02b_001.png", { 0, 0, 80, 80 });
        base->setScale(0.5);
        base->setColor({ 0, 0, 0 });
        base->setOpacity(45);
        base->setContentSize({ size.width * 2 + padding, size.height * 2 + padding });
        addChildAtPosition(base, Anchor::Center);

        valueInput = NumberInput::create(property->filter);
        valueInput->setValue(property->getValue());
        valueInput->valueCallback = [this](float value) { this->property->setValue(value); };
        addChildAtPosition(valueInput, Anchor::Center, { 0, -5 });

        auto labelProperty = CCLabelBMFont::create(property->label.c_str(), "chatFont.fnt");
        labelProperty->setScale(0.75);
        addChildAtPosition(labelProperty, Anchor::Top, { 0, -4 });

        return true;
    }
}