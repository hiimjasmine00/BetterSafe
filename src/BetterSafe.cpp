#include "BetterSafe.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/utils/ranges.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

#define DAILY_SAFE_URL "https://raw.githubusercontent.com/hiimjasmine00/the-safe/master/v2/daily.json"
#define WEEKLY_SAFE_URL "https://raw.githubusercontent.com/hiimjasmine00/the-safe/master/v2/weekly.json"
#define EVENT_SAFE_URL "https://raw.githubusercontent.com/hiimjasmine00/the-safe/master/v2/event.json"

SafeDate BetterSafe::parseDate(const std::string& date) {
    auto parts = string::split(date, "-");
    return {
        .year = parts.size() > 0 ? numFromString<int>(parts[0]).unwrapOr(1970) : 1970,
        .month = parts.size() > 1 ? numFromString<int>(parts[1]).unwrapOr(1) : 1,
        .day = parts.size() > 2 ? numFromString<int>(parts[2]).unwrapOr(1) : 1
    };
}

SafeDate BetterSafe::dateFromTime(time_t time) {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    auto timeinfo = std::localtime(&time);
    #pragma clang diagnostic pop
    return { timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday };
}

void BetterSafe::loadSafe(
    GJTimedLevelType type, EventListener<web::WebTask>&& listenerRef, const std::function<void()>& success, const std::function<void(int)>& failure
) {
    if (!getSafeLevels(type).empty()) return success();

    auto&& listener = std::move(listenerRef);

    listener.bind([failure, success, type](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) {
                return failure(res->code());
            }

            auto json = res->json().unwrapOr(std::vector<matjson::Value>());
            if (!json.isArray()) return success();

            getSafeLevels(type) = ranges::map<std::vector<SafeLevel>>(json.asArray().unwrap(), [type](const matjson::Value& level) {
                return SafeLevel {
                    .id = (int)level["id"].asInt().unwrapOr(0),
                    .timelyID = (int)level["timelyID"].asInt().unwrapOr(0),
                    .dates = level.contains("dates") && level["dates"].isArray() ? ranges::map<std::vector<SafeDate>>(level["dates"].asArray().unwrap(),
                        [](const matjson::Value& date) { return parseDate(date.asString().unwrapOr("1970-01-01")); }) : std::vector<SafeDate>(),
                    .type = type,
                    .tier = (int)level["tier"].asInt().unwrapOr(0)
                };
            });

            if (type != GJTimedLevelType::Event || EVENT_SAFE.size() < 2) return success();

            auto lastEventDate = EVENT_SAFE[1].dates.back();
            tm timeinfo = { 0, 0, 0, lastEventDate.day, lastEventDate.month - 1, lastEventDate.year - 1900 };
            auto currentDate = dateFromTime(time(0));
            tm currentTimeinfo = { 0, 0, 0, currentDate.day, currentDate.month - 1, currentDate.year - 1900 };
            auto truncatedTime = mktime(&currentTimeinfo);
            for (auto lastEventTime = mktime(&timeinfo) + 86400; lastEventTime <= truncatedTime; lastEventTime += 86400) {
                EVENT_SAFE[0].dates.push_back(dateFromTime(lastEventTime));
            }

            success();
        }
    });

    switch (type) {
        case GJTimedLevelType::Daily: listener.setFilter(web::WebRequest().get(DAILY_SAFE_URL)); break;
        case GJTimedLevelType::Weekly: listener.setFilter(web::WebRequest().get(WEEKLY_SAFE_URL)); break;
        case GJTimedLevelType::Event: listener.setFilter(web::WebRequest().get(EVENT_SAFE_URL)); break;
        default: break;
    }
}

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    return ranges::reduce<std::vector<SafeLevel>>(getSafeLevels(type), [year, month](std::vector<SafeLevel>& levels, const SafeLevel& level) {
        ranges::push(levels, ranges::map<std::vector<SafeLevel>>(ranges::filter<std::vector<SafeDate>>(level.dates, [year, month](const SafeDate& date) {
            return date.year == year && date.month == month;
        }), [&level](const auto&) { return level; }));
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

int BetterSafe::getDifficultyFromLevel(GJGameLevel* level) {
    if (level->m_demon > 0) return level->m_demonDifficulty > 0 ? level->m_demonDifficulty + 4 : 6;
    else if (level->m_autoLevel) return -1;
    else if (level->m_ratings < 5) return 0;
    else return level->m_ratingsSum / level->m_ratings;
}
