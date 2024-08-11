#include "head.hpp"

BarColor def1 = BarColor{.mode = 0, .follower = 0, .color=ccColor3B(0, 255, 0)};
BarColor def2 = BarColor{.mode = 0, .follower = 0, .color=ccColor3B(255, 255, 0)};

SettingNode* BarColorValue::createNode(float width) {
    return BarColorNode::create(this, width);
};

ccColor3B BarColorNode::getActualColor() {
	auto gm = GameManager::sharedState();
	ccColor3B c;	
	switch(m_current.mode){
    case 0:
        c =  ccColor3B(0, 255, 255*(m_value->getKey().find("practice") != std::string::npos));
        break;
	case 1:
		switch(m_current.follower){
			case 0:
				c = gm->colorForIdx(gm->getPlayerColor());
				break;
			case 1:
				c = gm->colorForIdx(gm->getPlayerColor2());
				break;
			case 2:
				c = gm->colorForIdx(gm->getPlayerGlowColor());
				break;
			default:
				break;

		}
        break;
	case 2:
		c = m_current.color;
        break;
	}
    return c;
}

bool BarColorNode::init(BarColorValue* value, float width) {
    if (!SettingNode::init(value))
        return false;

    setContentSize(CCSize(width, 90.f));
    m_current = value->getVal();
    setZOrder(-1);
    // title
    auto title = CCLabelBMFont::create(
        //m_value->getKey().c_str(),
        Mod::get()->getMetadata().getRawJSON()["settings"][m_value->getKey()]["name"].as_string().c_str(),
        "goldFont.fnt"
    );//m_value->getDefiniti().getDisplayName()
    title->setScale(0.8);
    title->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    title->setPosition(CCPoint(width/2, 75.f));
    addChild(title);
    // bg
    auto bg = CCScale9Sprite::create("squareBG.png"_spr, CCRect(0.f, 0.f, 80.f, 80.f));
    bg->setContentSize(CCSize(2*width-20.f, 160.f));
    bg->setScale(0.5);
    bg->setPosition(CCPoint(width/2, 45.f));
    bg->setID("bg");
    bg->setColor(getActualColor());
    bg->setOpacity(128);
    bg->setZOrder(-1);
    addChild(bg);

    // plz set the apply button higher touch priority geode team 
    //auto layer = CCLayer::create();
    //layer->setTouchPriority(3);
    //layer->setID("node-menu");
    //addChild(layer);

    // modeMenu
    auto modeMenu = CCMenu::create();
    modeMenu->setPosition(CCPoint(0.f, 5.f));
    modeMenu->setContentSize(CCSize(width, 70.f));
    modeMenu->setID("mode-switch");

    for (int i=0; i<3; i++){
        // label
        auto label = CCLabelBMFont::create(mode[i], "bigFont.fnt");
        label->setScale(0.5);
        label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

        // button
        auto button = CCMenuItemSpriteExtra::create(label, this, menu_selector(BarColorNode::onModeSwitch));
        button->setPosition(CCPoint(width/2 + 100*(i-1), 40.f));
        button->setTag(i);
        modeMenu->addChild(button);
    }

    auto modebg = CCScale9Sprite::create("squareBG.png"_spr, CCRect(0.f, 0.f, 80.f, 80.f));
    //modebg->setColor(ccColor3B(0.f, 0.f, 0.f));
    modebg->setOpacity(128);
    modebg->setContentSize(CCSize(160.f, 40.f));
    modebg->setScale(0.5);
    modebg->setPosition(CCPoint(width/2 + 100*(m_current.mode-1), 40.f));
    modebg->setZOrder(-1);
    modebg->setID("bg");
    modeMenu->addChild(modebg);

    auto descImg = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    descImg->setScale(0.6);
    auto descBtn = CCMenuItemSpriteExtra::create(descImg, this, menu_selector(BarColorNode::onDescription));
    descBtn->setPosition(CCPoint(width - 20.f, 70.f));
    modeMenu->addChild(descBtn);
    addChild(modeMenu);

    // subMenu
    auto subMenu = CCMenu::create();
    subMenu->setID("sub-options");

    auto defNode = CCNode::create();
    defNode->setTag(0);
    const char* defText = m_value->getKey().find("default") != std::string::npos ?
        "This mod does nothing here by default." : "Following common option above.";
    //common ? "As if this mod is not installed at all." : "Following common option above.";

    auto defLabel = CCLabelBMFont::create(defText, "bigFont.fnt");
    defLabel->setScale(0.4);
    defLabel->setPosition(CCPoint(width/2, 15.f));
    defLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    defNode->addChild(defLabel);

    subMenu->addChild(defNode);
    
    auto followNode = CCNode::create();
    followNode->setTag(1);
    auto followLabel = CCLabelBMFont::create("Channel:", "bigFont.fnt");
    followLabel->setScale(0.4);
    //followLabel->setAnchorPoint(CCPoint(0.f, 0.5));
    followLabel->setPosition(CCPoint(40.f, 15.f));
    followNode->addChild(followLabel);

    auto syncMenu = CCMenu::create();
    syncMenu->setID("sync-switch");
    syncMenu->setPosition(CCPoint(10.f, 0.f));
    for (int i=0; i<3; i++){
        auto label = CCLabelBMFont::create(follower[i], "bigFont.fnt");
        label->setScale(0.4);
        label->setContentWidth(120.f);
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        label->setPosition(CCPoint(width/2 + 20 + 90*(i-1), 15.f));
        label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        followNode->addChild(label);

        auto button = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(BarColorNode::onSyncSwitch), 0.5);
        button->setPosition(CCPoint(width/2 + 90*(i-1), 15.f));
        button->setTag(i);
        button->toggle(i == m_current.follower);
        syncMenu->addChild(button);
    }
    followNode->addChild(syncMenu);

    subMenu->addChild(followNode);

    auto manualNode = CCNode::create();
    manualNode->setTag(2);
    auto manualLabel = CCLabelBMFont::create("Click this to pick a color by your own", "bigFont.fnt");
    manualLabel->setScale(0.4);
    manualLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

    auto manualMenu = CCMenu::create();
    manualMenu->setPosition(CCPoint(0.f, 0.f));
    manualNode->addChild(manualMenu);
    auto manualButton = CCMenuItemSpriteExtra::create(manualLabel, this, menu_selector(BarColorNode::onPickColor));
    manualButton->setPosition(CCPoint(width/2, 15.f));    
    manualMenu->addChild(manualButton);

    subMenu->addChild(manualNode);
    // subMenu config
    subMenu->setContentSize(CCSize(width, 60.f));
    subMenu->setPosition(CCPoint(0.f, 5.f));
    addChild(subMenu);

    changeVisibleState();
    return true;
}

