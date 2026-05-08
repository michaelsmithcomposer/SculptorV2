#include <Geode/Geode.hpp>
#include <ranges>
#include <alphalaneous.editortab_api/include/EditorTabAPI.hpp>
#include "../lib/Manager.hpp"
#include "../lib/PropertyEditor.hpp"
#include "../sculptor/Layer.hpp"

using namespace geode::prelude;
using namespace Clipper2Lib;

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
    
    scrollListener = ScrollWheelEvent().listen([this](double x, double y) -> ListenerResult { return this->handleScroll(x, y); });    

    return true;
}

Manager* Manager::get() {
    return static_cast<Manager*>(EditorUI::get()->getChildByID("manager"_spr));
}

void Manager::update(float dt) {   

   
}

void Manager::onTabEnter() {
    if (!page) {
        createTab();
    }
}

void Manager::onTabExit() {

}

void Manager::createTab() {

    Form* form = Form::create();
    form->paths = { { {0, 0}, {25, 150}, {150, 50} } };
    Layer* layer = form->createLayer(LayerStyle::Corners);
    form->createLayer(LayerStyle::Corners2);

    selectedForm = form;
    selectedLayer = layer;

    log::info("init count: {}", selectedForm->layers.size());

    forms.push_back(form);

	CCSize size = { 375, 90 };    

	page = CCNode::create();
    page->setID("page"_spr);
	page->setAnchorPoint({ 0.f, 0.f });
	page->setContentSize(size);
	page->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Between)->setAutoScale(false));    

    {
        float w = size.width / 9.f;

        auto menu = CCMenu::create();
        menu->setContentSize({ w, size.height });
        menu->setLayout(ColumnLayout::create()->setAxisAlignment(AxisAlignment::Even)->setAxisReverse(true));
        menu->addChild(CCMenuItemSpriteExtra::create(BasedButtonSprite::create(CCNode::create(), BaseType::Editor, 0, static_cast<int>(EditorBaseColor::Gray)), this, menu_selector(Manager::onButton)));
        menu->addChild(CCMenuItemSpriteExtra::create(BasedButtonSprite::create(CCNode::create(), BaseType::Editor, 0, static_cast<int>(EditorBaseColor::Gray)), this, menu_selector(Manager::onButton)));
        menu->updateLayout();
        page->addChild(menu);

    }
    {
        float w = size.width * (2.f / 9.f);

        auto scroll = ScrollLayer::create({ w, size.height }, false, true);
        this->layerScroll = scroll;
        scroll->setContentLayerSize({ w, size.height * 2 });
        scroll->scrollToTop();
        scroll->setTouchPriority(-1);        

        auto menu = CCMenu::create();     
        this->layerMenu = menu;
        menu->setContentSize({ w, size.height * 2 });
        menu->setPosition(0, 0);
        menu->setAnchorPoint({ 0, 0 });
        menu->setLayout(ColumnLayout::create()->setGap(2)->setAxisAlignment(AxisAlignment::End)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));        
        scroll->m_contentLayer->addChild(menu);
        page->addChild(scroll);
    }
    {
        float w = size.width * (2.f / 3.f);
        float topH = 15;

        auto menu = CCNode::create();
        menu->setContentSize({ w, size.height });
        menu->setLayout(ColumnLayout::create()->setAxisAlignment(AxisAlignment::Between)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));

        auto topMenu = CCMenu::create();
        topMenu->setContentSize({ w, topH });
        topMenu->setLayout(RowLayout::create()->setGap(3)->setAxisAlignment(AxisAlignment::Even)->setAutoScale(false)->setCrossAxisOverflow(false));
        menu->addChild(topMenu);

        for (const auto& [i, mod] : std::views::enumerate(modSources)) {
            auto spr = BasedButtonSprite::create(CCLabelBMFont::create(modSourceAsString[mod].c_str(), "bigFont.fnt"), BaseType::Editor, 0, static_cast<int>(EditorBaseColor::Green));
            spr->setScale(0.5);
            auto button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onModSelectButton));
            button->setTag(i);
            topMenu->addChild(button);
            modSourceButtons.push_back(button);
        }
        topMenu->updateLayout();

        auto scroll = ScrollLayer::create({ w, size.height - topH }, false, false);
        scroll->setContentLayerSize({ w * 2, size.height - topH });
        scroll->scrollToTop();

        auto column = CCNode::create();
        column->setContentSize({ w * 2, size.height - topH });
        column->setLayout(ColumnLayout::create()->setGap(3.5)->setAxisAlignment(AxisAlignment::Between)->setAxisReverse(true)->setAutoScale(false)->setCrossAxisOverflow(false));
        for (int i = 0; i < 3; i++) {
            auto row = CCNode::create();
            row->setContentSize({ w * 2, (size.height - topH) / 3.f });
            row->setLayout(RowLayout::create()->setGap(3.5)->setAxisAlignment(AxisAlignment::Start)->setAutoScale(false)->setCrossAxisOverflow(false));
            column->addChild(row);
            propertyMenus.push_back(row);
        }
        column->updateLayout();      

        
        scroll->m_contentLayer->addChild(column);
        menu->addChild(scroll);
        menu->updateLayout();
        page->addChild(menu);
    }  
    



	page->updateLayout();
    EditorUI::get()->getChildByIDRecursive("sculptor-tab-bar"_spr)->getChildByType<BoomScrollLayer>(0)->addChild(page);


    updateLayerMenu();
    updatePropertiesMenu();
    setModSource(ModSource::Noise);


}

