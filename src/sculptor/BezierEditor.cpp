#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/Geometry.hpp"
#include "../lib/DragNode.hpp"
#include "../sculptor/BezierEditor.hpp"
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

namespace Sculptor {

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

    void BezierEditor::setPoint(int curveIndex, int pointIndex, CCPoint point) {
        this->curves[curveIndex][pointIndex] = std::move(point);
        setCurveDirty(curveIndex);
        if (pointIndex == 0) {
            setCurveDirty(safeModulo(curveIndex - 1, curves.size()));
        }
        else if (pointIndex == getCurve(curveIndex).size()) {
            setCurveDirty(safeModulo(curveIndex + 1, curves.size()));
        }
    }

    void BezierEditor::setCurve(int index, BezierCurve curve) {
        this->curves[index] = std::move(curve);
        setCurveDirty(index);
        setCurveDirty(safeModulo(index - 1, curves.size()));
        setCurveDirty(safeModulo(index + 1, curves.size()));
    }

    void BezierEditor::setCurves(BezierCurves curves) {
        this->curves = std::move(curves);
        setDirty();
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
                drawLines(dashes.data(), dashes.size(), 0.23, { 1.0, 1.0, 1.0, 0.35 });
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
        int c = safeModulo(curveIndex, curves.size());
        int p = safeModulo(pointIndex, curves[c].size());
        int index = 0;
        for (int i = 0; i < c; i++) {
            index += curves[i].size() - 1;
        }
        return index + p;
    }

    std::vector<std::pair<int, int>> BezierEditor::nodeToCurvePointIndices(int i) {
        if (i == 0) {
            return { std::make_pair(curves.size() - 1, curves.back().size() - 1),
                     std::make_pair(0, 0) };
        }

        int accum = 0;
        for (const auto& [n, curve] : std::views::enumerate(curves)) {
            int size = curve.size() - 1;
            if (accum + size > i) {
                return { std::make_pair(n, i - accum) };
            }
            else if (accum + size == i) {
                return { std::make_pair(n, i - accum),
                         std::make_pair((n + 1) % curves.size(), 0) };
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
            auto indices = nodeToCurvePointIndices(getNodeIndex(node));

            if (indices.size() > 1) {
                auto l = indices.front().first;
                auto r = indices.back().first;
                setCurve(l, BezierCurve({ getCurve(l).front(), getCurve(r).back() }));
                removeCurve(r);
                rebuildNodes();
            }

            else {
                auto [curveIndex, pointIndex] = indices[0];
                removePoint(curveIndex, pointIndex);
            }
            };
        node->onClick = [this, node](CCPoint point) {
            if (CCKeyboardDispatcher::get()->m_bAltPressed) {
                auto indices = nodeToCurvePointIndices(getNodeIndex(node));
                if (indices.size() > 1) {
                    auto [c0, p0] = indices.front();
                    auto [c1, p1] = indices.back();
                    auto left = getCurve(c0);
                    auto right = getCurve(c1);
                    left.points.append_range(right.points | std::views::drop(1));
                    setCurve(c0, left);
                    removeCurve(c1);
                    nodes[curvePointToNodeIndex(c0, p0)]->simulateClick();
                }
                else {
                    auto [c, p] = indices[0];
                    auto curve = getCurve(c);
                    BezierCurve left(curve.points | std::views::take(p + 1));
                    BezierCurve right(curve.points | std::views::drop(p));
                    setCurve(c, left);
                    addCurve(c + 1, right);
                    nodes[curvePointToNodeIndex(c + 1, 0)]->simulateClick();

                }
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
        CCPoint projection;
        bool ctrl = CCKeyboardDispatcher::get()->m_bControlPressed;
        projection = ctrl ? Poly::fromBezierCurves(curves).projectionOf(mouse) : getApproximation().projectionOf(mouse);
        bool condition = (mouse.getDistance(projection) < visibilityRadius) && std::ranges::none_of(nodes, [projection](DragNode* node) { return node->getPosition().getDistance(projection) < visibilityRadius; });

        if (!UINode && condition) {
            addUINode();

            UINode->onClick = [this](CCPoint point) {
                if (CCKeyboardDispatcher::get()->m_bControlPressed) {
                    auto i = Poly::fromBezierCurves(curves).edgeIndicesContaining(point)[0];
                    auto [c, p] = nodeToCurvePointIndices(i).back();
                    addPoint(c, p + 1, point);
                    nodes[i + 1]->simulateClick();
                    removeUINode();
                }
                else {
                    for (const auto& [i, seq] : std::views::enumerate(m_approximation)) {
                        std::optional<float> t = seq.inverseLerp(point);
                        if (t) {
                            auto [l, r] = getCurve(i).split(*t);
                            if (CCKeyboardDispatcher::get()->m_bAltPressed) {
                                setCurve(i, BezierCurve({ l.front(), l.back() }));
                                addCurve(i + 1, BezierCurve({ r.front(), r.back() }));
                            }
                            else {
                                setCurve(i, l);
                                addCurve(i + 1, r);
                            }
                            nodes[curvePointToNodeIndex(i + 1, 0)]->simulateClick();
                            removeUINode();
                        }
                    }
                }
                };





        }
        else if (UINode && !condition) {
            removeUINode();
        }
        if (UINode) {
            UINode->setPosition(projection);
        }
    }
}