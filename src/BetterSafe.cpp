#include "BetterSafe.hpp"
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

            auto& safe = safes[type];
            for (auto& value : res->json().unwrapOrDefault()) {
                auto& level = safe.emplace_back();
                if (!value.isObject()) continue;
                GEODE_UNWRAP_INTO_IF_OK(level.id, value.get("id").andThen([](const matjson::Value& v) { return v.as<int>(); }));
                GEODE_UNWRAP_INTO_IF_OK(level.levelID, value.get("levelID").andThen([](const matjson::Value& v) { return v.as<int>(); }));
                GEODE_UNWRAP_INTO_IF_OK(level.dates, value.get("dates").map([](const matjson::Value& v) {
                    std::vector<SafeDate> dates;
                    for (auto& value : v) {
                        auto& date = dates.emplace_back(0, 0, 0);
                        auto i = 0;
                        for (auto& c : value.asString().unwrapOrDefault()) {
                            if (c >= '0' && c <= '9') {
                                switch (i) {
                                    case 0:
                                        date.year *= 10;
                                        date.year += c - '0';
                                        break;
                                    case 1:
                                        date.month *= 10;
                                        date.month += c - '0';
                                        break;
                                    case 2:
                                        date.day *= 10;
                                        date.day += c - '0';
                                        break;
                                }
                            }
                            else if (c == '-') i++;
                        }
                        if (date.year == 0 && date.month == 0 && date.day == 0) {
                            date.year = 1970;
                            date.month = 1;
                            date.day = 1;
                        }
                    }
                    return dates;
                }));
                level.type = type;
                GEODE_UNWRAP_INTO_IF_OK(level.tier, value.get("tier").andThen([](const matjson::Value& v) { return v.as<int>(); }));
            }

            success();
        }
    });

    constexpr std::array urls = { DAILY_SAFE_URL, WEEKLY_SAFE_URL, EVENT_SAFE_URL };
    listener->setFilter(web::WebRequest().get(urls[(int)type]));
}

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    std::vector<SafeLevel> result;
    for (auto& level : safes[type]) {
        for (auto& date : level.dates) {
            if (date.year == year && date.month == month) result.push_back(level);
        }
    }
    return result;
}
