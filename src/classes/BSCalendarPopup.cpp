#include "BSCalendarPopup.hpp"
#include "BSSelectPopup.hpp"
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJDifficultySprite.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/loader/Mod.hpp>
#include <jasmine/search.hpp>
#include <jasmine/setting.hpp>

using namespace geode::prelude;

BSCalendarPopup* BSCalendarPopup::create(CCObject* obj, SEL_MenuHandler onSafe, GJTimedLevelType type) {
    auto ret = new BSCalendarPopup();
    if (ret->initAnchored(300.0f, 280.0f, obj, onSafe, type, type == GJTimedLevelType::Daily ? "GJ_square01.png" : "GJ_square05.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSCalendarPopup::setup(CCObject* obj, SEL_MenuHandler onSafe, GJTimedLevelType type) {
    setID("BSCalendarPopup");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");
    m_noElasticity = true;

    m_type = type;

    if (type == GJTimedLevelType::Event) m_bgSprite->setColor({ 190, 47, 242 });

    constexpr std::array weekdays = {
        std::make_pair("Mon", "mon-label"),
        std::make_pair("Tue", "tue-label"),
        std::make_pair("Wed", "wed-label"),
        std::make_pair("Thu", "thu-label"),
        std::make_pair("Fri", "fri-label"),
        std::make_pair("Sat", "sat-label"),
        std::make_pair("Sun", "sun-label")
    };

    auto sundayFirst = jasmine::setting::getValue<bool>("sunday-first");
    for (int i = 0; i < weekdays.size(); i++) {
        auto [text, id] = weekdays[(i + (sundayFirst ? 6 : 0)) % 7];
        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setPosition({ i * 38.0f + 36.0f, 249.0f });
        label->setScale(0.5f);
        label->setID(id);
        m_mainLayer->addChild(label);
    }

    m_prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        if (m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth)) {
            loadMonth(m_month == 1 ? m_year - 1 : m_year, m_month == 1 ? 12 : m_month - 1);
        }
    });
    m_prevButton->setPosition({ -34.5f, 140.0f });
    m_prevButton->setVisible(false);
    m_prevButton->setID("prev-button");
    m_buttonMenu->addChild(m_prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    m_nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        if (m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth)) {
            loadMonth(m_month == 12 ? m_year + 1 : m_year, m_month == 12 ? 1 : m_month + 1);
        }
    });
    m_nextButton->setPosition({ 334.5f, 140.0f });
    m_nextButton->setVisible(false);
    m_nextButton->setID("next-button");
    m_buttonMenu->addChild(m_nextButton);

    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition(firstArrow->getContentSize() / 2.0f - CCPoint { 20.0f, 0.0f });
    firstArrow->addChild(otherFirstArrow);
    firstArrow->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) {
        if (m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth)) {
            loadMonth(m_minYear, m_minMonth);
        }
    });
    m_firstButton->setPosition({ -27.5f, 175.0f });
    m_firstButton->setVisible(false);
    m_firstButton->setID("first-button");
    m_buttonMenu->addChild(m_firstButton);

    auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    lastArrow->setFlipX(true);
    auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherLastArrow->setPosition(lastArrow->getContentSize() / 2.0f + CCPoint { 20.0f, 0.0f });
    otherLastArrow->setFlipX(true);
    lastArrow->addChild(otherLastArrow);
    lastArrow->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) {
        if (m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth)) {
            loadMonth(m_maxYear, m_maxMonth);
        }
    });
    m_lastButton->setPosition({ 327.5f, 175.0f });
    m_lastButton->setVisible(false);
    m_lastButton->setID("last-button");
    m_buttonMenu->addChild(m_lastButton);

    m_calendarMenu = CCMenu::create();
    m_calendarMenu->setPosition({ 150.0f, 140.0f });
    m_calendarMenu->setContentSize({ 300.0f, 280.0f });
    m_calendarMenu->ignoreAnchorPointForPosition(false);
    m_calendarMenu->setID("calendar-menu");
    m_mainLayer->addChild(m_calendarMenu);

    m_monthLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_monthLabel->setScale(0.6f);
    m_monthButton = CCMenuItemExt::createSpriteExtra(m_monthLabel, [this](auto) {
        BSSelectPopup::create(this, [this](int year, int month) {
            loadMonth(year, month);
        })->show();
    });
    m_monthButton->setPosition({ 150.0f, 265.0f });
    m_monthButton->setEnabled(false);
    m_monthButton->setID("month-button");
    m_buttonMenu->addChild(m_monthButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setContentSize({ 300.0f, 280.0f });
    m_loadingCircle->m_sprite->setPosition({ 150.0f, 140.0f });
    m_loadingCircle->setParentLayer(m_mainLayer);
    m_loadingCircle->setID("loading-circle");
    m_loadingCircle->show();

    auto safeButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_safeBtn_001.png"), obj, onSafe);
    safeButton->setPosition({ 340.0f, 25.0f });
    safeButton->setTag(91508); // my birthday
    safeButton->setID("safe-button");
    m_buttonMenu->addChild(safeButton);

    auto refreshButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 1.0f, [this](auto) {
        m_loadingCircle->setVisible(true);
        closeHoverNode();
        m_calendarMenu->removeAllChildren();
        m_prevButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_firstButton->setVisible(false);
        m_lastButton->setVisible(false);
        m_monthButton->setEnabled(false);
        BetterSafe::safes[m_type].clear();
        loadSafe(true);
    });
    refreshButton->setPosition({ 340.0f, 80.0f });
    refreshButton->setID("refresh-button");
    m_buttonMenu->addChild(refreshButton);

    loadSafe();
    handleTouchPriority(this);
    queueInMainThread([this] { m_initialized = true; });

    return true;
}

