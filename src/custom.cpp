#include "head.hpp"

const char* colors[3] = {"Main", "Secondary", "Glow"};
std::string titles[4] = {"Level Info Menu", "Pause Menu", "Official Levels", "Quests Page"};
std::string items[5] = {"Normal", "Practice", "Top", "Middle", "Bottom"};

const std::string tabs[9] = {
    "info-menu-normal", "info-menu-practice",
    "pause-menu-normal", "pause-menu-practice", 
    "official-levels-normal", "official-levels-practice",
    "quest-top", "quest-middle", "quest-bottom"
};

SettingNodeV3* AdvancedSetting::createNode(float width) {
    return AdvancedSettingNode::create(
        std::static_pointer_cast<AdvancedSetting>(shared_from_this()),
        width
    );
}

bool AdvancedMenu::setup() {
    //auto winSize = CCDirector::sharedDirector()->getWinSize();
    this->setTitle("Advanced Setup Menu");
    // current Tab
    currentTab = Mod::get()->getSavedValue<int64_t>("current-tab", 0);
    currentConfig = Mod::get()->getSavedValue<BarColor>(
        tabs[currentTab],
        tabs[currentTab].find("practice") ? def2 : def1
    );

    // layer
    auto ml = static_cast<CCNode*>(this->getChildren()->objectAtIndex(0));
    ml->setID("main-layer");
    auto menu = ml->getChildByType<CCMenu>(0);
    menu->setID("close-menu");

    // division line
    auto divisionLine = CCLayerColor::create(ccc4(255, 255, 255, 192));
    divisionLine->setPosition(CCPoint(125.f, 20.f));
    divisionLine->setContentSize(CCSize(2.f, 220.f));
    ml->addChild(divisionLine);

    // Item Switch Menu
    auto itemsMenu = CCMenu::create();
    itemsMenu->setPosition(CCPoint(15.f, 10.f));
    itemsMenu->setContentSize(CCSize(100.f, 240.f));
    itemsMenu->setID("items-menu");
    ml->addChild(itemsMenu);

    // Item Switch Menu GoldFont
    
    for (int i=0; i<4; i++) {
        auto title = CCLabelBMFont::create(titles[i].c_str(), "goldFont.fnt", 140.f);
        title->setPosition(CCPoint(50.f, 220.f-i*50.f));
        title->setScale(0.5);
        itemsMenu->addChild(title);
    }

    for (int i=0; i<9; i++) {
        auto label = CCLabelBMFont::create(items[i<6 ? i%2 : i-4].c_str(), "bigFont.fnt", 140.f);
        label->setScale(0.4);
        if (i == currentTab)
            label->setColor(ccc3(0, 255, 0));
        
        auto button = CCMenuItemSpriteExtra::create(label, this, menu_selector(AdvancedMenu::onSwitchTab));
        button->setPosition(CCPoint(50.f, i<6 ? 205.f-(i/2)*50.f-(i%2)*15.f : 145-i*15.f));
        label->setTag(i);        
        button->setTag(i);
        itemsMenu->addChild(button);
    }
    // Setup Workspace
    auto setupMenu = CCMenu::create();
    setupMenu->setPosition(CCPoint(135.f, 40.f));
    setupMenu->setContentSize(CCSize(260.f, 180.f));
    setupMenu->setID("setup-menu");
    ml->addChild(setupMenu);

    // workspace title
    // tell users what they are currently setting for
    auto setupTitle = CCLabelBMFont::create("?", "goldFont.fnt", 260.f);
    setupTitle->setPosition(CCPoint(0.f, 190.f));
    setupTitle->setAnchorPoint(CCPoint(0.f, 0.6));
    setupTitle->setScale(0.5);
    setupTitle->setID("setup-title");
    setupMenu->addChild(setupTitle);

    // setup default
    auto setupDefaultMenu = CCMenu::create();
    setupDefaultMenu->setPosition(CCPoint(0.f, 180.f));
    setupDefaultMenu->setContentSize(CCSize(260.f, 30.f));
    setupDefaultMenu->setTag(0);
    setupMenu->addChild(setupDefaultMenu);

    auto defaultToggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(AdvancedMenu::onSetMode), 0.5);
    defaultToggler->setPosition(CCPoint(10.f, 20.f));
    defaultToggler->setCascadeOpacityEnabled(true);
    defaultToggler->setID("toggler");
    setupDefaultMenu->addChild(defaultToggler);

    auto defaultLabel = CCLabelBMFont::create("Default", "bigFont.fnt", 260.f);
    defaultLabel->setPosition(CCPoint(20.f, 20.f));
    defaultLabel->setAnchorPoint(CCPoint(0.f, 0.5));
    defaultLabel->setScale(0.5);
    defaultLabel->setID("title-label");
    setupDefaultMenu->addChild(defaultLabel);

    auto defaultHint = CCLabelBMFont::create("Follow common config outside.", "chatFont.fnt", 260.f);
    defaultHint->setPosition(CCPoint(20.f, 5.f));
    defaultHint->setAnchorPoint(CCPoint(0.f, 0.5));
    defaultHint->setColor(ccc3(255, 255, 0));
    defaultHint->setScale(0.7);
    setupDefaultMenu->addChild(defaultHint);

    // setup follow
    auto setupFollowMenu = CCMenu::create();
    setupFollowMenu->setPosition(CCPoint(0.f, 140.f));
    setupFollowMenu->setContentSize(CCSize(260.f, 50.f));
    setupFollowMenu->setTag(1);
    setupMenu->addChild(setupFollowMenu);

    auto followToggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(AdvancedMenu::onSetMode), 0.5);
    followToggler->setPosition(CCPoint(10.f, 40.f));
    followToggler->setCascadeOpacityEnabled(true);
    followToggler->setID("toggler");
    setupFollowMenu->addChild(followToggler);

    auto followLabel = CCLabelBMFont::create("Follow", "bigFont.fnt", 260.f);
    followLabel->setPosition(CCPoint(20.f, 40.f));
    followLabel->setAnchorPoint(CCPoint(0.f, 0.5));
    followLabel->setScale(0.5);
    followLabel->setID("title-label");
    setupFollowMenu->addChild(followLabel);

    auto followHint = CCLabelBMFont::create("Sync with a player color channel.", "chatFont.fnt", 260.f);
    followHint->setPosition(CCPoint(20.f, 25.f));
    followHint->setAnchorPoint(CCPoint(0.f, 0.5));
    followHint->setColor(ccc3(255, 255, 0));
    followHint->setScale(0.7);
    setupFollowMenu->addChild(followHint);

    auto followSubMenu = CCMenu::create();
    followSubMenu->setPosition(CCPoint(80.f, 0.f));
    followSubMenu->setContentSize(CCSize(100.f, 20.f));
    followSubMenu->setID("sub-menu");
    setupFollowMenu->addChild(followSubMenu);

    auto arrowLeft = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    arrowLeft->setScale(0.25);
    arrowLeft->setFlipX(true);
    auto btnLeft = CCMenuItemSpriteExtra::create(arrowLeft, this, menu_selector(AdvancedMenu::onSetFollower));
    btnLeft->setPosition(CCPoint(10.f, 10.f));
    btnLeft->setTag(1);    
    followSubMenu->addChild(btnLeft);

    auto arrowRight = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    arrowRight->setScale(0.25);
    auto btnRight = CCMenuItemSpriteExtra::create(arrowRight, this, menu_selector(AdvancedMenu::onSetFollower));
    btnRight->setPosition(CCPoint(90.f, 10.f));
    btnRight->setTag(2);    
    followSubMenu->addChild(btnRight);

    auto followerLabel = CCLabelBMFont::create("Follower", "bigFont.fnt", 80.f);
    followerLabel->setScale(0.4);
    followerLabel->setPosition(CCPoint(50.f, 10.f));
    followerLabel->setID("follower-label");
    followSubMenu->addChild(followerLabel);

    // setup manual
    auto setupManualMenu = CCMenu::create();
    setupManualMenu->setPosition(CCPoint(0.f, 100.f));
    setupManualMenu->setContentSize(CCSize(160.f, 55.f));
    setupManualMenu->setTag(2);
    setupMenu->addChild(setupManualMenu);

    auto manualToggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(AdvancedMenu::onSetMode), 0.5);
    manualToggler->setPosition(CCPoint(10.f, 45.f));
    manualToggler->setID("toggler");
    manualToggler->setCascadeOpacityEnabled(true);
    setupManualMenu->addChild(manualToggler);

    auto manualLabel = CCLabelBMFont::create("Manual", "bigFont.fnt", 260.f);
    manualLabel->setPosition(CCPoint(20.f, 45.f));
    manualLabel->setAnchorPoint(CCPoint(0.f, 0.5));
    manualLabel->setScale(0.5);
    manualLabel->setID("title-label");
    setupManualMenu->addChild(manualLabel);

    auto manualHint = CCLabelBMFont::create("Pick the color manually.", "chatFont.fnt", 260.f);
    manualHint->setPosition(CCPoint(20.f, 30.f));
    manualHint->setAnchorPoint(CCPoint(0.f, 0.5));
    manualHint->setColor(ccc3(255, 255, 0));
    manualHint->setScale(0.7);
    setupManualMenu->addChild(manualHint);

    auto manualSubMenu = CCMenu::create();
    manualSubMenu->setPosition(CCPoint(20.f, 0.f));
    manualSubMenu->setContentSize(CCSize(160.f, 25.f));
    manualSubMenu->setID("sub-menu");
    setupManualMenu->addChild(manualSubMenu);

    auto colorLabel = ColorChannelSprite::create();
    colorLabel->setScale(0.5);
    colorLabel->setID("color-frame");
    auto colorButton = CCMenuItemSpriteExtra::create(colorLabel, this, menu_selector(AdvancedMenu::onPickColor));
    colorButton->setPosition(CCPoint(12.f, 12.f));
    colorButton->setID("color-button");
    manualSubMenu->addChild(colorButton);

    auto pickColorText = CCLabelBMFont::create("<-Pick your color here", "bigFont.fnt", 180.f);
    pickColorText->setScale(0.4);
    pickColorText->setPosition(CCPoint(25.f, 12.f));
    pickColorText->setAnchorPoint(CCPoint(0.f, 0.5));
    manualSubMenu->addChild(pickColorText);

    // apply

    auto applySprite = ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 0.8);
    applySprite->setScale(0.8);
    auto applyBtn = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(AdvancedMenu::onClose));
    applyBtn->setPosition(CCPoint(380.f, 30.f));
    menu->addChild(applyBtn);

    initialize();
    return true;
}

