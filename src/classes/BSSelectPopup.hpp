#include <Geode/ui/Popup.hpp>

using SelectMonthCallback = const std::function<void(int, int)>&;

class BSSelectPopup : public geode::Popup<int, int, int, int, int, int, SelectMonthCallback> {
protected:
    int m_year;
    int m_month;

    bool setup(int, int, int, int, int, int, SelectMonthCallback) override;
public:
    static BSSelectPopup* create(int, int, int, int, int, int, SelectMonthCallback);
};
