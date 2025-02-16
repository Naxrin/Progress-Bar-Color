#include "head.hpp"

bool ModeCell::setup() {

    this->setContentSize(CCPoint(240.f, 24.f));
    this->setPositionX(0.f);
    this->setID("title");

    this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(ModeCell::onToggle), 0.5);
    m_toggler->setPosition(CCPoint(10.f, 12.f));
    m_toggler->setID("toggler");
    m_toggler->setCascadeOpacityEnabled(true);
    this->addChild(m_toggler);

    this->m_label = CCLabelBMFont::create("title", "bigFont.fnt", 260.f);
    m_label->setPosition(CCPoint(20.f, 12.f));
    m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    m_label->setColor(ccc3(127, 127, 127));
    m_label->setScale(0.5);
    m_label->setID("label");
    this->addChild(m_label);

    return true;
}

bool FollowerCell::setup() {

    this->setContentSize(CCPoint(240.f, 20.f));

    auto sprArwL = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    sprArwL->setScale(0.25);
    sprArwL->setFlipX(true);

    auto btnArwL = CCMenuItemSpriteExtra::create(sprArwL, this, menu_selector(FollowerCell::onArrow));
    btnArwL->setPosition(CCPoint(10.f, 10.f));
    btnArwL->setTag(1);    
    this->addChild(btnArwL);

    auto sprArwR = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    sprArwR->setScale(0.25);

    auto btnArwR = CCMenuItemSpriteExtra::create(sprArwR, this, menu_selector(FollowerCell::onArrow));
    btnArwR->setPosition(CCPoint(190.f, 10.f));
    btnArwR->setTag(2);
    this->addChild(btnArwR);

    this->m_label = CCLabelBMFont::create("follower", "bigFont.fnt", 170.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_label->setPosition(CCPoint(100.f, 10.f));
    m_label->setScale(0.5);
    m_label->setID("label");
    this->addChild(m_label);

    return true;
}

bool ColorCell::setup() {

    this->setContentSize(CCPoint(240.f, 20.f));

    auto spr = ColorChannelSprite::create();
    spr->setScale(0.5);
    spr->setID("color");
    this->m_btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ColorCell::onButton));
    m_btn->setPosition(CCPoint(10.f, 10.f));
    m_btn->setID("color-button");
    this->addChild(m_btn);

    auto label = CCLabelBMFont::create("<-Pick your color here", "bigFont.fnt", 180.f);
    label->setScale(0.4);
    label->setPosition(CCPoint(25.f, 10.f));
    label->setAnchorPoint(CCPoint(0.f, 0.5));
    this->addChild(label);

    return true;
}

bool MultiColorCell::setup() {

    this->setContentSize(CCPoint(240.f, 20.f));

    auto sprZero = ColorChannelSprite::create();
    sprZero->setScale(0.5);

    this->m_btnZero = CCMenuItemSpriteExtra::create(sprZero, this, menu_selector(MultiColorCell::onButton));
    m_btnZero->setPosition(CCPoint(10.f, 10.f));
    m_btnZero->setID("zero");
    this->addChild(m_btnZero);

    auto lbfZero = CCLabelBMFont::create("Initial", "bigFont.fnt", 70.f);
    lbfZero->setScale(0.4);
    lbfZero->setPosition(CCPoint(25.f, 10.f));
    lbfZero->setAnchorPoint(CCPoint(0.f, 0.5));
    this->addChild(lbfZero);

    auto sprHdrd = ColorChannelSprite::create();
    sprHdrd->setScale(0.5);

    this->m_btnHdrd = CCMenuItemSpriteExtra::create(sprHdrd, this, menu_selector(MultiColorCell::onButton));
    m_btnHdrd->setPosition(CCPoint(110.f, 10.f));
    m_btnHdrd->setID("hdrd");
    this->addChild(m_btnHdrd);

    auto lbfHdrd = CCLabelBMFont::create("Terminal", "bigFont.fnt", 70.f);
    lbfHdrd->setScale(0.4);
    lbfHdrd->setPosition(CCPoint(125.f, 10.f));
    lbfHdrd->setAnchorPoint(CCPoint(0.f, 0.5));
    this->addChild(lbfHdrd);

    return true;
}

/*
// preserved
bool ToggleCell::setup() {

    this->setContentSize(CCPoint(240.f, 20.f));
    this->setID("instant");

    this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(ToggleCell::onToggle), 0.5);
    m_toggler->setPosition(CCPoint(10.f, 10.f));
    m_toggler->setID("toggler");
    m_toggler->setCascadeOpacityEnabled(true);
    this->addChild(m_toggler);

    auto label = CCLabelBMFont::create("New Best Mode", "bigFont.fnt", 200.f);
    label->setPosition(CCPoint(25.f, 10.f));
    label->setAnchorPoint(CCPoint(0.f, 0.5));
    label->setScale(0.4);
    label->setID("label");
    this->addChild(label);

    return true;
}
*/

