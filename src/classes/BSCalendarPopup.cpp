#include "BSCalendarPopup.hpp"
#include "BSSelectPopup.hpp"
#include <Geode/binding/GameLevelManager.hpp>
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

    GameLevelManager::get()->m_levelManagerDelegate = this;

    m_prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        if (m_year == m_firstYear && m_month == m_firstMonth) return;

        if (m_month == 1) {
            m_year--;
            m_month = 12;
        }
        else m_month--;
        loadMonth();
    });
    m_prevButton->setPosition({ -34.5f, 140.0f });
    m_prevButton->setVisible(false);
    m_prevButton->setID("prev-button");
    m_buttonMenu->addChild(m_prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    m_nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        if (m_year == m_currentYear && m_month == m_currentMonth) return;
        if (m_month == 12) {
            m_year++;
            m_month = 1;
        }
        else m_month++;
        loadMonth();
    });
    m_nextButton->setPosition({ 334.5f, 140.0f });
    m_nextButton->setVisible(false);
    m_nextButton->setID("next-button");
    m_buttonMenu->addChild(m_nextButton);

    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition(firstArrow->getContentSize() / 2 - CCPoint { 20.0f, 0.0f });
    firstArrow->addChild(otherFirstArrow);
    firstArrow->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) {
        m_year = m_firstYear;
        m_month = m_firstMonth;
        loadMonth();
    });
    m_firstButton->setPosition({ -27.5f, 175.0f });
    m_firstButton->setVisible(false);
    m_firstButton->setID("first-button");
    m_buttonMenu->addChild(m_firstButton);

    auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    lastArrow->setFlipX(true);
    auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherLastArrow->setPosition(lastArrow->getContentSize() / 2 + CCPoint { 20.0f, 0.0f });
    otherLastArrow->setFlipX(true);
    lastArrow->addChild(otherLastArrow);
    lastArrow->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) {
        m_year = m_currentYear;
        m_month = m_currentMonth;
        loadMonth();
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
        BSSelectPopup::create(m_year, m_month, m_firstYear, m_firstMonth, m_currentYear, m_currentMonth, [this](int year, int month) {
            m_year = year;
            m_month = month;
            loadMonth();
        })->show();
    });
    m_monthButton->setPosition({ 150.0f, 265.0f });
    m_monthButton->setEnabled(false);
    m_monthButton->setID("month-button");
    m_buttonMenu->addChild(m_monthButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(m_mainLayer);
    m_loadingCircle->setContentSize({ 300.0f, 280.0f });
    m_loadingCircle->m_sprite->setPosition({ 150.0f, 140.0f });
    m_loadingCircle->retain();
    m_loadingCircle->show();
    m_loadingCircle->setID("loading-circle");

    auto safeButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_safeBtn_001.png"), obj, onSafe);
    safeButton->setPosition({ 340.0f, 25.0f });
    safeButton->setTag(91508); // my birthday
    safeButton->setID("safe-button");
    m_buttonMenu->addChild(safeButton);

    auto refreshButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 1.0f, [this, type](auto) {
        m_loadingCircle->setVisible(true);
        if (m_hoverNode) m_hoverNode->close();
        m_calendarMenu->removeAllChildren();
        m_prevButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_firstButton->setVisible(false);
        m_lastButton->setVisible(false);
        m_monthButton->setEnabled(false);
        BetterSafe::getSafeLevels(type).clear();
        BetterSafe::loadSafe(type, std::move(m_listener), [this, type] {
            if (!BetterSafe::getSafeLevels(type).empty()) loadMonth();
        }, [this](int code) {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load safe levels. Please try again later.", "OK")->show();
            m_loadingCircle->setVisible(false);
        });
    });
    refreshButton->setPosition({ 340.0f, 80.0f });
    refreshButton->setID("refresh-button");
    m_buttonMenu->addChild(refreshButton);

    auto dateNow = BetterSafe::dateFromTime(time(0));
    m_year = dateNow.year;
    m_month = dateNow.month;
    BetterSafe::loadSafe(type, std::move(m_listener), [this, type] {
        auto& safe = BetterSafe::getSafeLevels(type);
        if (safe.empty()) return;

        auto firstLevel = safe[0];
        auto lastLevel = safe.back();
        auto firstDate = ranges::find(firstLevel.dates, [this](const SafeDate& date) { return date.year == m_year && date.month == m_month; });
        if (!firstDate.has_value()) {
            auto lastDate = !firstLevel.dates.empty() ? firstLevel.dates.back() : SafeDate { m_year, m_month, 1 };
            m_currentYear = lastDate.year;
            m_currentMonth = lastDate.month;
        } else {
            m_currentYear = firstDate->year;
            m_currentMonth = firstDate->month;
        }
        auto startDate = !lastLevel.dates.empty() ? lastLevel.dates[0] : SafeDate { m_year, m_month, 1 };
        m_firstYear = startDate.year;
        m_firstMonth = startDate.month;
        loadMonth();
    }, [this](int code) {
        FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load safe levels. Please try again later.", "OK")->show();
        m_loadingCircle->setVisible(false);
    });

    handleTouchPriority(this);

    return true;
}

