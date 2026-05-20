#pragma once

#include <Geode/Geode.hpp>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"


using namespace geode::prelude;
using namespace Sculptor;

class Form;

enum class LayerStyle {
	Solid,
	Outline,
	Glow,
	OutlineUniform,
	Lines,
	Texture,
	Debug,

	Count
};



struct LayerPropertyInfo {
	std::string label;
	float defaultValue;
	CommonFilter filter;
	std::optional<GDProperty> gdProperty;

};

class LayerProperty : public ModulatableProperty {
public:

	enum class Name {
		OffsetPath,
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

	LayerProperty(Name name) :
		ModulatableProperty(info[name].label, info[name].filter, info[name].defaultValue), name(name) {}

	static std::unordered_map<Name, LayerPropertyInfo> info;
};

inline std::vector<LayerProperty::Name> offsetProperties = { LayerProperty::Name::OffsetX,
															 LayerProperty::Name::OffsetY,
															 LayerProperty::Name::OffsetRotation,
															 LayerProperty::Name::OffsetScaleX,
															 LayerProperty::Name::OffsetScaleY,
															 LayerProperty::Name::OffsetHue,
															 LayerProperty::Name::OffsetSaturation,
															 LayerProperty::Name::OffsetValue };


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
	std::unordered_map<LayerProperty::Name, LayerProperty> properties;

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
	void deleteAllObjects();

	static std::vector<LayerProperty::Name> propertyNamesByStyle(LayerStyle style);
	static std::unordered_map<LayerStyle, LayerStyleInfo> layerStyleInfo;

private:

	

	std::vector<GameObject*> objects;
	std::unordered_map<int, std::vector<GameObject*>> objectsByID;
	std::unordered_map<int, std::vector<GDProperties>> styleEvaluationByID;

	std::string getUniqueName(LayerStyle style);


	
	void updateStyleEvaluation();
	void updateIDMap();		

	float evaluateProperty(LayerProperty::Name name, std::optional<GDProperties> objProps = std::nullopt);
	std::vector<GDProperties> evaluateStyle();
	Polys getDecompositionInflated();
	RightTriangles getTriangulationInflated();

	GDProperties buildObject(std::function<GDProperties(std::optional<GDProperties>)> func);	
	/*bool objectNeedsRecreation(GameObject* object, GDProperties& props);*/

	std::vector<GDProperties> fillSolid();
	std::vector<GDProperties> fillOutline();
	std::vector<GDProperties> fillGlow();
	std::vector<GDProperties> fillOutlineUniform();
	std::vector<GDProperties> fillLines();
	std::vector<GDProperties> fillTexture();

	std::vector<GDProperties> fillDebug();

	
	

	
	

};


