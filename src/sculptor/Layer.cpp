
#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"
#include "../lib/Manager.hpp"
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"

using namespace geode::prelude;


float LayerProperty::evaluate(GDProperties& objProps, Layer* layer) {
	float result = value;
	for (const auto& [modSource, amount] : modulators) {
		switch (modSource) {
		case ModSource::Noise:
		{
			result += amount * CCRANDOM_MINUS1_1();
			break;
		}
		case ModSource::X:
		{
			CCRect bounds = layer->form->getApproximation().boundingBox();
			float x = bounds.origin.x;
			result += amount * inverseLerp(x, x + bounds.size.width, any_cast<float>(objProps.properties.at(GDProperty::X)));
			break;
		}
		case ModSource::Y:
		{
			CCRect bounds = layer->form->getApproximation().boundingBox();
			float y = bounds.origin.y;
			result += amount * inverseLerp(y, y + bounds.size.height, any_cast<float>(objProps.properties.at(GDProperty::Y)));
			break;
		}
		}
	}
	return result;
}

float LayerProperty::getValue() {
	return value;
}

void LayerProperty::setValue(float value) {
	bool changed = (this->value != value);
	this->value = value;
	if (changed && callback) {
		callback();
	}
}

float LayerProperty::getModValue(ModSource mod) {
	return this->modulators[mod];
}

void LayerProperty::setModValue(ModSource mod, float value) {
	bool changed = (this->modulators[mod] != value);
	this->modulators[mod] = value;
	if (changed && callback) {
		callback();
	}
}

void LayerProperty::setCallback(std::function<void()> callback) {
	this->callback = callback;
}

//

void Layer::addProperty(LayerProperty::Name name) {
	auto [p, success] = properties.emplace(name, name);
	LayerProperty* prop = &p->second;
	prop->setCallback([this] { updateObjects(); });
}

LayerProperty* Layer::getProperty(LayerProperty::Name name) {
	auto p = properties.find(name);
	if (p != properties.end()) return &p->second;
	return nullptr;
}

std::vector<LayerProperty::Name> Layer::propertyNamesByStyle(LayerStyle style) {
	auto props = offsetProperties;
	props.append_range(layerStyleInfo[style].uniqueProperties);
	return props;
}

std::string Layer::getUniqueName(LayerStyle style) {	
	return layerStyleInfo[style].label;
}

void Layer::updateObjects() {	

	updateStyleEvaluation();
	for (const auto& [ID, props] : styleEvaluationByID) {
		int difference = props.size() - objectsByID[ID].size();
		if (difference > 0) {
			for (int i = 0; i < difference; i++) {
				objects.push_back(Manager::get()->createObject(ID, { 0, 0 }));
			}
		}
		else if (difference < 0) {
			for (int i = 0; i < abs(difference); i++) {
				auto obj = objectsByID.at(ID).at(i);
				EditorUI::get()->deleteObject(obj, false);
				std::erase(objects, obj);
			}
		}
		updateIDMap();
		for (const auto& [prop, obj] : std::views::zip(props, objectsByID[ID])) {
			prop.applyTo(obj);
		}
	}
	
	
}

void Layer::updateStyleEvaluation() {
	styleEvaluationByID.clear();
	for (auto& props : evaluateStyle()) {	
		styleEvaluationByID[any_cast<int>(props.properties[GDProperty::ID])].push_back(props);
	}
}

void Layer::updateIDMap() {
	objectsByID.clear();
	for (const auto& object : objects) {
		objectsByID[object->m_objectID].push_back(object);
	}
}


std::vector<GDProperties> Layer::evaluateStyle() {

	auto result = layerStyleInfo[style].fillFunction(this);	
	
	for (auto& props : result) {
		props.applyGenericLayerProperties(this);
	}	

	return result;

}

std::unordered_map<LayerStyle, LayerStyleInfo> Layer::layerStyleInfo = {
	{ LayerStyle::Solid, {
		"Solid", {
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillSolid(); }}},
	{LayerStyle::Outline, {
		"Outline", {
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillOutline(); }}},
	{LayerStyle::Glow, {
		"Glow", {
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillGlow(); }}},
	{LayerStyle::OutlineUniform, {
		"OutlineUniform", {
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillOutlineUniform(); }}},
	{LayerStyle::Lines, {
		"Lines", {
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillLines(); }}},
	{LayerStyle::Texture,  {
		"Texture", {
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillTexture(); }}},
	{LayerStyle::Debug,  {
		"Debug", {},
		[](Layer* layer) { return layer->fillDebug(); }}},
};


std::vector<GDProperties> Layer::fillSolid() {
	std::vector<GDProperties> result;	
	for (const auto& [i, triangle] : std::views::enumerate(form->getTriangulation())) {					
		if (form->getApproximation().containsEdge(triangle.hypotenuse())) {
			result.append_range(GDProperties::fromRightTriangleExterior(triangle));
		}
		else {
			result.append_range(GDProperties::fromRightTriangleInterior(triangle));
		}
	}
	return result;
}

std::vector<GDProperties> Layer::fillOutline() {
	std::vector<GDProperties> result;

	return result;
}

std::vector<GDProperties> Layer::fillGlow() {
	std::vector<GDProperties> result;

	return result;
}

std::vector<GDProperties> Layer::fillOutlineUniform() {
	std::vector<GDProperties> result;

	return result;
}

std::vector<GDProperties> Layer::fillLines() {
	std::vector<GDProperties> result;

	return result;
}

std::vector<GDProperties> Layer::fillTexture() {
	std::vector<GDProperties> result;

	return result;
}

std::vector<GDProperties> Layer::fillDebug() {
	std::vector<GDProperties> result;

	CCRect bounds = form->getApproximation().boundingBox();
	int countX = floor(bounds.size.width / gdUnit);
	int countY = floor(bounds.size.height / gdUnit);
	
	for (int i = 0; i < countX; i++) {
		for (int j = 0; j < countY; j++) {
			CCPoint point = { bounds.origin.x + i * gdUnit, bounds.origin.y + j * gdUnit };
			if (form->getApproximation().contains(point)) {
				result.push_back(GDProperties({ {GDProperty::ID, (i % 2 != j % 2) ? 1 : 2}, {GDProperty::X, point.x}, {GDProperty::Y, point.y}, {GDProperty::ScaleX, 0.3f}, {GDProperty::ScaleY, 0.3f} }));
			}
		}
	}
	
	return result;
}




