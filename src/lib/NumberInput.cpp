#include <Geode/Geode.hpp>

#include "../lib/NumberInput.hpp"
#include "../lib/utilities.hpp"
#include "../lib/geometry.hpp"

using namespace geode::prelude;

NumberInput* NumberInput::create(CommonFilter filter, std::optional<float> min, std::optional<float> max, std::optional<int> leading, std::optional<int> precision) {
    auto node = new NumberInput();
    if (node && node->init(filter, min, max, leading, precision)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool NumberInput::init(CommonFilter filter, std::optional<float> min, std::optional<float> max, std::optional<int> leading, std::optional<int> precision) {
    if (!TextInput::init(80, "", "bigFont.fnt")) return false;    

    mouseListener = MouseInputEvent().listen([this](MouseInputData data) { this->handleMouseData(data); });
    scrollListener = ScrollWheelEvent().listen([this](double x, double y) -> ListenerResult { return this->handleScroll(x, y); });

    setCommonFilter(filter);    
    this->filter = filter;          
    setCallback([this](const std::string& text) { this->onTextEdit(text); });

    minLeadingDigits = leading.value_or(3);
    decimalPrecision = precision.value_or((filter == CommonFilter::Float) ? 2 : 0);
    float maxFloat = std::pow(10, minLeadingDigits) - std::pow(10, -decimalPrecision);
    int maxInt = std::pow(10, minLeadingDigits) - 1;
    minValue = min.value_or((filter == CommonFilter::Float) ? -maxFloat : ((filter == CommonFilter::Int) ? -maxInt : 0));
    maxValue = max.value_or((filter == CommonFilter::Float) ? maxFloat : maxInt);   

    drawNode = CCDrawNode::create();
    drawNode->setZOrder(-1);
    addChild(drawNode);      

    setValue(0);      

    return true;
}

void NumberInput::onEnter() {
    TextInput::onEnter();
    this->scheduleUpdate();
}

void NumberInput::update(float dt) {    
    for (int i = 0; i < getString().size(); i++) {
        spriteAtIndex(i)->setColor(ccc3(255, 255, 255));
    }

    if (!typing) {
        setFocusIndex();
    }    
}

float NumberInput::getValue() {
    try {
        return std::stof(getString());
    }
    catch (...) {
        return 0;
    }    
}

void NumberInput::setValue(float value) {   
    formatString(value);
    if (valueCallback) {
        valueCallback(value);
    }
}

void NumberInput::formatString(float value) {
    int count = minLeadingDigits + decimalPrecision;
    float clampedValue = clamp(value, minValue, maxValue);
    std::string padding = " ";
    if (clampedValue < 0) {
        count++;
        padding = "";
    }
    if (filter == CommonFilter::Float) {
        count++;
    }
    auto str = padding + std::format("{:0{}.{}f}", clampedValue, count, decimalPrecision);    
    setString(str);
}

ListenerResult NumberInput::handleScroll(double x, double y) {
    if (focusIndex.has_value() && !typing) {
        int i = focusIndex.value();
        float m = magnitudeAtIndex(i);
        float v = getValue() + y * m;        
        setValue(v);
        return ListenerResult::Stop;
    }
    return ListenerResult::Propagate;
}

void NumberInput::handleMouseData(MouseInputData data) {
    if (data.action == MouseInputData::Action::Press) {        
        CCRect rect = { 0, 0, boundingBox().size.width, boundingBox().size.height };        
        if (rect.containsPoint(convertToNodeSpace(getMousePos()))) {
            setString("");
            typing = true;
        }
        else {
            setValue(getValue());
            typing = false;
        }
    }    
}

void NumberInput::onTextEdit(const std::string& text) {
    
}

void NumberInput::setFocusIndex() {       
    
    focusIndex = std::nullopt;

    for (int i = 0; i < minLeadingDigits + decimalPrecision; i++) {
        int index = stringIndex(i);
        auto sprite = spriteAtIndex(index);
        auto box = sprite->boundingBox();
        auto origin = convertToNodeSpace(sprite->getParent()->convertToWorldSpace(box.origin));
        auto corner = convertToNodeSpace(sprite->getParent()->convertToWorldSpace(box.origin + box.size));
        CCRect rect = { origin.x, origin.y, corner.x - origin.x,  corner.y - origin.y };        
        rect.inflateRect(-1);
        if (rect.containsPoint(convertToNodeSpace(getMousePos()))) {
            sprite->setColor(colorHighlight);
            focusIndex = i;            
            return;            
        }   
    }
}

int NumberInput::stringIndex(int i) {      
    return (i < minLeadingDigits) ? i + 1 : i + 2;
}


float NumberInput::magnitudeAtIndex(int i) {
    if (i < minLeadingDigits) {
        return std::pow(10, (minLeadingDigits - i - 1));
    }
    else {
        return std::pow(10, -(i - minLeadingDigits + 1));
    }
}

CCFontSprite* NumberInput::spriteAtIndex(int i) {
    return static_cast<CCFontSprite*>(getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->getChildByIndex(i));    
}

