#include <Geode/Geode.hpp>
#include "../lib/PropertyEditor.hpp"
#include "../sculptor/Layer.hpp"

using namespace geode::prelude;

PropertyEditor* PropertyEditor::create(LayerProperty* property) {
    auto node = new PropertyEditor();
    if (node && node->init(property)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool PropertyEditor::init(LayerProperty* property) {
    if (!CCNode::init()) return false;     

    this->property = property;

    createEditor();       

    return true;
}

void PropertyEditor::setModSource(ModSource modSource) {
    this->modSource = modSource;
    modInput->setValue(property->getModValue(modSource));    
}

void PropertyEditor::createEditor() {
    setScale(0.5);
    CCSize size = { 160, 40 };
    float padding = 10;

    setContentSize(size);

    base = NineSlice::create("square02b_001.png", { 0, 0, 80, 80 });
    base->setScale(0.5);
    base->setColor({ 0, 0, 0 });
    base->setOpacity(45);
    base->setContentSize({ size.width * 2 + padding, size.height * 2 + padding });
    addChildAtPosition(base, Anchor::Center);

    auto node = CCNode::create();
    node->setContentSize(size);
    node->setLayout(RowLayout::create());

    valueInput = NumberInput::create(LayerProperty::info[property->name].filter);
    valueInput->setValue(property->getValue());
    valueInput->valueCallback = [this](float value) { this->property->setValue(value); };
    node->addChild(valueInput);

    modInput = NumberInput::create(LayerProperty::info[property->name].filter);
    modInput->setValue(property->getModValue(modSource));
    modInput->valueCallback = [this](float value) { this->property->setModValue(modSource, value); };
    node->addChild(modInput);
    node->updateLayout();

    addChild(node);

    auto labelProperty = CCLabelBMFont::create(LayerProperty::info[property->name].label.c_str(), "chatFont.fnt");
    labelProperty->setScale(0.75);
    addChildAtPosition(labelProperty, Anchor::Top, { 0, -4 });

    auto labelMod = CCLabelBMFont::create("+-", "chatFont.fnt");
    labelMod->setScale(0.75);
    addChildAtPosition(labelMod, Anchor::Center, { 10, -5 });
}
