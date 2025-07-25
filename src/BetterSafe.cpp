#include "BetterSafe.hpp"
#include <Geode/utils/ranges.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

#define DAILY_SAFE_URL "https://the-safe.hiimjasmine00.com/daily"
#define WEEKLY_SAFE_URL "https://the-safe.hiimjasmine00.com/weekly"
#define EVENT_SAFE_URL "https://the-safe.hiimjasmine00.com/event"

void BetterSafe::loadSafe(
    GJTimedLevelType type,
    EventListener<web::WebTask>* listener,
    std::function<void()> success,
    std::function<void(int)> failure
) {
    if (!safes[type].empty()) return success();

    listener->bind([failure = std::move(failure), success = std::move(success), type](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            safes[type] = ranges::map<std::vector<SafeLevel>>(
                res->json().unwrapOrDefault().asArray().unwrapOrDefault(),
                [type](const matjson::Value& v) {
                    SafeLevel level;
                    GEODE_UNWRAP_INTO_IF_OK(level.id, v.get("id").andThen([](const matjson::Value& v) { return v.as<int>(); }));
                    GEODE_UNWRAP_INTO_IF_OK(level.levelID, v.get("levelID").andThen([](const matjson::Value& v) { return v.as<int>(); }));
                    GEODE_UNWRAP_INTO_IF_OK(level.dates, v.get("dates").andThen([](const matjson::Value& v) {
                        return v.asArray().map([](const std::vector<matjson::Value>& vec) {
                            return ranges::map<std::vector<SafeDate>>(vec, [](const matjson::Value& d) {
                                SafeDate date;
                                if (!d.isString()) return date;
                                auto parts = string::split(d.asString().unwrap(), "-");
                                if (parts.size() < 1) return date;
                                GEODE_UNWRAP_INTO_IF_OK(date.year, numFromString<int>(parts[0]));
                                if (parts.size() < 2) return date;
                                GEODE_UNWRAP_INTO_IF_OK(date.month, numFromString<int>(parts[1]));
                                if (parts.size() < 3) return date;
                                GEODE_UNWRAP_INTO_IF_OK(date.day, numFromString<int>(parts[2]));
                                return date;
                            });
                        });
                    }));
                    level.type = type;
                    GEODE_UNWRAP_INTO_IF_OK(level.tier, v.get("tier").andThen([](const matjson::Value& v) { return v.as<int>(); }));
                    return level;
                }
            );

            success();
        }
    });

    constexpr std::array urls = { DAILY_SAFE_URL, WEEKLY_SAFE_URL, EVENT_SAFE_URL };
    listener->setFilter(web::WebRequest().get(urls[(int)type]));
}

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    using SafeLevels = std::vector<SafeLevel>;

    return ranges::reduce<SafeLevels>(safes[type], [year, month](SafeLevels& levels, const SafeLevel& level) {
        ranges::push(levels, ranges::reduce<SafeLevels>(level.dates, [&level, year, month](SafeLevels& dates, const SafeDate& date) {
            if (date.year == year && date.month == month) dates.push_back(level);
        }));
    });
}