SettingNodeV3* AdvancedSetting::createNode(float width) {
    return AdvancedSettingNode::create(
        std::static_pointer_cast<AdvancedSetting>(shared_from_this()),
        width
    );
}

inline CCLabelBMFont* createHint(const char* name) {
    auto label = CCLabelBMFont::create(name, "chatFont.fnt", 240, CCTextAlignment::kCCTextAlignmentLeft);
    label->setPositionX(10.f);
    label->setScale(0.7);
    label->setColor(ccc3(255, 255, 0));
    label->setAnchorPoint(CCPoint(0.f, 0.f));
    label->setID("hint");
    return label;
}

bool AdvancedMenu::setup() {
    this->setTitle("Advanced Setup");

    // current tab
    this->m_tab = Mod::get()->getSavedValue<int64_t>("current-tab", 0);
    // load current config
    this->m_currentConfig = Mod::get()->getSavedValue<BarColor>(
        tabs[m_tab],
        tabs[m_tab].find("practice") ? def2 : def1
    );

    // division line
    auto lrcDivLine = CCLayerColor::create(ccc4(255, 255, 255, 192));
    lrcDivLine->setPosition(CCPoint(125.f, 16.f));
    lrcDivLine->setContentSize(CCSize(2.f, 230.f));
    m_mainLayer->addChild(lrcDivLine);

    // item switch menu
    this->m_menuItems = CCMenu::create();
    m_menuItems->setPosition(CCPoint(15.f, 10.f));
    m_menuItems->setContentSize(CCSize(100.f, 240.f));
    m_menuItems->setID("items-menu");
    m_mainLayer->addChild(m_menuItems);

    // Item Switch Menu GoldFont
    for (int i=0; i<4; i++) {
        auto title = CCLabelBMFont::create(titles[i].c_str(), "goldFont.fnt", 140.f);
        title->setPosition(CCPoint(50.f, 230.f-i*54.f));
        title->setScale(0.6);
        m_menuItems->addChild(title);
    }

    // Tab Switch Button BigFont
    for (int i=0; i<9; i++) {
        auto label = CCLabelBMFont::create(items[i<6 ? i%2 : i-4].c_str(), "bigFont.fnt", 140.f);
        label->setScale(0.5);
        
        auto button = CCMenuItemSpriteExtra::create(label, this, menu_selector(AdvancedMenu::onSwitchTab));
        button->setPosition(CCPoint(50.f, i<6 ? 213.f-(i/2)*54.f-(i%2)*17.f : 153-i*17.f));
        button->setColor(i == m_tab ? ccc3(0, 255, 0) : ccc3(127, 127, 127));
        label->setTag(i);
        button->setTag(i);
        m_menuItems->addChild(button);
    }

    // Setup Workspace
    this->m_menuSetup = CCMenu::create();
    m_menuSetup->setPosition(CCPoint(130.f, 40.f));
    m_menuSetup->setContentSize(CCSize(240.f, 180.f));
    m_menuSetup->setID("setup-menu");
    m_mainLayer->addChild(m_menuSetup);

    // workspace title
    // tell users what they are currently setting for
    auto lbfSetupTitle = CCLabelBMFont::create("?", "goldFont.fnt", 260.f);
    lbfSetupTitle->setPosition(CCPoint(135.f, 235.f));
    lbfSetupTitle->setAnchorPoint(CCPoint(0.f, 0.6));
    lbfSetupTitle->setScale(0.6);
    lbfSetupTitle->setID("setup-title");
    m_mainLayer->addChild(lbfSetupTitle);
    
    auto cellDefault = ModeCell::create("Default");
    cellDefault->setTag(0);
    m_menuSetup->addChild(cellDefault);

    auto lbfDefault = createHint("Apply common setup in geode settings menu.");
    lbfDefault->setTag(0);
    m_menuSetup->addChild(lbfDefault);

    auto cellFollow = ModeCell::create("Follow");
    cellFollow->setTag(1);
    m_menuSetup->addChild(cellFollow);

    auto lbfFollow = createHint("Paint the progress bar here following a player's picked color");
    lbfFollow->setTag(1);
    m_menuSetup->addChild(lbfFollow);

    auto cellFollowTarget = FollowerCell::create();
    cellFollowTarget->setTag(1);
    m_menuSetup->addChild(cellFollowTarget);

    auto cellManual = ModeCell::create("Manual");
    cellManual->setTag(2);
    m_menuSetup->addChild(cellManual);

    auto lbfManual = createHint("Pick the best color by your own.");
    lbfManual->setTag(2);
    m_menuSetup->addChild(lbfManual);

    auto cellManualColor = ColorCell::create();
    cellManualColor->setTag(2);
    m_menuSetup->addChild(cellManualColor);

    auto cellProgress = ModeCell::create("Progress");
    cellProgress->setTag(3);
    m_menuSetup->addChild(cellProgress);

    auto lbfProgress = createHint("Tint regarding progress percentage.");
    lbfProgress->setTag(3);
    m_menuSetup->addChild(lbfProgress);

    auto cellProgressColor = MultiColorCell::create();
    cellProgressColor->setTag(3);
    m_menuSetup->addChild(cellProgressColor);

    auto cellRandom = ModeCell::create("Random");
    cellRandom->setTag(4);
    m_menuSetup->addChild(cellRandom);

    auto lbfRandom = createHint("Experimental (won't crash though)");
    lbfRandom->setTag(4);
    m_menuSetup->addChild(lbfRandom);

    // apply
    auto sprApply = ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 0.8);
    sprApply->setScale(0.8);
    auto btnApply = CCMenuItemSpriteExtra::create(sprApply, this, menu_selector(AdvancedMenu::onClose));
    btnApply->setPosition(CCPoint(380.f, 30.f));
    m_mainLayer->getChildByType<CCMenu>(0)->addChild(btnApply);

    initialize();
    return true;
}

