#include <Geode/Geode.hpp>
#include <ranges>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include "../lib/Manager.hpp"
#include "../lib/PropertyEditor.hpp"
#include "../sculptor/Layer.hpp"
#include "../sculptor/Serialization.hpp"

using namespace geode::prelude;
using namespace Clipper2Lib;

namespace Sculptor {

    Manager* Manager::create() {
        auto node = new Manager();
        if (node && node->init()) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    bool Manager::init() {
        if (!CCNode::init()) return false;

        this->scheduleUpdate();

        setID("manager"_spr);

        mouseListener = MouseInputEvent().listen([this](MouseInputData data) { return this->handleMouseData(data); });
        scrollListener = ScrollWheelEvent().listen([this](double x, double y) -> ListenerResult { return this->handleScroll(x, y); });

        return true;
    }

    Manager* Manager::get() {
        return static_cast<Manager*>(LevelEditorLayer::get()->getChildByID("manager"_spr));
    }

    void Manager::update(float dt) {

        time += dt;

        if (debugPanel) {
            debugPanel->setValue("FPS", 1.f / dt);
        }
        

    }

    bool Manager::shouldSelectObject(GameObject* object) {
        for (const auto& form : forms) {
            for (const auto& layer : form->layers) {
                if (std::ranges::contains(layer->getObjects(), object)) {
                    return false;
                }
            }
        }
        return !inTab();
    }

    void Manager::onTabEnter() {
        if (!page) {
            createTab();
        }
        EditorUI::get()->getChildByIDRecursive("toolbar-categories-menu")->setVisible(false);
        EditorUI::get()->getChildByIDRecursive("toolbar-toggles-menu")->setVisible(false);
        EditorUI::get()->getChildByIDRecursive("spacer-line-left")->setVisible(false);
        EditorUI::get()->getChildByIDRecursive("spacer-line-right")->setVisible(false);
    }

    void Manager::onTabExit() {
        EditorUI::get()->getChildByIDRecursive("toolbar-categories-menu")->setVisible(true);
        EditorUI::get()->getChildByIDRecursive("toolbar-toggles-menu")->setVisible(true);
        EditorUI::get()->getChildByIDRecursive("spacer-line-left")->setVisible(true);
        EditorUI::get()->getChildByIDRecursive("spacer-line-right")->setVisible(true);
        deselect();
    }

    void Manager::createTab() {

        auto color = ccc3(0, 255, 255);
        auto hsv = cchsv(0, 0, 0, false, false);
        LevelEditorLayer::get()->updateColor(color, 0.f, 1, true, 1.f, hsv, 2, false, nullptr, 0, 0);
        LevelEditorLayer::get()->updateLevelColors();

        CCSize size = { 569, 90 };

        page = CCNode::create();
        page->setID("page"_spr);
        page->setAnchorPoint({ 0.f, 0.f });
        page->setPositionX(-96);
        page->setContentSize(size);
        page->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Between)->setAutoScale(false));


        layerMenu = createLayerMenu({ size.width * (1.f / 9.f), size.height }, 1);
        page->addChild(layerMenu);

        layerPropertiesScroll = createPropertiesScroll({ size.width * (5.f / 9.f), size.height }, 3);
        page->addChild(layerPropertiesScroll);

        modulatorMenu = createModulatorMenu({ size.width * (1.f / 9.f), size.height }, 2);
        page->addChild(modulatorMenu);

        modulatorPropertiesScroll = createPropertiesScroll({ size.width * (1.5f / 9.f), size.height }, 3);
        page->addChild(modulatorPropertiesScroll);



        page->updateLayout();
        auto tab = EditorUI::get()->getChildByIDRecursive("sculptor-tab-bar"_spr)->getChildByType<BoomScrollLayer>(0);
        tab->addChild(page);
        tab->addChild(createFormPanel({ 50, 20 }, { 0, 100 }));
        debugPanel = DebugPanel::create({ 100, 50 }, { 0, 125 });
        tab->addChild(debugPanel);


