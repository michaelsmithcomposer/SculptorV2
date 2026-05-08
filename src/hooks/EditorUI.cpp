#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "../lib/Manager.hpp"
#include "../lib/NumberInput.hpp"
#include "../lib/PropertyEditor.hpp"
#include "../lib/Interface.hpp"

using namespace geode::prelude;

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

		auto manager = Manager::create();
		manager->setID("manager"_spr);
		this->addChild(manager);

		return true;
	}

};