void BSCalendarPopup::closeHoverNode() {
    if (m_hoverNode) {
        auto [year, month, day] = m_currentDay;
        m_hoverNode->close();
        m_currentDay.year = year;
        m_currentDay.month = month;
        m_currentDay.day = day;
    }
}

void BSCalendarPopup::loadSafe(bool refresh) {
    BetterSafe::loadSafe(m_type, m_listener, [this, refresh] {
        auto& safe = BetterSafe::safes[m_type];
        if (safe.empty()) return;

        auto timeinfo = fmt::localtime(time(0));
        auto currentYear = timeinfo.tm_year + 1900;
        auto currentMonth = timeinfo.tm_mon + 1;

        auto& frontDates = safe.front().dates;
        auto maxDate = frontDates.end() - (int)!frontDates.empty();
        m_maxYear = maxDate != frontDates.end() ? maxDate->year : currentYear;
        m_maxMonth = maxDate != frontDates.end() ? maxDate->month : currentMonth;

        auto& backDates = safe.back().dates;
        auto minDate = backDates.begin();
        m_minYear = minDate != backDates.end() ? minDate->year : currentYear;
        m_minMonth = minDate != backDates.end() ? minDate->month : currentMonth;

        loadMonth(m_year == 0 ? currentYear : m_year, m_month == 0 ? currentMonth : m_month, refresh);
    }, [this](int code) {
        FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load safe levels. Please try again later.", "OK")->show();
        m_loadingCircle->setVisible(false);
    });
}

void BSCalendarPopup::loadMonth(int year, int month, bool refresh) {
    if (month < 1 || month > 12) return;

    m_year = year;
    m_month = month;

    closeHoverNode();
    m_calendarMenu->removeAllChildren();
    m_monthLabel->setString(fmt::format("{} {}", BSSelectPopup::months[m_month - 1], m_year).c_str());
    m_monthButton->updateSprite();
    m_monthButton->setEnabled(false);
    m_prevButton->setVisible(false);
    m_nextButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);
    if (levelSafe.empty()) return setupMonth(nullptr);

    auto searchObject = jasmine::search::getObject(levelSafe, [](const SafeLevel& level) {
        return fmt::to_string(level.levelID);
    });

    auto glm = GameLevelManager::get();
    if (!refresh) {
        if (auto levels = glm->getStoredOnlineLevels(jasmine::search::getKey(searchObject))) {
            return setupMonth(levels);
        }
    }

    m_loadingCircle->setVisible(true);
    glm->m_levelManagerDelegate = this;
    glm->getOnlineLevels(searchObject);
}

void BSCalendarPopup::loadLevelsFinished(CCArray* levels, const char*, int) {
    setupMonth(levels);
}

void BSCalendarPopup::loadLevelsFailed(const char*, int) {
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create("Load Failed", "Failed to load safe levels. Please try again later.", "OK")->show();
}

void BSCalendarPopup::onEnter() {
    CCLayer::onEnter();
    if (m_initialized) loadMonth(m_year, m_month);
}

