#include "../utils.hpp"
#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/LevelPage.hpp>

/*
"revamp-ui": {
            "type": "bool",
            "name": "UI Revamp",
            "description": "Whether or not the mod should revamp the UI. Turn this off only if you want the other settings.",
            "default": true
        },
        "level-preview": {
            "type": "bool",
            "name": "Show Level Preview",
            "description": "If the mod should show a preview of the level. Turn this off if you are experiencing lag.",
            "default": true
        }
*/

class $modify(LevelPage) {
    static void onModify(auto& self) {
        if (!self.setHookPriority("LevelPage::init", 0x8008135)) { // 0x100001, doggo suggested this for some reason
            log::warn("Failed to set hook priority.");
        }
        if (Loader::get()->isModLoaded("uproxide.more_difficulties")) {
            if (!self.setHookPriority("LevelPage::updateDynamicPage", 100000)) {
                log::warn("Failed to set hook priority.");
            }
        }
    }
    struct Fields {
        TextArea* m_textArea;
        CCLabelBMFont* achievement1Title;
        CCLabelBMFont* achievement2Title;
        CCSprite* achievement1Icon;
        CCSprite* achievement2Icon;
        CCSprite* levelBG;

        CCScale9Sprite* achievement2Display; //1st achivement display

        TextArea* m_textArea1Desc;   //1st achievement label
        TextArea* m_textAreaDesc;  //2st achievement label
    };
    void createClippingNode() {
        auto clippingNode = CCClippingNode::create();
        clippingNode->setContentSize(m_levelDisplaySize);
        clippingNode->setAnchorPoint({0.5, 0.5});

        auto mask2 = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
        mask2->setAnchorPoint({0.5, 0.5});
        mask2->setContentSize(m_levelDisplay->getContentSize());
        //mask2->setPosition({winSize.width / 2, (winSize.height / 2) + 50.F});
        clippingNode->setStencil(mask2);
        clippingNode->setAlphaThreshold(0.05f);

        m_fields->levelBG = CCSprite::create();
        m_fields->levelBG->setScale(1.25F);//1.05F);
        m_fields->levelBG->setOpacity(150);
        //m_levelDisplay->addChildAtPosition(m_fields->levelBG, Anchor::Center);
        clippingNode->addChildAtPosition(m_fields->levelBG, Anchor::BottomLeft);
        m_levelDisplay->addChildAtPosition(clippingNode, Anchor::Center);
        clippingNode->setZOrder(-1);

        m_levelDisplay->updateLayout();
        clippingNode->setAnchorPoint({0, 0});


        auto moveAround = CCSequence::create(
            CCEaseInOut::create(CCMoveBy::create(1.5f, ccp(0, 3)), 2.0f),
            CCEaseInOut::create(CCMoveBy::create(1.5f, ccp(0, -3)), 2.0f),
            nullptr
        );
        m_fields->levelBG->runAction(CCRepeatForever::create(moveAround));
    }
    // a recreation* of LevelPage::init
    bool init(GJGameLevel* level) { // haha level isnt even used! but erm what is the point of level if its never used???
        if (!Mod::get()->getSettingValue<bool>("enabled")) return LevelPage::init(level);
        if (!Mod::get()->getSettingValue<bool>("overcharge-menu")) {
            if (!LevelPage::init(level)) return false;
            createClippingNode();
            return true;
        }
        /*if (Loader::get()->isModLoaded("user95401.mainlevelseditora")) {
            Loader::get()->queueInMainThread([]() {
                FLAlertLayer::create("Main Levels Editor mod detected!", "<cg>Main Levels Editor</c> is not supported and will probably crash for this mod.\nPlease turn off the mod in order to continue using <cy>Mod Name</c>.", "OK")->show();
            });
            enableMod = false;
            return LevelPage::init(level);
        }*/
        if (!CCLayer::init()) return false;

        // i only now realized calloc already did this... i wasted like 2+ hours reversing this, great!
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_progressObjects = CCArray::create();
        m_levelObjects = CCArray::create();
        
        m_progressObjects->retain();
        m_levelObjects->retain();
        m_levelMenu = CCMenu::create();
        //m_levelDisplaySize = ccp(340.F, 95.F);
        m_levelDisplaySize = ccp(135.F, 210.F);
        m_levelMenu->setPosition({(winSize.width / 2) - 203.F, (winSize.height / 2) - 105.F});
        this->addChild(m_levelMenu, -1);

        m_levelDisplay = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
        if (Mod::get()->getSettingValue<bool>("level-preview")) {
            m_levelDisplay->setOpacity(0);
        } else {
            m_levelDisplay->setOpacity(75);
        }
        m_levelDisplay->setContentSize(m_levelDisplaySize);
        m_levelDisplay->setPosition({winSize.width / 2, (winSize.height / 2) + 50.F});
        auto sprite = CCSprite::create();
        sprite->addChild(m_levelDisplay);
        sprite->setContentSize(m_levelDisplay->getContentSize());

        m_levelDisplay->setPosition({m_levelDisplay->getContentSize() / 2});
        auto playBtnD = CCMenuItemSpriteExtra::create(sprite, nullptr, this, menu_selector(LevelPage::onPlay));
        //playBtn->m_scaleMultiplier = 1.1F;
        playBtnD->setEnabled(false);
        //playBtnD->setVisible(false);
        playBtnD->setAnchorPoint({0, 0});
        //playBtnD->setPosition({winSize.width / 2, (winSize.height / 2) + 50.F});
        m_levelMenu->addChild(playBtnD);
        auto playSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        playSpr->setScale(0.45F);
        auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(LevelPage::onPlay));
        auto practiceSpr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
        practiceSpr->setScale(0.575F);
        auto practiceBtn = CCMenuItemExt::createSpriteExtra(practiceSpr, [this](CCObject* sender) {
            Utils::setPractice(true);
            LevelPage::onPlay(sender);
        });