void AdvancedMenu::initialize() {
    auto setupMenu = getChildByID("main-layer")->getChildByID("setup-menu");

    static_cast<CCLabelBMFont*>(setupMenu->getChildByID("setup-title"))
        ->setCString((titles[currentTab<6 ? currentTab/2 : 3] + " - " + items[currentTab<6 ? currentTab%2 : currentTab-4]).c_str());
    float Y = 190.f;
    int height[3] = {0, 20, 25};
    // setup menu config
    for (int i=0; i<3; i++) {
        // set position
        Y -= 40.f;
        setupMenu->getChildByTag(i)->setPosition(CCPoint(0.f, Y-height[i]));
        // set togglers
        static_cast<CCMenuItemToggler*>(setupMenu->getChildByTag(i)->getChildByID("toggler"))->toggle(i == currentConfig.mode);
        if (i == currentConfig.mode) {
            Y -= height[i];
            // green label color
            static_cast<CCLabelBMFont*>(setupMenu->getChildByTag(i)->getChildByID("title-label"))->setColor(ccc3(0, 255, 0));
            if (i) {
                setupMenu->getChildByTag(i)->getChildByID("sub-menu")->setScale(1);
                static_cast<CCMenu*>(setupMenu->getChildByTag(i)->getChildByID("sub-menu"))->setOpacity(255);
            }
        }
        else  {
            // white label color
            static_cast<CCLabelBMFont*>(setupMenu->getChildByTag(i)->getChildByID("title-label"))->setColor(ccc3(255, 255, 255));
            if (i) {
                setupMenu->getChildByTag(i)->getChildByID("sub-menu")->setScale(0);
                static_cast<CCMenu*>(setupMenu->getChildByTag(i)->getChildByID("sub-menu"))->setOpacity(0);
            }
        }
    }
    // set follower label
    static_cast<CCLabelBMFont*>(setupMenu->getChildByTag(1)->getChildByID("sub-menu")->getChildByID("follower-label"))->setCString(colors[currentConfig.follower]);
    // set manual color frame
    static_cast<ColorChannelSprite*>(setupMenu->getChildByTag(2)->getChildByID("sub-menu")->getChildByID("color-button")->getChildByID("color-frame"))->setColor(currentConfig.color);
}

