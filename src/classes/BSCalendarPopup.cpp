#include "BSCalendarPopup.hpp"
#include "BSSelectPopup.hpp"
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameToolbox.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>

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

    auto sundayFirst = Mod::get()->getSettingValue<bool>("sunday-first");
    createWeekdayLabel("Mon", (int)sundayFirst);
    createWeekdayLabel("Tue", (int)sundayFirst + 1);
    createWeekdayLabel("Wed", (int)sundayFirst + 2);
    createWeekdayLabel("Thu", (int)sundayFirst + 3);
    createWeekdayLabel("Fri", (int)sundayFirst + 4);
    createWeekdayLabel("Sat", (int)sundayFirst + 5);
    createWeekdayLabel("Sun", sundayFirst ? 0 : 6);

    m_prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        if (m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth))
            loadMonth(m_month == 1 ? m_year - 1 : m_year, m_month == 1 ? 12 : m_month - 1);
    });
    m_prevButton->setPosition({ -34.5f, 140.0f });
    m_prevButton->setVisible(false);
    m_prevButton->setID("prev-button");
    m_buttonMenu->addChild(m_prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    m_nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        if (m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth))
            loadMonth(m_month == 12 ? m_year + 1 : m_year, m_month == 12 ? 1 : m_month + 1);
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
        if (m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth)) loadMonth(m_minYear, m_minMonth);
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
        if (m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth)) loadMonth(m_maxYear, m_maxMonth);
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
        BSSelectPopup::create(this, [this](int year, int month) { loadMonth(year, month); })->show();
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
        if (m_hoverNode) m_hoverNode->close();
        m_calendarMenu->removeAllChildren();
        m_prevButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_firstButton->setVisible(false);
        m_lastButton->setVisible(false);
        m_monthButton->setEnabled(false);
        BetterSafe::getSafeLevels(m_type).clear();
        loadSafe(true);
    });
    refreshButton->setPosition({ 340.0f, 80.0f });
    refreshButton->setID("refresh-button");
    m_buttonMenu->addChild(refreshButton);

    handleTouchPriority(this);
    loadSafe();

    return true;
}

void BSCalendarPopup::createWeekdayLabel(const char* text, int idx) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setPosition({ idx * 38.0f + 36.0f, 249.0f });
    label->setScale(0.5f);
    label->setID(fmt::format("{}-label", string::toLower(text)));
    m_mainLayer->addChild(label);
}