void BarColorNode::changeVisibleState() {
    // visible state
    for (auto* obj: CCArrayExt<CCObject*>(getChildByID("sub-options")->getChildren())) {
        static_cast<CCNode*>(obj)->setVisible(obj->getTag() == m_current.mode);
    }
}

void BarColorNode::onModeSwitch(CCObject *sender) {
    if (m_current.mode != sender->getTag()){
        m_current.mode = sender->getTag();

        auto frame = getChildByID("mode-switch")->getChildByID("bg");
        auto action = CCMoveTo::create(0.2, static_cast<CCNode*>(sender)->getPosition());
        auto easeout = CCEaseOut::create(action, 0.5);
        frame->runAction(easeout);

        auto c = getActualColor();
        auto tint = CCTintTo::create(0.2, c.r, c.g, c.b);
        static_cast<CCSprite*>(getChildByID("bg"))->runAction(tint);
        changeVisibleState();
    }
    dispatchChanged();
}

void BarColorNode::onSyncSwitch(CCObject *sender) {
    m_current.follower = sender->getTag();
    auto c = getActualColor();
    auto tint = CCTintTo::create(0.2, c.r, c.g, c.b);
    static_cast<CCSprite*>(getChildByID("bg"))->runAction(tint);

    for (auto* obj: CCArrayExt<CCObject*>(getChildByID("sub-options")->getChildByTag(1)->getChildByID("sync-switch")->getChildren())) {
        static_cast<CCMenuItemToggler*>(obj)->toggle(false);
    }
    dispatchChanged();
}

void BarColorNode::setColor(ccColor4B const &c) {
    m_current.color = ccColor3B(c.r, c.g, c.b);

    auto tint = CCTintTo::create(0.2, c.r, c.g, c.b);
    static_cast<CCSprite*>(getChildByID("bg"))->runAction(tint);

    dispatchChanged();
}

void BarColorNode::onPickColor(CCObject *sender) {
    auto color = static_cast<CCSprite*>(getChildByID("bg"))->getColor();
    auto popup = MyColorPickPopup::create(ccColor4B(color.r, color.g, color.b, 255), false, this);
    
    //popup->setColorTarget(static_cast<CCSprite*>(static_cast<CCNode*>(sender)->getChildren()->objectAtIndex(0)));
    log::info("{}", static_cast<CCLabelBMFont*>(static_cast<CCNode*>(sender)->getChildren()->objectAtIndex(0))->getString());
    popup->show();
    dispatchChanged();
}


void BarColorNode::onDescription(CCObject *sender) {
    auto js = Mod::get()->getMetadata().getRawJSON()["settings"][m_value->getKey()];
    FLAlertLayer::create(
        js["name"].as_string().c_str(),
        js["description"].as_string().c_str(),
        "OK"
    )->show();
}