void AdvancedMenu::onSwitchTab(CCObject* sender) {
    int tag = sender->getTag();
    if (tag != currentTab) {
        Mod::get()->setSavedValue(tabs[currentTab], currentConfig);
        auto ml = getChildByID("main-layer");
        // white the selected tab
        static_cast<CCLabelBMFont*>(ml->getChildByID("items-menu")->getChildByTag(currentTab)->getChildren()->objectAtIndex(0))->runAction(CCTintTo::create(0.3, 255, 255, 255));
        // switch to current config
        currentTab = tag;
        currentConfig = Mod::get()->getSavedValue<BarColor>(
            tabs[currentTab],
            tabs[currentTab].find("practice") ? def2 : def1
        );
        // green the new tab
        static_cast<CCLabelBMFont*>(ml->getChildByID("items-menu")->getChildByTag(currentTab)->getChildren()->objectAtIndex(0))->runAction(CCTintTo::create(0.3, 0, 255, 0));
        //setup menu
        auto seq = CCSequence::create(
            CCFadeOut::create(0.15),
            CCCallFunc::create(this, callfunc_selector(AdvancedMenu::initialize)),// this function has no arg
            CCFadeIn::create(0.15),
            nullptr
        );
        ml->getChildByID("setup-menu")->runAction(seq);
    }
}

