
#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"
#include "../lib/Manager.hpp"
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

namespace Sculptor {

	std::unordered_map<Layer::Style, LayerStyleInfo> Layer::info = {
		{Style::Solid, {
			"Solid", {
				LayerProperty::Name::OffsetPath,
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer},
			[](Layer* layer) { return layer->fillSolid(); }}},
		{Style::Outline, {
			"Outline", {
				LayerProperty::Name::OffsetPath,
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer,
				LayerProperty::Name::LineWidth},
			[](Layer* layer) { return layer->fillOutline(); }}},
		{Style::Glow, {
			"Glow", {
				LayerProperty::Name::OffsetPath,
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer,
				LayerProperty::Name::GlowWidth},
			[](Layer* layer) { return layer->fillGlow(); }}},
		{Style::Strips, {
			"Glow", {
				LayerProperty::Name::OffsetPath,
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer,
				LayerProperty::Name::StripAngle},
			[](Layer* layer) { return layer->fillStrips(); }}},
		{Style::OutlineUniform, {
			"Uniform", {
				LayerProperty::Name::OffsetPath,
				LayerProperty::Name::ID,
				LayerProperty::Name::Spacing,
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer},
			[](Layer* layer) { return layer->fillOutlineUniform(); }}},
		{Style::Lines, {
			"Lines", {
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer},
			[](Layer* layer) { return layer->fillLines(); }}},
		{Style::Texture,  {
			"Texture", {
				LayerProperty::Name::Color,
				LayerProperty::Name::ZLayer},
			[](Layer* layer) { return layer->fillTexture(); }}},	
	};

	std::unordered_map<LayerProperty::Name, LayerPropertyInfo> LayerProperty::info = {
			{Name::OffsetPath, {"Path Offset", 0, CommonFilter::Float, false}},
			{Name::OffsetX, {"X", 0, CommonFilter::Float, true, GDProperty::X}},
			{Name::OffsetY, {"Y", 0, CommonFilter::Float, true, GDProperty::Y}},
			{Name::OffsetRotation, {"Rotation", 0, CommonFilter::Float, true, GDProperty::Rotation}},
			{Name::OffsetScaleX, {"Scale X", 0, CommonFilter::Float, true, GDProperty::ScaleX}},
			{Name::OffsetScaleY, {"Scale Y", 0, CommonFilter::Float, true, GDProperty::ScaleY}},
			{Name::OffsetHue, {"Hue", 0, CommonFilter::Float, true, GDProperty::Hue}},
			{Name::OffsetSaturation, {"Saturation", 0, CommonFilter::Float, true, GDProperty::Saturation}},
			{Name::OffsetValue, {"Value", 0, CommonFilter::Float, true, GDProperty::Value}},

			{Name::Color, {"Color", 1, CommonFilter::Uint, true,  GDProperty::Color}},
			{Name::ZLayer, {"Z Layer", 0, CommonFilter::Int, true, GDProperty::ZLayer}},

			{Name::LineWidth, {"Line Width", 1, CommonFilter::Float, true}},
			{Name::GlowWidth, {"Glow Width", 1, CommonFilter::Float, true}},
			{Name::ID, {"ID", 1, CommonFilter::Uint, true, GDProperty::ID}},
			{Name::Spacing, {"Spacing", gdUnit, CommonFilter::Float, false}},
			{Name::Texture, {"Texture", 0, CommonFilter::Uint, true}},
			{Name::StripAngle, {"Strip Angle", 0, CommonFilter::Float, true}},
	};

	LayerProperty* Layer::getProperty(LayerProperty::Name name) {
		auto p = properties.find(name);
		if (p != properties.end()) return &p->second;
		return nullptr;
	}

	std::vector<LayerProperty::Name> Layer::propertyNamesByStyle(Style style) {
		auto props = info[style].uniqueProperties;
		props.append_range(offsetProperties);
		return props;
	}

	std::string Layer::getUniqueName(Style style) {
		return info[style].label;
	}

