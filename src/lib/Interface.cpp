#include <Geode/Geode.hpp>
#include "../lib/Interface.hpp"
#include "../sculptor/Layer.hpp"

using namespace geode::prelude;

GameObject* GDProperties::applyTo(GameObject* object) {

	if (properties.contains("x")) object->setPositionX(any_cast<float>(properties.at("x")));
	if (properties.contains("y")) object->setPositionY(any_cast<float>(properties.at("y")));
	if (properties.contains("rotation")) object->setRotation(any_cast<float>(properties.at("rotation")));
	if (properties.contains("scale-x")) object->updateCustomScaleX(any_cast<float>(properties.at("scale-x")));
	if (properties.contains("scale-y")) object->updateCustomScaleY(any_cast<float>(properties.at("scale-y")));
	if (properties.contains("z-layer")) object->setCustomZLayer(any_cast<int>(properties.at("z-layer")));
	if (properties.contains("color")) object->setMainColorMode(any_cast<int>(properties.at("color")));
	if (properties.contains("secondary-color")) object->setSecondaryColorMode(any_cast<int>(properties.at("secondary-color")));
	if (properties.contains("hue")) object->m_baseColor->m_hsv.h = any_cast<float>(properties.at("hue"));
	if (properties.contains("saturation")) object->m_baseColor->m_hsv.s = any_cast<float>(properties.at("saturation"));
	if (properties.contains("value")) object->m_baseColor->m_hsv.v = any_cast<float>(properties.at("value"));
	
	object->m_baseColor->m_hsv.absoluteBrightness = true;
	object->m_baseColor->m_hsv.absoluteSaturation = true;

	if (properties.contains("groups")) {
		object->resetGroups();
		for (const auto& id : any_cast<std::vector<int>>(properties.at("groups"))) {
			object->addToGroup(id);
		}
	}

	return object;
}

void GDProperties::applyLayerPropertyOffsets(Layer* layer) {

	
	float x = layer->getProperty("offset-x")->evaluate(*this, layer);
	if (properties.contains("x")) {
		properties["x"] = any_cast<float>(properties.at("x")) + x;
	}
	else {
		properties["x"] = x;
	}	
	
	float y = layer->getProperty("offset-y")->evaluate(*this, layer);
	if (properties.contains("y")) {
		properties["y"] = any_cast<float>(properties.at("y")) + y;
	}
	else {
		properties["y"] = y;
	}
	
	float rotation = layer->getProperty("offset-rotation")->evaluate(*this, layer);
	if (properties.contains("rotation")) {
		properties["rotation"] = any_cast<float>(properties.at("rotation")) + rotation;
	}
	else {
		properties["rotation"] = rotation;
	}

	float scaleX = layer->getProperty("offset-scale-x")->evaluate(*this, layer);
	if (properties.contains("scale-x")) {
		properties["scale-x"] = any_cast<float>(properties.at("scale-x")) + scaleX;
	}
	else {
		properties["scale-x"] = 1 + scaleX;
	}

	float scaleY = layer->getProperty("offset-scale-y")->evaluate(*this, layer);
	if (properties.contains("scale-y")) {
		properties["scale-y"] = any_cast<float>(properties.at("scale-y")) + scaleY;
	}
	else {
		properties["scale-y"] = 1 + scaleY;
	}

	float hue = layer->getProperty("offset-hue")->evaluate(*this, layer);
	if (properties.contains("hue")) {
		properties["hue"] = any_cast<float>(properties.at("hue")) + hue;
	}
	else {
		properties["hue"] = hue;
	}

	float saturation = layer->getProperty("offset-saturation")->evaluate(*this, layer);
	if (properties.contains("saturation")) {
		properties["saturation"] = any_cast<float>(properties.at("saturation")) + saturation;
	}
	else {
		properties["saturation"] = saturation;
	}

	float value = layer->getProperty("offset-value")->evaluate(*this, layer);
	if (properties.contains("value")) {
		properties["value"] = any_cast<float>(properties.at("value")) + value;
	}
	else {
		properties["value"] = value;
	}
}
