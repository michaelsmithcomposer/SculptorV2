#pragma once

#include <Geode/Geode.hpp>
#include "../sculptor/Layer.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;

class Form : public CCDrawNode {
public:

	Clipper2Lib::PathsD paths;
	std::vector<Layer*> layers;

	static Form* create();
	bool init();	

	Layer* createLayer(const LayerStyle& style);
	void removeLayer(Layer* layer);

};