void BSCalendarPopup::setupMonth(CCArray* levels) {
    m_monthButton->setEnabled(true);
    m_loadingCircle->setVisible(false);

    m_prevButton->setVisible(m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth));
    m_firstButton->setVisible(m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth));
    m_nextButton->setVisible(m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth));
    m_lastButton->setVisible(m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth));

    if (!levels) return;

    tm timeinfo = { 0, 0, 0, 1, m_month - 1, m_year - 1900 };
    auto firstWeekday = fmt::localtime(mktime(&timeinfo)).tm_wday;
    if (!jasmine::setting::getValue<bool>("sunday-first")) firstWeekday = (firstWeekday + 6) % 7;

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);

    std::unordered_map<int, GJGameLevel*> levelMap;
    for (auto level : CCArrayExt<GJGameLevel*>(levels)) {
        levelMap[level->m_levelID.value()] = level;
    }

    constexpr std::array daysPerMonth = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    auto daysInMonth = m_month <= daysPerMonth.size() ? daysPerMonth[m_month - 1] : 0;
    if (m_month == 2 && m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0)) daysInMonth++;

    auto loader = Loader::get();
    auto demonsInBetween = loader->getLoadedMod("hiimjustin000.demons_in_between");
    auto enableDifficulties = demonsInBetween && demonsInBetween->getSettingValue<bool>("enable-difficulties");
    auto enableLegendary = demonsInBetween && demonsInBetween->getSettingValue<bool>("enable-legendary");
    auto enableMythic = demonsInBetween && demonsInBetween->getSettingValue<bool>("enable-mythic");
    auto moreDifficulties = loader->getLoadedMod("uproxide.more_difficulties");
    auto legacyDifficulties = moreDifficulties && moreDifficulties->getSettingValue<bool>("legacy-difficulties");
    auto showCheckmarks = jasmine::setting::getValue<bool>("show-checkmarks");
    auto showDays = jasmine::setting::getValue<bool>("show-days");
    auto showParticles = jasmine::setting::getValue<bool>("show-particles");
    auto sfc = CCSpriteFrameCache::get();

    for (int i = 0; i < daysInMonth; i++) {
        auto d = i + 1;
        auto safeLevelIt = std::ranges::find_if(levelSafe, [this, d](const SafeLevel& level) {
            return std::ranges::any_of(level.dates, [this, d](const SafeDate& date) {
                return date.year == m_year && date.month == m_month && date.day == d;
            });
        });
        if (safeLevelIt == levelSafe.end()) continue;
        auto& safeLevel = *safeLevelIt;

        auto levelIt = levelMap.find(safeLevel.levelID);
        if (levelIt == levelMap.end()) continue;
        auto level = levelIt->second;

        auto completedLevel = showCheckmarks && GameStatsManager::get()->hasCompletedLevel(level);

        auto diffNode = CCNode::create();
        diffNode->setScale(0.75f);
        diffNode->setContentSize({ 40.0f, 40.0f });
        diffNode->setAnchorPoint({ 0.5f, 0.5f });

        auto stencil = CCLayerColor::create({ 0, 0, 0, 255 }, 40.0f, 35.0f);
        stencil->ignoreAnchorPointForPosition(false);
        stencil->setPosition({ 20.0f, 20.0f });
        auto clipNode = CCClippingNode::create(stencil);
        clipNode->setPosition({ 20.0f, 20.0f });
        clipNode->setContentSize({ 40.0f, 40.0f });
        clipNode->setAnchorPoint({ 0.5f, 0.5f });

        auto difficulty = level->m_demon > 0 ? GJGameLevel::demonIconForDifficulty((DemonDifficultyType)level->m_demonDifficulty) :
            level->m_autoLevel ? -1 : level->getAverageDifficulty();
        auto featureState = level->m_featured > 0 ? level->m_isEpic + 1 : 0;

        std::string diffFrame;
        CCPoint diffPos = { 20.0f, 14.5f };

        if (moreDifficulties) {
            auto stars = level->m_stars.value();
            if (moreDifficulties->getSavedValue<bool>("casual", true) && difficulty == 3 && stars == 4) {
                diffFrame = fmt::format("uproxide.more_difficulties/MD_Difficulty04{}.png", legacyDifficulties ? "_Legacy" : "");
            }
            else if (moreDifficulties->getSavedValue<bool>("tough", true) && difficulty == 4 && stars == 7) {
                diffFrame = fmt::format("uproxide.more_difficulties/MD_Difficulty07{}.png", legacyDifficulties ? "_Legacy" : "");
            }
            else if (moreDifficulties->getSavedValue<bool>("cruel", true) && difficulty == 5 && stars == 9) {
                diffFrame = fmt::format("uproxide.more_difficulties/MD_Difficulty09{}.png", legacyDifficulties ? "_Legacy" : "");
            }
        }

        if (enableDifficulties && safeLevel.tier > 0) {
            constexpr std::array difficulties = {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 15,
                16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
            };
            constexpr std::array<CCPoint, 21> positions = {
                CCPoint { 0.0f, 0.0f },
                { -0.125f, -0.25f }, { -0.125f, -0.25f }, { -0.125f, -0.25f }, { -0.125f, -0.375f }, { -0.125f, -0.25f },
                { -0.125f, -0.25f }, { -0.125f, -0.375f }, { -0.125f, 0.5f }, { -0.125f, 0.5f }, { -0.125f, 0.25f },
                { -0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.0f, 0.5f },
                { 0.0f, 1.25f }, { 0.0f, 1.25f }, { 0.0f, 1.125f }, { 0.0f, 1.125f }, { 0.0f, 1.125f }
            };
            if (safeLevel.tier < difficulties.size()) {
                auto diff = difficulties[safeLevel.tier];
                diffFrame = fmt::format("hiimjustin000.demons_in_between/DIB_{:02}{}_btn_001.png",
                    diff, enableLegendary && featureState == 3 ? "_4" : enableMythic && featureState == 4 ? "_5" : "");
                diffPos += positions[diff];
            }
        }

        auto diffSprite = GJDifficultySprite::create(difficulty, GJDifficultyName::Short);
        diffSprite->updateFeatureState((GJFeatureState)featureState);
        auto diffIcon = CCSprite::createWithSpriteFrame(diffFrame.empty() ? diffSprite->displayFrame() : sfc->spriteFrameByName(diffFrame.c_str()));
        diffIcon->setPosition(diffPos);
        diffIcon->setOpacity(completedLevel ? 150 : 255);
        clipNode->addChild(diffIcon);

        diffNode->addChild(clipNode);

        auto diffSize = diffSprite->getContentSize() / 2.0f;
        for (Ref child = diffSprite->getChildByIndex(0); diffSprite->getChildrenCount() > 0; child = diffSprite->getChildByIndex(0)) {
            child->removeFromParentAndCleanup(false);
            child->setPosition(child->getPosition() - diffSize + CCPoint { 20.0f, 14.5f });
            if (auto rgbaProtocol = typeinfo_cast<CCRGBAProtocol*>(child.data())) {
                rgbaProtocol->setOpacity(completedLevel ? 150 : 255);
            }
            else if (typeinfo_cast<CCParticleSystem*>(child.data())) {
                child->setVisible(showParticles && !completedLevel);
            }
            diffNode->addChild(child);
        }

        if (completedLevel) {
            auto completedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            completedIcon->setPosition({ 20.0f, 20.0f });
            diffNode->addChild(completedIcon, 1);
        }
        else if (showDays) {
            auto dayLabel = CCLabelBMFont::create(fmt::to_string(d).c_str(), "bigFont.fnt");
            dayLabel->setScale(0.4f);
            dayLabel->setPosition({ 40.0f, 0.0f });
            dayLabel->setAnchorPoint({ 1.0f, 0.0f });
            diffNode->addChild(dayLabel, 1);
        }

        auto hoverButton = CCMenuItemExt::createSpriteExtra(diffNode, [this, d, level, safeLevel](CCMenuItemSpriteExtra* sender) {
            if (m_hoverNode) {
                auto [year, month, day] = m_currentDay;
                m_hoverNode->close();
                if (year == m_year && month == m_month && d == day) return;
            }
            createHoverNode(sender, safeLevel, level);
            m_currentDay.year = m_year;
            m_currentDay.month = m_month;
            m_currentDay.day = d;
        });
        hoverButton->setPosition({ (i + firstWeekday) % 7 * 38.0f + 36.0f, 219.0f - (int)((i + firstWeekday) / 7) * 38.0f });
        hoverButton->setID(fmt::format("level-button-{}", d));
        m_calendarMenu->addChild(hoverButton);

        if (m_year == m_currentDay.year && m_month == m_currentDay.month && d == m_currentDay.day) {
            createHoverNode(hoverButton, safeLevel, level);
        }
    }
}

void BSCalendarPopup::createHoverNode(CCMenuItemSpriteExtra* sender, const SafeLevel& safeLevel, GJGameLevel* gameLevel) {
    m_hoverNode = BSHoverNode::create(safeLevel, gameLevel, [this] {
        m_hoverNode = nullptr;
        m_currentDay.year = 0;
        m_currentDay.month = 0;
        m_currentDay.day = 0;
    });
    m_hoverNode->setPosition(sender->getPosition() + CCPoint {
        0.0f,
        sender->getContentHeight() / 2.0f + m_hoverNode->getContentHeight() / 2.0f + 5.0f
    });
    m_mainLayer->addChild(m_hoverNode, 200);
}

BSCalendarPopup::~BSCalendarPopup() {
    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}
