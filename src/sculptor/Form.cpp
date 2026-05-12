#include <Geode/Geode.hpp>
#include <ranges>
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"
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
        delete *l;      
        layers.erase(l); 
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


PathsD Form::calculateDecomposition() const {        
    
    PathD path = getApproximation().asPath();
    return Union({ path }, FillRule::EvenOdd);
    
}

RightTriangles Form::calculateTriangulation() const {
    RightTriangles result;

    PathsD triangles;
    Triangulate(getDecomposition(), 6, triangles);
    for (const auto& path : triangles) {
        auto triangle = Triangle::create(ccp(path.at(0).x, path.at(0).y), ccp(path.at(1).x, path.at(1).y), ccp(path.at(2).x, path.at(2).y));
        if (!triangle) continue;
        RightTrianglePair pair = (*triangle).orthogonalize();
        result.push_back(std::move(pair.left));
        result.push_back(std::move(pair.right));
    }

    return result;
}