        if (Loader::get()->isModLoaded("dankmeme.globed2")) {
            playBtn->setPosition({43, 40});
            practiceBtn->setPosition({92, 40});
        } else {
            playBtn->setPosition({43, 50});
            practiceBtn->setPosition({92, 50});
        }
        //m_levelMenu->addChild(sprite); 
        m_levelMenu->addChild(playBtn); 
        m_levelMenu->addChild(practiceBtn); 
        m_progressObjects->addObject(m_levelMenu);

        auto progressDisplay = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
        progressDisplay->setOpacity(75);
        progressDisplay->setContentSize({260.F, 70.F});
        progressDisplay->setPosition({(winSize.width / 2) + 70.F, (winSize.height / 2) + 70.F});
        this->addChild(progressDisplay, 3);
        m_progressObjects->addObject(progressDisplay);

        auto normProgressBar = CCSprite::create("GJ_progressBar_001.png");
        normProgressBar->setColor({0,0,0});
        normProgressBar->setOpacity(125);
        normProgressBar->setScale(0.7F);
        progressDisplay->addChildAtPosition(normProgressBar, Anchor::Center, {0, 8});

        m_normalProgressBar = CCSprite::create("GJ_progressBar_001.png");
        m_normalProgressBar->setScaleX(0.992F);
        m_normalProgressBar->setScaleY(0.825F);
        m_normalProgressBar->setColor({0,255,0});
        m_normalProgressBar->setID("normal-progress-bar");
        m_progressWidth = m_normalProgressBar->getTextureRect().size.width;
        normProgressBar->addChild(m_normalProgressBar, 1);
        m_normalProgressBar->setAnchorPoint({0, 0.5});

        // robtop, what is this (or well, compiler, what is this)
        m_normalProgressBar->setPosition({std::fma(m_progressWidth, 1.008F, -m_progressWidth) / 2.F, normProgressBar->getContentHeight() / 2});

        auto practProgressBar = CCSprite::create("GJ_progressBar_001.png");
        practProgressBar->setColor({0,0,0});
        practProgressBar->setOpacity(125);
        practProgressBar->setScale(0.7F);
        progressDisplay->addChildAtPosition(practProgressBar, Anchor::Center, {0, -22});

        m_practiceProgressBar = CCSprite::create("GJ_progressBar_001.png");
        m_practiceProgressBar->setScaleX(0.992F);
        m_practiceProgressBar->setScaleY(0.825F);
        m_practiceProgressBar->setColor({0,255,255});
        m_practiceProgressBar->setID("practice-progress-bar");
        practProgressBar->addChild(m_practiceProgressBar, 1);
        m_practiceProgressBar->setAnchorPoint({0, 0.5});
        m_practiceProgressBar->setPosition({m_normalProgressBar->getPositionX(), practProgressBar->getContentHeight() / 2});

        m_progressObjects->addObject(normProgressBar);
        m_progressObjects->addObject(practProgressBar);

        
        //this->addChild(CCLabelBMFont::create(" ", "bigFont.fnt")); // i love node ids

        m_normalProgressLabel = CCLabelBMFont::create(" ","bigFont.fnt");
        m_normalProgressLabel->setID("normal-progress-label");
        progressDisplay->addChild(m_normalProgressLabel, 4);
        m_normalProgressLabel->setPosition(normProgressBar->getPosition());
        m_normalProgressLabel->setScale(0.35F);
        m_progressObjects->addObject(m_normalProgressLabel);

        m_practiceProgressLabel = CCLabelBMFont::create(" ","bigFont.fnt");
        m_practiceProgressLabel->setID("practice-progress-label");
        progressDisplay->addChild(m_practiceProgressLabel, 4);
        m_practiceProgressLabel->setPosition(practProgressBar->getPosition());
        m_practiceProgressLabel->setScale(0.35F);
        m_progressObjects->addObject(m_practiceProgressLabel);

        auto normalModeLbl = CCLabelBMFont::create("Normal Mode", "bigFont.fnt");
        normalModeLbl->setID("normal-mode-label");
        progressDisplay->addChild(normalModeLbl, 4);
        normalModeLbl->setPosition({normProgressBar->getPositionX(), normProgressBar->getPositionY() + 13.F}); // 56
        normalModeLbl->setScale(0.315F);
        m_progressObjects->addObject(normalModeLbl);

        auto practiceModeLbl = CCLabelBMFont::create("Practice Mode", "bigFont.fnt");
        practiceModeLbl->setID("practice-mode-label");
        progressDisplay->addChild(practiceModeLbl, 4);
        practiceModeLbl->setPosition({practProgressBar->getPositionX(), practProgressBar->getPositionY() + 13.F});
        practiceModeLbl->setScale(0.315F);
        m_progressObjects->addObject(practiceModeLbl);

