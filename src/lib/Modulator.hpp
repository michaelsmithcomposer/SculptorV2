#pragma once

#include <Geode/Geode.hpp>
#include "../lib/Interface.hpp"
#include "../lib/PropertyEditor.hpp"


using namespace geode::prelude;
using namespace Sculptor;


class Form;

struct ModulatorPropertyInfo {
	std::string label;
	float defaultValue;
	CommonFilter filter;
};

class ModulatorProperty : public Property {
public:

	enum class Name {
		Ramp
	};

	Name name;

	ModulatorProperty(Name name) :
		Property(info[name].label, info[name].filter, info[name].defaultValue), name(name) {
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
	Form* form;

	Modulator(Form* form, Type type) : form(form), type(type) {
		label = info[type].label;
		for (const auto& name : info[type].propertyNames) {
			addProperty(name);
		}		
	}
	Modulator(const Modulator&) = delete;
	Modulator& operator=(const Modulator&) = delete;

	void addProperty(ModulatorProperty::Name name);
	ModulatorProperty* getProperty(ModulatorProperty::Name name);

	float evaluate(GDProperties& objProps, Layer* layer) { return info[type].evaluate(this, objProps, layer); }

	static std::unordered_map<Type, ModulatorTypeInfo> info;

private:

	std::unordered_map<ModulatorProperty::Name, ModulatorProperty> properties;

	float evaluateNoise(GDProperties&, Layer*);
	float evaluateX(GDProperties&, Layer*);
	float evaluateY(GDProperties&, Layer*);
	float evaluatePerlin(GDProperties&, Layer*);
	float evaluateNormal(GDProperties&, Layer*);

};



