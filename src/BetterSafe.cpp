#include "BetterSafe.hpp"
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

constexpr std::array urls = {
    "https://the-safe.hiimjasmine00.com/daily",
    "https://the-safe.hiimjasmine00.com/weekly",
    "https://the-safe.hiimjasmine00.com/event"
};

std::map<GJTimedLevelType, std::vector<SafeLevel>> BetterSafe::safes = {
    { GJTimedLevelType::Daily, {} },
    { GJTimedLevelType::Weekly, {} },
    { GJTimedLevelType::Event, {} }
};

void BetterSafe::loadSafe(
    GJTimedLevelType type,
    EventListener<web::WebTask>& listener,
    std::function<void()> success,
    std::function<void(int)> failure
) {
    if (!safes[type].empty()) return success();

    listener.bind([failure = std::move(failure), success = std::move(success), type](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            auto json = res->json().andThen([](matjson::Value&& v) {
                return std::move(v).asArray();
            });
            if (!json.isOk()) return success();

            auto& safe = safes[type];
            for (auto& value : json.unwrap()) {
                auto id = value.get<int>("id");
                if (!id.isOk()) continue;

                auto levelID = value.get<int>("levelID");
                if (!levelID.isOk()) continue;

                auto dates = value.get<std::vector<matjson::Value>>("dates").map([](const std::vector<matjson::Value>& v) {
                    std::vector<SafeDate> dates;
                    for (auto& value : v) {
                        auto& date = dates.emplace_back();
                        auto str = value.asString();
                        if (!str.isOk()) continue;

                        auto split = string::split(str.unwrap(), "-");
                        if (split.size() < 3) continue;

                        auto& year = split[0];
                        std::from_chars(year.data(), year.data() + year.size(), date.year);
                        auto& month = split[1];
                        std::from_chars(month.data(), month.data() + month.size(), date.month);
                        auto& day = split[2];
                        std::from_chars(day.data(), day.data() + day.size(), date.day);
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
    });

    listener.setFilter(web::WebRequest().get(urls[(int)type]));
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
