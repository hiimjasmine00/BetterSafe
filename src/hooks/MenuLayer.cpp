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
        (void)self.getHook("MenuLayer::init").map([](Hook* hook) {
            hook->setAutoEnable(false);
            if (auto overcharged = Loader::get()->getInstalledMod("ninxout.redash")) {
                if (overcharged->isEnabled()) {
                    hook->setAutoEnable(true);
                    afterPriority(hook, overcharged);
                }
                else new EventListener([hook](ModStateEvent* e) {
                    afterPriority(hook, e->getMod());
                    (void)hook->enable().inspectErr([](const std::string& err) {
                        log::error("Failed to enable MenuLayer::init hook: {}", err);
                    });
                }, ModStateFilter(overcharged, ModEventType::Loaded));
            }
            return hook;
        }).inspectErr([](const std::string& err) { log::error("Failed to get MenuLayer::init hook: {}", err); });
    }

    static void afterPriority(Hook* hook, Mod* mod) {
        auto address = hook->getAddress();
        auto modHooks = mod->getHooks();
        auto modHook = std::ranges::find_if(modHooks, [address](Hook* h) { return h->getAddress() == address; });
        if (modHook == modHooks.end()) return log::error("Failed to find MenuLayer::init hook in ninxout.redash");

        auto priority = (*modHook)->getPriority();
        if (hook->getPriority() >= priority) hook->setPriority(priority - 1);
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto redashMenu = getChildByID("ninxout.redash/redash-menu");
        if (!redashMenu) return true;

        auto dailiesMenu = redashMenu->getChildByID("ninxout.redash/dailies-menu");
        if (!dailiesMenu) return true;

        auto f = m_fields.self();

        auto dailyNode = dailiesMenu->getChildByID("daily-node");
        if (auto dailySafeButton = static_cast<CCMenuItem*>(dailyNode ? dailyNode->getChildByIDRecursive("safe-button") : nullptr)) {
            f->m_dailySafeListener = dailySafeButton->m_pListener;
            f->m_dailySafeSelector = dailySafeButton->m_pfnSelector;
            dailySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheDailySafe));
        }

        auto weeklyNode = dailiesMenu->getChildByID("weekly-node");
        if (auto weeklySafeButton = static_cast<CCMenuItem*>(weeklyNode ? weeklyNode->getChildByIDRecursive("safe-button") : nullptr)) {
            f->m_weeklySafeListener = weeklySafeButton->m_pListener;
            f->m_weeklySafeSelector = weeklySafeButton->m_pfnSelector;
            weeklySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheWeeklySafe));
        }

        auto eventNode = dailiesMenu->getChildByID("event-node");
        if (auto eventSafeButton = static_cast<CCMenuItem*>(eventNode ? eventNode->getChildByIDRecursive("safe-button") : nullptr)) {
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
