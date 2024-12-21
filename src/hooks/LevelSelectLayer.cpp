#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/LevelSelectLayer.hpp>

class $modify(LevelSelectLayer) {
    bool init(int page) {
        if (!LevelSelectLayer::init(page)) return false;
        if (!Mod::get()->getSettingValue<bool>("enabled")) return true;
        /*int iVar19;
        int iVar20 = 1;
        auto levelsArray = CCArray::create();
        do {
            auto GLM = GameLevelManager::sharedState();
            iVar19 = iVar20 + 1;
            auto level = GLM->getMainLevel(iVar20,true);
            levelsArray->addObject(level);
            iVar20 = iVar19;
        } while (iVar19 != 23);*/

       
        m_groundLayer->setPositionY(35);
        return true;
    }
};


