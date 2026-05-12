#pragma once

#include <Geode/Geode.hpp>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"


using namespace geode::prelude;

class Form;

enum class ModSource {
	Noise,
	X,
	Y
};

constexpr std::array<ModSource, 3> modSources = { ModSource::Noise, ModSource::X, ModSource::Y };

inline std::unordered_map<ModSource, std::string> modSourceAsString = {
	{ModSource::Noise, "noise"},
	{ModSource::X, "x"},
	{ModSource::Y, "y"}	
};

struct LayerPropertyInfo {
	std::string label;
	float defaultValue;
	CommonFilter filter;
	std::optional<GDProperty> gdProperty;

};

class LayerProperty {
public:

	enum class Name {
		OffsetX,
		OffsetY,
		OffsetRotation,
		OffsetScaleX,
		OffsetScaleY,
		OffsetHue,
		OffsetSaturation,
		OffsetValue,

		Color,
		ZLayer,

		LineWidth,
		GlowWidth,
		ID,
		Spacing,
		Texture

	};

	Name name;

	std::unordered_map<ModSource, float> modulators;

	float evaluate(GDProperties& objProps, Layer* layer);

	float getValue();
	void setValue(float value);

	float getModValue(ModSource mod);
	void setModValue(ModSource mod, float value);

	void setCallback(std::function<void()> callback);

	LayerProperty(Name name) : name(name) { value = info.at(this->name).defaultValue; }
	LayerProperty(const LayerProperty&) = delete;
	LayerProperty& operator=(const LayerProperty&) = delete;
	LayerProperty& operator=(LayerProperty&&) = default;

	inline static std::unordered_map<Name, LayerPropertyInfo> info = {
		{Name::OffsetX, {"X", 0, CommonFilter::Float, GDProperty::X}},
		{Name::OffsetY, {"Y", 0, CommonFilter::Float, GDProperty::Y}},
		{Name::OffsetRotation, {"Rotation", 0, CommonFilter::Float, GDProperty::Rotation}},
		{Name::OffsetScaleX, {"Scale X", 0, CommonFilter::Float, GDProperty::ScaleX}},
		{Name::OffsetScaleY, {"Scale Y", 0, CommonFilter::Float, GDProperty::ScaleY}},
		{Name::OffsetHue, {"Hue", 0, CommonFilter::Float, GDProperty::Hue}},
		{Name::OffsetSaturation, {"Saturation", 0, CommonFilter::Float, GDProperty::Saturation}},
		{Name::OffsetValue, {"Value", 0, CommonFilter::Float, GDProperty::Value}},

		{Name::Color, {"Color", 1, CommonFilter::Uint, GDProperty::Color}},
		{Name::ZLayer, {"Z Layer", 0, CommonFilter::Int, GDProperty::ZLayer}},

		{Name::LineWidth, {"Line Width", 1, CommonFilter::Float}},
		{Name::GlowWidth, {"Glow Width", 1, CommonFilter::Float}},
		{Name::ID, {"ID", 1, CommonFilter::Uint, GDProperty::ID}},
		{Name::Spacing, {"Spacing", gdUnit, CommonFilter::Float}},
		{Name::Texture, {"Texture", 0, CommonFilter::Uint}},
	};	

private:

	float value;

	std::function<void()> callback;

};

inline std::vector<LayerProperty::Name> offsetProperties = { LayerProperty::Name::OffsetX,
															 LayerProperty::Name::OffsetY,
															 LayerProperty::Name::OffsetRotation,
															 LayerProperty::Name::OffsetScaleX,
															 LayerProperty::Name::OffsetScaleY,
															 LayerProperty::Name::OffsetHue,
															 LayerProperty::Name::OffsetSaturation,
															 LayerProperty::Name::OffsetValue };

enum class LayerStyle {
	Solid,
	Outline,
	Glow,
	OutlineUniform,
	Lines,
	Texture,
};

struct LayerStyleInfo {
	std::string label;
	std::vector<LayerProperty::Name> uniqueProperties;
	std::function<std::vector<GDProperties>(Layer* layer)> fillFunction;
};




class Layer { 
public:
	
	Form* form;
	std::string name;
	LayerStyle style;	

	Layer(Form* form, LayerStyle style) : form(form), style(style), name(getUniqueName(style)) {
		for (const auto& name : propertyNamesByStyle(style)) {
			addProperty(name);
		}
		updateObjects();
	}
	Layer(const Layer&) = delete;
	Layer& operator=(const Layer&) = delete;

	void addProperty(LayerProperty::Name name);
	LayerProperty* getProperty(LayerProperty::Name name);

	void updateObjects();

	static std::vector<LayerProperty::Name> propertyNamesByStyle(LayerStyle style);

private:

	std::unordered_map<LayerProperty::Name, LayerProperty> properties;	

	std::vector<GameObject*> objects;
	std::unordered_map<int, std::vector<GameObject*>> objectsByID;
	std::unordered_map<int, std::vector<GDProperties>> styleEvaluationByID;

	std::string getUniqueName(LayerStyle style);


	
	void updateStyleEvaluation();
	void updateIDMap();		

	std::vector<GDProperties> evaluateStyle();

	std::vector<GDProperties> fillSolid();
	std::vector<GDProperties> fillOutline();
	std::vector<GDProperties> fillGlow();
	std::vector<GDProperties> fillOutlineUniform();
	std::vector<GDProperties> fillLines();
	std::vector<GDProperties> fillTexture();

	static std::unordered_map<LayerStyle, LayerStyleInfo> layerStyleInfo;
	

	
	

};

