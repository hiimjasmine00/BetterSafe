#include "BetterSafe.hpp"
#include <Geode/loader/ModEvent.hpp>
#include <jasmine/web.hpp>

using namespace geode::prelude;

constexpr std::array<std::string_view, 3> urls = {
    "https://the-safe.hiimjasmine00.com/daily",
    "https://the-safe.hiimjasmine00.com/weekly",
    "https://the-safe.hiimjasmine00.com/event"
};

std::map<GJTimedLevelType, std::vector<SafeLevel>> BetterSafe::safes = {
    { GJTimedLevelType::Daily, {} },
    { GJTimedLevelType::Weekly, {} },
    { GJTimedLevelType::Event, {} }
};

std::map<int, LadderDemon> BetterSafe::gddl = {};

$on_mod(Loaded) {
    DemonsInBetweenLoadedEvent().listen([](const std::map<int, LadderDemon>& gddl) {
        BetterSafe::gddl = gddl;
    }).leak();
}

time_t toTimestamp(std::string date) {
    auto parts = string::split(date, '-');
    if (parts.size() != 3) return 0;

    auto yearRes = numFromString<uint16_t>(parts[0]);
    auto monthRes = numFromString<uint8_t>(parts[1]);
    auto dayRes = numFromString<uint8_t>(parts[2]);
    if (!yearRes.isOk() || !monthRes.isOk() || !dayRes.isOk()) return 0;

    auto year = yearRes.unwrap();
    auto month = monthRes.unwrap();
    auto day = dayRes.unwrap();

    tm timeinfo = {};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;

    #ifdef GEODE_IS_WINDOWS
    return _mkgmtime(&timeinfo);
    #else
    return timegm(&timeinfo);
    #endif
}

void BetterSafe::loadSafe(GJTimedLevelType type, TaskHolder<web::WebResponse>& listener, Function<void()> success, Function<void(int)> failure) {
    if (!safes[type].empty()) return success();

    listener.spawn(
        web::WebRequest().get(std::string(urls[(int)type])),
        [failure = std::move(failure), success = std::move(success), type](web::WebResponse res) mutable {
            if (!res.ok()) return failure(res.code());

            auto& safe = safes[type];
            for (auto& value : jasmine::web::getArray(res)) {
                auto id = value.get<int>("id");
                if (!id.isOk()) continue;

                auto levelID = value.get<int>("levelID");
                if (!levelID.isOk()) continue;

                auto startDateRes = value.get("startDate");
                if (!startDateRes.isOk()) continue;

                auto endDateRes = value.get("endDate");
                if (!endDateRes.isOk()) continue;

                auto startDate = startDateRes.unwrap();
                auto endDate = endDateRes.unwrap();

                std::vector<SafeDate> dates;
                if (startDate.isString() && endDate.isString()) {
                    auto startTimestamp = toTimestamp(startDate.asString().unwrap());
                    auto endTimestamp = toTimestamp(endDate.asString().unwrap());

                    if (startTimestamp == 0 || endTimestamp == 0) continue;

                    for (auto timestamp = startTimestamp; timestamp <= endTimestamp; timestamp += 86400) {
                        tm timeinfo = {};
                        #ifdef GEODE_IS_WINDOWS
                        gmtime_s(&timeinfo, &timestamp);
                        #else
                        gmtime_r(&timestamp, &timeinfo);
                        #endif

                        dates.emplace_back((uint16_t)(timeinfo.tm_year + 1900), (uint8_t)(timeinfo.tm_mon + 1), (uint8_t)(timeinfo.tm_mday));
                    }
                }

                safe.emplace_back(id.unwrap(), levelID.unwrap(), std::move(dates), type);
            }

            success();
        }
    );
}

int BetterSafe::getDays(int year, int month) {
    if (month < 1 || month > 12) return 0;

    constexpr std::array daysPerMonth = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    auto daysInMonth = month <= daysPerMonth.size() ? daysPerMonth[month - 1] : 0;
    if (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) daysInMonth++;

    return daysInMonth;
}

std::vector<SafeLevel*> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    std::vector<SafeLevel*> result;
    auto days = getDays(year, month);
    auto& safe = safes[type];
    for (int i = 0; i < days; i++) {
        auto it = std::ranges::find_if(safe, [year, month, d = i + 1](const SafeLevel& level) {
            return std::ranges::any_of(level.dates, [year, month, d](const SafeDate& date) {
                return date.year == year && date.month == month && date.day == d;
            });
        });
        result.push_back(it != safe.end() ? &*it : nullptr);
    }
    return result;
}