	void Layer::updateObjects() {
		
		
		updateStyleEvaluation();
		
		
		for (auto& [ID, objs] : objectsByID) {
			if (!styleEvaluationByID.contains(ID)) {
				for (auto* obj : objs) {
					EditorUI::get()->deleteObject(obj, false);
					std::erase(objects, obj);
				}
			}
		}		
		for (const auto& [ID, props] : styleEvaluationByID) {
			int difference = props.size() - objectsByID[ID].size();
			if (difference > 0) {
				for (int i = 0; i < difference; i++) {
					auto object = Manager::get()->createObject(ID, form->getPosition());
					object->m_updateParents = true;
					objects.push_back(object);
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
		for (auto& [ID, vec] : styleEvaluationByID) {
			vec.clear();
		}

		auto t0 = std::chrono::steady_clock::now();

		info[style].fillFunction(this);

		auto t1 = std::chrono::steady_clock::now();
		
		if (Manager::get()->debugPanel) {
			Manager::get()->debugPanel->setValue(fmt::format("{} style", info[style].label), msBetween(t0, t1));			
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

	GDProperties& Layer::buildObject(std::function<GDProperties(std::optional<GDProperties>)> func) {
		auto base = func(std::nullopt);
		auto props = func(base);
		props.applyGenericLayerProperties(this);
		props.properties[GDProperty::X] = any_cast<float>(props.properties.at(GDProperty::X)) + form->getPositionX();
		props.properties[GDProperty::Y] = any_cast<float>(props.properties.at(GDProperty::Y)) + form->getPositionY();
		auto& bucket = styleEvaluationByID[any_cast<int>(props.properties.at(GDProperty::ID))];
		bucket.push_back(std::move(props));
		return bucket.back();
	}

	void Layer::fillSolid() {

		//auto t0 = std::chrono::steady_clock::now();

		//std::vector<GDProperties> result;
		auto triangulation = getTriangulationInflated();
		//auto t1 = std::chrono::steady_clock::now();
		for (const auto& [i, triangle] : std::views::enumerate(triangulation)) {
			buildObject([&](std::optional<GDProperties> context) {
				return GDProperties::fromRightTriangle(triangle, true);
			});
			Line hy = triangle.hypotenuse();
			hy.shrink(0.1 * triangle.antialiasingWidth());
			if (form->getApproximation().containsEdge(triangle.hypotenuse())) {
				Line line = Line(hy.a - hy.normal() * triangle.antialiasingWidth() * 0.25, hy.b - hy.normal() * triangle.antialiasingWidth() * 0.25);
				buildObject([&](std::optional<GDProperties> context) {
					return GDProperties::fromLine(line, triangle.antialiasingWidth() * 0.5);
				});
			}
			else {
				buildObject([&](std::optional<GDProperties> context) {
					return GDProperties::fromLine(hy, triangle.antialiasingWidth());
				});
			}
		}
		/*auto t2 = std::chrono::steady_clock::now();

		if (Manager::get()->debugPanel) {
			Manager::get()->debugPanel->setValue(fmt::format("{} style:tri", info[style].label), msBetween(t0, t1));
			Manager::get()->debugPanel->setValue(fmt::format("{} style:gen", info[style].label), msBetween(t1, t2));
		}*/

		//return result;
	}

	void Layer::fillOutline() {

		//auto t0 = std::chrono::steady_clock::now();

		//std::vector<GDProperties> result;
		float width = getProperty(LayerProperty::Name::LineWidth)->getValue();
		auto decomposition = getDecompositionInflated();
		//auto t1 = std::chrono::steady_clock::now();

		for (const auto& poly : decomposition) {
			for (const auto& edge : poly.edges()) {
				buildObject([&](std::optional<GDProperties> context) {
					float width = evaluateProperty(LayerProperty::Name::LineWidth, context);
					return GDProperties::fromLine(edge, width);
				});
			}
		}
		for (const auto& poly : decomposition) {
			for (const auto& point : poly.points) {
				buildObject([&](std::optional<GDProperties> context) {
					float radius = evaluateProperty(LayerProperty::Name::LineWidth, context) / 2;
					return GDProperties::fromCircle(Circle({ point, radius }));
				});
			}
		}
		/*auto t2 = std::chrono::steady_clock::now();

		if (Manager::get()->debugPanel) {
			Manager::get()->debugPanel->setValue(fmt::format("{} style:decomp", info[style].label), msBetween(t0, t1));
			Manager::get()->debugPanel->setValue(fmt::format("{} style:gen", info[style].label), msBetween(t1, t2));
		}*/

		//return result;
	}

	void Layer::fillGlow() {

		//auto t0 = std::chrono::steady_clock::now();

		//std::vector<GDProperties> result;
		auto decomposition = getDecompositionInflated();
		//auto t1 = std::chrono::steady_clock::now();

		for (const auto& poly : decomposition) {
			for (const auto& edge : poly.edges()) {
				using enum GDProperty;

				auto base = buildObject([&](std::optional<GDProperties> context) {
					float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
					return GDProperties({ {ID, nameID["glow-quarter"]}, {X, edge.a.x}, {Y, edge.a.y}, {ScaleX, width}, {ScaleY, width} });
				});
				//result.push_back(base);
				float scale = evaluateProperty(LayerProperty::Name::GlowWidth, base);

				Line middle = edge;
				middle.shrink(scale * (gdUnit / 4.0));
				CCPoint normal = poly.normalAt(middle.midpoint());

				if (edge.length() > scale * (gdUnit / 2.0)) {
					buildObject([&](std::optional<GDProperties> context) {
						float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
						CCPoint position = middle.midpoint() + normal * (width * (gdUnit / 6.0));
						float rotation = -CC_RADIANS_TO_DEGREES(normal.getAngle() - (PI / 2));
						return GDProperties({ {ID, nameID["glow-line-quarter"]}, {X, position.x}, {Y, position.y}, {Rotation,  rotation}, {ScaleX, middle.length() / (gdUnit / 2)}, {ScaleY, width} });
					});

					buildObject([&](std::optional<GDProperties> context) {
						float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
						CCPoint position = middle.b + normal * (width * (gdUnit / 6.0)) + polar(width * (gdUnit / 6.0), normal.getAngle() + (PI / 2));
						float rotation = -CC_RADIANS_TO_DEGREES(normal.getAngle() - (PI / 2));
						return GDProperties({ {ID, nameID["glow-corner-quarter"]}, {X, position.x}, {Y, position.y}, {Rotation, rotation}, {ScaleX, width}, {ScaleY, width} });
					});

					buildObject([&](std::optional<GDProperties> context) {
						float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
						CCPoint position = middle.a + normal * (width * (gdUnit / 6.0)) + polar(width * (gdUnit / 6.0), normal.getAngle() - (PI / 2));
						float rotation = -CC_RADIANS_TO_DEGREES(normal.getAngle() + PI);
						return GDProperties({ {ID, nameID["glow-corner-quarter"]}, {X, position.x}, {Y, position.y}, {Rotation, rotation}, {ScaleX, width}, {ScaleY, width} });
					});
				}
				else if (edge.length() > scale * (gdUnit / 4.0)) {
					buildObject([&](std::optional<GDProperties> context) {
						float width = evaluateProperty(LayerProperty::Name::GlowWidth, context);
						return GDProperties({ {ID, nameID["glow-quarter"]}, {X, edge.midpoint().x}, {Y, edge.midpoint().y}, {ScaleX, width}, {ScaleY, width} });
					});
				}


			}
		}
		//auto t2 = std::chrono::steady_clock::now();

		/*if (Manager::get()->debugPanel) {
			Manager::get()->debugPanel->setValue(fmt::format("{} style:decomp", info[style].label), msBetween(t0, t1));
			Manager::get()->debugPanel->setValue(fmt::format("{} style:gen", info[style].label), msBetween(t1, t2));
		}*/

		//return result;
	}

	void Layer::fillOutlineUniform() {

		//auto t0 = std::chrono::steady_clock::now();

		//std::vector<GDProperties> result;

		float spacing = evaluateProperty(LayerProperty::Name::Spacing);
		auto decomposition = getDecompositionInflated();
		//auto t1 = std::chrono::steady_clock::now();

		for (const auto& poly : decomposition) {
			int count = poly.length() / spacing;
			for (int i = 0; i < count; i++) {
				float t = static_cast<float>(i) / count;
				CCPoint position = poly.lerp(t);
				buildObject([&](std::optional<GDProperties> context) {
					int ID = validateID(evaluateProperty(LayerProperty::Name::ID, context));
					return GDProperties({ {GDProperty::ID, ID}, {GDProperty::X, position.x}, {GDProperty::Y, position.y} });
				});
			}

		}

		//auto t2 = std::chrono::steady_clock::now();

		/*if (Manager::get()->debugPanel) {
			Manager::get()->debugPanel->setValue(fmt::format("{} style:decomp", info[style].label), msBetween(t0, t1));
			Manager::get()->debugPanel->setValue(fmt::format("{} style:gen", info[style].label), msBetween(t1, t2));
		}*/

		//return result;
	}

	void Layer::fillStrips() {
		//std::vector<GDProperties> result;

		for (const auto& poly : getDecompositionInflated()) {


		}

		/*return result;*/
	}

	void Layer::fillLines() {
		//std::vector<GDProperties> result;

		//return result;
	}

	void Layer::fillTexture() {
		//std::vector<GDProperties> result;

		//return result;
	}
}

