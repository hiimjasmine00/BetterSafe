#include "BSCalendarPopup.hpp"
#include "BSSelectPopup.hpp"
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
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

    handleTouchPriority(this);
    loadSafe();
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

void BSCalendarPopup::createWeekdayLabel(const char* text, int idx) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setPosition({ idx * 38.0f + 36.0f, 249.0f });
    label->setScale(0.5f);
    label->setID(fmt::format("{}-label", string::toLower(text)));
    m_mainLayer->addChild(label);
}

void BSCalendarPopup::loadSafe(bool refresh) {
    BetterSafe::loadSafe(m_type, &m_listener, [this, refresh] {
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

    auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, ranges::reduce<std::string>(levelSafe,
        [](std::string& str, const SafeLevel& level) { str += (str.empty() ? "" : ",") + fmt::to_string(level.levelID); }));

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
    auto mod = Mod::get();
    if (!mod->getSettingValue<bool>("sunday-first")) firstWeekday = (firstWeekday + 6) % 7;

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_type);
    auto levelMap = ranges::reduce<std::unordered_map<int, GJGameLevel*>>(CCArrayExt<GJGameLevel*>(levels),
        [](std::unordered_map<int, GJGameLevel*>& acc, GJGameLevel* obj) {
            acc[obj->m_levelID.value()] = obj;
        });

    constexpr std::array daysPerMonth = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    auto daysInMonth = m_month <= daysPerMonth.size() ? daysPerMonth[m_month - 1] : 0;
    if (m_month == 2 && (m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0))) daysInMonth++;

    auto showCheckmarks = mod->getSettingValue<bool>("show-checkmarks");
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (int i = 0; i < daysInMonth; i++) {
        auto safeLevelIt = std::ranges::find_if(levelSafe, [this, i = i + 1](const SafeLevel& level) {
            return std::ranges::any_of(level.dates, [this, i](const SafeDate& date) {
                return date.year == m_year && date.month == m_month && date.day == i;
            });
        });
        if (safeLevelIt == levelSafe.end()) continue;

        auto& safeLevel = *safeLevelIt;
        auto gameLevelIt = levelMap.find(safeLevel.levelID);
        if (gameLevelIt == levelMap.end()) continue;

        auto gameLevel = gameLevelIt->second;
        auto levelDifficulty = gameLevel->m_demon > 0 ? gameLevel->m_demonDifficulty > 0 ? gameLevel->m_demonDifficulty + 4 : 6 :
            gameLevel->m_autoLevel ? -1 : gameLevel->m_ratings < 5 ? 0 : gameLevel->m_ratingsSum / gameLevel->m_ratings;
        auto diffFrame = levelDifficulty == -1 ? "diffIcon_auto_btn_001.png" : fmt::format("diffIcon_{:02}_btn_001.png", levelDifficulty);
        auto featureState = gameLevel->m_featured > 0 ? gameLevel->m_isEpic + 1 : 0;

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

        CCPoint featurePosition;
        if (auto demonsInBetween = Loader::get()->getLoadedMod("hiimjustin000.demons_in_between")) {
            if (demonsInBetween->getSettingValue<bool>("enable-difficulties") && safeLevel.tier > 0) {
                constexpr std::array difficulties = {
                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 15,
                    16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
                };
                if (safeLevel.tier < difficulties.size()) {
                    diffFrame = fmt::format("hiimjustin000.demons_in_between/DIB_{:02}_001.png", difficulties[safeLevel.tier]);
                    featurePosition.y = 9.5f;
                }
            }
        }

        auto completedLevel = showCheckmarks && GameStatsManager::get()->hasCompletedLevel(gameLevel);

        auto diffIcon = CCSprite::createWithSpriteFrameName(diffFrame.c_str());
        diffIcon->setScale(0.75f);
        diffIcon->setOpacity(255 - (completedLevel * 105));

        if (featurePosition.y > 0.0f) featurePosition.x = diffIcon->getContentWidth() / 2.0f;
        else featurePosition = diffIcon->getContentSize() / 2.0f - CCPoint { 0.0f, 5.5f };

        if (featureState > 0) {
            CCSprite* featureIcon = nullptr;
            if (Loader::get()->isModLoaded("uproxide.animated_fire") && featureState > 1) {
                constexpr std::array prefixes = {
                    "uproxide.animated_fire/EpicFrame_",
                    "uproxide.animated_fire/LegendaryFrame_",
                    "uproxide.animated_fire/MythicFrame_"
                };
                auto prefix = prefixes[featureState - 2];
                auto spriteFrames = CCArray::create();
                for (int i = 1; i < 9; i++) {
                    spriteFrames->addObject(spriteFrameCache->spriteFrameByName(fmt::format("{}{:02}.png", prefix, i).c_str()));
                }
                featureIcon = CCSprite::createWithSpriteFrame(static_cast<CCSpriteFrame*>(spriteFrames->objectAtIndex(0)));
                featureIcon->runAction(CCRepeatForever::create(CCAnimate::create(CCAnimation::createWithSpriteFrames(spriteFrames, 1.0f / 12.0f))));
                featurePosition.y += 15.875f + (featureState == 2);
            }
            else {
                constexpr std::array frames = {
                    "GJ_featuredCoin_001.png", "GJ_epicCoin_001.png", "GJ_epicCoin2_001.png", "GJ_epicCoin3_001.png"
                };
                featureIcon = CCSprite::createWithSpriteFrameName(featureState <= frames.size() ? frames[featureState - 1] : "");
            }
            if (featureIcon) {
                featureIcon->setPosition(featurePosition);
                featureIcon->setOpacity(255 - (completedLevel * 105));
                diffIcon->addChild(featureIcon, -2);
            }
        }

        if (completedLevel) {
            auto completedIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            completedIcon->setPosition(diffIcon->getContentSize() / 2.0f);
            diffIcon->addChild(completedIcon, 1);
        }
        else {
            auto dayLabel = CCLabelBMFont::create(fmt::to_string(i + 1).c_str(), "bigFont.fnt");
            dayLabel->setScale(0.4f);
            dayLabel->setPosition({ diffIcon->getContentWidth() + 3.0f, -3.0f });
            dayLabel->setAnchorPoint({ 1.0f, 0.0f });
            diffIcon->addChild(dayLabel, 1);
        }

        auto hoverButton = CCMenuItemExt::createSpriteExtra(diffIcon, [this, i, gameLevel, safeLevel](CCMenuItemSpriteExtra* sender) {
            if (m_hoverNode) {
                auto [year, month, day] = m_currentDay;
                m_hoverNode->close();
                if (year == m_year && month == m_month && i + 1 == day) return;
            }
            createHoverNode(sender, safeLevel, gameLevel);
            m_currentDay.year = m_year;
            m_currentDay.month = m_month;
            m_currentDay.day = i + 1;
        });
        hoverButton->setPosition({ (i + firstWeekday) % 7 * 38.0f + 36.0f, 219.0f - (int)((i + firstWeekday) / 7) * 38.0f });
        hoverButton->setID(fmt::format("level-button-{}", i + 1));
        m_calendarMenu->addChild(hoverButton);

        if (m_year == m_currentDay.year && m_month == m_currentDay.month && i + 1 == m_currentDay.day)
            createHoverNode(hoverButton, safeLevel, gameLevel);

        if (completedLevel || featureState < 0 || featureState > 4) continue;

        auto RP = Loader::get()->getLoadedMod("sawblade.rating_particles");
        if (RP) {
            constexpr std::array toggles = {
                "show-rate", "show-featured", "show-epic", "show-legendary", "show-mythic"
            };
            if (!RP->getSettingValue<bool>(toggles[featureState])) continue;
        }
        else if (featureState != 4) continue;

        constexpr std::array colors = {
            "rate-color", "featured-color", "epic-color", "legendary-color", "mythic-color"
        };
        auto color = colors[featureState];

        auto particles = CCParticleSystemQuad::createWithTotalParticles(RP ? RP->getSettingValue<int64_t>("amount") : 30, false);
        particles->setDuration(-1.0f);
        particles->setLife(1.3f);
        particles->setLifeVar(0.2f);
        particles->setEmissionRate(RP ? RP->getSettingValue<int64_t>("amount") : 20.0f);
        particles->setAngle(90.0f);
        particles->setAngleVar(0.0f);
        particles->setSpeed(RP ? RP->getSettingValue<int64_t>("speed") : 10.0f);
        particles->setSpeedVar(5.0f);
        particles->setPosVar({ 20.0f, 20.0f });
        particles->setGravity({ 0.0f, 0.0f });
        particles->setRadialAccel(8.0f);
        particles->setRadialAccelVar(0.0f);
        particles->setTangentialAccel(0.0f);
        particles->setTangentialAccelVar(0.0f);
        particles->setStartSize(RP ? RP->getSettingValue<int64_t>("size") : 20.0f);
        particles->setStartSizeVar(1.0f);
        particles->setStartSpin(0.0f);
        particles->setStartSpinVar(0.0f);
        particles->setStartColor(to4F(to4B(RP ? RP->getSettingValue<ccColor3B>(color) : ccColor3B { 0, 255, 255 })));
        particles->setStartColorVar({ 0.0f, 0.0f, 0.0f, 0.0f });
        particles->setEndSize(1.0f);
        particles->setEndSizeVar(1.0f);
        particles->setEndSpin(0.0f);
        particles->setEndSpinVar(0.0f);
        particles->setEndColor(to4F(to4B(RP ? lighten3B(RP->getSettingValue<ccColor3B>(color), 30) : ccColor3B { 0, 200, 255 })));
        particles->setEndColorVar({ 0.0f, 0.0f, 0.0f, 0.0f });
        particles->m_fFadeInTime = 0.27f;
        particles->m_fFadeInTimeVar = 0.0f;
        particles->m_fFadeOutTime = 0.27f;
        particles->m_fFadeOutTimeVar = 0.0f;
        particles->setStartRadius(0.0f);
        particles->setStartRadiusVar(0.0f);
        particles->setEndRadius(0.0f);
        particles->setEndRadiusVar(0.0f);
        particles->setRotatePerSecond(0.0f);
        particles->setRotatePerSecondVar(0.0f);
        particles->setEmitterMode(kCCParticleModeGravity);
        particles->setPositionType(kCCPositionTypeGrouped);
        particles->setBlendAdditive(RP ? RP->getSettingValue<bool>("blending") : true);
        particles->m_bStartSpinEqualToEnd = false;
        particles->setRotationIsDir(false);
        particles->m_bDynamicRotationIsDir = false;
        particles->m_uParticleIdx = RP ? RP->getSettingValue<int64_t>("texture") : 0;
        particles->setDisplayFrame(spriteFrameCache->spriteFrameByName(fmt::format("particle_{:02}_001.png", particles->m_uParticleIdx).c_str()));
        particles->m_bUseUniformColorMode = false;
        particles->m_fFrictionPos = 0.0f;
        particles->m_fFrictionPosVar = 0.25f;
        particles->m_fRespawn = 0.0f;
        particles->m_fRespawnVar = 0.0f;
        particles->m_bOrderSensitive = false;
        particles->m_bStartSizeEqualToEnd = false;
        particles->m_bStartRadiusEqualToEnd = false;
        particles->m_bStartRGBVarSync = false;
        particles->m_bEndRGBVarSync = false;
        particles->m_fFrictionSize = 0.0f;
        particles->m_fFrictionSizeVar = 0.0f;
        particles->m_fFrictionRot = 0.0f;
        particles->m_fFrictionRotVar = 0.0f;
        particles->setPosition(featurePosition);
        particles->setScale(0.9f);
        particles->resetSystem();
        diffIcon->addChild(particles, -1);
        particles->update(0.15f);
        particles->update(0.15f);
        particles->update(0.15f);
        particles->update(0.15f);
        particles->update(0.15f);
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
