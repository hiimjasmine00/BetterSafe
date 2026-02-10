#include "BetterSafe.hpp"
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

                auto dates = value.get<std::vector<matjson::Value>>("dates").map([](const std::vector<matjson::Value>& v) {
                    std::vector<SafeDate> dates;
                    for (auto& value : v) {
                        auto& date = dates.emplace_back();
                        if (auto str = value.asString()) {
                            auto split = string::splitView(str.unwrap(), "-");
                            if (split.size() >= 3) {
                                if (auto year = numFromString<int>(split[0])) {
                                    date.year = year.unwrap();
                                }
                                if (auto month = numFromString<int>(split[1])) {
                                    date.month = month.unwrap();
                                }
                                if (auto day = numFromString<int>(split[2])) {
                                    date.day = day.unwrap();
                                }
                            }
                        }
                    }
                    return dates;
                });
                if (!dates.isOk()) continue;

                auto tier = value.get<int>("tier");
                if (!tier.isOk()) continue;

                safe.emplace_back(id.unwrap(), levelID.unwrap(), dates.unwrap(), type, tier.unwrap());
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
