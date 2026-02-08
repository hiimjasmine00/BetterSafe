#include "../classes/BSCalendarPopup.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <jasmine/button.hpp>
#include <jasmine/hook.hpp>

using namespace geode::prelude;
using namespace jasmine::button;

class $modify(BSMenuLayer, MenuLayer) {
    static void onModify(ModifyBase<ModifyDerive<BSMenuLayer, MenuLayer>>& self) {
        if (auto hook = jasmine::hook::get(self.m_hooks, "MenuLayer::init", false)) {
            if (auto overcharged = Loader::get()->getInstalledMod("ninxout.redash")) {
                if (overcharged->isLoaded()) {
                    hook->setAutoEnable(true);
                    ModifyBase<ModifyDerive<BSMenuLayer, MenuLayer>>::setHookPriorityAfterPost(hook, overcharged);
                }
                else {
                    ModStateEvent(ModEventType::Loaded, overcharged).listen([hook, overcharged] {
                        ModifyBase<ModifyDerive<BSMenuLayer, MenuLayer>>::setHookPriorityAfterPost(hook, overcharged);
                        jasmine::hook::toggle(hook, true);
                    }).leak();
                }
            }
        }
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto redashMenu = getChildByID("ninxout.redash/redash-menu");
        if (!redashMenu) return true;

        auto dailiesMenu = redashMenu->getChildByID("ninxout.redash/dailies-menu");
        if (!dailiesMenu) return true;

        if (auto dailyNode = dailiesMenu->getChildByID("daily-node")) {
            ButtonHooker::create(
                static_cast<CCMenuItem*>(dailyNode->getChildByIDRecursive("safe-button")),
                this, menu_selector(BSMenuLayer::onTheDailySafe)
            );
        }

        if (auto weeklyNode = dailiesMenu->getChildByID("weekly-node")) {
            ButtonHooker::create(
                static_cast<CCMenuItem*>(weeklyNode->getChildByIDRecursive("safe-button")),
                this, menu_selector(BSMenuLayer::onTheWeeklySafe)
            );
        }

        if (auto eventNode = dailiesMenu->getChildByID("event-node")) {
            ButtonHooker::create(
                static_cast<CCMenuItem*>(eventNode->getChildByIDRecursive("safe-button")),
                this, menu_selector(BSMenuLayer::onTheEventSafe)
            );
        }

        return true;
    }

    void onTheDailySafe(CCObject* sender) {
        if (auto hooker = static_cast<ButtonHooker*>(static_cast<CCNode*>(sender)->getUserObject("hooker"_spr))) {
            BSCalendarPopup::create(hooker->m_listener, hooker->m_selector, GJTimedLevelType::Daily)->show();
        }
    }

    void onTheWeeklySafe(CCObject* sender) {
        if (auto hooker = static_cast<ButtonHooker*>(static_cast<CCNode*>(sender)->getUserObject("hooker"_spr))) {
            BSCalendarPopup::create(hooker->m_listener, hooker->m_selector, GJTimedLevelType::Weekly)->show();
        }
    }

    void onTheEventSafe(CCObject* sender) {
        if (auto hooker = static_cast<ButtonHooker*>(static_cast<CCNode*>(sender)->getUserObject("hooker"_spr))) {
            BSCalendarPopup::create(hooker->m_listener, hooker->m_selector, GJTimedLevelType::Event)->show();
        }
    }
};
