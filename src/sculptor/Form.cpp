#include <Geode/Geode.hpp>
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;
using namespace Clipper2Lib;

Form* Form::create() {
    auto node = new Form();
    if (node && node->init()) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool Form::init() {
    if (!CCDrawNode::init()) return false;

    //this->scheduleUpdate();
    
    LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->addChild(this);

    return true;
}

Layer* Form::createLayer(const LayerStyle& style) {
    Layer* layer = new Layer(this, style);
    this->layers.push_back(layer);
    return layer;
}

void Form::removeLayer(Layer* layer) {
    auto l = std::find(layers.begin(), layers.end(), layer);
    if (l != layers.end()) {
        delete *l;      
        layers.erase(l); 
    }
}