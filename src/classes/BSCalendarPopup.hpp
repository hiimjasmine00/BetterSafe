#include "BSHoverNode.hpp"
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/ui/Popup.hpp>

class BSCalendarPopup : public geode::Popup<cocos2d::CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType>, public LevelManagerDelegate {
protected:
    geode::EventListener<geode::utils::web::WebTask> m_listener;
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

    bool setup(CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType) override;

    void closeHoverNode();
    void createHoverNode(CCMenuItemSpriteExtra*, const SafeLevel&, GJGameLevel*);
    void loadMonth(int, int, bool = false);
    void loadSafe(bool = false);
    void setupMonth(cocos2d::CCArray*);
public:
    static BSCalendarPopup* create(CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType);

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
