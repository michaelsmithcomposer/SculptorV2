#pragma once

#include <Geode/Geode.hpp>
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"


using namespace geode::prelude;

namespace Sculptor {

	class Form;

	struct LayerPropertyInfo {
		std::string label;
		float defaultValue;
		CommonFilter filter;
		bool modulatable;
		std::optional<GDProperty> gdProperty;

	};

	class LayerProperty : public ModulatableProperty {
	public:

		using ModulatableProperty::ModulatableProperty;

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
			Texture,
			StripAngle,
		};

		Name name;

		LayerProperty(Name name, std::optional<float> value = std::nullopt, std::optional<std::unordered_map<Modulator*, float>> modulators = std::nullopt) :
			ModulatableProperty(info[name].label, info[name].filter, info[name].defaultValue), name(name) {
			if (value) {
				setValue(*value);
			}
			if (modulators) {
				this->modValues = *modulators;
			}
		}

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
		std::function<void(Layer* layer)> fillFunction;
	};


	class Layer {
	public:

		enum class Style {
			Solid,
			Outline,
			Glow,
			OutlineUniform,
			Strips,
			Lines,
			Texture,			

			Count
		};

		Form* form;
		std::string name;
		Style style;
		std::unordered_map<LayerProperty::Name, LayerProperty> properties;

		Layer(Style style, std::optional<std::unordered_map<LayerProperty::Name, LayerProperty>> properties = std::nullopt) : style(style), name(getUniqueName(style)) {
			for (const auto& name : propertyNamesByStyle(style)) {
				if (properties && (*properties).contains(name)) {
					this->properties.emplace(name, std::move((*properties).at(name)));
				}
				else {
					this->properties.emplace(name, name);
				}
				this->properties.at(name).setCallback([this] { updateObjects(); });
			}
		}
		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;

		LayerProperty* getProperty(LayerProperty::Name name);

		void updateObjects();
		void deleteAllObjects();
		std::vector<GameObject*> getObjects() { return objects; }

		static std::vector<LayerProperty::Name> propertyNamesByStyle(Style style);
		static std::unordered_map<Style, LayerStyleInfo> info;

	private:



		std::vector<GameObject*> objects;
		std::unordered_map<int, std::vector<GameObject*>> objectsByID;
		std::unordered_map<int, std::vector<GDProperties>> styleEvaluationByID;

		std::string getUniqueName(Style style);



		void updateStyleEvaluation();
		void updateIDMap();

		float evaluateProperty(LayerProperty::Name name, std::optional<GDProperties> objProps = std::nullopt);

		Polys getDecompositionInflated();
		RightTriangles getTriangulationInflated();

		GDProperties& buildObject(std::function<GDProperties(std::optional<GDProperties>)> func);

		void fillSolid();
		void fillOutline();
		void fillGlow();
		void fillOutlineUniform();
		void fillStrips();
		void fillLines();
		void fillTexture();
		







	};




}