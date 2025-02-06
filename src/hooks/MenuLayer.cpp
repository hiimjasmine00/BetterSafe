#include "../classes/BSCalendarPopup.hpp"
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(BSMenuLayer, MenuLayer) {
    struct Fields {
        CCObject* m_dailySafeListener;
        SEL_MenuHandler m_dailySafeSelector;
        CCObject* m_weeklySafeListener;
        SEL_MenuHandler m_weeklySafeSelector;
        CCObject* m_eventSafeListener;
        SEL_MenuHandler m_eventSafeSelector;
    };

    static void onModify(ModifyBase<ModifyDerive<BSMenuLayer, MenuLayer>>& self) {
        auto initHook = self.getHook("MenuLayer::init");
        if (initHook.isErr()) return log::error("Failed to find MenuLayer::init hook: {}", initHook.unwrapErr());

        if (auto overcharged = Loader::get()->getInstalledMod("ninxout.redash")) {
            if (overcharged->shouldLoad()) return (void)self.setHookPriorityAfterPost("MenuLayer::init", overcharged);
        }

        initHook.unwrap()->setAutoEnable(false);
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto redashMenu = getChildByID("ninxout.redash/redash-menu");
        if (!redashMenu) return true;

        auto dailiesMenu = redashMenu->getChildByID("ninxout.redash/dailies-menu");
        if (!dailiesMenu) return true;

        auto f = m_fields.self();

        auto dailyNode = dailiesMenu->getChildByID("daily-node");
        auto dailySafeButton = static_cast<CCMenuItemSpriteExtra*>(dailyNode ? dailyNode->getChildByIDRecursive("safe-button") : nullptr);
        if (dailyNode && dailySafeButton) {
            f->m_dailySafeListener = dailySafeButton->m_pListener;
            f->m_dailySafeSelector = dailySafeButton->m_pfnSelector;
            dailySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheDailySafe));
        }

        auto weeklyNode = dailiesMenu->getChildByID("weekly-node");
        auto weeklySafeButton = static_cast<CCMenuItemSpriteExtra*>(weeklyNode ? weeklyNode->getChildByIDRecursive("safe-button") : nullptr);
        if (weeklyNode && weeklySafeButton) {
            f->m_weeklySafeListener = weeklySafeButton->m_pListener;
            f->m_weeklySafeSelector = weeklySafeButton->m_pfnSelector;
            weeklySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheWeeklySafe));
        }

        auto eventNode = dailiesMenu->getChildByID("event-node");
        auto eventSafeButton = static_cast<CCMenuItemSpriteExtra*>(eventNode ? eventNode->getChildByIDRecursive("safe-button") : nullptr);
        if (eventNode && eventSafeButton) {
            f->m_eventSafeListener = eventSafeButton->m_pListener;
            f->m_eventSafeSelector = eventSafeButton->m_pfnSelector;
            eventSafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheEventSafe));
        }

        return true;
    }

    void onTheDailySafe(CCObject*) {
        auto f = m_fields.self();
        BSCalendarPopup::create(f->m_dailySafeListener, f->m_dailySafeSelector, GJTimedLevelType::Daily)->show();
    }

    void onTheWeeklySafe(CCObject*) {
        auto f = m_fields.self();
        BSCalendarPopup::create(f->m_weeklySafeListener, f->m_weeklySafeSelector, GJTimedLevelType::Weekly)->show();
    }

    void onTheEventSafe(CCObject*) {
        auto f = m_fields.self();
        BSCalendarPopup::create(f->m_eventSafeListener, f->m_eventSafeSelector, GJTimedLevelType::Event)->show();
    }
};
