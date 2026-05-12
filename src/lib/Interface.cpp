#include <Geode/Geode.hpp>
#include "../lib/Interface.hpp"
#include "../lib/Geometry.hpp"
#include "../sculptor/Layer.hpp"

using namespace geode::prelude;
using namespace Sculptor;


GameObject* GDProperties::applyTo(GameObject* object) const {
	using enum GDProperty;
	if (properties.contains(X)) object->setPositionX(any_cast<float>(properties.at(X)));
	if (properties.contains(Y)) object->setPositionY(any_cast<float>(properties.at(Y)));
	if (properties.contains(Rotation)) object->setRotation(any_cast<float>(properties.at(Rotation)));
	if (properties.contains(ScaleX)) object->updateCustomScaleX(any_cast<float>(properties.at(ScaleX)));
	if (properties.contains(ScaleY)) object->updateCustomScaleY(any_cast<float>(properties.at(ScaleY)));
	if (properties.contains(ZLayer)) object->setCustomZLayer(any_cast<int>(properties.at(ZLayer)));
	if (properties.contains(Color)) object->setMainColorMode(any_cast<int>(properties.at(Color)));
	if (properties.contains(SecondaryColor)) object->setSecondaryColorMode(any_cast<int>(properties.at(SecondaryColor)));
	if (properties.contains(Hue)) object->m_baseColor->m_hsv.h = any_cast<float>(properties.at(Hue));
	if (properties.contains(Saturation)) object->m_baseColor->m_hsv.s = any_cast<float>(properties.at(Saturation));
	if (properties.contains(Value)) object->m_baseColor->m_hsv.v = any_cast<float>(properties.at(Value));
	
	object->m_baseColor->m_hsv.absoluteBrightness = true;
	object->m_baseColor->m_hsv.absoluteSaturation = true;

	if (properties.contains(Groups)) {
		object->resetGroups();
		for (const auto& id : any_cast<std::vector<int>>(properties.at(Groups))) {
			object->addToGroup(id);
		}
	}

	return object;
}

void GDProperties::applyGenericLayerProperties(Layer* layer) {

	for (const auto& name : offsetProperties) {
		float v = layer->getProperty(name)->evaluate(*this, layer);
		auto prop = *LayerProperty::info[name].gdProperty;
		if (properties.contains(prop)) {
			properties[prop] = any_cast<float>(properties.at(prop)) + v;
		}
		else {
			if (name == LayerProperty::Name::OffsetScaleX || name == LayerProperty::Name::OffsetScaleY) {
				properties[prop] = 1 + v;
			}
			else {
				properties[prop] = v;
			}			
		}
	}	

	auto color = layer->getProperty(LayerProperty::Name::Color)->evaluate(*this, layer);
	if (color) {
		properties[GDProperty::Color] = static_cast<int>(color);
	}

	auto z = layer->getProperty(LayerProperty::Name::ZLayer)->evaluate(*this, layer);
	if (z) {
		properties[GDProperty::ZLayer] = static_cast<int>(z);
	}	
}

GDProperties GDProperties::fromRightTriangle(const RightTriangle& triangle, bool inflateEpsilon) {
	using enum GDProperty;
	GDProperties props = { {} };
	CCPoint position = triangle.hypotenuse().midpoint();
	props.properties[ID] = nameID["triangle-unit"];
	props.properties[X] = position.x;
	props.properties[Y] = position.y;
	props.properties[Rotation] = static_cast<float>(-CC_RADIANS_TO_DEGREES(PI + Line(triangle.b(), triangle.c()).angle()));	
	props.properties[ScaleX] = abs(triangle.bc() / gdUnit) + (inflateEpsilon ? 0.001f : 0.f);
	props.properties[ScaleY] = abs(triangle.ab() / gdUnit) + (inflateEpsilon ? 0.001f : 0.f);
	return props;
}

std::vector<GDProperties> GDProperties::fromRightTriangleInterior(const RightTriangle& triangle) {
	std::vector<GDProperties> result;
	result.push_back(GDProperties::fromRightTriangle(triangle, true));
	Line hy = Line(triangle.a(), triangle.c());	
	hy.shrink(0.1 * triangle.antialiasingWidth());
	auto lineProps = GDProperties::fromLine(hy, triangle.antialiasingWidth());
	if (lineProps) result.push_back(*lineProps);	
	return result;
}

std::vector<GDProperties> GDProperties::fromRightTriangleExterior(const RightTriangle& triangle) {
	std::vector<GDProperties> result;
	result.push_back(GDProperties::fromRightTriangle(triangle));
	Line hy = triangle.hypotenuse();
	hy.shrink(0.1 * triangle.antialiasingWidth());
	Line line = Line(hy.a - hy.normal() * triangle.antialiasingWidth() * 0.25, hy.b - hy.normal() * triangle.antialiasingWidth() * 0.25);
	auto lineProps = GDProperties::fromLine(line, triangle.antialiasingWidth() * 0.5);
	if (lineProps) result.push_back(*lineProps);	
	return result;
}

std::optional<GDProperties> GDProperties::fromLine(const Line& line, float width) {

	/*if (width < 0.15) {
		return std::nullopt;
	}*/

	using enum GDProperty;
	GDProperties props = { {} };
	CCPoint position = line.a.lerp(line.b, 0.5);
	props.properties[ID] = nameID["line"];
	props.properties[X] = position.x;
	props.properties[Y] = position.y;
	props.properties[Rotation] = static_cast<float>(-CC_RADIANS_TO_DEGREES(line.angle()));
	props.properties[ScaleX] = line.length() / gdUnit;
	props.properties[ScaleY] = width;
	return props;
}

