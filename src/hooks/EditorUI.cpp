#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include "../lib/Manager.hpp"
#include "../lib/NumberInput.hpp"
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"
#include "../sculptor/Serialization.hpp"

using namespace geode::prelude;
using namespace Sculptor;

#include <Geode/modify/EditorUI.hpp>
class $modify(SculptorEditorUI, EditorUI) {
	bool init(LevelEditorLayer * layer) {
		if (!EditorUI::init(layer)) return false;

		alpha::editor_tabs::addTab("sculptor"_spr, alpha::editor_tabs::EDIT, [this] {

			std::vector<Ref<CCNode>> nodes;
			auto root = CCNode::create();
			nodes.push_back(root);
			return alpha::editor_tabs::createEditButtonBar(nodes);

		}, [] { return CCNode::create(); }, 
			
			[](bool state, auto tab) {
				
				if (state) {
					Manager::get()->onTabEnter();
				}
				else {
					Manager::get()->onTabExit();
				}

		}, [](int rows, int cols, auto tab) {});
		

		return true;
	}

	void selectObject(GameObject* object, bool ignoreFilter) {
		if (!Manager::get()->shouldSelectObject(object)) return;
		EditorUI::selectObject(object, ignoreFilter);
	}

	void selectObjects(CCArray* objects, bool ignoreFilter) {
		auto toSelect = CCArray::create();
		for (const auto& object : CCArrayExt<GameObject*>(objects)) {
			if (Manager::get()->shouldSelectObject(object)) {
				toSelect->addObject(object);
			}
		}		
		EditorUI::selectObjects(toSelect, ignoreFilter);
	}

};