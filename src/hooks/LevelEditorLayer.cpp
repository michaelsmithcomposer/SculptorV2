#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/LevelEditorLayer.hpp>
class $modify(MyEditor, LevelEditorLayer) {
	bool init(GJGameLevel * level, bool noUI) {
		if (!LevelEditorLayer::init(level, noUI)) return false;


		return true;
	}

};
