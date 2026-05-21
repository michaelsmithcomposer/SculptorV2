#include <Geode/Geode.hpp>
#include "../lib/Interface.hpp"
#include "../lib/Geometry.hpp"
#include "../sculptor/Layer.hpp"

using namespace geode::prelude;

namespace Sculptor {

	int validateID(int ID) {
		return validObjectIDs.contains(ID) ? ID : 1;
	}

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

		object->m_updateParents = true;
		LevelEditorLayer::get()->updateObjectSection(object);
		LevelEditorLayer::get()->reorderObjectSection(object);

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

		auto color = layer->getProperty(LayerProperty::Name::Color);
		if (color) {
			properties[GDProperty::Color] = static_cast<int>(color->evaluate(*this, layer));
		}

		auto z = layer->getProperty(LayerProperty::Name::ZLayer);
		if (z) {
			properties[GDProperty::ZLayer] = static_cast<int>(z->evaluate(*this, layer));
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

	GDProperties GDProperties::fromLine(const Line& line, float width) {

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

	GDProperties GDProperties::fromCircle(const Circle& circle) {
		using enum GDProperty;
		GDProperties props = { {} };
		props.properties[ID] = nameID["circle-unit"];
		props.properties[X] = circle.origin.x;
		props.properties[Y] = circle.origin.y;
		props.properties[ScaleX] = (circle.radius * 2) / gdUnit;
		props.properties[ScaleY] = (circle.radius * 2) / gdUnit;
		return props;
	}

}