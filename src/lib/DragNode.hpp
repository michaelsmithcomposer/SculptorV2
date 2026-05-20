#pragma once

#include <Geode/Geode.hpp>
#include "../lib/Geometry.hpp"

using namespace geode::prelude;
using namespace Sculptor;

class DragNode : public CCDrawNode {
public:

	static DragNode* create();
	bool init();

	std::function<void(CCPoint)> onMove;
	std::function<void(CCPoint)> onClick;
	std::function<void(CCPoint)> onRightClick;		
	std::function<void()> onMouseEnter;
	std::function<void()> onMouseExit;

	void simulateClick();

private:

	ListenerHandle mouseListener;

	

	bool drag = false;
	CCPoint offset;
	CCPoint lastPosition = { 0, 0 };	

	static constexpr float size = 6.f;
	

	ListenerResult handleMouseData(MouseInputData data, bool doCallbacks = true);	

	void update(float dt);
};