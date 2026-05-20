#include <Geode/Geode.hpp>
#include <ranges>
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"
#include "../lib/Manager.hpp"
#include "../sculptor/BezierEditor.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;
using namespace Clipper2Lib;

Form* Form::create(BezierCurves curves) {
    auto node = new Form();
    if (node && node->init(curves)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool Form::init(BezierCurves curves) {
    if (!CCNode::init()) return false;   

    scheduleUpdate();
    
    LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->addChild(this);
    bezierEditor = BezierEditor::create(this, curves);    
    addChild(bezierEditor);    

    return true;
}

void Form::onExit() {
	Manager::get()->selectedForm = nullptr;
    Manager::get()->selectedLayer = nullptr;
    Manager::get()->selectedModulator = nullptr;
    for (auto& layer : layers) {
        layer->deleteAllObjects();
        delete layer;
	}
	CCNode::onExit();
}

void Form::update(float dt) {
    startUpdate();    

    endUpdate();
}

void Form::startUpdate() {
    postUpdate = false;
}

void Form::endUpdate() {
    postUpdate = true;
    recalculateIfDirty();
}


Layer* Form::createLayer(const LayerStyle& style) {
    Layer* layer = new Layer(this, style);
    this->layers.push_back(layer);  
    return layer;
}

void Form::removeLayer(Layer* layer) {
    auto l = std::find(layers.begin(), layers.end(), layer);
    if (l != layers.end()) {
        if (Manager::get()->selectedLayer == layer) {
			Manager::get()->selectedLayer = nullptr;
        }
        layer->deleteAllObjects();
        delete *l;      
        layers.erase(l); 
    }    
}

Modulator* Form::createModulator(const Modulator::Type& type) {
    Modulator* modulator = new Modulator(this, type);
    this->modulators.push_back(modulator);
    return modulator;
}

void Form::removeModulator(Modulator* modulator) {
    auto m = std::find(modulators.begin(), modulators.end(), modulator);
    if (m != modulators.end()) {
        if (Manager::get()->selectedModulator == modulator) {
            Manager::get()->selectedModulator = nullptr;
        }
        for (auto& layer : layers) {
            for (auto& [name, prop] : layer->properties) {
                prop.removeModulator(modulator);
            }
        }
        delete* m;
        modulators.erase(m);
    }
}

void Form::setDirty() {
    dirty = true;       
}


void Form::recalculateIfDirty(bool force) const {    
    if ((dirty && postUpdate) || force) {        
        dirty = false;
        bezierEditor->recalculateIfDirty(true);
        m_approximation = calculateApproximation();
        m_decomposition = calculateDecomposition();
        m_triangulation = calculateTriangulation();              

        for (const auto& layer : layers) {
            layer->updateObjects();
        }
        
    }
}

Poly Form::calculateApproximation() const {    
    return bezierEditor->getApproximation();
}


Polys Form::calculateDecomposition() const {         
    Polys result;
    
    PathsD paths = Union({ getApproximation().asPath() }, FillRule::EvenOdd);
    for (const auto& path : paths) {
        result.push_back(Sequence::fromPath(path));
    }

    return result;    
}

RightTriangles Form::calculateTriangulation() const {
	return triangulatePolygons(getDecomposition());
}

