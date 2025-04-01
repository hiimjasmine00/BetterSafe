#include "BetterSafe.hpp"
#include <Geode/utils/ranges.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

#define DAILY_SAFE_URL "https://raw.githubusercontent.com/hiimjasmine00/the-safe/master/v2/daily.json"
#define WEEKLY_SAFE_URL "https://raw.githubusercontent.com/hiimjasmine00/the-safe/master/v2/weekly.json"
#define EVENT_SAFE_URL "https://raw.githubusercontent.com/hiimjasmine00/the-safe/master/v2/event.json"

void BetterSafe::loadSafe(GJTimedLevelType type, EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    if (!getSafeLevels(type).empty()) return success();

    listener->bind([failure = std::move(failure), success = std::move(success), type](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            auto json = res->json().unwrapOr(std::vector<matjson::Value>());
            if (!json.isArray()) return success();

            getSafeLevels(type) = ranges::map<std::vector<SafeLevel>>(json.asArray().unwrap(), [type](const matjson::Value& level) {
                return SafeLevel {
                    .id = (int)level["id"].asInt().unwrapOr(0),
                    .timelyID = (int)level["timelyID"].asInt().unwrapOr(0),
                    .dates = level.contains("dates") && level["dates"].isArray() ? ranges::map<std::vector<SafeDate>>(level["dates"].asArray().unwrap(),
                        [](const matjson::Value& date) {
                            auto parts = string::split(date.asString().unwrapOr("1970-01-01"), "-");
                            return SafeDate {
                                .year = (uint16_t)(parts.size() > 0 ? numFromString<int>(parts[0]).unwrapOr(1970) : 1970),
                                .month = (uint8_t)(parts.size() > 1 ? numFromString<int>(parts[1]).unwrapOr(1) : 1),
                                .day = (uint8_t)(parts.size() > 2 ? numFromString<int>(parts[2]).unwrapOr(1) : 1)
                            };
                        }) : std::vector<SafeDate>(),
                    .type = type,
                    .tier = (int)level["tier"].asInt().unwrapOr(0)
                };
            });

            if (type != GJTimedLevelType::Event || EVENT_SAFE.size() < 2) return success();

            auto currentTime = time(0);
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            auto currentTimeinfo = std::localtime(&currentTime);
            #pragma clang diagnostic pop
            currentTimeinfo->tm_hour = 0;
            currentTimeinfo->tm_min = 0;
            currentTimeinfo->tm_sec = 0;
            auto& [lastYear, lastMonth, lastDay] = EVENT_SAFE[1].dates.back();
            tm timeinfo = { 0, 0, 0, lastDay, lastMonth - 1, lastYear - 1900, 0, 0, currentTimeinfo->tm_isdst };
            auto truncatedTime = mktime(currentTimeinfo);
            for (auto lastTime = mktime(&timeinfo) + 86400; lastTime <= truncatedTime; lastTime += 86400) {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Wdeprecated-declarations"
                auto timeinfo = std::localtime(&lastTime);
                #pragma clang diagnostic pop
                EVENT_SAFE[0].dates.push_back({
                    .year = (uint16_t)(timeinfo->tm_year + 1900),
                    .month = (uint8_t)(timeinfo->tm_mon + 1),
                    .day = (uint8_t)timeinfo->tm_mday
                });
            }

            success();
        }
    });

    auto url = "";
    switch (type) {
        case GJTimedLevelType::Daily: url = DAILY_SAFE_URL; break;
        case GJTimedLevelType::Weekly: url = WEEKLY_SAFE_URL; break;
        case GJTimedLevelType::Event: url = EVENT_SAFE_URL; break;
        default: return;
    }
    listener->setFilter(web::WebRequest().get(url));
}

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    using SafeLevels = std::vector<SafeLevel>;

    return ranges::reduce<SafeLevels>(getSafeLevels(type), [year, month](SafeLevels& levels, const SafeLevel& level) {
        ranges::push(levels, ranges::reduce<SafeLevels>(level.dates, [&level, year, month](SafeLevels& dates, const SafeDate& date) {
            if (date.year == year && date.month == month) dates.push_back(level);
        }));
    });
}

std::vector<SafeLevel>& BetterSafe::getSafeLevels(GJTimedLevelType type) {
    switch (type) {
        case GJTimedLevelType::Daily: return DAILY_SAFE;
        case GJTimedLevelType::Weekly: return WEEKLY_SAFE;
        case GJTimedLevelType::Event: return EVENT_SAFE;
        default: {
            static std::vector<SafeLevel> empty;
            return empty;
        }
    }
}
