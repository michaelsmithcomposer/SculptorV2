#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/Geometry.hpp"
#include "../lib/DragNode.hpp"
#include "../sculptor/BezierEditor.hpp"
#include "../sculptor/Form.hpp"

using namespace geode::prelude;
using namespace Sculptor;

BezierEditor* BezierEditor::create(Form* form, BezierCurves curves) {
    auto node = new BezierEditor();
    if (node && node->init(form, curves)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool BezierEditor::init(Form* form, BezierCurves curves) {
    if (!CCDrawNode::init()) return false;

    scheduleUpdate();
    this->setBlendFunc({ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR });

    this->form = form;

    addUINode();
    setCurves(curves);    
    rebuildNodes();
    recalculateIfDirty(true);

    return true;
}

void BezierEditor::update(float dt) {
    startUpdate();

    updateUINode();

    endUpdate();
}

void BezierEditor::startUpdate() {
    postUpdate = false;
}

void BezierEditor::endUpdate() {
    postUpdate = true;
    recalculateIfDirty();    
    redraw();
    
}


void BezierEditor::addCurve(int i, BezierCurve curve) {
    setDirty();
    curves.insert(curves.begin() + i, curve);
    rebuildNodes();    

}

void BezierEditor::removeCurve(int i) {
    setDirty();
    curves.erase(curves.begin() + i);
    rebuildNodes();
    
}

void BezierEditor::addPoint(int curveIndex, int pointIndex, CCPoint point) {
    setDirty();
    curves[curveIndex].points.insert(curves[curveIndex].points.begin() + pointIndex, point);
    rebuildNodes();
}

void BezierEditor::removePoint(int curveIndex, int pointIndex) { 
    setDirty();    
    curves[curveIndex].points.erase(curves[curveIndex].points.begin() + pointIndex);   
    rebuildNodes();   
}


void BezierEditor::setDirty() {
    std::fill(dirty.begin(), dirty.end(), true);
    form->setDirty();
}
void BezierEditor::setCurveDirty(int i) {
    dirty[i] = true;
    form->setDirty();
}

void BezierEditor::redraw() {
    clear();
    for (const auto& curve : curves) {
        for (const auto& edge : curve.edges()) {
            Points dashes = Sequence::createDashedLine(edge, 4, 0.35).points;
            drawLines(dashes.data(), dashes.size(), 0.23, { 1.0, 1.0, 1.0, 0.5 });
        }
    }
}

void BezierEditor::redrawIfDirty() {
    if (std::ranges::any_of(dirty, std::identity{})) {
        redraw();
    }
}


void BezierEditor::recalculateIfDirty(bool force) {   

    m_approximation.resize(this->curves.size());
    dirty.resize(this->curves.size(), true);

    if (postUpdate || force) {          
        for (const auto& [i, curve] : std::views::enumerate(curves)) {
            if (dirty[i]) {
                dirty[i] = false;
                m_approximation[i] = curve.approximateUniformT();                
            }
        }               
    }
}

const Poly BezierEditor::getApproximation() const {        
    Poly result;   
    for (const auto& sequence : m_approximation) {        
        result.points.append_range(sequence | std::views::take(sequence.size() - 1));
    }       
    return result;
}

int BezierEditor::getNodeIndex(DragNode* node) {
    auto i = std::ranges::find(nodes, node);
    return std::distance(nodes.begin(), i);
}

int BezierEditor::curvePointToNodeIndex(int curveIndex, int pointIndex) {
    int index = 0;
    for (int i = 0; i < curveIndex; i++) {
        index += curves[i].size() - 1;
    }
    return (index + pointIndex) % curves.size();
}

std::vector<std::pair<int, int>> BezierEditor::nodeToCurvePointIndices(int i) {   
    if (i == 0) {
        return { std::make_pair(0, 0),
                 std::make_pair(curves.size() - 1, curves.back().size() - 1)};
    }

    int accum = 0;
    for (const auto& [n, curve] : std::views::enumerate(curves)) {
        int size = curve.size() - 1;        
        if (accum + size > i) {
            return { std::make_pair(n, i - accum) };
        }
        else if (accum + size == i) {
            return { std::make_pair(n, i - accum),
                     std::make_pair((n + 1) % curves.size(), 0)};
        }
        accum += size;
    }
}

//

void BezierEditor::rebuildNodes() {
    removeAllNodes();

   

    for (const auto& curve : curves) {
        for (const auto& point : curve | std::views::take(curve.size() - 1)) {
            addNode(point);
        }
    }
}

DragNode* BezierEditor::addNode(CCPoint position) {
    auto node = DragNode::create();
    node->onMove = [this, node](CCPoint point) { 
        for (auto [curveIndex, pointIndex] : nodeToCurvePointIndices(getNodeIndex(node))) {
            setPoint(curveIndex, pointIndex, point);
        }        
    };
    node->onRightClick = [this, node](CCPoint point) {
        for (auto [curveIndex, pointIndex] : nodeToCurvePointIndices(getNodeIndex(node))) {
            removePoint(curveIndex, pointIndex);           
        }
    };
    node->setPosition(position);
    nodes.push_back(node);
    addChild(node);
    return node;
}

void BezierEditor::removeNode(DragNode* node) {
    node->removeFromParent();
    std::erase(nodes, node);
}

void BezierEditor::removeAllNodes() {
    for (auto node : nodes) {
        node->removeFromParent();
    }
    nodes.clear();
}

DragNode* BezierEditor::addUINode() {
    UINode = DragNode::create();    
    addChild(UINode);
    return UINode;
}

void BezierEditor::removeUINode() {
    UINode->removeFromParent();
    UINode = nullptr;
}

void BezierEditor::updateUINode() {
    auto mouse = convertToNodeSpace(getMousePos());
    auto projection = Poly::fromBezierCurves(curves).projectionOf(mouse);
    bool condition = (mouse.getDistance(projection) < visibilityRadius) && std::ranges::none_of(nodes, [projection](DragNode* node) { return node->getPosition().getDistance(projection) < visibilityRadius; });

    if (!UINode && condition) {
        addUINode();
    }
    else if (UINode && !condition) {
        removeUINode();
    }
    if (UINode) {
        UINode->setPosition(projection);
    }
}