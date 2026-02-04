#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/utils/web.hpp>

struct SafeDate {
    uint16_t year = 1970;
    uint8_t month = 1;
    uint8_t day = 1;
};

struct SafeLevel {
    int id = 0;
    int levelID = 0;
    std::vector<SafeDate> dates;
    GJTimedLevelType type = GJTimedLevelType::Daily;
    int tier = 0;
};

class BetterSafe {
public:
    static std::map<GJTimedLevelType, std::vector<SafeLevel>> safes;

    static void loadSafe(
        GJTimedLevelType, geode::async::TaskHolder<geode::utils::web::WebResponse>&, geode::Function<void()>, geode::Function<void(int)>
    );
    static std::vector<SafeLevel> getMonth(int year, int month, GJTimedLevelType type);
};
