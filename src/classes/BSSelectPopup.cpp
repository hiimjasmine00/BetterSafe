#include "BSSelectPopup.hpp"
#include "BSCalendarPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

BSSelectPopup* BSSelectPopup::create(BSCalendarPopup* popup, SelectMonthCallback callback) {
    auto ret = new BSSelectPopup();
    if (ret->init(popup, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSSelectPopup::init(BSCalendarPopup* popup, SelectMonthCallback callback) {
    auto type = popup->getType();
    if (!Popup::init(300.0f, 150.0f, type == GJTimedLevelType::Daily ? "GJ_square01.png" : "GJ_square05.png")) return false;

    setID("BSSelectPopup");
    setTitle("Select Month");
    m_title->setID("select-date-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");
    m_noElasticity = true;

    m_callback = std::move(callback);
    m_year = popup->getYear();
    m_minYear = popup->getMinYear();
    m_minMonth = popup->getMinMonth();
    m_maxYear = popup->getMaxYear();
    m_maxMonth = popup->getMaxMonth();
    m_buttons.reserve(months.size());

    if (type == GJTimedLevelType::Event) m_bgSprite->setColor({ 190, 47, 242 });

    for (int i = 0; i < months.size(); i++) {
        auto month = i + 1;
        auto monthLabel = CCLabelBMFont::create(months[i], "bigFont.fnt");
        monthLabel->limitLabelWidth(65.0f, 1.0f, 0.0f);
        auto monthButton = CCMenuItemSpriteExtra::create(monthLabel, this, menu_selector(BSSelectPopup::onMonth));
        monthButton->setPosition({ (i % 4) * 70.0f + 50.0f, 100.0f - floorf(i / 4.0f) * 35.0f });
        monthButton->setTag(month);
        monthButton->setID(fmt::format("month-button-{}", month));
        m_buttonMenu->addChild(monthButton);
        m_buttons.push_back(monthButton);
    }

    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    m_prevButton = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(BSSelectPopup::onPrevYear));
    m_prevButton->setPosition({ -34.5f, 75.0f });
    m_prevButton->setID("prev-button");
    m_buttonMenu->addChild(m_prevButton);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSprite->setFlipX(true);
    m_nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(BSSelectPopup::onNextYear));
    m_nextButton->setPosition({ 334.5f, 75.0f });
    m_nextButton->setID("next-button");
    m_buttonMenu->addChild(m_nextButton);

    page(m_year);
    handleTouchPriority(this);

    return true;
}

void BSSelectPopup::onPrevYear(cocos2d::CCObject* sender) {
    if (m_year > m_minYear) page(m_year - 1);
}

void BSSelectPopup::onNextYear(cocos2d::CCObject* sender) {
    if (m_year < m_maxYear) page(m_year + 1);
}

void BSSelectPopup::onMonth(cocos2d::CCObject* sender) {
    m_callback(m_year, sender->getTag());
    onClose(nullptr);
}

void BSSelectPopup::page(int year) {
    m_year = year;
    setTitle(fmt::format("Select Month ({})", m_year));

    if (m_year == m_minYear) {
        m_prevButton->setVisible(false);
        for (int i = 0; i < m_buttons.size(); i++) {
            m_buttons[i]->setVisible(i + 1 >= m_minMonth);
        }
    }
    else m_prevButton->setVisible(true);

    if (m_year == m_maxYear) {
        m_nextButton->setVisible(false);
        for (int i = 0; i < m_buttons.size(); i++) {
            m_buttons[i]->setVisible(i + 1 <= m_maxMonth);
        }
    }
    else m_nextButton->setVisible(true);

    if (m_year > m_minYear && m_year < m_maxYear) {
        for (int i = 0; i < m_buttons.size(); i++) {
            m_buttons[i]->setVisible(true);
        }
    }
}
