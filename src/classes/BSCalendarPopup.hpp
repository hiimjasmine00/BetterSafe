#include "BSHoverNode.hpp"
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/ui/Popup.hpp>

class BSCalendarPopup : public geode::Popup, public LevelManagerDelegate {
protected:
    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    std::vector<std::pair<SafeLevel*, GJGameLevel*>> m_levels;
    GJTimedLevelType m_type;
    int m_year;
    int m_minYear;
    int m_maxYear;
    int m_month;
    int m_minMonth;
    int m_maxMonth;
    cocos2d::CCMenu* m_calendarMenu;
    cocos2d::CCLabelBMFont* m_monthLabel;
    CCMenuItemSpriteExtra* m_monthButton;
    BSHoverNode* m_hoverNode;
    LoadingCircle* m_loadingCircle;
    CCMenuItemSpriteExtra* m_prevButton;
    CCMenuItemSpriteExtra* m_nextButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;
    SafeDate m_currentDay;
    bool m_initialized;

    bool init(cocos2d::CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType);
    void onPrevMonth(cocos2d::CCObject*);
    void onNextMonth(cocos2d::CCObject*);
    void onFirstMonth(cocos2d::CCObject*);
    void onLastMonth(cocos2d::CCObject*);
    void onMonth(cocos2d::CCObject*);
    void onRefresh(cocos2d::CCObject*);
    void onLevel(cocos2d::CCObject*);
    void closeHoverNode();
    void createHoverNode(CCMenuItemSpriteExtra*, SafeLevel*, GJGameLevel*);
    void loadMonth(int, int, bool = false);
    void loadSafe(bool = false);
    void setupMonth(cocos2d::CCArray*);
public:
    static BSCalendarPopup* create(cocos2d::CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType);

    GJTimedLevelType getType() const { return m_type; }
    int getYear() const { return m_year; }
    int getMonth() const { return m_month; }
    int getMinYear() const { return m_minYear; }
    int getMaxYear() const { return m_maxYear; }
    int getMinMonth() const { return m_minMonth; }
    int getMaxMonth() const { return m_maxMonth; }

    void onEnter() override;
    void loadLevelsFinished(cocos2d::CCArray*, const char*, int) override;
    void loadLevelsFailed(const char*, int) override;

    ~BSCalendarPopup() override;
};
