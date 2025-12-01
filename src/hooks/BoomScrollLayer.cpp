#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/BoomScrollLayer.hpp>

class $modify(OLBoomScrollLayer, BoomScrollLayer) {
    bool init(cocos2d::CCArray* pages, int offset, bool looped, cocos2d::CCArray* dynamicObjects, DynamicScrollDelegate* delegate) {
        if (!BoomScrollLayer::init(pages,offset,looped,dynamicObjects,delegate)) return false;
        if (!Mod::get()->getSettingValue<bool>("enabled") || !Mod::get()->getSettingValue<bool>("page-icons")) return true;
        // this is so dumb
        if ((pages != nullptr && pages->count() == 3) && (dynamicObjects != nullptr && dynamicObjects->count() > 1) && looped) {
            Loader::get()->queueInMainThread([this, offset, looped, dynamicObjects]() {
                if (auto LSL = typeinfo_cast<LevelSelectLayer*>(this->getParent())) {
                    this->unschedule(schedule_selector(BoomScrollLayer::updateDots));
                    if (auto batch = this->getChildByType<CCSpriteBatchNode>(0)) {
                        batch->removeMeAndCleanup();
                        //auto node = CCNode::create();
                        m_dots->removeAllObjects();
                        auto menu = CCMenu::create();
                        for (int i = 0; i < dynamicObjects->count(); i++) {
                            auto level = static_cast<GJGameLevel*>(dynamicObjects->objectAtIndex(i));
                            int difficulty = static_cast<int>(level->m_difficulty);
                            if (difficulty < 1) {
                                difficulty = 1;
                            } else if (difficulty > 6) {
                                difficulty = 6;
                            }
                            auto texture = (GameStatsManager::sharedState()->getStat("8") < level->m_requiredCoins) ? "GJLargeLock_001.png" : fmt::format("diffIcon_{:02}_btn_001.png",difficulty);
                            CCSprite* dot;
                            // assume more games
                            if (dynamicObjects->count() == 4) {
                                if (i >= 3) {
                                    dot = CCSprite::create("smallDot.png");
                                } else {
                                    dot = CCSprite::createWithSpriteFrameName(texture.c_str());
                                    dot->setScale(0.35F);
                                }
                            } else {
                                if (i >= dynamicObjects->count() - 2) {
                                    dot = CCSprite::create("smallDot.png");
                                } else {
                                    dot = CCSprite::createWithSpriteFrameName(texture.c_str());
                                    if (!strcmp(texture.c_str(), "GJLargeLock_001.png")) {
                                        dot->setScale(0.15F);
                                    } else {
                                        dot->setScale(0.35F);
                                    }
                                }
                            }
                            auto dotBtn = CCMenuItemSpriteExtra::create(dot, this, menu_selector(OLBoomScrollLayer::goToPage));
                            dotBtn->setTag(i);
                            menu->addChild(dotBtn, 5);
                            m_dots->addObject(dotBtn);
                        }
                        menu->setPosition(0, 0);
                        this->addChild(menu, 5);
                        this->schedule(schedule_selector(BoomScrollLayer::updateDots));
                    }
                }
            });
        }
        return true;
    }
    void goToPage(CCObject* sender) {
        if (auto menuItem = static_cast<CCMenuItemSpriteExtra*>(sender)) {
            quickUpdate();
            moveToPage(menuItem->getTag());
            quickUpdate();
        }
    }
};
