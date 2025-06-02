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
    if (!getSafeLevels(type).empty()) return success();

    listener->bind([failure = std::move(failure), success = std::move(success), type](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            auto json = res->json().unwrapOr(matjson::Value::array());
            if (!json.isArray()) return success();

            getSafeLevels(type) = ranges::map<std::vector<SafeLevel>>(json.asArray().unwrap(), [type](const matjson::Value& level) {
                return SafeLevel {
                    .id = (int)level["id"].asInt().unwrapOr(0),
                    .levelID = (int)level["levelID"].asInt().unwrapOr(0),
                    .dates = level.contains("dates") && level["dates"].isArray() ? ranges::map<std::vector<SafeDate>>(
                        level["dates"].asArray().unwrap(),
                        [](const matjson::Value& date) {
                            auto parts = string::split(date.asString().unwrapOr("1970-01-01"), "-");
                            return SafeDate {
                                .year = (uint16_t)(parts.size() > 0 ? numFromString<int>(parts[0]).unwrapOr(1970) : 1970),
                                .month = (uint8_t)(parts.size() > 1 ? numFromString<int>(parts[1]).unwrapOr(1) : 1),
                                .day = (uint8_t)(parts.size() > 2 ? numFromString<int>(parts[2]).unwrapOr(1) : 1)
                            };
                        }
                    ) : std::vector<SafeDate>(),
                    .type = type,
                    .tier = (int)level["tier"].asInt().unwrapOr(0)
                };
            });

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
