
#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"
#include "../lib/Manager.hpp"
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

std::unordered_map<LayerStyle, LayerStyleInfo> Layer::layerStyleInfo = {
	{ LayerStyle::Solid, {
		"Solid", {
			LayerProperty::Name::OffsetPath,
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer},
		[](Layer* layer) { return layer->fillSolid(); }}},
	{LayerStyle::Outline, {
		"Outline", {
			LayerProperty::Name::OffsetPath,
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer,
			LayerProperty::Name::LineWidth},
		[](Layer* layer) { return layer->fillOutline(); }}},
	{LayerStyle::Glow, {
		"Glow", {
			LayerProperty::Name::OffsetPath,
			LayerProperty::Name::Color,
			LayerProperty::Name::ZLayer,
			LayerProperty::Name::GlowWidth},
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

std::unordered_map<LayerProperty::Name, LayerPropertyInfo> LayerProperty::info = {
		{Name::OffsetPath, {"Path Offset", 0, CommonFilter::Float}},
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
	auto props = layerStyleInfo[style].uniqueProperties;
	props.append_range(offsetProperties);
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

void Layer::deleteAllObjects() {
	for (const auto& obj : objects) {
		EditorUI::get()->deleteObject(obj, false);
	}
	objects.clear();
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

float Layer::evaluateProperty(LayerProperty::Name name, std::optional<GDProperties> objProps) {
	if (objProps) {
		return getProperty(name)->evaluate(*objProps, this);
	}
	else {
		return getProperty(name)->getValue();
	}
		
}

std::vector<GDProperties> Layer::evaluateStyle() {					
	return layerStyleInfo[style].fillFunction(this);
}

Polys Layer::getDecompositionInflated() {
	float amount = evaluateProperty(LayerProperty::Name::OffsetPath);
	if (amount == 0) {
		return form->getDecomposition();
	}
	else {
		Polys result;

		using namespace Clipper2Lib;
		PathsD paths;
		for (const auto& poly : form->getDecomposition()) {
			paths.push_back(poly.asPath());
		}
		PathsD inflated = InflatePaths(paths, amount, JoinType::Miter, EndType::Polygon);
		for (const auto& path : inflated) {
			result.push_back(Sequence::fromPath(path));
		}

		return result;
	}	
}

RightTriangles Layer::getTriangulationInflated() {
	float amount = evaluateProperty(LayerProperty::Name::OffsetPath);
	if (amount == 0) {
		return form->getTriangulation();
	}
	else {
		return triangulatePolygons(getDecompositionInflated());
	}
}

GDProperties Layer::buildObject(std::function<GDProperties(std::optional<GDProperties>)> func) {
	auto base = func(std::nullopt);
	auto props = func(base);
	props.applyGenericLayerProperties(this);
	props.properties[GDProperty::X] = any_cast<float>(props.properties.at(GDProperty::X)) + form->getPositionX();
	props.properties[GDProperty::Y] = any_cast<float>(props.properties.at(GDProperty::Y)) + form->getPositionY();
	return props;
}

std::vector<GDProperties> Layer::fillSolid() {
	std::vector<GDProperties> result;	
	for (const auto& [i, triangle] : std::views::enumerate(getTriangulationInflated())) {		
		result.push_back(buildObject([&](std::optional<GDProperties> context) {			
			return GDProperties::fromRightTriangle(triangle, true);
		}));
		Line hy = triangle.hypotenuse();
		hy.shrink(0.1 * triangle.antialiasingWidth());
		if (form->getApproximation().containsEdge(triangle.hypotenuse())) {			
			Line line = Line(hy.a - hy.normal() * triangle.antialiasingWidth() * 0.25, hy.b - hy.normal() * triangle.antialiasingWidth() * 0.25);
			result.push_back(buildObject([&](std::optional<GDProperties> context) {
				return GDProperties::fromLine(line, triangle.antialiasingWidth() * 0.5);
			}));			
		}
		else {			
			result.push_back(buildObject([&](std::optional<GDProperties> context) {
				return GDProperties::fromLine(hy, triangle.antialiasingWidth());
			}));
		}
	}
	return result;
}

std::vector<GDProperties> Layer::fillOutline() {
	std::vector<GDProperties> result;
	float width = getProperty(LayerProperty::Name::LineWidth)->getValue();

	for (const auto& poly : getDecompositionInflated()) {
		for (const auto& edge : poly.edges()) {
			result.push_back(buildObject([&](std::optional<GDProperties> context) {
				float width = evaluateProperty(LayerProperty::Name::LineWidth, context);
				return GDProperties::fromLine(edge, width);
			}));			
		}		
	}
	for (const auto& poly : getDecompositionInflated()) {
		for (const auto& point : poly.points) {
			result.push_back(buildObject([&](std::optional<GDProperties> context) {
				float radius = evaluateProperty(LayerProperty::Name::LineWidth, context) / 2;
				return GDProperties::fromCircle(Circle({ point, radius }));
			}));
		}			
	}

	return result;
}

std::vector<GDProperties> Layer::fillGlow() {
	std::vector<GDProperties> result;

	for (const auto& poly : getDecompositionInflated()) {
		for (const auto& edge : poly.edges()) {
			using enum GDProperty;			

			auto base = buildObject([&](std::optional<GDProperties> context) {
				float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
				return GDProperties({ {ID, nameID["glow-quarter"]}, {X, edge.a.x}, {Y, edge.a.y}, {ScaleX, width}, {ScaleY, width} });
				});
			result.push_back(base);
			float scale = evaluateProperty(LayerProperty::Name::GlowWidth, base);

			Line middle = edge;
			middle.shrink(scale * (gdUnit / 4.0));
			CCPoint normal = poly.normalAt(middle.midpoint());			

			if (edge.length() > scale * (gdUnit / 2.0)) {									
				result.push_back(buildObject([&](std::optional<GDProperties> context) {					
					float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
					CCPoint position = middle.midpoint() + normal * (width * (gdUnit / 6.0));
					float rotation = -CC_RADIANS_TO_DEGREES(normal.getAngle() - (PI / 2));
					return GDProperties({ {ID, nameID["glow-line-quarter"]}, {X, position.x}, {Y, position.y}, {Rotation,  rotation}, {ScaleX, middle.length() / (gdUnit / 2)}, {ScaleY, width} });
				}));

				result.push_back(buildObject([&](std::optional<GDProperties> context) {
					float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
					CCPoint position = middle.b + normal * (width * (gdUnit / 6.0)) + polar(width * (gdUnit / 6.0), normal.getAngle() + (PI / 2));
					float rotation = -CC_RADIANS_TO_DEGREES(normal.getAngle() - (PI / 2));					
					return GDProperties({ {ID, nameID["glow-corner-quarter"]}, {X, position.x}, {Y, position.y}, {Rotation, rotation}, {ScaleX, width}, {ScaleY, width} });
				}));	

				result.push_back(buildObject([&](std::optional<GDProperties> context) {
					float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
					CCPoint position = middle.a + normal * (width * (gdUnit / 6.0)) + polar(width * (gdUnit / 6.0), normal.getAngle() - (PI / 2));
					float rotation = -CC_RADIANS_TO_DEGREES(normal.getAngle() + PI);					
					return GDProperties({ {ID, nameID["glow-corner-quarter"]}, {X, position.x}, {Y, position.y}, {Rotation, rotation}, {ScaleX, width}, {ScaleY, width} });
				}));
			}
			else if (edge.length() > scale * (gdUnit / 4.0)) {	
				result.push_back(buildObject([&](std::optional<GDProperties> context) {					
					float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
					return GDProperties({ {ID, nameID["glow-quarter"]}, {X, edge.midpoint().x}, {Y, edge.midpoint().y}, {ScaleX, width}, {ScaleY, width} });
				}));				
			}
			
			
		}
	}

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




