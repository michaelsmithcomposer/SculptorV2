#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

class Manager : public CCNode {
public:

	float time = 0;

	static Manager* create();
	static Manager* get();
	bool init();
	void createTab();
	void onTabEnter();
	void onTabExit();

	GameObject* createObject(int ID, CCPoint position) {
		return EditorUI::get()->createObject(ID, roundTo(position, 4));
	}

	bool inTab() {
		return alpha::editor_tabs::getCurrentTab().unwrapOrDefault() == "sculptor"_spr;
	}

private:

	void update(float dt);

	ListenerHandle scrollListener;

	CCNode* page;
	std::vector<Form*> forms;	

	Form* selectedForm;
	Layer* selectedLayer;
	ModSource selectedModSource;

	ScrollLayer* layerScroll;
	CCMenu* layerMenu;
	std::vector<CCNode*> propertyMenus;
	std::vector<PropertyEditor*> propertyEditors;
	std::vector<CCMenuItemSpriteExtra*> modSourceButtons;

	void updateLayerMenu();

	void updatePropertiesMenu();

	void setModSource(ModSource modSource);

	void onButton(CCObject* sender);

	void onLayerSelectButton(CCObject* sender);

	void onModSelectButton(CCObject* sender);

	ListenerResult handleScroll(double x, double y);

	float sinTime(float frequency, float amplitude) {
		return amplitude * sinf(time * frequency * 2.0f * M_PI);
	}


};