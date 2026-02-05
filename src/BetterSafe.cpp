#include "BetterSafe.hpp"
#include <jasmine/convert.hpp>
#include <jasmine/web.hpp>

using namespace geode::prelude;

std::array<std::string, 3> urls = {
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
        web::WebRequest().get(urls[(int)type]),
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
                                jasmine::convert::to(split[0], date.year);
                                jasmine::convert::to(split[1], date.month);
                                jasmine::convert::to(split[2], date.day);
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

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    std::vector<SafeLevel> result;
    for (auto& level : safes[type]) {
        for (auto& date : level.dates) {
            if (date.year == year && date.month == month) result.push_back(level);
        }
    }
    return result;
}
