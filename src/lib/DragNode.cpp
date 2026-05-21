#include <Geode/Geode.hpp>
#include "../lib/Geometry.hpp"
#include "../lib/DragNode.hpp"
#include "../lib/Utilities.hpp"

using namespace geode::prelude;

namespace Sculptor {

    DragNode* DragNode::create() {
        auto node = new DragNode();
        if (node && node->init()) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    bool DragNode::init() {
        if (!CCDrawNode::init()) return false;

        scheduleUpdate();


        this->setBlendFunc({ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR });
        mouseListener = MouseInputEvent().listen([this](MouseInputData data) { return this->handleMouseData(data); });

        setContentSize({ size, size });
        setAnchorPoint({ 0.5, 0.5 });
        drawRect({ 0, 0, size, size }, { 0.f, 0.f, 0.f, 0.f }, 0.25, { 1.0, 1.0, 1.0, 0.35 });

        return true;
    }


    ListenerResult DragNode::handleMouseData(MouseInputData data, bool doCallbacks) {

        if (data.action == MouseInputData::Action::Press) {
            CCPoint mouse = convertToNodeSpace(getMousePos());
            if (rangeContains(0, size, mouse.x) && rangeContains(0, size, mouse.y)) {
                drag = true;
                auto position = getPosition();
                offset = getParent()->convertToNodeSpace(getMousePos()) - position;
                if (data.button == MouseInputData::Button::Left && onClick && doCallbacks) { onClick(position); }
                if (data.button == MouseInputData::Button::Right && onRightClick && doCallbacks) { onRightClick(position); }
                return ListenerResult::Stop;
            }
        }
        else {
            drag = false;
        }
        return ListenerResult::Propagate;
    }

    void DragNode::update(float dt) {
        auto position = getPosition();
        if (drag) {
            CCPoint mouse = getParent()->convertToNodeSpace(getMousePos());
            setPosition(mouse - offset);
            if (onMove && (position != lastPosition)) {
                onMove(position);
            }
        }
        lastPosition = position;


    }

    void DragNode::simulateClick() {
        MouseInputData data = MouseInputData(MouseInputData::Button::Left, MouseInputData::Action::Press, 0, {});
        handleMouseData(data, false);
    }

}