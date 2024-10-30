#include "../utils.hpp"
#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>

class $modify(PlayLayer) {
    bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        if (Utils::isPractice()) {
            Utils::setPractice(false);
            togglePracticeMode(true);
        }
        return true;
    }
};
