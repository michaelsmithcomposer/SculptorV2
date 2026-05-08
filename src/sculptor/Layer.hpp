#pragma once

#include <Geode/Geode.hpp>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"


using namespace geode::prelude;

class Form;

enum class LayerStyle {
	Solid,
	Outline,
	Corners,
	Corners2,
};

inline std::unordered_map<LayerStyle, std::string> styleAsString = {
	{LayerStyle::Solid, "solid"},
	{LayerStyle::Corners, "corners"},
	{LayerStyle::Corners2, "corners2"},
	{LayerStyle::Outline, "outline"},
};

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

inline std::unordered_map<LayerStyle, std::vector<std::string>> propertyNamesByStyle = {
	{LayerStyle::Solid, { "offset-x", "offset-y", "offset-rotation", "offset-scale-x", "offset-scale-y", "offset-hue", "offset-saturation", "offset-value" } },
	{LayerStyle::Corners, { "offset-x", "offset-y", "offset-rotation", "offset-scale-x", "offset-scale-y", "offset-hue", "offset-saturation", "offset-value" } },
	{LayerStyle::Corners2, { "offset-x", "offset-y", "offset-rotation", "offset-scale-x", "offset-scale-y", "offset-hue", "offset-saturation", "offset-value" } }
};

class Layer { 
public:
	
	Form* form;
	std::string name;
	LayerStyle style;
	
	CCRect boundingBox;

	Layer(Form* form, LayerStyle style) : form(form), style(style), name(getUniqueName(style)) {
		for (const auto& propName : propertyNamesByStyle.at(style)) {
			addProperty(propName);
		}
		updateObjects();
	}
	Layer(const Layer&) = delete;
	Layer& operator=(const Layer&) = delete;

	void addProperty(const std::string& name);
	LayerProperty* getProperty(const std::string& name);

private:

	std::unordered_map<std::string, LayerProperty> properties;	

	std::vector<GameObject*> objects;
	std::unordered_map<int, std::vector<GameObject*>> objectsByID;
	std::unordered_map<int, int> targetCountByID;

	std::string getUniqueName(LayerStyle style);


	void updateObjects();
	void updateIDMap();	
	void updateBoundingBox();

	std::vector<GDProperties> evaluateStyle();

	std::vector<GDProperties> fillSolid();

	std::vector<GDProperties> fillCorners();

	std::vector<GDProperties> fillCorners2();

};

class LayerProperty {
public:

	std::string name;

	std::unordered_map<ModSource, float> modulators;

	float evaluate(GDProperties& objProps, Layer* layer);
	
	float getValue();
	void setValue(float value);

	float getModValue(ModSource mod);
	void setModValue(ModSource mod, float value);

	void setCallback(std::function<void()> callback);
	
	LayerProperty(std::string name) : name(name) { value = layerPropertyDefault.at(this->name);	}
	LayerProperty(const LayerProperty&) = delete;
	LayerProperty& operator=(const LayerProperty&) = delete;
	LayerProperty& operator=(LayerProperty&&) = default;

private:

	float value;
	
	std::function<void()> callback;

};


