#include "BSHoverNode.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/ui/Layout.hpp>

using namespace geode::prelude;

BSHoverNode* BSHoverNode::create(const SafeLevel& level, GJGameLevel* gameLevel, HoverCallback callback) {
    auto ret = new BSHoverNode();
    if (ret->init(level, gameLevel, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSHoverNode::init(const SafeLevel& level, GJGameLevel* gameLevel, HoverCallback callback) {
    if (!CCLayer::init()) return false;

    setAnchorPoint({ 0.5f, 0.5f });
    setContentSize({ 80.0f, 50.0f });
    setID("BSHoverNode");
    ignoreAnchorPointForPosition(false);

    CCTouchDispatcher::get()->registerForcePrio(this, 2);

    m_callback = std::move(callback);

    auto background = CCScale9Sprite::create("square02_001.png");
    background->setContentSize({ 80.0f, 50.0f });
    background->setPosition({ 40.0f, 25.0f });
    background->setColor({ 0, 0, 0 });
    background->setOpacity(150);
    background->setID("background");
    addChild(background);

    auto timedName = "";
    switch (level.type) {
        case GJTimedLevelType::Daily: timedName = "Daily"; break;
        case GJTimedLevelType::Weekly: timedName = "Weekly"; break;
        case GJTimedLevelType::Event: timedName = "Event"; break;
        default: break;
    }
    auto dailyLabel = CCLabelBMFont::create(fmt::format("{} #{}", timedName, level.timelyID).c_str(), "goldFont.fnt");
    dailyLabel->setPosition({ 40.0f, 45.0f });
    dailyLabel->setScale(0.3f);
    dailyLabel->setID("daily-label");
    addChild(dailyLabel);

    auto viewMenu = CCMenu::create();
    viewMenu->setPosition({ getContentWidth() / 2.0f, 0.0f });
    viewMenu->setID("view-menu");
    addChild(viewMenu);

    auto closeButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_closeBtn_001.png", 0.5f, [this](auto) { close(); });
    closeButton->setPosition({ -40.0f, 50.0f });
    closeButton->setID("close-button");
    viewMenu->addChild(closeButton, 1);

    auto nameLabel = CCLabelBMFont::create(gameLevel->m_levelName.c_str(), "bigFont.fnt");
    nameLabel->setScale(0.5f);
    auto nameButton = CCMenuItemExt::createSpriteExtra(nameLabel, [this, gameLevel](auto) {
        if (CCScene::get()->getHighestChildZ() <= getParent()->getParent()->getZOrder()) GameLevelManager::get()->gotoLevelPage(gameLevel);
    });
    nameButton->setPosition({ 0.0f, 35.0f });
    nameButton->setID("name-button");
    viewMenu->addChild(nameButton);

    auto creatorLabel = CCLabelBMFont::create(fmt::format("by {}", GEODE_ANDROID(std::string)(gameLevel->m_creatorName)).c_str(), "goldFont.fnt");
    creatorLabel->setScale(0.4f);
    auto creatorButton = CCMenuItemExt::createSpriteExtra(creatorLabel, [this, gameLevel](auto) {
        auto accountID = gameLevel->m_accountID.value();
        if (CCScene::get()->getHighestChildZ() <= getParent()->getParent()->getZOrder() && accountID > 0) ProfilePage::create(accountID, false)->show();
    });
    creatorButton->setPosition({ 0.0f, 23.0f });
    creatorButton->setID("creator-button");
    viewMenu->addChild(creatorButton);

    auto contentWidth = std::max({ nameLabel->getScaledContentWidth() + 6.0f, creatorLabel->getScaledContentWidth() + 6.0f, 80.0f });
    setContentWidth(contentWidth);
    background->setContentWidth(contentWidth);
    background->setPositionX(contentWidth / 2.0f);
    dailyLabel->setPositionX(contentWidth / 2.0f);
    viewMenu->setPositionX(contentWidth / 2.0f);
    closeButton->setPositionX(-contentWidth / 2.0f);

    auto starLayout = CCNode::create();
    starLayout->setPosition({ contentWidth / 2.0f, 10.0f });
    starLayout->setContentSize({ 80.0f, 15.0f });
    starLayout->setAnchorPoint({ 0.5f, 0.5f });
    starLayout->setLayout(RowLayout::create()->setGap(1.75f)->setAutoScale(false));
    starLayout->setID("star-layout");
    addChild(starLayout);

    auto gsm = GameStatsManager::get();

    auto starsLabel = CCLabelBMFont::create(std::to_string(gameLevel->m_stars.value()).c_str(), "bigFont.fnt");
    starsLabel->setScale(0.4f);
    starsLabel->setColor({ 255, 255, (uint8_t)(255 - gsm->hasCompletedLevel(gameLevel) * 205) });
    starsLabel->setID("stars-label");
    starLayout->addChild(starsLabel);

    auto starSprite = CCSprite::createWithSpriteFrameName(gameLevel->m_levelLength == 5 ? "moon_small01_001.png" : "star_small01_001.png");
    starSprite->setID("star-sprite");
    starLayout->addChild(starSprite);

    auto levelID = gameLevel->m_levelID.value();
    auto coinsVerified = gameLevel->m_coinsVerified.value() > 0;
    for (int i = 1; i <= gameLevel->m_coins; i++) {
        auto coinStr = fmt::format("{}_{}", levelID, i);
        auto hasCoin = gsm->hasUserCoin(coinStr.c_str()) || gsm->hasPendingUserCoin(coinStr.c_str());
        auto coinSprite = CCSprite::createWithSpriteFrameName("usercoin_small01_001.png");
        coinSprite->setColor({
            (uint8_t)(165 + hasCoin * 90),
            (uint8_t)(113 + coinsVerified * 52 + hasCoin * (62 + coinsVerified * 28)),
            (uint8_t)(48 + coinsVerified * 117 + hasCoin * (17 + coinsVerified * 73))
        });
        coinSprite->setID(fmt::format("coin-sprite-{}", i).c_str());
        starLayout->addChild(coinSprite);
    }

    starLayout->updateLayout();

    setTouchEnabled(true);
    setKeypadEnabled(true);
    handleTouchPriority(this);

    return true;
}

void BSHoverNode::close() {
    setTouchEnabled(false);
    setKeypadEnabled(false);
    removeFromParent();
    m_callback();
}

void BSHoverNode::keyBackClicked() {
    close();
}

void BSHoverNode::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
}

bool BSHoverNode::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    auto position = convertToWorldSpace({ 0.0f, 0.0f });
    auto& size = getContentSize();
    auto touchPosition = touch->getLocation();
    return position.x >= touchPosition.x && position.x + size.width <= touchPosition.x
        && position.y >= touchPosition.y && position.y + size.height <= touchPosition.y;
}

BSHoverNode::~BSHoverNode() {
    CCTouchDispatcher::get()->unregisterForcePrio(this);
}
