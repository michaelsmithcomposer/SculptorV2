
#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

void Layer::addProperty(const std::string& name) {	
	auto [p, success] = properties.emplace(name, name);
	LayerProperty* prop = &p->second;
	prop->setCallback([this] { updateObjects(); });
}

LayerProperty* Layer::getProperty(const std::string& name) {
	auto p = properties.find(name);
	if (p != properties.end()) return &p->second;
	return nullptr;
}

std::string Layer::getUniqueName(LayerStyle style) {	
	return styleAsString[style];
}

void Layer::updateObjects() {
	/*log::info("Layer Style: {}, Properties:", style);
	for (const auto& propName : propertyNamesByStyle.at(style)) {
		log::info("Name: {}, Value: {}", propName, getProperty(propName)->getValue());
		for (const auto& [mod, amount] : getProperty(propName)->modulators) {
			log::info("ModSource: {}, Amount: {}", static_cast<int>(mod), amount);
		}
	}*/

	auto styleProperties = evaluateStyle();

	for (const auto& [ID, count] : targetCountByID) {
		int difference = count - objectsByID[ID].size();
		if (difference > 0) {
			for (int i = 0; i < difference; i++) {
				objects.push_back(EditorUI::get()->createObject(ID, { 0, 0 }));
			}
		}
		else if (difference < 0) {
			for (int i = 0; i < difference; i++) {
				auto obj = objectsByID.at(ID).at(i);
				EditorUI::get()->deleteObject(obj, false);
				std::erase(objects, obj);
			}
		}
	}	

	for (const auto& [props, obj] : std::views::zip(styleProperties, objects)) {
		props.applyTo(obj);
	}

	updateIDMap();
	updateBoundingBox();
}

void Layer::updateIDMap() {
	objectsByID.clear();
	for (const auto& object : objects) {
		objectsByID[object->m_objectID].push_back(object);
	}
}

void Layer::updateBoundingBox() {
	CCPoint min = { 0, 0 };
	CCPoint max = { 0, 0 };
	std::ranges::sort(objects, [](GameObject* a, GameObject* b) { return a->getPositionX() < b->getPositionX(); });
	min.x = objects.front()->getPositionX();
	max.x = objects.back()->getPositionX();
	std::ranges::sort(objects, [](GameObject* a, GameObject* b) { return a->getPositionY() < b->getPositionY(); });
	min.y = objects.front()->getPositionY();
	max.y = objects.back()->getPositionY();
	boundingBox = { min.x, min.y, max.x - min.x, max.y - min.y };
	
}

std::vector<GDProperties> Layer::evaluateStyle() {

	std::vector<GDProperties> result;

	switch (style) {
		case LayerStyle::Solid:
			result = fillSolid();
			break;
		case LayerStyle::Corners:
			result = fillCorners();
			break;
		case LayerStyle::Corners2:
			result = fillCorners2();
			break;
	}
	
	for (auto& props : result) {
		props.applyLayerPropertyOffsets(this);
	}

	targetCountByID.clear();
	for (const auto& obj : result) {
		targetCountByID[any_cast<int>(obj.properties.at("ID"))]++;
	}

	return result;

}

std::vector<GDProperties> Layer::fillSolid() {
	std::vector<GDProperties> result;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			GDProperties props({ {"ID", nameID["square-unit"]}, {"x", i * 30.f}, {"y", j * 30.f} });
			result.push_back(props);
		}
	}
	return result;
}

std::vector<GDProperties> Layer::fillCorners() {
	std::vector<GDProperties> result;
	for (const auto& path : form->paths) {
		for (const auto& point : path) {
			result.push_back(GDProperties({ {"ID", 1}, {"x", static_cast<float>(point.x)}, {"y", static_cast<float>(point.y)} }));
		}
	}
	return result;
}

std::vector<GDProperties> Layer::fillCorners2() {
	std::vector<GDProperties> result;
	for (const auto& path : form->paths) {
		for (const auto& point : path) {
			result.push_back(GDProperties({ {"ID", nameID["square-unit"]}, {"x", static_cast<float>(point.x)}, {"y", static_cast<float>(point.y)} }));
		}
	}
	return result;
}

//

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
				float x = layer->boundingBox.origin.x;
				result += amount * inverseLerp(x, x + layer->boundingBox.size.width, any_cast<float>(objProps.properties.at("x")));
				break;
			}
			case ModSource::Y:
			{
				float y = layer->boundingBox.origin.y;
				result += amount * inverseLerp(y, y + layer->boundingBox.size.height, any_cast<float>(objProps.properties.at("y")));
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
	this->value = value;
	if (callback) {
		callback();
	}
}

float LayerProperty::getModValue(ModSource mod) {
	return this->modulators[mod];
}

void LayerProperty::setModValue(ModSource mod, float value) {
	this->modulators[mod] = value;
	if (callback) {
		callback();
	}
}

void LayerProperty::setCallback(std::function<void()> callback) {
	this->callback = callback;
}