void BSCalendarPopup::createWeekdayLabel(const char* text, int idx) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setPosition({ idx * 38.0f + 36.0f, 249.0f });
    label->setScale(0.5f);
    label->setID(fmt::format("{}-label", string::toLower(text)));
    m_mainLayer->addChild(label);
}

void BSCalendarPopup::loadMonth() {
    m_loadingCircle->setVisible(true);

    if (m_hoverNode) m_hoverNode->close();

    m_calendarMenu->removeAllChildren();
    m_monthLabel->setString(fmt::format("{} {}", MONTHS[m_month - 1], m_year).c_str());
    m_monthButton->updateSprite();
    m_monthButton->setEnabled(false);

    m_prevButton->setVisible(false);
    m_nextButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);
    if (levelSafe.empty()) return loadLevelsFinished(CCArray::create(), "");

    auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, string::join(ranges::map<std::vector<std::string>>(levelSafe, [](const SafeLevel& level) {
        return std::to_string(level.id);
    }), ","));
    auto glm = GameLevelManager::get();
    std::string key = searchObject->getKey();
    if (auto storedLevels = glm->getStoredOnlineLevels(key.substr(std::max(0, (int)key.size() - 256)).c_str()))
        loadLevelsFinished(storedLevels, searchObject->getKey());
    else glm->getOnlineLevels(searchObject);
}

void BSCalendarPopup::loadLevelsFinished(CCArray* levels, const char*) {
    CC_SAFE_RELEASE(m_levels);
    CC_SAFE_RETAIN(levels);
    m_levels = levels;
    setupMonth();
}

void BSCalendarPopup::loadLevelsFailed(const char*) {
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create("Load Failed", "Failed to load safe levels. Please try again later.", "OK")->show();
}

