#pragma once

#include <Geode/Geode.hpp>
#include "../lib/Interface.hpp"
#include "../lib/PropertyEditor.hpp"


using namespace geode::prelude;

namespace Sculptor {

	class Form;

	struct ModulatorPropertyInfo {
		std::string label;
		float defaultValue;
		CommonFilter filter;
	};

	class ModulatorProperty : public Property {
	public:

		using Property::Property;

		enum class Name {
			Ramp
		};

		Name name;

		ModulatorProperty(Name name, std::optional<float> value = std::nullopt) :
			Property(info[name].label, info[name].filter, info[name].defaultValue), name(name) {
			if (value) {
				setValue(*value);
			}
		}

		static std::unordered_map<Name, ModulatorPropertyInfo> info;
	};

	struct ModulatorTypeInfo {
		std::string label;
		std::vector<ModulatorProperty::Name> propertyNames;
		std::function<float(Modulator*, GDProperties&, Layer*)> evaluate;

		std::string spriteName;
		ccColor3B color;
	};


	class Modulator {
	public:

		enum class Type {
			Noise,
			X,
			Y,
			Perlin,
			Normal,

			Count
		};

		Type type;
		std::string label;
		Form* form = nullptr;
		std::unordered_map<ModulatorProperty::Name, ModulatorProperty> properties;

		Modulator(Type type, std::optional<std::unordered_map<ModulatorProperty::Name, ModulatorProperty>> properties = std::nullopt);
		Modulator(const Modulator&) = delete;
		Modulator& operator=(const Modulator&) = delete;

		ModulatorProperty* getProperty(ModulatorProperty::Name name);

		float evaluate(GDProperties& objProps, Layer* layer) { return info[type].evaluate(this, objProps, layer); }

		static std::unordered_map<Type, ModulatorTypeInfo> info;

	private:

		float evaluateNoise(GDProperties&, Layer*);
		float evaluateX(GDProperties&, Layer*);
		float evaluateY(GDProperties&, Layer*);
		float evaluatePerlin(GDProperties&, Layer*);
		float evaluateNormal(GDProperties&, Layer*);

	};

}

