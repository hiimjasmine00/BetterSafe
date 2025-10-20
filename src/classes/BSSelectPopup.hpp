#include <Geode/ui/Popup.hpp>

class BSCalendarPopup;

using SelectMonthCallback = std::function<void(int, int)>;

class BSSelectPopup : public geode::Popup<BSCalendarPopup*, SelectMonthCallback> {
protected:
    SelectMonthCallback m_callback;
    int m_year;
    int m_minMonth;
    int m_minYear;
    int m_maxMonth;
    int m_maxYear;
    geode::Ref<cocos2d::CCArray> m_buttons;
    CCMenuItemSpriteExtra* m_prevButton;
    CCMenuItemSpriteExtra* m_nextButton;

    bool setup(BSCalendarPopup*, SelectMonthCallback) override;
    void page(int);
public:
    static constexpr std::array months = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    static BSSelectPopup* create(BSCalendarPopup*, SelectMonthCallback);
};