void AdvancedMenu::onSetMode(CCObject* sender) {
    // new selected
    int newmode = static_cast<CCNode*>(sender)->getParent()->getTag();

    auto setupMenu = getChildByID("main-layer")->getChildByID("setup-menu");
    float Y = 190.f;
    int height[3] = {0, 20, 25};
    // setup menu config
    for (int i=0; i<3; i++) {
        // switch toggle
        static_cast<CCMenuItemToggler*>(setupMenu->getChildByTag(i)->getChildByID("toggler"))->toggle(false);
        if (newmode != currentConfig.mode) {
            Y -= 40.f;
            setupMenu->getChildByTag(i)->runAction(CCMoveTo::create(0.15, CCPoint(0.f, Y-height[i])));//->setPosition(CCPoint(0.f, Y));
            if (i == newmode) {
                Y -= height[i];            
                static_cast<CCLabelBMFont*>(setupMenu->getChildByTag(i)->getChildByID("title-label"))->runAction(CCTintTo::create(0.15, 0, 255, 0));
                if (i) {
                    setupMenu->getChildByTag(i)->getChildByID("sub-menu")->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.15, 1, 1)));//->setVisible(false);
                    setupMenu->getChildByTag(i)->getChildByID("sub-menu")->runAction(CCEaseExponentialOut::create(CCFadeIn::create(0.15)));
                }
            }        
            if (i == currentConfig.mode) {
                static_cast<CCLabelBMFont*>(setupMenu->getChildByTag(i)->getChildByID("title-label"))->runAction(CCTintTo::create(0.15, 127, 127, 127));//->setColor(ccc3(0, 255, 0));
                if (i) {
                    setupMenu->getChildByTag(i)->getChildByID("sub-menu")->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.15, 1, 0)));//->setVisible(false);
                    setupMenu->getChildByTag(i)->getChildByID("sub-menu")->runAction(CCEaseExponentialOut::create(CCFadeOut::create(0.15)));                    
                }
            }   
        }
    }
    // edit members
    currentConfig.mode = newmode;
}

void AdvancedMenu::onSetFollower(CCObject* sender) {
    int tag = sender->getTag();
    currentConfig.follower = (currentConfig.follower + tag*2) % 3;
    auto ter = static_cast<CCLabelBMFont*>(getChildByID("main-layer")->getChildByID("setup-menu")->getChildByTag(1)->getChildByID("sub-menu")->getChildByID("follower-label"));
    static_cast<CCLabelBMFont*>(getChildByID("main-layer")->getChildByID("setup-menu")->getChildByTag(1)->getChildByID("sub-menu")->getChildByID("follower-label"))
        ->setCString(colors[currentConfig.follower]);
}

void AdvancedMenu::onPickColor(CCObject* sender) {
    //auto target = static_cast<ColorChannelSprite*>(static_cast<CCSprite*>(sender)->getChildByID("color-frame"));    
    auto popup = ColorPickPopup::create(currentConfig.color);
    //popup->setColorTarget(target);
    popup->setDelegate(this);
    popup->show();
}

void AdvancedMenu::updateColor(ccColor4B const& color) {
    currentConfig.color = to3B(color);
    static_cast<ColorChannelSprite*>(
        getChildByID("main-layer")
        ->getChildByID("setup-menu")
        ->getChildByTag(2)
        ->getChildByID("sub-menu")
        ->getChildByID("color-button")
        ->getChildByID("color-frame")
        )->setColor(to3B(color));
}

void AdvancedMenu::onClose(CCObject* sender) {
    Mod::get()->setSavedValue(tabs[currentTab], currentConfig);
    Popup::onClose(sender);
}
