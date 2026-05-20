#pragma once

#include <Geode/Geode.hpp>
#include "../lib/utilities.hpp"

using namespace geode::prelude;

class NumberInput : public TextInput {
public:

    static NumberInput* create(CommonFilter filter = CommonFilter::Float, std::optional<float> min = std::nullopt, std::optional<float> max = std::nullopt, std::optional<int> leading = std::nullopt, std::optional<int> precision = std::nullopt);

    bool init(CommonFilter filter, std::optional<float> min = std::nullopt, std::optional<float> max = std::nullopt, std::optional<int> leading = std::nullopt, std::optional<int> precision = std::nullopt);

    float getValue();

    void setValue(float value);

    void setBaseColor(ccColor3B color) { colorBase = color; }
    void setHighlightColor(ccColor3B color) { colorHighlight = color; }

    std::function<void(float)> valueCallback;

private:

    CommonFilter filter;
    CCDrawNode* drawNode;
    ListenerHandle mouseListener;
    ListenerHandle scrollListener;
    std::optional<int> focusIndex;
    int minLeadingDigits;
    int decimalPrecision;

    float maxValue;
    float minValue;  

    ccColor3B colorBase = { 255, 255, 255 };
    ccColor3B colorHighlight = { 200, 200, 200 };

    bool typing = false;   

    void onEnter();

    void update(float dt);

    void formatString(float value);

    void handleMouseData(MouseInputData data);

    ListenerResult handleScroll(double x, double y);     

    void onTextEdit(const std::string& text);
    
    void setFocusIndex();

    int stringIndex(int i);

    float magnitudeAtIndex(int i);
  
    CCFontSprite* spriteAtIndex(int i);
};
