#include "../classes/BSCalendarPopup.hpp"
#include <Geode/modify/DailyLevelPage.hpp>

using namespace geode::prelude;

class $modify(BSDailyLevelPage, DailyLevelPage) {
    void onTheSafe(CCObject* sender) {
        if (!sender || sender->getTag() == 91508) return DailyLevelPage::onTheSafe(sender);
        auto item = static_cast<CCMenuItem*>(sender);
        BSCalendarPopup::create(item->m_pListener, item->m_pfnSelector, m_type)->show();
    }
};