        m_nameLabel = CCLabelBMFont::create(" ", "bigFont.fnt");
        m_levelDisplay->addChildAtPosition(m_nameLabel, Anchor::Top, {0, -30});
        m_nameLabel->setVisible(false);
        m_levelObjects->addObject(m_nameLabel);

        m_difficultySprite = CCSprite::createWithSpriteFrameName("diffIcon_01_btn_001.png"); // GJDifficultySprite
        m_difficultySprite->setScale(0.9F);
        m_difficultySprite->setID("difficulty-sprite");
        m_levelDisplay->addChildAtPosition(m_difficultySprite, Anchor::Center, {0, 30});
        m_levelObjects->addObject(m_difficultySprite);

        m_starsSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        m_starsSprite->setScale(0.375F);
        m_levelDisplay->addChildAtPosition(m_starsSprite, Anchor::Center, {5, 3});
        m_levelObjects->addObject(m_starsSprite);

        m_starsLabel = CCLabelBMFont::create(" ","bigFont.fnt");
        m_starsLabel->setScale(0.3F);
        m_starsLabel->setAnchorPoint({1.0, 0.5});
        m_levelDisplay->addChildAtPosition(m_starsLabel, Anchor::Center, {-2, 3});
        m_levelObjects->addObject(m_starsLabel);

        m_coins = CCArray::create();
        m_coins->retain();

        for (int i = 0; i < 3; i++) {
            auto coin = CCSprite::createWithSpriteFrameName("GJ_coinsIcon_001.png");
            coin->setScale(0.75F);
            m_levelDisplay->addChildAtPosition(coin, Anchor::Center, {((i) * 18.5F) - 18.F, -13});
            m_coins->addObject(coin);
            m_levelObjects->addObject(coin);
        }

        m_dynamicObjects = CCArray::create();
        m_dynamicObjects->retain();
        if (PlatformToolbox::isControllerConnected()) {
            auto controllerBtn = CCSprite::createWithSpriteFrameName("controllerBtn_Start_001.png");
            sprite->addChild(controllerBtn, 10);
            controllerBtn->setPosition({m_levelDisplay->getPositionX(), m_levelDisplay->getPositionY() - 35.F});
            m_levelObjects->addObject(controllerBtn);
        }
        
        m_fields->m_textArea = TextArea::create(" ", "bigFont.fnt", 1.0F, 300.F, {0.5, 0.5}, 30.F, true);
        /*
        for (size_t i = 0; i < textArea->m_label->getChildrenCount(); i++) {
            if (auto label = typeinfo_cast<CCLabelBMFont*>(textArea->m_label->getChildren()->objectAtIndex(i))) {
                label->setAnchorPoint({0.5, 0.5});
            }
        }
        */
        m_fields->m_textArea->setScale(0.45F);
        m_levelDisplay->addChildAtPosition(m_fields->m_textArea, Anchor::Top, {0, -23});
        m_fields->m_textArea->setScale(Utils::calculateScale(strlen(m_nameLabel->getString()), 10, 25, 0.75F, 0.4F));
        m_levelObjects->addObject(m_fields->m_textArea);

        auto achievementDisplay = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
        achievementDisplay->setOpacity(75);
        achievementDisplay->setContentSize({260.F, 130.F});
        achievementDisplay->setPosition({(winSize.width / 2) + 70.F, (winSize.height / 2) - 40.F});
        this->addChild(achievementDisplay, 3);
        m_progressObjects->addObject(achievementDisplay);

        auto achievementLabel = CCLabelBMFont::create("Achievements", "bigFont.fnt");
        achievementLabel->setAnchorPoint({0.5, 1.0});
        achievementLabel->setScale(0.425F);
        achievementDisplay->addChildAtPosition(achievementLabel, Anchor::Top, {0, -5});


        if (auto gm = GameManager::sharedState()) {
            auto achievement1Display = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
            achievement1Display->setOpacity(75);
            achievement1Display->setContentSize({300.F, 55.F});
            achievement1Display->setScale(0.8F);
            achievementDisplay->addChildAtPosition(achievement1Display, Anchor::Center, {0, 18});

            m_fields->achievement1Title = CCLabelBMFont::create(" ", "goldFont.fnt");
            m_fields->achievement1Title->setScale(0.575F);
            m_fields->achievement1Title->setAnchorPoint({0, 0.5});
            m_fields->m_textArea1Desc = TextArea::create("Complete This Level In Practice Mode", "bigFont.fnt", 0.4F, 200.F, {0, 0.5}, 15.F, true);
            m_fields->m_textArea1Desc->setAnchorPoint({0, 0.5});
            achievement1Display->addChildAtPosition(m_fields->achievement1Title, Anchor::TopLeft, {9, -11});
            achievement1Display->addChildAtPosition(m_fields->m_textArea1Desc, Anchor::Left, {9, 2});
            auto lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
            m_fields->achievement1Icon = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
            m_fields->achievement1Icon->setColor({175,175,175});
            m_fields->achievement1Icon->addChild(lock);

            m_fields->achievement1Icon->setScale(0.9F);
            m_fields->achievement1Icon->setAnchorPoint({0.5, 0.5});
            achievement1Display->addChildAtPosition(m_fields->achievement1Icon, Anchor::Right, {-25, 0});


            m_fields->achievement2Display = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
            m_fields->achievement2Display->setOpacity(75);
            m_fields->achievement2Display->setContentSize({300.F, 55.F});
            m_fields->achievement2Display->setScale(0.8F);
            achievementDisplay->addChildAtPosition(m_fields->achievement2Display, Anchor::Center, {0, -32});

            m_fields->achievement2Title = CCLabelBMFont::create(" ", "goldFont.fnt");
            m_fields->achievement2Title->setScale(0.575F);
            m_fields->achievement2Title->setAnchorPoint({0, 0.5});

            m_fields->m_textAreaDesc = TextArea::create("Complete This Level In Normal Mode", "bigFont.fnt", 0.4F, 200.F, { 0, 0.5 }, 15.F, true);
            m_fields->m_textAreaDesc->setAnchorPoint({0, 0.5});
            m_fields->achievement2Display->addChildAtPosition(m_fields->achievement2Title, Anchor::TopLeft, {9, -11});

          
           
            m_fields->achievement2Display->addChildAtPosition(m_fields->m_textAreaDesc, Anchor::Left, {9, 2});

            m_fields->achievement2Icon = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
            m_fields->achievement2Icon->setColor({175,175,175});
            m_fields->achievement2Icon->addChild(lock);
            m_fields->achievement2Icon->setScale(0.9F);
            m_fields->achievement2Icon->setAnchorPoint({0.5, 0.5});
            m_fields->achievement2Display->addChildAtPosition(m_fields->achievement2Icon, Anchor::Right, {-25, 0});
        }