void AdvancedMenu::initialize() {
    // title
    static_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("setup-title"))
        ->setCString((titles[m_tab<6 ? m_tab/2 : 3] + " - " + items[m_tab<6 ? m_tab%2 : m_tab-4]).c_str());

    float Y = 180.f;
    float h;
    // setup menu config
    for (auto node : CCArrayExt<CCNode*>(this->m_menuSetup->getChildren())) {
        // set ui status regarding current value
        if (node->getID() != "hint")
            static_cast<SetupCell*>(node)->setVal(m_currentConfig);

        // show
        if ((node->getTag() == m_currentConfig.mode || node->getID() == "title") && (this->getID() != "instant" || tabs[m_tab].find("pause"))) {
            node->setScaleY(node->getID() == "hint" ? 0.7 : 1);
            Y -= node->getScaledContentHeight();            
        } else
            node->setScaleY(0.f);
        node->setPositionY(Y);
    }
}

void AdvancedMenu::switchMode(int mode) {
    // nonsense
    if (mode == m_currentConfig.mode) {
        static_cast<ModeCell*>(this->m_menuSetup->getChildByTag(mode))->toggle();
        return;        
    }


    float Y = 180.f;
    float h;
    bool display;
    // setup menu config
    for (auto node : CCArrayExt<CCNode*>(this->m_menuSetup->getChildren())) {
        h = node->getID() == "hint" ? 0.7 : 1;
        // title
        if (node->getID() == "title") {
            display = true;
            if (node->getTag() == mode)
                static_cast<ModeCell*>(node)->toggle();
            else if (node->getTag() == m_currentConfig.mode)
                static_cast<ModeCell*>(node)->toggle();
        } else
            display = node->getTag() == mode && (this->getID() != "instant" || tabs[m_tab].find("pause"));

        if (node->getTag() == mode && node->getID() != "title")
            node->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.2, h, h)));
        if (node->getTag() == m_currentConfig.mode  && node->getID() != "title")
            node->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.2, h, 0)));
            
        // decrease height
        if (display)
            Y -= node->getContentHeight();
        // move node
        node->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.2, CCPoint(node->getID() == "title" ? 0.f : 10.f, Y))));
    }
    m_currentConfig.mode = mode;
}

void AdvancedMenu::onSwitchTab(CCObject* sender) {
    int tag = sender->getTag(); 
    // same tab
    if (tag == this->m_tab)
        return;

    // dump
    Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);

    // white the selected tab
    m_menuItems->getChildByTag(m_tab)->runAction(CCTintTo::create(0.3, 127, 127, 127));

    // switch to current config
    this->m_tab = tag;
    this->m_currentConfig = Mod::get()->getSavedValue<BarColor>(
        tabs[m_tab],
        tabs[m_tab].find("practice") ? def2 : def1
    );
    // green the new tab
    m_menuItems->getChildByTag(m_tab)->runAction(CCTintTo::create(0.3, 0, 255, 0));
    // setup menu
    this->m_menuSetup->runAction(CCSequence::create(
        CCFadeOut::create(0.15),
        CCCallFunc::create(this, callfunc_selector(AdvancedMenu::initialize)),
        CCFadeIn::create(0.15),
        nullptr
    ));
}

void AdvancedMenu::onClose(CCObject* sender) {
    Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
    Popup::onClose(sender);
}
