#include <Geode/Geode.hpp>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include "../lib/Manager.hpp"
#include "../sculptor/Serialization.hpp"

using namespace geode::prelude;
using namespace Sculptor;

#include <Geode/modify/LevelEditorLayer.hpp>
class $modify(MyEditor, LevelEditorLayer) {
	bool init(GJGameLevel * level, bool noUI) {
		if (!LevelEditorLayer::init(level, noUI)) return false;

		Manager* manager = Manager::create();
		this->addChild(manager);

		auto data = alpha::level_storage::getSaveContainer(LevelEditorLayer::get(), Mod::get());
		matjson::Serialize<Manager*>::fromJson(data["manager"]);
		

		return true;
	}

};
