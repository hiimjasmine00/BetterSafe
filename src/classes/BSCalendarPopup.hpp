#include "BSHoverNode.hpp"
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/ui/Popup.hpp>

class BSCalendarPopup : public geode::Popup<cocos2d::CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType>, public LevelManagerDelegate {
public:
    inline static std::vector<int> DAYS_IN_MONTH = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    inline static std::vector<std::string> MONTHS = {
        "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
    };
    inline static std::vector<int> DIFFICULTY_INDICES = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 15,
        16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
    };
protected:
    geode::EventListener<geode::utils::web::WebTask> m_listener;
    GJTimedLevelType m_type;
    int m_year;
    int m_firstYear;
    int m_currentYear;
    int m_month;
    int m_firstMonth;
    int m_currentMonth;
    cocos2d::CCArray* m_levels;
    cocos2d::CCMenu* m_calendarMenu;
    cocos2d::CCLabelBMFont* m_monthLabel;
    CCMenuItemSpriteExtra* m_monthButton;
    BSHoverNode* m_hoverNode;
    LoadingCircle* m_loadingCircle;
    CCMenuItemSpriteExtra* m_prevButton;
    CCMenuItemSpriteExtra* m_nextButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;

    bool setup(CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType) override;

    void createWeekdayLabel(const char* text, int idx);
    void loadMonth();
    void setupMonth();
public:
    static BSCalendarPopup* create(CCObject*, cocos2d::SEL_MenuHandler, GJTimedLevelType);

    void loadLevelsFinished(cocos2d::CCArray* levels, const char* key, int) override {
        loadLevelsFinished(levels, key);
    }
    void loadLevelsFinished(cocos2d::CCArray*, const char*) override;
    void loadLevelsFailed(const char* key, int) override {
        loadLevelsFailed(key);
    }
    void loadLevelsFailed(const char*) override;
    void setupPageInfo(gd::string, const char*) override {}

    ~BSCalendarPopup() override;
};
