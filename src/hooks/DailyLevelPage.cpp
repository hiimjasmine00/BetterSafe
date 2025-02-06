#include "../classes/BSCalendarPopup.hpp"
#include <Geode/modify/DailyLevelPage.hpp>

using namespace geode::prelude;

class $modify(BSDailyLevelPage, DailyLevelPage) {
    void onTheSafe(CCObject* sender) {
        if (sender->getTag() == 91508) DailyLevelPage::onTheSafe(sender);
        else BSCalendarPopup::create(this, menu_selector(DailyLevelPage::onTheSafe), m_type)->show();
    }
};