void Manager::updateLayerMenu() {
    layerMenu->removeAllChildren();
    if (selectedForm) {        
        for (const auto& [i, layer] : std::views::enumerate(selectedForm->layers)) {                 
            auto spr = ButtonSprite::create(layer->name.c_str());            
            if (selectedLayer != layer) {
                spr->setColor({ 70, 70, 70 });
            }
            spr->setScale(0.5);
            auto button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(Manager::onLayerSelectButton));
            button->setTag(i);
            layerMenu->addChild(button);
        }
        layerMenu->updateLayout();
    }
}

void Manager::updatePropertiesMenu() {
    propertyEditors.clear();
    for (auto& menu : propertyMenus) {
        menu->removeAllChildren();
    }
    if (selectedLayer) {
        for (const auto& [i, propertyName] : std::views::enumerate(propertyNamesByStyle[selectedLayer->style])) {
            LayerProperty* prop = selectedLayer->getProperty(propertyName);
            int row = i % 3;
            PropertyEditor* editor = PropertyEditor::create(prop);
            propertyMenus.at(row)->addChild(editor);
            propertyMenus.at(row)->updateLayout();
            propertyEditors.push_back(editor);           
        }
    }
}

void Manager::setModSource(ModSource modSource) {
    selectedModSource = modSource;
    for (const auto& editor : propertyEditors) {
        editor->setModSource(modSource);
    }
    for (const auto& [buttonSource, button] : std::views::zip(modSources, modSourceButtons)) {      
        auto color = (buttonSource == modSource) ? ccc3(255, 255, 255) : ccc3(70, 70, 70);
        auto spr = static_cast<CCSprite*>(button->getNormalImage());
        spr->setColor(color);
        button->setSprite(spr);
    }
}

void Manager::onButton(CCObject* sender) {

}

void Manager::onLayerSelectButton(CCObject* sender) {
    int i = sender->getTag();
    selectedLayer = selectedForm->layers.at(i);
    updateLayerMenu();
    updatePropertiesMenu();
}

void Manager::onModSelectButton(CCObject* sender) {    
    setModSource(modSources[sender->getTag()]);
}

ListenerResult Manager::handleScroll(double x, double y) {

    if (alpha::editor_tabs::getCurrentTab().unwrapOrDefault() == "sculptor"_spr) {       
        if (layerScroll->boundingBox().containsPoint(page->convertToNodeSpace(getMousePos()))) {
            layerScroll->scrollLayer(-y * 4);
        }
    }

    return ListenerResult::Propagate;
}