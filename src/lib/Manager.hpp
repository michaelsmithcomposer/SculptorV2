#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

class SelectionPopup;

class Manager : public CCNode {
public:

	float time = 0;

	Form* selectedForm;
	Layer* selectedLayer;
	Modulator* selectedModulator;

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

	CCPoint toEditorSpace(CCPoint point) {
		return LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->convertToNodeSpace(point);
	}

	CCPoint fromEditorSpace(CCPoint point) {
		return LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->convertToWorldSpace(point);
	}

private:

	void update(float dt);

	ListenerHandle scrollListener;
	ListenerHandle mouseListener;

	CCNode* page;
	std::vector<Form*> forms;	

	CCNode* layerMenu;
	CCNode* modulatorMenu;
	ScrollLayer* layerPropertiesScroll;
	ScrollLayer* modulatorPropertiesScroll;

	std::vector<ModulatablePropertyEditor*> layerPropertyEditors;
	std::vector<SinglePropertyEditor*> modulatorPropertyEditors;	

	CCNode* createFormPanel(CCSize size, CCPoint position);
	NineSlice* createBase(CCSize size);
	CCNode* createVerticalScroll(CCSize size, int columns);
	CCNode* createLayerMenu(CCSize size, int columns);
	CCNode* createModulatorMenu(CCSize size, int columns);
	ScrollLayer* createPropertiesScroll(CCSize size, int rows);

	SelectionPopup* selectionPopup;

	void updateLayerMenu();
	void updateModulatorMenu();
	void updateLayerPropertiesMenu();	
	void updateModulatorPropertiesMenu();	

	void onButton(CCObject* sender);

	void onFormNewButton(CCObject* sender);
	void onFormDeleteButton(CCObject* sender);

	void onLayerSelectButton(CCObject* sender);
	void onModulatorSelectButton(CCObject* sender);

	void onLayerNewButton(CCObject* sender);	
	void onModulatorNewButton(CCObject* sender);

	void onLayerDeleteButton(CCObject* sender);
	void onModulatorDeleteButton(CCObject* sender);
	

	void selectForm(Form* form) {
		bool changed = selectedForm != form;
		selectedForm = form;		
		if (changed) {
			for (auto& f : forms) {
				if (f == selectedForm) {
					f->bezierEditor->setVisible(true);
				}
				else {
					f->bezierEditor->setVisible(false);
				}
			}
			selectedModulator = !selectedForm->modulators.empty() ? selectedForm->modulators[0] : nullptr;
			selectedLayer = !selectedForm->layers.empty() ? selectedForm->layers[0] : nullptr;
		}		
		updateLayerMenu();
		updateModulatorMenu();
		updateLayerPropertiesMenu();
		updateModulatorPropertiesMenu();
	}
	void selectLayer(Layer* layer) {
		selectedLayer = layer;		
		updateLayerMenu();
		updateLayerPropertiesMenu();
	}
	void selectModulator(Modulator* modulator) {
		selectedModulator = modulator;
		updateModulatorMenu();
		updateLayerPropertiesMenu();
		updateModulatorPropertiesMenu();
	}

	ListenerResult handleScroll(double x, double y);
	ListenerResult handleMouseData(MouseInputData data);

	float sinTime(float frequency, float amplitude) {
		return amplitude * sinf(time * frequency * 2.0f * M_PI);
	}


};

class SelectionPopup : public geode::Popup {
public:
	static SelectionPopup* create(CCPoint position, CCSize size, std::vector<std::string> labels, std::function<void(int)> callback) {
		auto node = new SelectionPopup();
		if (node->init(position, size, labels, callback)) {
			node->autorelease();
			return node;
		}
		delete node;
		return nullptr;
	}

	bool init(CCPoint position, CCSize size, std::vector<std::string> labels, std::function<void(int)> callback);

	void onButton(CCObject* sender) {
		if (callback) {
			callback(sender->getTag());
		}
		onClose(nullptr);
	}

private:

	std::function<void(int)> callback;

};