#include "../utils.hpp"
#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/CCLayer.hpp>

class SubZeroSelectLayer : CCLayer, CCNode {};

class MeltdownSelectLayer : CCLayer, CCNode {};


class $modify(CCLayer) {
    bool init() {
        if (!CCLayer::init()) return false;


        if (typeinfo_cast<SubZeroSelectLayer*>(this) || typeinfo_cast<MeltdownSelectLayer*>(this)) {

            queueInMainThread([=] {
            
                if (auto ground = typeinfo_cast<GJGroundLayer*>(this->getChildByID("GJGroundLayer"))) {
                    ground->setPositionY(35);
                }
            });
            

        }
        return true;
    }
};
