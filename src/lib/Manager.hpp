#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

class Manager : public CCNode {
public:

	static Manager* create();
	static Manager* get();
	bool init();
	void createTab();
	void onTabEnter();
	void onTabExit();

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


};