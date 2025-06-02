#include "BSSelectPopup.hpp"
#include "BSCalendarPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

BSSelectPopup* BSSelectPopup::create(BSCalendarPopup* popup, SelectMonthCallback callback) {
    auto ret = new BSSelectPopup();
    if (ret->initAnchored(
        250.0f,
        150.0f,
        popup,
        std::move(callback),
        popup->getType() == GJTimedLevelType::Daily ? "GJ_square01.png" : "GJ_square05.png"
    )) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSSelectPopup::setup(BSCalendarPopup* popup, SelectMonthCallback callback) {
    setID("BSSelectPopup");
    setTitle("Select Date");
    m_title->setID("select-date-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");
    m_noElasticity = true;

    m_year = popup->getYear();
    m_month = popup->getMonth();

    if (popup->getType() == GJTimedLevelType::Event) m_bgSprite->setColor({ 190, 47, 242 });

    auto monthLabel = CCLabelBMFont::create(MONTHS[m_month - 1], "bigFont.fnt");
    monthLabel->setScale(0.9f);
    monthLabel->setPosition({ 125.0f, 100.0f });
    monthLabel->setID("month-label");
    m_mainLayer->addChild(monthLabel);

    auto yearLabel = CCLabelBMFont::create(std::to_string(m_year).c_str(), "bigFont.fnt");
    yearLabel->setScale(0.9f);
    yearLabel->setPosition({ 125.0f, 65.0f });
    yearLabel->setID("year-label");
    m_mainLayer->addChild(yearLabel);

    auto prevMonthButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 0.8f, [this, popup, monthLabel, yearLabel](auto) {
        if (m_year == popup->getMinYear() && m_month == popup->getMinMonth()) return;
        if (m_month == 1) {
            m_year--;
            m_month = 12;
        }
        else m_month--;
        monthLabel->setString(MONTHS[m_month - 1]);
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    prevMonthButton->setPosition({ 25.0f, 100.0f });
    prevMonthButton->setID("prev-month-button");
    m_buttonMenu->addChild(prevMonthButton);

    auto nextMonthButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextMonthButtonSprite->setFlipX(true);
    nextMonthButtonSprite->setScale(0.8f);
    auto nextMonthButton = CCMenuItemExt::createSpriteExtra(nextMonthButtonSprite, [this, popup, monthLabel, yearLabel](auto) {
        if (m_year == popup->getMaxYear() && m_month == popup->getMaxMonth()) return;
        if (m_month == 12) {
            m_year++;
            m_month = 1;
        }
        else m_month++;
        monthLabel->setString(MONTHS[m_month - 1]);
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    nextMonthButton->setPosition({ 225.0f, 100.0f });
    nextMonthButton->setID("next-month-button");
    m_buttonMenu->addChild(nextMonthButton);

    auto prevYearButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 0.8f, [this, popup, monthLabel, yearLabel](auto) {
        if (m_year == popup->getMinYear()) return;
        m_year--;
        if (m_year == popup->getMinYear() && m_month < popup->getMinMonth()) m_month = popup->getMinMonth();
        monthLabel->setString(MONTHS[m_month - 1]);
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    prevYearButton->setPosition({ 25.0f, 65.0f });
    prevYearButton->setID("prev-year-button");
    m_buttonMenu->addChild(prevYearButton);

    auto nextYearButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextYearButtonSprite->setFlipX(true);
    nextYearButtonSprite->setScale(0.8f);
    auto nextYearButton = CCMenuItemExt::createSpriteExtra(nextYearButtonSprite, [this, popup, monthLabel, yearLabel](auto) {
        if (m_year == popup->getMaxYear()) return;
        m_year++;
        if (m_year == popup->getMaxYear() && m_month > popup->getMaxMonth()) m_month = popup->getMaxMonth();
        monthLabel->setString(MONTHS[m_month - 1]);
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    nextYearButton->setPosition({ 225.0f, 65.0f });
    nextYearButton->setID("next-year-button");
    m_buttonMenu->addChild(nextYearButton);

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", "goldFont.fnt", "GJ_button_01.png", 0.8f), [
        this, callback = std::move(callback)
    ](auto) {
        callback(m_year, m_month);
        onClose(nullptr);
    });
    confirmButton->setPosition({ 125.0f, 25.0f });
    confirmButton->setID("confirm-button");
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}