void BSCalendarPopup::loadSafe(bool refresh) {
    BetterSafe::loadSafe(m_type, &m_listener, [this, refresh] {
        auto& safe = BetterSafe::getSafeLevels(m_type);
        if (safe.empty()) return;

        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
        auto currentTime = time(0);
        auto timeinfo = std::localtime(&currentTime);
        #pragma clang diagnostic pop
        auto currentYear = timeinfo->tm_year + 1900;
        auto currentMonth = timeinfo->tm_mon + 1;

        auto& frontDates = safe.front().dates;
        auto maxDate = frontDates.end() - (int)frontDates.empty();
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
    m_year = year;
    m_month = month;

    if (m_hoverNode) m_hoverNode->close();
    m_calendarMenu->removeAllChildren();
    m_monthLabel->setString(fmt::format("{} {}", BSSelectPopup::MONTHS[m_month - 1], m_year).c_str());
    m_monthButton->updateSprite();
    m_monthButton->setEnabled(false);
    m_prevButton->setVisible(false);
    m_nextButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);
    if (levelSafe.empty()) return setupMonth(nullptr);

    auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, ranges::reduce<std::string>(levelSafe,
        [](std::string& str, const SafeLevel& level) { str += (str.empty() ? "" : ",") + std::to_string(level.id); }));

    auto glm = GameLevelManager::get();
    if (!refresh) {
        std::string key = searchObject->getKey();
        if (auto storedLevels = glm->getStoredOnlineLevels(key.substr(std::max(0, (int)key.size() - 256)).c_str())) return setupMonth(storedLevels);
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

void BSCalendarPopup::setupMonth(CCArray* levels) {
    m_monthButton->setEnabled(true);
    m_loadingCircle->setVisible(false);

    m_prevButton->setVisible(m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth));
    m_firstButton->setVisible(m_year > m_minYear || (m_year == m_minYear && m_month > m_minMonth));
    m_nextButton->setVisible(m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth));
    m_lastButton->setVisible(m_year < m_maxYear || (m_year == m_maxYear && m_month < m_maxMonth));

    if (!levels) return;

    tm timeinfo = { 0, 0, 0, 1, m_month - 1, m_year - 1900 };
    auto time = mktime(&timeinfo);
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    auto firstWeekday = std::localtime(&time)->tm_wday;
    #pragma clang diagnostic pop
    if (!Mod::get()->getSettingValue<bool>("sunday-first")) firstWeekday = (firstWeekday + 6) % 7;

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);
    auto daysInMonth = 0;
    switch (m_month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: daysInMonth = 31; break;
        case 2: daysInMonth = 28 + (m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0)); break;
        case 4: case 6: case 9: case 11: daysInMonth = 30; break;
    }
    for (int i = 0; i < daysInMonth; i++) {
        auto safeLevelIt = std::ranges::find_if(levelSafe, [this, i](const SafeLevel& level) {
            return std::ranges::any_of(level.dates, [this, i](const SafeDate& date) {
                return date.year == m_year && date.month == m_month && date.day == i + 1;
            });
        });
        if (safeLevelIt == levelSafe.end()) continue;

        auto& safeLevel = *safeLevelIt;
        auto gameLevels = reinterpret_cast<GJGameLevel**>(levels->data->arr);
        auto gameLevelsEnd = gameLevels + levels->count();
        auto gameLevelIt = std::find_if(gameLevels, gameLevelsEnd, [&safeLevel](GJGameLevel* level) {
            return level->m_levelID.value() == safeLevel.id;
        });
        if (gameLevelIt == gameLevelsEnd) continue;

        auto gameLevel = *gameLevelIt;

        auto levelDifficulty = gameLevel->m_demon > 0 ? gameLevel->m_demonDifficulty > 0 ? gameLevel->m_demonDifficulty + 4 : 6 :
            gameLevel->m_autoLevel ? -1 : gameLevel->m_ratings < 5 ? 0 : gameLevel->m_ratingsSum / gameLevel->m_ratings;
        auto diffFrame = levelDifficulty == -1 ? "diffIcon_auto_btn_001.png" : fmt::format("diffIcon_{:02d}_btn_001.png", levelDifficulty);
        if (auto moreDifficulties = Loader::get()->getLoadedMod("uproxide.more_difficulties")) {
            auto legacy = moreDifficulties->getSettingValue<bool>("legacy-difficulties");
            auto levelStars = gameLevel->m_stars.value();
            if (moreDifficulties->getSavedValue<bool>("casual", true) && levelDifficulty == 3 && levelStars == 4)
                diffFrame = fmt::format("uproxide.more_difficulties/MD_Difficulty04Small{}.png", legacy ? "_Legacy" : "");
            else if (moreDifficulties->getSavedValue<bool>("tough", true) && levelDifficulty == 4 && levelStars == 7)
                diffFrame = fmt::format("uproxide.more_difficulties/MD_Difficulty07Small{}.png", legacy ? "_Legacy" : "");
            else if (moreDifficulties->getSavedValue<bool>("cruel", true) && levelDifficulty == 5 && levelStars == 9)
                diffFrame = fmt::format("uproxide.more_difficulties/MD_Difficulty09Small{}.png", legacy ? "_Legacy" : "");
        }

        auto hasBetweenDemon = false;
        if (auto demonsInBetween = Loader::get()->getLoadedMod("hiimjustin000.demons_in_between")) {
            if (demonsInBetween->getSettingValue<bool>("enable-difficulties") && safeLevel.tier > 0) {
                auto betweenDifficulty = 0;
                switch (safeLevel.tier) {
                    case 1: betweenDifficulty = 1; break;
                    case 2: betweenDifficulty = 2; break;
                    case 3: betweenDifficulty = 3; break;
                    case 4: betweenDifficulty = 4; break;
                    case 5: betweenDifficulty = 5; break;
                    case 6: betweenDifficulty = 6; break;
                    case 7: betweenDifficulty = 7; break;
                    case 8: betweenDifficulty = 8; break;
                    case 9: betweenDifficulty = 9; break;
                    case 10: betweenDifficulty = 10; break;
                    case 11: case 12: betweenDifficulty = 11; break;
                    case 13: betweenDifficulty = 12; break;
                    case 14: betweenDifficulty = 13; break;
                    case 15: case 16: betweenDifficulty = 14; break;
                    case 17: case 18: betweenDifficulty = 15; break;
                    case 19: betweenDifficulty = 16; break;
                    case 20: betweenDifficulty = 17; break;
                    case 21: betweenDifficulty = 18; break;
                    case 22: betweenDifficulty = 19; break;
                    case 23: case 24: case 25: case 26: case 27: case 28: case 29:
                    case 30: case 31: case 32: case 33: case 34: case 35: betweenDifficulty = 20; break;
                }
                if (betweenDifficulty > 0) {
                    diffFrame = fmt::format("hiimjustin000.demons_in_between/DIB_{:02d}_001.png", betweenDifficulty);
                    hasBetweenDemon = true;
                }
            }
        }

        auto diffIcon = CCSprite::createWithSpriteFrameName(diffFrame.c_str());
        diffIcon->setScale(0.75f);

        if (gameLevel->m_featured > 0) {
            auto featureFrame = "";
            switch (gameLevel->m_isEpic) {
                case 0: featureFrame = "GJ_featuredCoin_001.png"; break;
                case 1: featureFrame = "GJ_epicCoin_001.png"; break;
                case 2: featureFrame = "GJ_epicCoin2_001.png"; break;
                case 3: featureFrame = "GJ_epicCoin3_001.png"; break;
            }
            if (auto featureIcon = CCSprite::createWithSpriteFrameName(featureFrame)) {
                featureIcon->setPosition(diffIcon->getContentSize() / 2.0f + CCPoint { 0.0f, -5.5f });
                if (hasBetweenDemon) featureIcon->setPositionY(9.5f);
                diffIcon->addChild(featureIcon, -2);
                if (gameLevel->m_isEpic == 3) {
                    auto mythicParticles = GameToolbox::particleFromString(
                        "30a-1a1.3a0.2a20a90a0a10a5a20a20a0a0a8a0a0a0a4a1a0a0a0a0a1a0a1a0a1a0a1a1a0a0a0a0a0.784314"
                        "a0a1a0a1a0a0.27a0a0.27a0a0a0a0a0a0a0a0a2a1a0a0a0a0a0a0a0.25a0a0a0a0a0a0a0a0a0a0a0",
                        nullptr, false
                    );
                    mythicParticles->setPosition(featureIcon->getPosition());
                    mythicParticles->setScale(0.9f);
                    mythicParticles->resetSystem();
                    diffIcon->addChild(mythicParticles, -1);
                    for (int i = 0; i < 5; i++) {
                        mythicParticles->update(0.15f);
                    }
                }
            }
        }

        auto hoverButton = CCMenuItemExt::createSpriteExtra(diffIcon, [this, gameLevel, safeLevel](CCMenuItemSpriteExtra* sender) {
            if (m_hoverNode) m_hoverNode->close();
            m_hoverNode = BSHoverNode::create(safeLevel, gameLevel, [this] { m_hoverNode = nullptr; });
            m_hoverNode->setPosition(sender->getPosition() + CCPoint {
                0.0f,
                sender->getContentHeight() / 2.0f + m_hoverNode->getContentHeight() / 2.0f + 5.0f
            });
            m_mainLayer->addChild(m_hoverNode, 200);
        });
        hoverButton->setPosition({ (i + firstWeekday) % 7 * 38.0f + 36.0f, 219.0f - (int)((i + firstWeekday) / 7) * 38.0f });
        hoverButton->setID(fmt::format("level-button-{}", i + 1));
        m_calendarMenu->addChild(hoverButton);
    }
}

BSCalendarPopup::~BSCalendarPopup() {
    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}
