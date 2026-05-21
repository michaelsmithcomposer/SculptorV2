#pragma once

#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "../sculptor/Form.hpp"

using namespace geode::prelude;

namespace Sculptor {

	template <typename T>
	concept Formattable = fmt::is_formattable<T>::value;

	class SelectionPopup;
	class DebugPanel;

	class Manager : public CCNode {
	public:

		float time = 0;

		std::vector<Form*> forms;

		Form* selectedForm;
		Layer* selectedLayer;
		Modulator* selectedModulator;

		DebugPanel* debugPanel;

		static Manager* create();
		static Manager* get();
		bool init();
		void createTab();
		void onTabEnter();
		void onTabExit();

		GameObject* createObject(int ID, CCPoint position) {
			return EditorUI::get()->createObject(ID, roundTo(position, 4));
		}

		bool shouldSelectObject(GameObject* object);

		bool inTab() {
			return alpha::editor_tabs::getCurrentTab().unwrapOrDefault() == "sculptor"_spr;
		}

		CCPoint toEditorSpace(CCPoint point) {
			return LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->convertToNodeSpace(point);
		}

		CCPoint fromEditorSpace(CCPoint point) {
			return LevelEditorLayer::get()->getChildByID("main-node")->getChildByID("batch-layer")->convertToWorldSpace(point);
		}

		void deselect() {
			if (selectedForm) {
				selectedForm->bezierEditor->setVisible(false);
			}
			selectedForm = nullptr;
			selectedLayer = nullptr;
			selectedModulator = nullptr;
		}

		int getFormID(Form* form) {
			return std::ranges::find(forms, form) - forms.begin();
		}

		Form* addForm(Form* form) {
			forms.push_back(form);
			return form;
		}

		void onSave();

		static NineSlice* createBase(CCSize size);

	private:

		void update(float dt);

		ListenerHandle scrollListener;
		ListenerHandle mouseListener;

		CCNode* page;

		
		CCNode* layerMenu;
		CCNode* modulatorMenu;
		ScrollLayer* layerPropertiesScroll;
		ScrollLayer* modulatorPropertiesScroll;

		std::vector<ModulatablePropertyEditor*> layerPropertyEditors;
		std::vector<SinglePropertyEditor*> modulatorPropertyEditors;

		CCNode* createFormPanel(CCSize size, CCPoint position);

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

	class DebugPanel : public CCDrawNode {
	public:

		static DebugPanel* create(CCSize size, CCPoint position);
		bool init(CCSize size, CCPoint position);


		void setValue(std::string key, const Formattable auto& value);

	private:

		std::unordered_map<std::string, CCLabelBMFont*> labels;
		CCNode* column;
	};

}