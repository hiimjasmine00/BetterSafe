#include <Geode/ui/Popup.hpp>

class BSCalendarPopup;

using SelectMonthCallback = std::function<void(int, int)>;

class BSSelectPopup : public geode::Popup<BSCalendarPopup*, SelectMonthCallback> {
protected:
    int m_year;
    int m_month;

    bool setup(BSCalendarPopup*, SelectMonthCallback) override;
public:
    inline static constexpr std::array months = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    static BSSelectPopup* create(BSCalendarPopup*, SelectMonthCallback);
};
