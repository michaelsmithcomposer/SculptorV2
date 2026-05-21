#include <Geode/Geode.hpp>
#include "../lib/Manager.hpp"

using namespace geode::prelude;
using namespace Sculptor;

#include <Geode/modify/EditorPauseLayer.hpp>
class $modify(MyEditorPauseLayer, EditorPauseLayer) {

    /*void onSave(CCObject* sender) {
        SculptorManager::get()->onSave();
        EditorPauseLayer::onSave(sender);
    }*/

    void onSaveAndExit(CCObject* sender) {
        Manager::get()->onSave();
        EditorPauseLayer::onSaveAndExit(sender);
    }

    void onSaveAndPlay(CCObject* sender) {
        Manager::get()->onSave();
        EditorPauseLayer::onSaveAndPlay(sender);
    }
};