        // i really just wasted hours doing this
        /*
        int segments = 10;
        float radius = 8.0F; //8
        auto size = m_levelDisplay->getContentSize();
        
        auto mask = CCDrawNode::create();
        mask->setAnchorPoint({0.5, 0.5});
        auto color = ccc4FFromccc3B({1,1,1});

        float offset = 0.F;
        CCPoint rectVertices[4] = {
            ccp(offset, offset),
            ccp(offset, size.height - offset),
            ccp(size.width - offset, size.height - offset),
            ccp(size.width - offset, offset)
        };

        float otherOffsetX = offset + 22.F;
        float otherOffsetY = offset + 10.F;
        CCPoint rectVertices1[4] = {
            ccp(offset, offset + otherOffsetY),
            ccp(offset, (size.height - offset) - otherOffsetY),
            ccp(offset + otherOffsetX, (size.height - offset) - otherOffsetY),
            ccp(offset + otherOffsetX, offset + otherOffsetY),
        };

        CCPoint rectVertices2[4] = {
            ccp((size.width - offset), offset + otherOffsetY),
            ccp((size.width - offset), (size.height - offset) - otherOffsetY),
            ccp((size.width - offset) - otherOffsetX, (size.height - offset) - otherOffsetY),
            ccp((size.width - offset) - otherOffsetX, offset + otherOffsetY),
        };

        CCPoint rectVertices3[4] = {
            ccp(offset + otherOffsetX - 5.F, offset),
            ccp(offset + otherOffsetX - 5.F, size.height - offset),
            ccp((size.width - offset) - otherOffsetX + 5.F, size.height - offset),
            ccp((size.width - offset) - otherOffsetX + 5.F, offset),
        };

        //mask->drawPolygon(rectVertices1, 4, color, 0.f, color);
        //mask->drawPolygon(rectVertices2, 4, color, 0.f, color);
        //mask->drawPolygon(rectVertices3, 4, color, 0.f, color);

        float angleStep = M_PI * 0.5f / segments;

        offset = offset + 16.9F;
        for (int corner = 0; corner < 4; ++corner) {
            float startAngle;
            CCPoint startPoint;
            CCPoint lastPoint;
            
            switch(corner) {
                case 0: // Top-left
                    startAngle = M_PI * 0.5f; // 90 degrees, pointing down
                    startPoint = ccp(offset, size.height - offset); // Adjust to top-left
                    break;
                case 1: // Top-right
                    startAngle = 0; // 0 degrees, pointing right
                    startPoint = ccp(size.width - offset, size.height - offset); // Adjust to top-right
                    break;
                case 2: // Bottom-right
                    startAngle = -M_PI * 0.5f; // -90 degrees, pointing up
                    startPoint = ccp(size.width - offset, offset); // Adjust to bottom-right
                    break;
                case 3: // Bottom-left
                    startAngle = M_PI; // 180 degrees, pointing left
                    startPoint = ccp(offset, offset); // Adjust to bottom-left
                    break;
            }
            for (int i = 0; i <= segments; ++i) {
                float angle = startAngle + i * angleStep;
                CCPoint point = {
                    startPoint.x + radius * cos(angle),
                    startPoint.y + radius * sin(angle)
                };
                
                if (i > 0) {
                    mask->drawSegment(lastPoint, point, radius, color);
                }
                lastPoint = point;
            }
        }*/
        /*mask->drawCircle(ccp(cornerRadius + cornerOffset, cornerRadius + cornerOffset), cornerRadius, color, 0.f, color, segments);
        mask->drawCircle(ccp(cornerRadius + cornerOffset, (size.height - offset) - (cornerRadius + cornerOffset)), cornerRadius, color, 0.f, color, segments);
        mask->drawCircle(ccp((size.width - offset) - (cornerRadius + cornerOffset), (size.height - offset) - (cornerRadius + cornerOffset)), cornerRadius, color, 0.f, color, segments);
        mask->drawCircle(ccp((size.width - offset) - (cornerRadius + cornerOffset), cornerRadius + cornerOffset), cornerRadius, color, 0.f, color, segments);*/


