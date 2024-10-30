#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/BoomScrollLayer.hpp>

class $modify(BoomScrollLayer) {
    bool init(cocos2d::CCArray *p0, int p1, bool p2, cocos2d::CCArray *p3, DynamicScrollDelegate *p4) {
        if (!BoomScrollLayer::init(p0,p1,p2,p3,p4)) return false;
        if (!Mod::get()->getSettingValue<bool>("enabled")) return true;
        // this is so dumb
        if ((p0 != nullptr && p0->count() == 3) && (p3 != nullptr && p3->count() > 1) && p1 == 0 && p2) {
            Loader::get()->queueInMainThread([this, p1, p2, p3]() {
                if (auto LSL = typeinfo_cast<LevelSelectLayer*>(this->getParent())) {
                    this->unschedule(schedule_selector(BoomScrollLayer::updateDots));
                    if (auto batch = this->getChildByType<CCSpriteBatchNode>(0)) {
                        batch->removeMeAndCleanup();
                        auto node = CCNode::create();
                        m_dots->removeAllObjects();
                        for (int i = 0; i < p3->count(); i++) {
                            auto level = as<GJGameLevel*>(p3->objectAtIndex(i));
                            int difficulty = static_cast<int>(level->m_difficulty);
                            if (difficulty < 1) {
                                difficulty = 1;
                            } else if (difficulty > 6) {
                                difficulty = 6;
                            }
                            auto texture = (GameStatsManager::sharedState()->getStat("8") < level->m_requiredCoins) ? "GJLargeLock_001.png" : fmt::format("diffIcon_{:02}_btn_001.png",difficulty);
                            if (i >= p3->count() - 2) {
                                auto dot = CCSprite::create("smallDot.png");
                                node->addChild(dot, 5);
                                m_dots->addObject(dot);
                            } else {
                                auto dot = CCSprite::createWithSpriteFrameName(texture.c_str());
                                if (!strcmp(texture.c_str(), "GJLargeLock_001.png")) {
                                    dot->setScale(0.15F);
                                } else {
                                    dot->setScale(0.35F);
                                }
                                node->addChild(dot, 5);
                                m_dots->addObject(dot);
                            }
                        }
                        this->addChild(node, 5);
                        this->schedule(schedule_selector(BoomScrollLayer::updateDots));
                    }
                }
            });
        }
        return true;
    }
};