bool BarColorNode::hasNonDefaultValue() {
    auto def = m_value->getKey().find("practice") != std::string::npos ? def2 : def1;
    return !(m_current == def);
}

void BarColorNode::resetToDefault() {
    m_current = m_value->getKey().find("practice") != std::string::npos ? def2 : def1;
    // move mode switch
    auto frame = getChildByID("mode-switch")->getChildByID("bg");
    auto action = CCMoveTo::create(0.2, static_cast<CCNode*>(getChildByID("mode-switch")->getChildByTag(m_current.mode))->getPosition());
    auto easeout = CCEaseOut::create(action, 0.5);
    frame->runAction(easeout);
    // set visible
    changeVisibleState();
    // checkbox
    for (auto* obj: CCArrayExt<CCObject*>(getChildByID("sub-options")->getChildByTag(1)->getChildByID("sync-switch")->getChildren())){
        static_cast<CCMenuItemToggler*>(obj)->toggle(obj->getTag() == m_current.follower);        
    }
    // tint bg
    auto c = getActualColor();
    auto tint = CCTintTo::create(0.2, c.r, c.g, c.b);
    static_cast<CCSprite*>(getChildByID("bg"))->runAction(tint);
    // dispatch uncommited
    dispatchChanged();
}

ccColor3B convert(matjson::Value value) {
    return ccColor3B(
        GLubyte(value["r"].as_int()),
        GLubyte(value["g"].as_int()),
        GLubyte(value["b"].as_int())
    );
}

$on_mod(Loaded) {
    const char* key_list[12] = {
        "normal-default", "practice-default", 
        "info-menu-normal", "info-menu-practice",
        "pause-menu-normal", "pause-menu-practice", 
        "official-levels-normal", "official-levels-practice",
        "quest-default", "quest-top", "quest-middle", "quest-bottom"
    };

    // register
    for (int index = 0; index < 12; index ++){
        Mod::get()->addCustomSetting<BarColorValue>(
            key_list[index],
            index == 1 || index == 3 || index == 5 || index == 7 ? def2 : def1
        );
        Mod::get()->registerCustomSetting(
            key_list[index],
            std::make_unique<BarColorValue>(
                key_list[index],
                Mod::get()->getID(),
                index == 1 || index == 3 || index == 5 || index == 7 ? def2 : def1
            )
        );
    }

    auto set = Mod::get()->getSavedSettingsData();
    if (!Mod::get()->getSavedValue<bool>("mention") && set.contains("official-level")) {
        Mod::get()->setSavedValue("mention", true);

        // normal
        auto c1 = convert(set["normal-manual"]);
        bool p1 = (c1 == def1.color);
        if (int n = set["normal-follow"].as_int()){
            Mod::get()->setSettingValue(
                "normal-default",
                BarColor{.mode = 1, .follower = n-1, .color = c1}
            );
        } else {
            Mod::get()->setSettingValue(
                "normal-default",
                BarColor{.mode = 2-2*p1, .follower = 0, .color = c1}
            );            
        }

        // practice
        auto c2 = convert(set["practice-manual"]);
        bool p2 = c2 == def2.color;
        if (int p = set["practice-follow"].as_int()){
            Mod::get()->setSettingValue(
                "practice-default",
                BarColor{.mode = 1, .follower = p-1, .color = c2}
            );            
        } else {
            Mod::get()->setSettingValue(
                "practice-default",
                BarColor{.mode = 2-2*p2, .follower = 0, .color = c2}
            );
        }

        // toggle
        if (!set["level-info-layer"].as_bool()) {
            Mod::get()->setSettingValue(
                "info-menu-normal",
                BarColor{.mode = 2, .follower = 0, .color = def1.color}
            ); 
            Mod::get()->setSettingValue(
                "info-menu-practice",
                BarColor{.mode = 2, .follower = 0, .color = def2.color}
            ); 
        }
        if (!set["pause-layer"].as_bool()) {
            Mod::get()->setSettingValue(
                "pause-menu-normal",
                BarColor{.mode = 2, .follower = 0, .color = def1.color}
            ); 
            Mod::get()->setSettingValue(
                "pause-menu-practice",
                BarColor{.mode = 2, .follower = 0, .color = def2.color}
            ); 
        }
        if (!set["official-level"].as_bool()) {
            Mod::get()->setSettingValue(
                "official-levels-normal",
                BarColor{.mode = 2, .follower = 0, .color = def1.color}
            ); 
            Mod::get()->setSettingValue(
                "official-levels-practice",
                BarColor{.mode = 2, .follower = 0, .color = def2.color}
            ); 
        }
    }
}