void BSCalendarPopup::setupMonth() {
    m_monthButton->setEnabled(true);
    m_loadingCircle->setVisible(false);

    m_prevButton->setVisible(m_year != m_firstYear || m_month != m_firstMonth);
    m_firstButton->setVisible(m_year != m_firstYear || m_month != m_firstMonth);
    m_nextButton->setVisible(m_year != m_currentYear || m_month != m_currentMonth);
    m_lastButton->setVisible(m_year != m_currentYear || m_month != m_currentMonth);

    auto daysInMonth = m_month == 2 && m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0) ? 29 : DAYS_IN_MONTH[m_month - 1];
    tm timeinfo = { 0, 0, 0, 1, m_month - 1, m_year - 1900 };
    auto time = mktime(&timeinfo);
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    auto firstWeekday = std::localtime(&time)->tm_wday;
    #pragma clang diagnostic pop
    if (!Mod::get()->getSettingValue<bool>("sunday-first")) firstWeekday = (firstWeekday + 6) % 7;

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);
    for (int i = 0; i < daysInMonth; i++) {
        auto safeLevelOpt = ranges::find(levelSafe, [this, i](const SafeLevel& level) {
            return ranges::contains(level.dates, [this, i](const SafeDate& date) {
                return date.year == m_year && date.month == m_month && date.day == i + 1;
            });
        });
        if (!safeLevelOpt.has_value()) continue;
        auto safeLevel = safeLevelOpt.value();
        auto gameLevelOpt =  ranges::find(CCArrayExt<GJGameLevel*>(m_levels), [safeLevel](GJGameLevel* level) {
            return level->m_levelID == safeLevel.id;
        });
        if (!gameLevelOpt.has_value()) continue;
        auto gameLevel = gameLevelOpt.value();

        auto levelDifficulty = BetterSafe::getDifficultyFromLevel(gameLevel);
        auto diffFrame = levelDifficulty == -1 ? "diffIcon_auto_btn_001.png" : fmt::format("diffIcon_{:02d}_btn_001.png", levelDifficulty);
        if (auto moreDifficulties = Loader::get()->getLoadedMod("uproxide.more_difficulties")) {
            auto legacy = moreDifficulties->getSettingValue<bool>("legacy-difficulties");
            auto levelStars = gameLevel->m_stars.value();
            if (moreDifficulties->getSavedValue<bool>("casual", true) && levelDifficulty == 3 && levelStars == 4)
                diffFrame = legacy ? "uproxide.more_difficulties/MD_Difficulty04Small_Legacy.png" : "uproxide.more_difficulties/MD_Difficulty04Small.png";
            else if (moreDifficulties->getSavedValue<bool>("tough", true) && levelDifficulty == 4 && levelStars == 7)
                diffFrame = legacy ? "uproxide.more_difficulties/MD_Difficulty07Small_Legacy.png" : "uproxide.more_difficulties/MD_Difficulty07Small.png";
            else if (moreDifficulties->getSavedValue<bool>("cruel", true) && levelDifficulty == 5 && levelStars == 9)
                diffFrame = legacy ? "uproxide.more_difficulties/MD_Difficulty09Small_Legacy.png" : "uproxide.more_difficulties/MD_Difficulty09Small.png";
        }
        auto hasBetweenDemon = false;
        if (auto demonsInBetween = Loader::get()->getLoadedMod("hiimjustin000.demons_in_between")) {
            if (demonsInBetween->getSettingValue<bool>("enable-difficulties") && safeLevel.tier > 0) {
                diffFrame = fmt::format("hiimjustin000.demons_in_between/DIB_{:02d}_001.png", DIFFICULTY_INDICES[safeLevel.tier]);
                hasBetweenDemon = true;
            }
        }
        auto diffIcon = CCSprite::createWithSpriteFrameName(diffFrame.c_str());
        diffIcon->setScale(0.75f);
        auto featureFrame = "";
        auto levelFeature = gameLevel->m_featured > 0 ? gameLevel->m_isEpic + 1 : 0;
        switch (levelFeature) {
            case 1: featureFrame = "GJ_featuredCoin_001.png"; break;
            case 2: featureFrame = "GJ_epicCoin_001.png"; break;
            case 3: featureFrame = "GJ_epicCoin2_001.png"; break;
            case 4: featureFrame = "GJ_epicCoin3_001.png"; break;
        }
        if (levelFeature > 0) {
            auto featureIcon = CCSprite::createWithSpriteFrameName(featureFrame);
            featureIcon->setPosition(diffIcon->getContentSize() / 2 + CCPoint { 0.0f, -5.5f });
            if (hasBetweenDemon) featureIcon->setPositionY(9.5f);
            diffIcon->addChild(featureIcon, -2);
        }
        auto hoverButton = CCMenuItemExt::createSpriteExtra(diffIcon, [this, gameLevel, safeLevel](auto sender) {
            if (m_hoverNode) m_hoverNode->close();
            m_hoverNode = BSHoverNode::create(safeLevel, gameLevel, [this] { m_hoverNode = nullptr; });
            m_hoverNode->setPosition(sender->getPosition() + CCPoint { 0.0f, sender->getContentHeight() / 2 + m_hoverNode->getContentHeight() / 2 + 5.0f });
            m_mainLayer->addChild(m_hoverNode, 200);
        });
        hoverButton->setPosition({ (i + firstWeekday) % 7 * 38.0f + 36.0f, 219.0f - floorf(((float)i + firstWeekday) / 7) * 38.0f });
        hoverButton->setID(fmt::format("level-button-{}", i + 1));
        m_calendarMenu->addChild(hoverButton);
    }
}

BSCalendarPopup::~BSCalendarPopup() {
    CC_SAFE_RELEASE(m_levels);
    CC_SAFE_RELEASE(m_loadingCircle);
    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}
