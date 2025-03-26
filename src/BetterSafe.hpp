#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/utils/web.hpp>

struct SafeDate {
    uint16_t year;
    uint8_t month;
    uint8_t day;
};

struct SafeLevel {
    int id;
    int timelyID;
    std::vector<SafeDate> dates;
    GJTimedLevelType type;
    int tier;
};

class BetterSafe {
public:
    inline static std::vector<SafeLevel> DAILY_SAFE;
    inline static std::vector<SafeLevel> WEEKLY_SAFE;
    inline static std::vector<SafeLevel> EVENT_SAFE;

    static void loadSafe(GJTimedLevelType, geode::EventListener<geode::utils::web::WebTask>*, std::function<void()>, std::function<void(int)>);
    static std::vector<SafeLevel> getMonth(int year, int month, GJTimedLevelType type);
    static std::vector<SafeLevel>& getSafeLevels(GJTimedLevelType type);
};
