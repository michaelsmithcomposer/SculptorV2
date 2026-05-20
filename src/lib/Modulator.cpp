#include <Geode/Geode.hpp>
#include "../lib/Modulator.hpp"
#include "../lib/Interface.hpp"
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"
#include "../external/FastNoiseLite.h"

using namespace geode::prelude;
using namespace Sculptor;


std::unordered_map<Modulator::Type, ModulatorTypeInfo> Modulator::info = {
		{Type::Noise, {"Noise", {}, [](Modulator* modulator, GDProperties& objProps, Layer* layer) { return modulator->evaluateNoise(objProps, layer); }, "mod_noise.png"_spr, ccc3(150, 150, 255)}},
		{Type::X, {"X", {ModulatorProperty::Name::Ramp}, [](Modulator* modulator, GDProperties& objProps, Layer* layer) { return modulator->evaluateX(objProps, layer); }, "mod_x.png"_spr, ccc3(255, 150, 150)}},
		{Type::Y, {"Y", {ModulatorProperty::Name::Ramp}, [](Modulator* modulator, GDProperties& objProps, Layer* layer) { return modulator->evaluateY(objProps, layer); }, "mod_y.png"_spr, ccc3(150, 255, 150)}},
		/*{Type::Perlin, {"Perlin", {}, [](Modulator* modulator, GDProperties& objProps, Layer* layer) { return modulator->evaluatePerlin(objProps, layer); }}},*/
		{Type::Normal, {"Normal", {}, [](Modulator* modulator, GDProperties& objProps, Layer* layer) { return modulator->evaluateNormal(objProps, layer); }, "mod_normal.png"_spr, ccc3(200, 100, 200)}},
};

std::unordered_map<ModulatorProperty::Name, ModulatorPropertyInfo> ModulatorProperty::info = {
		{Name::Ramp, {"Ramp", 1, CommonFilter::Float}},
	
};

void Modulator::addProperty(ModulatorProperty::Name name) {
	auto [p, success] = properties.emplace(name, name);
	ModulatorProperty* prop = &p->second;
	prop->setCallback([this] { form->setDirty(); });
}

ModulatorProperty* Modulator::getProperty(ModulatorProperty::Name name) {
	auto p = properties.find(name);
	if (p != properties.end()) return &p->second;
	return nullptr;
}

float Modulator::evaluateNoise(GDProperties& objProps, Layer* layer) {
	return CCRANDOM_MINUS1_1();
}

float Modulator::evaluateX(GDProperties& objProps, Layer* layer) {
	CCRect bounds = layer->form->getApproximation().boundingBox();
	float x = bounds.origin.x;
	float exp = getProperty(ModulatorProperty::Name::Ramp)->getValue();
	return pow(clamp(inverseLerp(x, x + bounds.size.width, any_cast<float>(objProps.properties.at(GDProperty::X))), 0, 1), exp);	
}

float Modulator::evaluateY(GDProperties& objProps, Layer* layer) {
	CCRect bounds = layer->form->getApproximation().boundingBox();
	float y = bounds.origin.y;
	float exp = getProperty(ModulatorProperty::Name::Ramp)->getValue();
	return pow(clamp(inverseLerp(y, y + bounds.size.height, any_cast<float>(objProps.properties.at(GDProperty::Y))), 0, 1), exp);
}

float Modulator::evaluatePerlin(GDProperties& objProps, Layer* layer) {
	return 0;
}

float Modulator::evaluateNormal(GDProperties& objProps, Layer* layer) {
	auto normal = layer->form->getApproximation().normalAt({ any_cast<float>(objProps.properties.at(GDProperty::X)), any_cast<float>(objProps.properties.at(GDProperty::Y)) });
	return normal.dot({ 0, 1 });
}