        //CCLayerColor* mask = CCLayerColor::create({255, 255, 255});
        //mask->setContentSize(m_levelDisplaySize);
        createClippingNode();
		return true;
    }
    void updateDynamicPage(GJGameLevel* level) {
        LevelPage::updateDynamicPage(level);
        if (!Mod::get()->getSettingValue<bool>("enabled")) return;
        bool willOvercharge = Mod::get()->getSettingValue<bool>("overcharge-menu");
        /*if (auto llm = LocalLevelManager::sharedState()) { // haha get it
            level->m_levelString = llm->getMainLevelString(level->m_levelID.value());
        }*/
        if (willOvercharge) {
            int difficulty = static_cast<int>(level->m_difficulty);
            if (difficulty < 1) {
                difficulty = 1;
            } else if (difficulty > 6) {
                difficulty = 6;
            }
            m_difficultySprite->setDisplayFrame(GJDifficultySprite::create(difficulty, GJDifficultyName::Short)->displayFrame());
            if (Loader::get()->isModLoaded("uproxide.more_difficulties")) {
                if (auto spr = typeinfo_cast<CCSprite*>(this->getChildByIDRecursive("uproxide.more_difficulties/more-difficulties-spr"))) {
                    spr->setPosition({spr->getPositionX(), m_difficultySprite->getPositionY() - 5.F});
                    spr->setScale(1.F);
                    m_difficultySprite->setVisible(false);
                }
            } else {
                m_difficultySprite->setVisible(true);
            }
            m_fields->m_textArea->setString(m_nameLabel->getString());
            m_nameLabel->setVisible(false);
            m_fields->m_textArea->setScale(Utils::calculateScale(strlen(m_nameLabel->getString()), 7, 15, 0.75F, 0.4F));
        }

        int levelID = level->m_levelID.value();
        if ((levelID <= 22) ||
            (levelID >= 1001 && levelID <= 1003) ||
            (levelID >= 4001 && levelID <= 4003) && levelID > 0 && Mod::get()->getSettingValue<bool>("level-preview")) {
            m_fields->levelBG->removeAllChildrenWithCleanup(true);

            std::string spriteName = fmt::format("{}{}.png"_spr, levelID, (willOvercharge) ? "" : "-h");
            if (levelID == 22 && willOvercharge) {
                // lets hope this isnt against index rules!
                if (auto gm = GameManager::sharedState()) {
                    if (gm->m_playerName == "Colon") {
                        spriteName = "22secret.png"_spr;
                    }
                }
            }
            if (GameStatsManager::sharedState()->getStat("8") < m_level->m_requiredCoins) {
                spriteName = fmt::format("{}barely{}.png"_spr, levelID, (willOvercharge) ? "" : "-h");
            }
            /*
            auto spriteFrame = CCTextureCache::sharedTextureCache()->textureForKey(spriteName.c_str());
            if (spriteFrame) {
                m_fields->levelBG->setOpacity(150);
                m_fields->levelBG->setTexture(spriteFrame);
            }
*/
            auto sprite = CCSprite::create(spriteName.c_str());
            if (sprite != nullptr) {
                sprite->setOpacity((willOvercharge) ? 150 : 200);
                m_fields->levelBG->addChildAtPosition(sprite, Anchor::Center);
            }

        }
        if (!willOvercharge) return;
        auto achievementID1 = fmt::format("geometry.ach.level{:02}a", levelID);
        auto achievementID2 = fmt::format("geometry.ach.level{:02}b", levelID);
        if (Loader::get()->isModLoaded("bitz.moregames")) {
     
             switch (levelID) {
                case 1001:
                    m_fields->m_textArea1Desc->setString("Complete This Level In Normal Mode");
                    m_fields->m_textAreaDesc->setString("Collect all 3 Secret Coins on 'The Seven Seas'");
                    achievementID1 = "geometry.ach.mdlevel01b"; //Normal Mode achievement
                    achievementID2 = "geometry.ach.mdcoin01"; //3 Coins achievement
                    break;
                case 1002:
                    m_fields->m_textArea1Desc->setString("Complete This Level In Normal Mode"); //1st achievement label
                    m_fields->m_textAreaDesc->setString("Collect all 3 Secret Coins on 'Viking Arena'"); //2st achievement label
                    achievementID1 = "geometry.ach.mdlevel02b"; //Normal Mode achievement
                    achievementID2 = "geometry.ach.mdcoin02"; //3 Coins achievement
                    break;
                case 1003:
                    m_fields->m_textArea1Desc->setString("Complete This Level In Normal Mode"); //1st achievement label
                    m_fields->m_textAreaDesc->setString("Collect all 3 Secret Coins on 'Airborne Robots'");  //2st achievement label
                    achievementID1 = "geometry.ach.mdlevel03b"; //Normal Mode achievement
                    achievementID2 = "geometry.ach.mdcoin03"; //3 Coins achievement
                    break;
                case 4001:
                    m_fields->m_textArea1Desc->setString("Complete This Level In Normal Mode"); //1st achievement label
                    m_fields->m_textAreaDesc->setString("Collect all 3 Secret Coins on 'Press Start'");  //2st achievement label
                    achievementID1 = "geometry.ach.subzero.level001"; //Normal Mode achievement
                    achievementID2 = "geometry.ach.subzero.coins001"; //3 Coins achievement
                    break;
                case 4002:
                    m_fields->m_textArea1Desc->setString("Complete This Level In Normal Mode"); //1st achievement label
                    m_fields->m_textAreaDesc->setString("Collect all 3 Secret Coins on 'Nock Em'");  //2st achievement label
                    achievementID1 = "geometry.ach.subzero.level002"; //Normal Mode achievement
                    achievementID2 = "geometry.ach.subzero.coins002"; //3 Coins achievement
                    break;
                case 4003:
                    m_fields->m_textArea1Desc->setString("Complete This Level In Normal Mode"); //1st achievement label
                    m_fields->m_textAreaDesc->setString("Collect all 3 Secret Coins on 'Power Trip'");  //2st achievement label
                    achievementID1 = "geometry.ach.subzero.level003"; //Normal Mode achievement
                    achievementID2 = "geometry.ach.subzero.coins003"; //3 Coins achievement
                    break;
             }
        }
        if (auto am = AchievementManager::sharedState()) {
            if (level->m_levelID.value() > 0) {
                if (auto gm = GameManager::sharedState()) {
                    if (auto achievementData = as<CCDictionary*>(am->m_platformAchievements->objectForKey(achievementID1))) {
                        auto achievementTitle = achievementData->valueForKey("title")->getCString();
                        if (GameStatsManager::sharedState()->getStat("8") < m_level->m_requiredCoins) {
                            achievementTitle = "???";
                        }
                        m_fields->achievement1Title->setString(achievementTitle);
                        m_fields->achievement1Icon->removeAllChildrenWithCleanup(true);
                        
                        if (am->isAchievementEarned(achievementID1.c_str())) {
                            std::string value = "icon";
                            UnlockType unlockType;
                            int id = 1;
                            if (strlen(achievementData->valueForKey("icon")->getCString()) > 0) {
                                value = "icon";
                                unlockType = UnlockType::Cube;
                            }
                            if (strlen(achievementData->valueForKey("color")->getCString()) > 0) {
                                value = "color";
                                unlockType = UnlockType::Col1;
                            }
                            if (strlen(achievementData->valueForKey("color2")->getCString()) > 0) {
                                value = "color2";
                                unlockType = UnlockType::Col2;
                            }
                            std::string cString = achievementData->valueForKey(value)->getCString();
                            size_t pos = cString.find('_');
                            if (pos != std::string::npos) {
                                std::string numberPart = cString.substr(pos + 1);;
                                value = cString.substr(0, pos);;
                                if (value == "icon") {
                                    unlockType = UnlockType::Cube;
                                } else if (value == "ship") {
                                    unlockType = UnlockType::Ship;
                                } else if (value == "ball") {
                                    unlockType = UnlockType::Ball;
                                } else if (value == "dart") {
                                    unlockType = UnlockType::Dart;
                                } else if (value == "robot") {
                                    unlockType = UnlockType::Robot;
                                } else if (value == "spider") {
                                    unlockType = UnlockType::Spider;
                                } else if (value == "swing") {
                                    unlockType = UnlockType::Swing;
                                } else if (value == "color") {
                                    unlockType = UnlockType::Col1;
                                } else if (value == "color2") {
                                    unlockType = UnlockType::Col2;
                                }
                                id = std::stoi(numberPart);
                            }
                            GJItemIcon* newIcon = GJItemIcon::create(unlockType, id, {175, 175, 175}, {255, 255, 255}, false, false, false, {255, 255, 255});
                            
                            m_fields->achievement1Icon->addChildAtPosition(newIcon, Anchor::Center);
                            m_fields->achievement1Icon->setOpacity(0);
                        } else {
                            auto lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                            m_fields->achievement1Icon->addChildAtPosition(lock, Anchor::Center);
                            m_fields->achievement1Icon->setOpacity(255);
                        }
                    } else {
                        m_fields->achievement1Title->setString("N/A");
                        m_fields->achievement1Icon->removeAllChildrenWithCleanup(true);
                        auto lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                        m_fields->achievement1Icon->addChildAtPosition(lock, Anchor::Center);
                        m_fields->achievement1Icon->setOpacity(255);
                    }
                    if (auto achievementData = as<CCDictionary*>(am->m_platformAchievements->objectForKey(achievementID2))) {
                        auto achievementTitle = achievementData->valueForKey("title")->getCString();
                        if (GameStatsManager::sharedState()->getStat("8") < m_level->m_requiredCoins) {
                            achievementTitle = "???";
                        }
                        m_fields->achievement2Title->setString(achievementTitle);
                        m_fields->achievement2Icon->removeAllChildrenWithCleanup(true);

                        if (am->isAchievementEarned(achievementID2.c_str())) {
                            std::string value = "icon";
                            UnlockType unlockType;
                            int id = 1;
                            GJItemIcon* newIcon = GJItemIcon::create(UnlockType::Cube, 1, {175, 175, 175}, {255, 255, 255}, false, false, false, {255, 255, 255});
                            if (strlen(achievementData->valueForKey("icon")->getCString()) > 0) {
                                value = "icon";
                                unlockType = UnlockType::Cube;
                            }
                            if (strlen(achievementData->valueForKey("color")->getCString()) > 0) {
                                value = "color";
                                unlockType = UnlockType::Col1;
                            }
                            if (strlen(achievementData->valueForKey("color2")->getCString()) > 0) {
                                value = "color2";
                                unlockType = UnlockType::Col2;
                            }
                            std::string cString = achievementData->valueForKey(value)->getCString();
                            size_t pos = cString.find('_');
                            if (pos != std::string::npos) {
                                std::string numberPart = cString.substr(pos + 1);;
                                value = cString.substr(0, pos);;
                                if (value == "icon") {
                                    unlockType = UnlockType::Cube;
                                } else if (value == "ship") {
                                    unlockType = UnlockType::Ship;
                                } else if (value == "ball") {
                                    unlockType = UnlockType::Ball;
                                } else if (value == "dart") {
                                    unlockType = UnlockType::Dart;
                                } else if (value == "robot") {
                                    unlockType = UnlockType::Robot;
                                } else if (value == "spider") {
                                    unlockType = UnlockType::Spider;
                                } else if (value == "swing") {
                                    unlockType = UnlockType::Swing;
                                } else if (value == "color") {
                                    unlockType = UnlockType::Col1;
                                } else if (value == "color2") {
                                    unlockType = UnlockType::Col2;
                                }
                                id = std::stoi(numberPart);
                            }
                            newIcon = GJItemIcon::create(unlockType, id, {175, 175, 175}, {255, 255, 255}, false, false, false, {255, 255, 255});

                            m_fields->achievement2Icon->addChildAtPosition(newIcon, Anchor::Center);
                            m_fields->achievement2Icon->setOpacity(0);
                        } else {
                            auto lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                            m_fields->achievement2Icon->setOpacity(255);
                            m_fields->achievement2Icon->addChildAtPosition(lock, Anchor::Center);
                        }
                    } else {
                        m_fields->achievement2Title->setString("N/A");
                        m_fields->achievement2Icon->removeAllChildrenWithCleanup(true);
                        auto lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                        m_fields->achievement2Icon->addChildAtPosition(lock, Anchor::Center);
                        m_fields->achievement2Icon->setOpacity(255);
                    }
                    
                }
            }
        }
        bool didLabel = false;
        bool didSprite = false;
        for (size_t i = 0; i < m_dynamicObjects->count(); i++) {
            auto obj = m_dynamicObjects->objectAtIndex(i);
            if (GameStatsManager::sharedState()->getStat("8") < m_level->m_requiredCoins) {
                if (auto label = typeinfo_cast<CCLabelBMFont*>(obj)) {
                    if (didLabel || !strcmp(label->getString(), "Coming Soon!")) continue;
                    label->setAnchorPoint({0.5, 0.5});
                    label->setPosition({68, 158});
                    didLabel = true;
                }
                if (auto sprite = typeinfo_cast<CCSprite*>(obj)) {
                    if (didSprite || m_secretDoor == sprite || sprite->getPosition() == ccp(67.5F, 105.F)) continue;
                    sprite->setScale(1.25F);
                    sprite->setPosition({68, 185});
                    didSprite = true;
                }
            } else {
                if (auto label = typeinfo_cast<CCLabelBMFont*>(obj)) {
                    if (didLabel || !strcmp(label->getString(), "Coming Soon!")) continue;
                    label->setPositionX(110);
                    label->setAnchorPoint({1.0, 0.5});
                    didLabel = true;
                }
                if (auto sprite = typeinfo_cast<CCSprite*>(obj)) {
                    if (didSprite || m_secretDoor == sprite) continue;
                    sprite->setPositionX(122);
                    didSprite = true;
                }
            }
            
        }
        m_levelDisplay->updateLayout();
    }
    /*
	bool initOrig(GJGameLevel* level) {
        return LevelPage::init(level);
        if (!CCLayer::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_progressObjects = CCArray::create();
        m_levelObjects = CCArray::create();

        m_progressObjects->retain();
        m_levelObjects->retain();
        m_levelMenu = CCMenu::create();
        m_levelDisplaySize = ccp(340.F, 95.F);
        m_levelMenu->setPosition({winSize.width / 2, (winSize.height / 2) + 60.F});
        this->addChild(m_levelMenu, -1);
        m_levelDisplay = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
        m_levelDisplay->setOpacity(125);
        m_levelDisplay->setContentSize(m_levelDisplaySize);
        m_levelDisplay->setPosition({winSize.width / 2, (winSize.height / 2) + 50.F});
        auto sprite = CCSprite::create();
        sprite->addChild(m_levelDisplay);
        sprite->setContentSize(m_levelDisplay->getContentSize());

        m_levelDisplay->setPosition({m_levelDisplay->getContentSize() / 2});
        auto playBtn = CCMenuItemSpriteExtra::create(sprite, nullptr, this, menu_selector(LevelPage::onPlay));
        playBtn->m_scaleMultiplier = 1.1F;
        m_levelMenu->addChild(playBtn);
        m_progressObjects->addObject(m_levelMenu);

        auto normProgressBar = CCSprite::create("GJ_progressBar_001.png");
        normProgressBar->setColor({0,0,0});
        normProgressBar->setOpacity(125);
        normProgressBar->setScale(1.0F);
        this->addChild(normProgressBar, 3);
        normProgressBar->setPosition({winSize.width / 2, (winSize.height / 2) - 30.F});

        m_normalProgressBar = CCSprite::create("GJ_progressBar_001.png");
        m_normalProgressBar->setScaleX(0.992F);
        m_normalProgressBar->setScaleY(0.86F);
        m_normalProgressBar->setColor({0,255,0});
        m_progressWidth = m_normalProgressBar->getTextureRect().size.width;
        normProgressBar->addChild(m_normalProgressBar, 1);
        m_normalProgressBar->setAnchorPoint({0, 0.5});

        // robtop, what is this (or well, compiler, what is this)
        m_normalProgressBar->setPosition({std::fma(m_progressWidth, 1.008F, -m_progressWidth) / 2.F, normProgressBar->getContentHeight() / 2});

        auto practProgressBar = CCSprite::create("GJ_progressBar_001.png");
        practProgressBar->setColor({0,0,0});
        practProgressBar->setOpacity(125);
        practProgressBar->setScale(1.0F);
        this->addChild(practProgressBar, 3);
        practProgressBar->setPosition({winSize.width / 2, (normProgressBar->getPositionY()) - 50.F});

        m_practiceProgressBar = CCSprite::create("GJ_progressBar_001.png");
        m_practiceProgressBar->setScaleX(0.992F);
        m_practiceProgressBar->setScaleY(0.86F);
        m_practiceProgressBar->setColor({0,255,255});
        practProgressBar->addChild(m_practiceProgressBar, 1);
        m_practiceProgressBar->setAnchorPoint({0, 0.5});
        m_practiceProgressBar->setPosition({m_normalProgressBar->getPositionX(), practProgressBar->getContentHeight() / 2});

        m_progressObjects->addObject(normProgressBar);
        m_progressObjects->addObject(practProgressBar);

        m_normalProgressLabel = CCLabelBMFont::create(" ","bigFont.fnt");
        this->addChild(m_normalProgressLabel, 4);
        m_normalProgressLabel->setPosition(normProgressBar->getPosition());
        m_normalProgressLabel->setScale(0.5F);
        m_progressObjects->addObject(m_normalProgressLabel);

        m_practiceProgressLabel = CCLabelBMFont::create(" ","bigFont.fnt");
        this->addChild(m_practiceProgressLabel, 4);
        m_practiceProgressLabel->setPosition(practProgressBar->getPosition());
        m_practiceProgressLabel->setScale(0.5F);
        m_progressObjects->addObject(m_practiceProgressLabel);

        auto normalModeLbl = CCLabelBMFont::create("Normal Mode", "bigFont.fnt");
        this->addChild(normalModeLbl, 4);
        normalModeLbl->setPosition({normProgressBar->getPositionX(), normProgressBar->getPositionY() + 20.F});
        normalModeLbl->setScale(0.5F);
        m_progressObjects->addObject(normalModeLbl);

        auto practiceModeLbl = CCLabelBMFont::create("Practice Mode", "bigFont.fnt");
        this->addChild(practiceModeLbl, 4);
        practiceModeLbl->setPosition({practProgressBar->getPositionX(), practProgressBar->getPositionY() + 20.F});
        practiceModeLbl->setScale(0.5F);
        m_progressObjects->addObject(practiceModeLbl);

        m_nameLabel = CCLabelBMFont::create(" ", "bigFont.fnt");
        m_nameLabel->setAnchorPoint({0, 0.5});
        m_levelDisplay->addChild(m_nameLabel);
        m_levelObjects->addObject(m_nameLabel);

        m_difficultySprite = CCSprite::createWithSpriteFrameName("diffIcon_01_btn_001.png");
        m_difficultySprite->setScale(1.1F);
        m_levelDisplay->addChild(m_difficultySprite);
        m_levelObjects->addObject(m_difficultySprite);

        m_starsSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        m_starsSprite->setScale(0.7F);
        m_levelDisplay->addChild(m_starsSprite);
        m_starsSprite->setPosition({m_levelDisplaySize.width - 15.0F, m_levelDisplaySize.height - 13.0F});
        m_levelObjects->addObject(m_starsSprite);

        m_starsLabel = CCLabelBMFont::create(" ","bigFont.fnt");
        m_levelDisplay->addChild(m_starsLabel);
        m_starsLabel->setAnchorPoint({1.0, 0.5});
        m_starsLabel->setPosition(m_starsSprite->getPositionX() - 12.F, m_starsSprite->getPositionY() + 0.5F);
        m_starsLabel->setScale(0.5F);
        m_levelObjects->addObject(m_starsLabel);

        m_coins = CCArray::create();
        m_coins->retain();

        for (int i = 3; i > 0; i--) {
            auto coin = CCSprite::createWithSpriteFrameName("GJ_coinsIcon_001.png");
            m_levelDisplay->addChild(coin);
            coin->setScale(1.0F);
            coin->setPosition({(m_levelDisplaySize.width - 16.0F) + ((i - 1) * -26.F), 16.0F});
            m_coins->addObject(coin);
            m_levelObjects->addObject(coin);
        }

        m_dynamicObjects = CCArray::create();
        m_dynamicObjects->retain();
        if (PlatformToolbox::isControllerConnected()) {
            auto controllerBtn = CCSprite::createWithSpriteFrameName("controllerBtn_Start_001.png");
            sprite->addChild(controllerBtn, 10);
            controllerBtn->setPosition({m_levelDisplay->getPositionX(), m_levelDisplay->getPositionY() - 35.F});
            m_levelObjects->addObject(controllerBtn);
        }
		return true;
	}
    */
};