        updateLayerMenu();
        updateModulatorMenu();
        updateLayerPropertiesMenu();
        updateModulatorPropertiesMenu();






    }

    CCNode* Manager::createFormPanel(CCSize size, CCPoint position) {
        auto node = CCNode::create();
        node->setContentSize(size);
        node->setPosition(position);
        node->addChildAtPosition(createBase(size), Anchor::Center);

        auto row = CCMenu::create();
        row->setContentSize(size);
        row->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even)->setAutoScale(false)->setCrossAxisOverflow(false));
        {
            auto spr = CCSprite::create("create.png"_spr);
            spr->setScale(size.height / spr->getContentHeight());
            row->addChild(CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onFormNewButton)));
        }
        {
            auto spr = CCSprite::create("delete.png"_spr);
            spr->setScale(size.height / spr->getContentHeight());
            row->addChild(CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onFormDeleteButton)));
        }
        row->updateLayout();
        node->addChild(row);
        row->setPosition(0, 0);
        return node;
    }

    NineSlice* Manager::createBase(CCSize size) {
        auto base = NineSlice::create("square02b_001.png", { 0, 0, 80, 80 });
        base->setScale(0.5);
        base->setColor({ 0, 0, 0 });
        base->setOpacity(45);
        base->setContentSize({ size.width * 2, size.height * 2 });
        return base;
    }

    CCNode* Manager::createVerticalScroll(CCSize size, int columns) {
        auto scroll = ScrollLayer::create(size, false, true);
        scroll->setContentLayerSize({ size.width, size.height * 2 });
        scroll->scrollToTop();
        scroll->setTouchPriority(-1);

        auto row = CCNode::create();
        row->setID("row"_spr);
        row->setContentSize({ size.width, size.height * 2 });
        row->setLayout(RowLayout::create()->setGap(2)->setAutoScale(false)->setCrossAxisOverflow(false));
        for (int i = 0; i < columns; i++) {
            auto menu = CCMenu::create();
            menu->setContentSize({ size.width / columns, size.height * 2 });
            menu->setPosition(0, 0);
            menu->setAnchorPoint({ 0, 0 });
            menu->setLayout(ColumnLayout::create()->setGap(2)->setAxisAlignment(AxisAlignment::End)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));
            row->addChild(menu);
        }
        row->updateLayout();
        scroll->m_contentLayer->addChild(row);
        return scroll;
    }

    CCNode* Manager::createLayerMenu(CCSize size, int columns) {

        float h = 13;

        auto node = CCNode::create();
        node->setContentSize(size);
        node->addChildAtPosition(createBase(size), Anchor::Center);
        auto column = CCNode::create();
        column->setContentSize(size);
        column->setLayout(ColumnLayout::create()->setGap(0)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));
        node->addChild(column);

        auto topMenu = CCMenu::create();
        topMenu->setContentSize({ size.width, h });
        topMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::End)->setAutoScale(false)->setCrossAxisOverflow(false));
        auto topLabel = CCLabelBMFont::create("Layers", "chatFont.fnt");
        topLabel->setScale((h / topLabel->getContentHeight()) * 0.75);
        topMenu->addChild(topLabel);
        auto deleteSprite = CCSprite::create("delete.png"_spr);
        deleteSprite->setScale(h / deleteSprite->getContentHeight());
        topMenu->addChild(CCMenuItemSpriteExtra::create(deleteSprite, this, menu_selector(Manager::onLayerDeleteButton)));
        topMenu->updateLayout();
        column->addChild(topMenu);

        column->addChild(createVerticalScroll({ size.width, size.height - h }, 1));
        column->updateLayout();


        return node;
    }

    CCNode* Manager::createModulatorMenu(CCSize size, int columns) {

        float h = 13;

        auto node = CCNode::create();
        node->setContentSize(size);
        node->addChildAtPosition(createBase(size), Anchor::Center);
        auto column = CCNode::create();
        column->setContentSize(size);
        column->setLayout(ColumnLayout::create()->setGap(0)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));
        node->addChild(column);

        auto topMenu = CCMenu::create();
        topMenu->setContentSize({ size.width, h });
        topMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::End)->setAutoScale(false)->setCrossAxisOverflow(false));
        auto topLabel = CCLabelBMFont::create("Modulators", "chatFont.fnt");
        topLabel->setScale((h / topLabel->getContentHeight()) * 0.75);
        topMenu->addChild(topLabel);
        auto deleteSprite = CCSprite::create("delete.png"_spr);
        deleteSprite->setScale(h / deleteSprite->getContentHeight());
        topMenu->addChild(CCMenuItemSpriteExtra::create(deleteSprite, this, menu_selector(Manager::onModulatorDeleteButton)));
        topMenu->updateLayout();
        column->addChild(topMenu);

        column->addChild(createVerticalScroll({ size.width, size.height - h }, 2));
        column->updateLayout();


        return node;
    }

    ScrollLayer* Manager::createPropertiesScroll(CCSize size, int rows) {
        auto scroll = ScrollLayer::create(size, false, false);
        scroll->setContentLayerSize({ size.width * 2, size.height });
        scroll->scrollToTop();

        auto column = CCNode::create();
        column->setID("column"_spr);
        column->setContentSize({ size.width * 2, size.height });
        column->setLayout(ColumnLayout::create()->setGap(3.5)->setAxisAlignment(AxisAlignment::Between)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));
        for (int i = 0; i < rows; i++) {
            auto row = CCNode::create();
            row->setContentSize({ size.width * 2, size.height / rows });
            row->setLayout(RowLayout::create()->setGap(3.5)->setAxisAlignment(AxisAlignment::Start)->setAutoScale(false)->setCrossAxisOverflow(false));
            column->addChild(row);
        }
        column->updateLayout();
        scroll->m_contentLayer->addChild(column);
        return scroll;
    }

    void Manager::updateLayerMenu() {
        if (!inTab()) return;

        auto row = layerMenu->getChildByIDRecursive("row"_spr);
        auto columns = CCArrayExt<CCNode*>(row->getChildren());
        for (auto& column : columns) {
            column->removeAllChildren();
        }
        if (selectedForm) {
            for (const auto& [i, layer] : std::views::enumerate(selectedForm->layers)) {
                auto spr = ButtonSprite::create(layer->name.c_str());
                spr->setScale(0.5);
                spr->setCascadeOpacityEnabled(true);
                if (selectedLayer != layer) {
                    spr->setOpacity(45);
                }
                auto button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onLayerSelectButton));
                button->setTag(i);
                columns[0]->addChild(button);
                columns[0]->updateLayout();
            }
            auto spr = ButtonSprite::create("   +   ");
            spr->setScale(0.5);
            auto button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onLayerNewButton));
            columns[0]->addChild(button);
            columns[0]->updateLayout();
        }
    }

    void Manager::updateModulatorMenu() {
        if (!inTab()) return;

        auto row = modulatorMenu->getChildByIDRecursive("row"_spr);
        auto columns = CCArrayExt<CCNode*>(row->getChildren());
        for (auto& column : columns) {
            column->removeAllChildren();
        }
        if (selectedForm) {
            for (const auto& [i, modulator] : std::views::enumerate(selectedForm->modulators)) {
                auto info = Modulator::info[modulator->type];
                auto spr = CCSprite::create(info.spriteName.c_str());
                spr->setScale(0.5);
                if (selectedModulator != modulator) {
                    spr->setOpacity(45);
                }
                auto button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onModulatorSelectButton));
                button->setTag(i);
                int column = i % 2;
                columns[column]->addChild(button);
                columns[column]->updateLayout();

            }
            auto spr = ButtonSprite::create("+");
            spr->setScale(0.5);
            auto button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onModulatorNewButton));
            columns[selectedForm->modulators.size() % 2]->addChild(button);
            columns[selectedForm->modulators.size() % 2]->updateLayout();
        }
    }

    void Manager::updateLayerPropertiesMenu() {
        if (!inTab()) return;

        auto column = layerPropertiesScroll->m_contentLayer->getChildByID("column"_spr);
        auto rows = CCArrayExt<CCNode*>(column->getChildren());
        for (auto& row : rows) {
            row->removeAllChildren();
        }
        if (selectedLayer) {
            for (const auto& [i, name] : std::views::enumerate(Layer::propertyNamesByStyle(selectedLayer->style))) {
                LayerProperty* prop = selectedLayer->getProperty(name);
                int row = i % 3;
                if (LayerProperty::info[name].modulatable) {
                    rows[row]->addChild(ModulatablePropertyEditor::create(prop, { 160, 40 }));
                }
                else {
                    rows[row]->addChild(SinglePropertyEditor::create(prop, { 160, 40 }));
                }
                rows[row]->updateLayout();
            }
        }
    }

    void Manager::updateModulatorPropertiesMenu() {
        if (!inTab()) return;

        auto column = modulatorPropertiesScroll->m_contentLayer->getChildByID("column"_spr);
        auto rows = CCArrayExt<CCNode*>(column->getChildren());
        for (auto& row : rows) {
            row->removeAllChildren();
        }
        if (selectedModulator) {
            for (const auto& [i, name] : std::views::enumerate(Modulator::info[selectedModulator->type].propertyNames)) {
                ModulatorProperty* prop = selectedModulator->getProperty(name);
                int row = i % 3;
                auto editor = SinglePropertyEditor::create(prop, { 80, 40 });
                rows[row]->addChild(editor);
                rows[row]->updateLayout();
            }
        }
    }


    void Manager::onButton(CCObject* sender) {

    }

    void Manager::onFormNewButton(CCObject* sender) {
        auto pos = toEditorSpace(CCDirector::get()->getWinSize() / 2);
        auto form = Form::create(pos, { BezierCurve({ccp(-30, -30), ccp(30, -30)}), BezierCurve({ccp(30, -30), ccp(30, 30)}), BezierCurve({ccp(30, 30), ccp(-30, 30)}), BezierCurve({ccp(-30, 30), ccp(-30, -30)}) });
        form->addModulator(new Modulator(Modulator::Type::Noise));
        form->addLayer(new Layer(Layer::Style::Solid));
        selectForm(addForm(form));
    }

    void Manager::onFormDeleteButton(CCObject* sender) {
        if (selectedForm) {
            auto it = std::ranges::find(forms, selectedForm);
            if (it != forms.end()) {
                (*it)->removeFromParent();
                forms.erase(it);
                deselect();
            }
        }
    }

    void Manager::onLayerSelectButton(CCObject* sender) {
        int i = sender->getTag();
        selectLayer(selectedForm->layers.at(i));

    }

    void Manager::onModulatorSelectButton(CCObject* sender) {
        int i = sender->getTag();
        selectModulator(selectedForm->modulators.at(i));

    }

    void Manager::onLayerNewButton(CCObject* sender) {
        std::vector<std::string> labels;
        for (int i = 0; i < static_cast<int>(Layer::Style::Count); i++) {
            labels.push_back(Layer::info[static_cast<Layer::Style>(i)].label);
        }
        selectionPopup = SelectionPopup::create({ 50, 75 }, { 100, 150 }, labels, [this](int i) {
            selectLayer(selectedForm->addLayer(new Layer(static_cast<Layer::Style>(i))));
            });
        selectionPopup->show();
    }

    void Manager::onModulatorNewButton(CCObject* sender) {
        std::vector<std::string> labels;
        for (int i = 0; i < static_cast<int>(Modulator::Type::Count); i++) {
            labels.push_back(Modulator::info[static_cast<Modulator::Type>(i)].label);
        }
        selectionPopup = SelectionPopup::create({ 375, 75 }, { 100, 150 }, labels, [this](int i) {
            auto modulator = selectedForm->addModulator(new Modulator(static_cast<Modulator::Type>(i)));
            selectModulator(modulator);
            });
        selectionPopup->show();
    }

    void Manager::onLayerDeleteButton(CCObject* sender) {
        selectedForm->removeLayer(selectedLayer);
        selectLayer(selectedForm->layers[0]);
    }

    void Manager::onModulatorDeleteButton(CCObject* sender) {
        selectedForm->removeModulator(selectedModulator);
        selectModulator(selectedForm->modulators[0]);
    }

    ListenerResult Manager::handleScroll(double x, double y) {

        /*if (inTab()) {
            if (layerScroll->boundingBox().containsPoint(page->convertToNodeSpace(getMousePos()))) {
                layerScroll->scrollLayer(-y * 4);
            }
        }*/

        return ListenerResult::Propagate;
    }

    ListenerResult Manager::handleMouseData(MouseInputData data) {

        if (!inTab()) return ListenerResult::Propagate;

        if (data.action == MouseInputData::Action::Press) {
            for (const auto& form : forms) {
                if (form->getApproximation().contains(form->convertToNodeSpace(getMousePos()))) {
                    selectForm(form);
                    break;
                }
            }
        }


        return ListenerResult::Propagate;
    }

    bool SelectionPopup::init(CCPoint position, CCSize size, std::vector<std::string> labels, std::function<void(int)> callback) {
        if (!Popup::init(size))
            return false;

        this->callback = callback;
        setCloseButtonSpr(CCSprite::create(), 0);
        m_mainLayer->setPosition(position);

        auto scroll = ScrollLayer::create(size);
        scroll->setContentLayerSize({ size.width, size.height * 2 });

        auto menu = CCMenu::create();
        menu->setContentSize({ size.width, size.height * 2 });
        menu->setLayout(ColumnLayout::create()->setGap(5)->setAxisAlignment(AxisAlignment::Even)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));
        menu->setAnchorPoint({ 0.f, 0.f });
        for (const auto& [i, label] : std::views::enumerate(labels)) {
            auto sprite = ButtonSprite::create(label.c_str());
            sprite->setScale(0.5);
            auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(SelectionPopup::onButton));
            button->setTag(i);
            menu->addChild(button);
        }
        menu->updateLayout();
        menu->setPosition(0, 0);

        scroll->m_contentLayer->addChild(menu);
        scroll->scrollToTop();
        m_mainLayer->addChild(scroll);

        return true;
    }

    void Manager::onSave() {

        deselect();
        alpha::level_storage::setSavedValue(LevelEditorLayer::get(), "manager", matjson::Serialize<Manager*>::toJson(this));

    }

    //


    DebugPanel* DebugPanel::create(CCSize size, CCPoint position) {
        auto node = new DebugPanel();
        if (node && node->init(size, position)) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    bool DebugPanel::init(CCSize size, CCPoint position) {
        if (!CCDrawNode::init()) return false;

        labels = { {} };

        setContentSize(size);
        setPosition(position);
        addChildAtPosition(Manager::createBase(size), Anchor::Center);

        column = CCNode::create();
        addChild(column);
        column->setContentSize(size);
        column->setLayout(ColumnLayout::create()->setGap(0)->setAxisAlignment(AxisAlignment::End)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));

        return true;
    }

    void DebugPanel::setValue(std::string key, const Formattable auto& value) {
        if (!labels.contains(key)) {
            auto label = CCLabelBMFont::create(fmt::format("{}: {}", key, value).c_str(), "chatFont.fnt");
            label->setScale(0.33);
            column->addChild(label);
            column->updateLayout();
            labels[key] = label;
        }
        else {
            labels[key]->setString(fmt::format("{}: {}", key, value).c_str());
        }
    }
}