#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/utils/web.hpp>
#include <hiimjustin000.demons_in_between/DemonsInBetweenAPI.hpp>

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
};

namespace BetterSafe {
    extern std::map<GJTimedLevelType, std::vector<SafeLevel>> safes;
    extern std::map<int, LadderDemon> gddl;

    void loadSafe(
        GJTimedLevelType type, geode::async::TaskHolder<geode::utils::web::WebResponse>& listener,
        geode::Function<void()> success, geode::Function<void(int)> failure
    );
    int getDays(int year, int month);
    std::vector<SafeLevel*> getMonth(int year, int month, GJTimedLevelType type);
}
