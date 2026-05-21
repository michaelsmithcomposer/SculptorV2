#include <Geode/Geode.hpp>
#include <ranges>
#include "../sculptor/Layer.hpp"
#include "../sculptor/Form.hpp"
#include "../lib/Manager.hpp"
#include "../sculptor/BezierEditor.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;
using namespace Clipper2Lib;

namespace Sculptor {

    Form* Form::create(CCPoint position, BezierCurves curves) {
        auto node = new Form();
        if (node && node->init(position, curves)) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    bool Form::init(CCPoint position, BezierCurves curves) {
        if (!CCNode::init()) return false;

        scheduleUpdate();

        LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->addChild(this);
        bezierEditor = BezierEditor::create(this, curves);
        bezierEditor->setVisible(false);
        addChild(bezierEditor);

        setPosition(position);
        this->position = position;

        return true;
    }

    void Form::onExit() {
        Manager::get()->deselect();
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


    Layer* Form::addLayer(Layer* layer) {
        this->layers.push_back(layer);
        layer->form = this;
        layer->updateObjects();
        return layer;
    }

    void Form::removeLayer(Layer* layer) {
        auto l = std::find(layers.begin(), layers.end(), layer);
        if (l != layers.end()) {
            if (Manager::get()->selectedLayer == layer) {
                Manager::get()->selectedLayer = nullptr;
            }
            layer->deleteAllObjects();
            delete* l;
            layers.erase(l);
        }
    }

    Modulator* Form::addModulator(Modulator* modulator) {
        this->modulators.push_back(modulator);
        modulator->form = this;
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
            auto t0 = std::chrono::steady_clock::now();
            bezierEditor->recalculateIfDirty(true);
            auto t1 = std::chrono::steady_clock::now();
            m_approximation = calculateApproximation();
            auto t2 = std::chrono::steady_clock::now();
            m_decomposition = calculateDecomposition();
            auto t3 = std::chrono::steady_clock::now();
            m_triangulation = calculateTriangulation();
            auto t4 = std::chrono::steady_clock::now();
            for (const auto& layer : layers) {
                layer->updateObjects();
            }
            auto t5 = std::chrono::steady_clock::now();

            if (Manager::get()->debugPanel) {
                Manager::get()->debugPanel->setValue("Bezier", msBetween(t0, t1));
                Manager::get()->debugPanel->setValue("Approx", msBetween(t1, t2));
                Manager::get()->debugPanel->setValue("Decomp", msBetween(t2, t3));
                Manager::get()->debugPanel->setValue("Tri", msBetween(t3, t4));
                Manager::get()->debugPanel->setValue("Total", msBetween(t0, t5));
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
}

