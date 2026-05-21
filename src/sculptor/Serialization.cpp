
#include <Geode/Geode.hpp>
#include "../sculptor/Serialization.hpp"

using namespace geode::prelude;
using namespace Sculptor;

geode::Result<CCPoint> matjson::Serialize<CCPoint>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(double x, value["x"].asDouble());
	GEODE_UNWRAP_INTO(double y, value["y"].asDouble());
	return geode::Ok(ccp(x, y));
}

matjson::Value matjson::Serialize<CCPoint>::toJson(const CCPoint& point) {
	return matjson::makeObject({ {"x", point.x}, {"y", point.y} });
}

//

geode::Result<Sequence> matjson::Serialize<Sequence>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(Points points, value["points"].as<Points>());	
	return geode::Ok(Sequence(points));
}

matjson::Value matjson::Serialize<Sequence>::toJson(const Sequence& sequence) {
	return matjson::makeObject({ {"points", sequence.points} });
}

//

geode::Result<BezierCurve> matjson::Serialize<BezierCurve>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(Points points, value["points"].as<Points>());
	return geode::Ok(BezierCurve(points));
}

matjson::Value matjson::Serialize<BezierCurve>::toJson(const BezierCurve& curve) {
	return matjson::makeObject({ {"points", curve.points} });
}

//

geode::Result<Sculptor::Modulator*> matjson::Serialize<Sculptor::Modulator*>::fromJson(const matjson::Value& value) {	
	using namespace Sculptor;
	GEODE_UNWRAP_INTO(int type, value["type"].as<int>());
	std::unordered_map<ModulatorProperty::Name, ModulatorProperty> properties;

	for (const auto& prop : value["properties"]) {
		GEODE_UNWRAP_INTO(int nameValue, prop["name"].as<int>());
		GEODE_UNWRAP_INTO(float propValue, prop["value"].as<float>());
		auto name = static_cast<ModulatorProperty::Name>(nameValue);
		properties.emplace(name, ModulatorProperty(name, propValue));
	}

	return geode::Ok(new Sculptor::Modulator(static_cast<Sculptor::Modulator::Type>(type), std::move(properties)));
}

matjson::Value matjson::Serialize<Sculptor::Modulator*>::toJson(const Sculptor::Modulator* modulator) {
	std::vector<matjson::Value> props;
	for (const auto& [name, prop] : modulator->properties) {
		props.push_back(matjson::makeObject({
			{"name", static_cast<int>(name)},
			{"value", prop.getValue()},
		}));
	}
	return matjson::makeObject({ {"type", static_cast<int>(modulator->type)}, {"properties", props} });
}

//

geode::Result<Sculptor::Layer*> matjson::Serialize<Sculptor::Layer*>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(int formID, value["formID"].as<int>());
	GEODE_UNWRAP_INTO(int style, value["style"].as<int>());
	std::unordered_map<Sculptor::LayerProperty::Name, Sculptor::LayerProperty> properties;

	for (const auto& prop : value["properties"]) {
		GEODE_UNWRAP_INTO(int nameValue, prop["name"].as<int>());
		GEODE_UNWRAP_INTO(float propValue, prop["value"].as<float>());
		auto name = static_cast<Sculptor::LayerProperty::Name>(nameValue);

		std::unordered_map<Sculptor::Modulator*, float> modulators;
		for (const auto& modulator : prop["modulators"]) {
			GEODE_UNWRAP_INTO(float modID, modulator["modulatorID"].as<int>());
			GEODE_UNWRAP_INTO(float modValue, modulator["value"].as<float>());
			modulators.emplace(Manager::get()->forms[formID]->modulators[modID], modValue);
		}
		
		properties.emplace(name, Sculptor::LayerProperty(name, propValue, modulators));
	}

	return geode::Ok(new Sculptor::Layer(static_cast<Sculptor::Layer::Style>(style), std::move(properties)));
}

matjson::Value matjson::Serialize<Sculptor::Layer*>::toJson(const Sculptor::Layer* layer) {
	std::vector<matjson::Value> props;
	for (const auto& [name, prop] : layer->properties) {
		std::vector<matjson::Value> modulators;
		for (const auto& [modulator, value] : prop.modValues) {
			modulators.push_back(matjson::makeObject({
				{"modulatorID", layer->form->getModulatorID(modulator)},
				{"value", value}
			}));
		}
		props.push_back(matjson::makeObject({
			{"name", static_cast<int>(name)},
			{"value", prop.getValue()},
			{"modulators", modulators}
		}));
	}
	return matjson::makeObject({ {"formID", Manager::get()->getFormID(layer->form)},  {"style", static_cast<int>(layer->style)}, {"properties", props}});
}

//

geode::Result<Form*> matjson::Serialize<Form*>::fromJson(const matjson::Value& value) {
	GEODE_UNWRAP_INTO(CCPoint position, value["position"].as<CCPoint>());
	GEODE_UNWRAP_INTO(BezierCurves curves, value["curves"].as<BezierCurves>());
	Form* form = Form::create(position, curves);
	if (!form) {
		log::info("no form");
	}
	Manager::get()->addForm(form);

	for (const auto& modulatorValue : value["modulators"]) {
		form->addModulator(matjson::Serialize<Sculptor::Modulator*>::fromJson(modulatorValue).unwrap());
	}

	for (const auto& layerValue : value["layers"]) {
		form->addLayer(matjson::Serialize<Layer*>::fromJson(layerValue).unwrap());
	}

	return geode::Ok(form);
}

matjson::Value matjson::Serialize<Form*>::toJson(const Form* form) {
	std::vector<matjson::Value> layers;
	for (const auto& layer : form->layers) {
		layers.push_back(matjson::Serialize<Layer*>::toJson(layer));
	}
	std::vector<matjson::Value> modulators;
	for (const auto& modulator : form->modulators) {
		modulators.push_back(matjson::Serialize<Sculptor::Modulator*>::toJson(modulator));
	}
	return matjson::makeObject({ {"position", form->position}, {"curves", form->bezierEditor->getCurves()}, {"layers", layers}, {"modulators", modulators}});
}

//


geode::Result<Manager*> matjson::Serialize<Manager*>::fromJson(const matjson::Value& value) {

	GEODE_UNWRAP_INTO(int ID, value["ID"].as<int>());
	std::vector<GameObject*> toRemove;
	for (const auto& object : CCArrayExt<GameObject*>(LevelEditorLayer::get()->getAllObjects())) {
		if (object->belongsToGroup(ID)) {
			toRemove.push_back(object);
		}
	}
	for (auto& object : toRemove) {
		LevelEditorLayer::get()->removeObject(object, false);
	}

	for (const auto& formValue : value["forms"]) {
		matjson::Serialize<Form*>::fromJson(formValue);
	}

	return geode::Ok(Manager::get());
}

matjson::Value matjson::Serialize<Manager*>::toJson(const Manager* manager) {

	int ID = LevelEditorLayer::get()->getNextFreeGroupID({});
	std::vector<matjson::Value> forms;
	for (const auto& form : manager->forms) {
		forms.push_back(matjson::Serialize<Form*>::toJson(form));
		for (const auto& layer : form->layers) {
			for (auto object : layer->getObjects()) {
				object->addToGroup(ID);
			}
		}
	}
	return matjson::makeObject({ {"forms", forms}, {"ID", ID} });

}