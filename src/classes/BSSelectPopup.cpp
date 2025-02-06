#include "BSCalendarPopup.hpp"
#include "BSSelectPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

BSSelectPopup* BSSelectPopup::create(int year, int month, int minYear, int minMonth, int maxYear, int maxMonth, SelectMonthCallback callback) {
    auto ret = new BSSelectPopup();
    if (ret->initAnchored(250.0f, 150.0f, year, month, minYear, minMonth, maxYear, maxMonth, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSSelectPopup::setup(int year, int month, int minYear, int minMonth, int maxYear, int maxMonth, SelectMonthCallback callback) {
    setTitle("Select Date");

    m_noElasticity = true;
    m_year = year;
    m_month = month;

    auto monthLabel = CCLabelBMFont::create(BSCalendarPopup::MONTHS[month - 1].c_str(), "bigFont.fnt");
    monthLabel->setScale(0.9f);
    monthLabel->setPosition({ 125.0f, 100.0f });
    m_mainLayer->addChild(monthLabel);

    auto yearLabel = CCLabelBMFont::create(std::to_string(year).c_str(), "bigFont.fnt");
    yearLabel->setScale(0.9f);
    yearLabel->setPosition({ 125.0f, 65.0f });
    m_mainLayer->addChild(yearLabel);

    auto prevMonthButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 0.8f, [this, minMonth, minYear, monthLabel, yearLabel](auto) {
        if (m_year == minYear && m_month == minMonth) return;
        if (m_month == 1) {
            m_year--;
            m_month = 12;
        }
        else m_month--;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    prevMonthButton->setPosition({ 25.0f, 100.0f });
    m_buttonMenu->addChild(prevMonthButton);

    auto nextMonthButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextMonthButtonSprite->setFlipX(true);
    nextMonthButtonSprite->setScale(0.8f);
    auto nextMonthButton = CCMenuItemExt::createSpriteExtra(nextMonthButtonSprite, [this, maxMonth, maxYear, monthLabel, yearLabel](auto) {
        if (m_year == maxYear && m_month == maxMonth) return;
        if (m_month == 12) {
            m_year++;
            m_month = 1;
        }
        else m_month++;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    nextMonthButton->setPosition({ 225.0f, 100.0f });
    m_buttonMenu->addChild(nextMonthButton);

    auto prevYearButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 0.8f, [this, minMonth, minYear, monthLabel, yearLabel](auto) {
        if (m_year == minYear) return;
        m_year--;
        if (m_year == minYear && m_month < minMonth) m_month = minMonth;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    prevYearButton->setPosition({ 25.0f, 65.0f });
    m_buttonMenu->addChild(prevYearButton);

    auto nextYearButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextYearButtonSprite->setFlipX(true);
    nextYearButtonSprite->setScale(0.8f);
    auto nextYearButton = CCMenuItemExt::createSpriteExtra(nextYearButtonSprite, [this, maxMonth, maxYear, monthLabel, yearLabel](auto) {
        if (m_year == maxYear) return;
        m_year++;
        if (m_year == maxYear && m_month > maxMonth) m_month = maxMonth;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    nextYearButton->setPosition({ 225.0f, 65.0f });
    m_buttonMenu->addChild(nextYearButton);

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", "goldFont.fnt", "GJ_button_01.png", 0.8f), [this, callback](auto) {
        callback(m_year, m_month);
        onClose(nullptr);
    });
    confirmButton->setPosition({ 125.0f, 25.0f });
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}
