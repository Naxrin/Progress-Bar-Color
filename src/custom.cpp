#include "Geode/loader/Event.hpp"
#include "ccTypes.h"
#include "head.hpp"

std::set<std::string> have_cache;



bool C4Button::init(CCObject* target, SEL_MenuHandler callback) {
    this->spr = ColorChannelSprite::create();
    this->spr->setScale(0.5);

    if (!CCMenuItemSpriteExtra::init(spr, spr, target, callback))
        return false;

    this->label = CCLabelBMFont::create("alpha", "bigFont.fnt");
    this->label->setScale(0.3);
    this->addChildAtPosition(this->label, Anchor::Center);
    return true;
}

bool ModeCell::init(const char* name) {                                               
    if (!SetupCell::init())
        return false;

    this->setContentHeight(24.f);
    this->setPositionX(0.f);
    this->setID("title");

    this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(ModeCell::onToggle), 0.5);
    m_toggler->setPosition(ccp(10.f, 12.f));
    m_toggler->setID("toggler");
    m_toggler->setCascadeOpacityEnabled(true);
    this->addChild(m_toggler);

    this->m_label = CCLabelBMFont::create(name, "bigFont.fnt", 260.f);
    m_label->setPosition(ccp(20.f, 12.f));
    m_label->setAnchorPoint(ccp(0.f, 0.5));
    m_label->setColor(ccc3(127, 127, 127));
    m_label->setScale(0.5);
    m_label->setID("label");
    this->addChild(m_label);

    return true;
}

bool EnumCell::init(bool type) {
    if (!SetupCell::init())
        return false;

    this->type = type;

    auto sprArwL = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    sprArwL->setScale(0.4f);

    auto btnArwL = CCMenuItemSpriteExtra::create(sprArwL, this, menu_selector(EnumCell::onArrow));
    btnArwL->setPosition(ccp(15.f, 10.f));
    //btnArwL->setScaleX(2);
    btnArwL->setTag(1);
    
    this->addChild(btnArwL);

    auto sprArwR = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    sprArwR->setScale(0.4f);
    sprArwR->setFlipX(true);

    auto btnArwR = CCMenuItemSpriteExtra::create(sprArwR, this, menu_selector(EnumCell::onArrow));
    btnArwR->setPosition(ccp(185.f, 10.f));
    //btnArwR->setScaleX(2);
    btnArwR->setTag(2);
    this->addChild(btnArwR);

    this->m_label = CCLabelBMFont::create("-", "bigFont.fnt", 170.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_label->setPosition(ccp(100.f, 10.f));
    m_label->setScale(0.5);
    m_label->setID("label");
    this->addChild(m_label);

    return true;
}

bool ColorCell::init() {
    if (!SetupCell::init())
        return false;

    this->m_btn = C4Button::create(this, menu_selector(ColorCell::onButton));
    m_btn->setPosition(ccp(10.f, 10.f));
    m_btn->setID("color-button");
    this->addChild(m_btn);

    auto label = CCLabelBMFont::create("<-Pick your color here", "bigFont.fnt", 180.f);
    label->setScale(0.4);
    label->setPosition(ccp(25.f, 10.f));
    label->setAnchorPoint(ccp(0.f, 0.5));
    this->addChild(label);

    return true;
}

bool SliderInputCell::init(Signal target, const char* text) {
    if (!SetupCell::init())
        return false;

    this->target = target;

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setPosition(ccp(0.f, 10.f));
    label->setAnchorPoint(ccp(0.f, 0.5f));
    label->setScale(0.4f);
    this->addChild(label);

    this->m_inputer = TextInput::create(60.f, text);
    this->m_inputer->setPosition(ccp(110.f, 10.f));
    this->m_inputer->setScale(0.65f);
    this->m_inputer->setFilter(fmt::format("{}{}0123456789", target == Signal::Speed || target == Signal::Length ? "-" : "",
        target == Signal::Length || target == Signal::Speed ? "." : ""));
    this->m_inputer->setDelegate(this);
    this->addChild(this->m_inputer);

    this->m_slider = Slider::create(this, menu_selector(SliderInputCell::onSlider));
    this->m_slider->setPosition(ccp(200.f, 10.f));
    this->m_slider->setAnchorPoint(ccp(0.f, 0.f));
    this->m_slider->setScale(0.4f);
    this->m_slider->m_delegate = this;
    this->addChild(m_slider);

    return true;
}

void SliderInputCell::onSlider(CCObject* sender) {
    float s = this->m_slider->getValue();
    switch (this->target) {
    case Signal::Length:
        this->val = 2 * s - 1;
        this->m_inputer->setString(numToString<float>(this->val, 2));
        break;
    case Signal::Speed:
        this->val = 10 * s - 5;
        this->m_inputer->setString(numToString<float>(this->val, 2));
        break;
    case Signal::Phase:
        this->val = int(360 * s);
        this->m_inputer->setString(numToString<int>(this->val));
        break;
    case Signal::Satu:
        this->val = int(100 * s);
        this->m_inputer->setString(numToString<int>(this->val));
        break;
    case Signal::Brit:    
        this->val = int(100 * s);
        this->m_inputer->setString(numToString<int>(this->val));
        break;
    default:
        break;
    }
}

void SliderInputCell::sliderBegan(Slider* p) {
    SignalEvent<bool>(Signal::Slider).send(true);
}

void SliderInputCell::sliderEnded(Slider* p) {
    SignalEvent<float>(this->target).send(this->val);
    SignalEvent<bool>(Signal::Slider).send(false);
}

void SliderInputCell::textChanged(CCTextInputNode* p) {
    switch (this->target) {
    case Signal::Length:
        this->val = numFromString<float>(p->getString()).unwrapOrDefault();
        this->m_slider->setValue(val > 1 ? 1 : val < -1 ? 0 : (val + 1.f) / 2.f);
        break;
    case Signal::Speed:
        this->val = numFromString<float>(p->getString()).unwrapOrDefault();
        this->m_slider->setValue(val > 5 ? 1 : val < -5 ? 0 : (val + 5.f) / 10.f);
        break;
    case Signal::Phase:
        this->val = numFromString<int>(p->getString()).unwrapOrDefault();
        this->val = this->val > 360 ? 360 : this->val;
        this->m_slider->setValue(val / 360.f);
        break;
    case Signal::Satu:
        this->val = numFromString<int>(p->getString()).unwrapOr(100);
        this->val = this->val > 100 ? 100 : this->val;
        this->m_slider->setValue(val / 100.f);
        break;
    case Signal::Brit:
        this->val = numFromString<int>(p->getString()).unwrapOr(100);
        this->val = this->val > 100 ? 100 : this->val;
        this->m_slider->setValue(val / 100.f);
        break;
    default:
        return;
    }
    SignalEvent<float>(this->target).send(this->val);
}

void SliderInputCell::setVal(BarColor const &val) {
    switch (this->target) {
    case Signal::Length:
        this->val = val.length;
        this->m_slider->setValue(val.length > 1 ? 1 : val.length < -1 ? 0 : (val.length + 1) / 2);
        this->m_inputer->setString(numToString<float>(val.length, 2));
        return;
    case Signal::Speed:
        this->val = val.speed;
        this->m_slider->setValue(val.speed > 5 ? 1 : val.speed < -5 ? 0 : (val.speed + 5.f) / 10.f);
        this->m_inputer->setString(numToString<float>(val.speed, 2));
        return;
    case Signal::Phase:
        this->val = val.phase;
        this->m_slider->setValue(val.phase / 360.f);
        this->m_inputer->setString(numToString<int>(val.phase));
        return;
    case Signal::Satu:
        this->val = val.satu;
        this->m_slider->setValue(val.satu / 100.f);
        this->m_inputer->setString(numToString<int>(val.satu));
        return;
    case Signal::Brit:
        this->val = val.brit;
        this->m_slider->setValue(val.brit / 100.f);
        this->m_inputer->setString(numToString<int>(val.brit));
        return;
    default:
        return;
    }
}

bool MultiColorCell::init() {
    if (!SetupCell::init())
        return false;

    this->m_btnZero = C4Button::create(this, menu_selector(MultiColorCell::onButton));
    m_btnZero->setPosition(ccp(10.f, 10.f));
    m_btnZero->setID("zero");
    m_btnZero->setTag(1);
    this->addChild(m_btnZero);

    auto lbfZero = CCLabelBMFont::create("#1", "bigFont.fnt", 70.f);
    lbfZero->setScale(0.4);
    lbfZero->setPosition(ccp(25.f, 10.f));
    lbfZero->setAnchorPoint(ccp(0.f, 0.5));
    this->addChild(lbfZero);

    this->m_btnHdrd = C4Button::create(this, menu_selector(MultiColorCell::onButton));
    m_btnHdrd->setPosition(ccp(110.f, 10.f));
    m_btnHdrd->setID("hdrd");
    m_btnHdrd->setTag(2);
    this->addChild(m_btnHdrd);

    auto lbfHdrd = CCLabelBMFont::create("#2", "bigFont.fnt", 70.f);
    lbfHdrd->setScale(0.4);
    lbfHdrd->setPosition(ccp(125.f, 10.f));
    lbfHdrd->setAnchorPoint(ccp(0.f, 0.5));
    this->addChild(lbfHdrd);

    return true;
}

bool ToggleCell::init(bool randa) {
    if (!SetupCell::init())
        return false;

    this->setID(randa ? "randa" : "async");
    this->randa = randa;

    this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(ToggleCell::onToggle), 0.5);
    m_toggler->setPosition(ccp(10.f, 10.f));
    m_toggler->setID("toggler");
    m_toggler->setCascadeOpacityEnabled(true);
    this->addChild(m_toggler);

    auto label = CCLabelBMFont::create(this->randa ? "Random Alpha" : "Async Speed", "bigFont.fnt", 200.f);
    label->setPosition(ccp(25.f, 10.f));
    label->setAnchorPoint(ccp(0.f, 0.5f));
    label->setScale(0.4f);
    label->setID("label");
    this->addChild(label);

    auto spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    spr->setScale(0.6f);
    auto desc = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ToggleCell::onDesc));
    desc->setPosition(ccp(35.f + label->getScaledContentWidth(), 10.f));
    this->addChild(desc);

    return true;
}

bool InputCell::init(bool frag) {
    if (!SetupCell::init())
        return false;

    this->setID(frag ? "frag" : "vert");
    this->frag = frag;

    auto label = CCLabelBMFont::create(this->frag ? "Frag" : "Vert", "bigFont.fnt", 200.f);
    label->setPosition(ccp(0.f, 10.f));
    label->setAnchorPoint(ccp(0.f, 0.5f));
    label->setScale(0.4f);
    label->setID("label");
    this->addChild(label);
    
    this->m_inputer = TextInput::create(320.f, "file name only");
    m_inputer->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789{}`~!@#$%^&*()-_=+[]{};',. \"");
    m_inputer->setPosition(ccp(150.f, 10.f));
    //m_inputer->getInputNode()->getTextLabel()->setPositionX(-140.f);
    //m_inputer->getInputNode()->getTextLabel()->setAnchorPoint(ccp(0.f, 0.5f));
    m_inputer->setScale(0.65f);
    m_inputer->setID("inputer");
    m_inputer->setDelegate(this);
    //log::debug("filder = {} by default", m_inputer->getInputNode()->m_allowedChars);
    this->addChild(m_inputer);

    return true;
}

SettingNodeV3* AdvancedSetting::createNode(float width) {
    return AdvancedSettingNode::create(
        std::static_pointer_cast<AdvancedSetting>(shared_from_this()),
        width
    );
}

bool PreviewBar::init() {
    if (!CCLayer::init())
        return false;

    this->setAnchorPoint(ccp(0.5f, 0.5f));
    this->ignoreAnchorPointForPosition(false);

    // preview sprite
    this->base = CCSprite::create("GJ_progressBar_001.png");
    base->setScale(0.56f);
    this->size = base->getContentSize();
    this->setContentSize(size * 0.56);
    base->setPosition(size * 0.28);
    //base->setPosition(ccp(240.f, 15.f + size.height / 2));
    base->setColor(ccBLACK);
    base->setOpacity(125);
    this->addChild(base);

    this->target = CCSprite::create("GJ_progressBar_001.png");
    target->setAnchorPoint(ccp(0.f, 0.5f));
    target->setScale(0.992);
    target->setScaleY(0.860);
    target->setPosition(ccp(size.width * 0.004, size.height * 0.5));
    this->base->addChild(target);

    this->setTouchEnabled(true);
    this->setTouchMode(ccTouchesMode::kCCTouchesOneByOne);

    this->setProgress(1.f);
    this->scheduleUpdate();
    return true;
}

void PreviewBar::update(float dt) {
    this->delta = fmod(this->delta + dt * Mod::get()->getSettingValue<double>("speed"), 1);
    update_shader(this->target, this->config, this->program, dt, this->delta, this->deltac);
    this->program->updateUniforms();
}

bool PreviewBar::ccTouchBegan(CCTouch *touch, CCEvent* event) {
    auto t = this->convertTouchToNodeSpace(touch);
    auto c = this->getContentSize();
    return t.x >= 0 && t.x <= c.width && t.y >= 0 && t.y <= c.height;
}

void PreviewBar::ccTouchMoved(CCTouch *touch, CCEvent* event) {
    auto x = this->convertTouchToNodeSpace(touch).x;
    auto w = this->getContentWidth();
    x = x > w ? w : (x < 0 ? 0 : x);
    this->setProgress(x / w);
}

void PreviewBar::setProgress(float p) {
    this->progress = 100 * p;
    this->target->setTextureRect(CCRect(0.f, 0.f, p * size.width, size.height));
    if (this->program) {
        this->updateUniform("progress", this->progress);
        //this->program->updateUniforms();
    }

}

void PreviewBar::initShader(std::string advKey, std::string defKey, ccColor3B defCol) {
    CC_SAFE_DELETE(this->program);
    this->program = init_shader(this->target, advKey, defKey, "preview",
        this->config, defCol, this->progress);
    this->delta = 0;
    this->deltac = 0;
}

inline CCLabelBMFont* createHint(const char* name) {
    auto label = CCLabelBMFont::create(name, "chatFont.fnt", 240, CCTextAlignment::kCCTextAlignmentLeft);
    label->setPositionX(10.f);
    label->setScale(0.7);
    label->setColor(ccc3(255, 255, 0));
    label->setAnchorPoint(ccp(0.f, 0.f));
    label->setID("hint");
    return label;
}

bool AdvancedMenu::init() {
    if (!Popup::init(420.f, 280.f))
        return false;

    this->setTitle("Progress Bar Color Setup");
    have_cache.clear();

    // current tab
    this->m_tab = Mod::get()->getSavedValue<int64_t>("current-tab", 0);

    // division line
    auto lrcDivLine = CCLayerColor::create(ccc4(255, 255, 255, 192));
    lrcDivLine->setPosition(ccp(125.f, 16.f));
    lrcDivLine->setContentSize(CCSize(2.f, 230.f));
    m_mainLayer->addChild(lrcDivLine);

    // scroller
    auto scrollerItems = ScrollLayer::create(CCSize(150.f, 240.f));
    scrollerItems->setPosition(ccp(-35.f, 10.f));
    scrollerItems->setID("items-scroller");
    m_mainLayer->addChild(scrollerItems);

    // item switch menu
    this->m_menuItems = CCMenu::create();
    m_menuItems->setPosition(ccp(50.f, 0.f));
    m_menuItems->setID("items-menu");
    scrollerItems->m_contentLayer->addChild(m_menuItems);

    int i = 8, j = 22;
    float h = 17.f;

    for (bool type : types) {
        // gold font title
        if (type) {
            i--;

            auto title = CCLabelBMFont::create(titles[i].c_str(), "goldFont.fnt", 140.f);
            title->setPosition(ccp(50.f, h));
            title->setScale(0.6);

            m_menuItems->addChild(title);
            h += 20.f;
        }
        // big font option tab
        else {
            j--;

            auto label = CCLabelBMFont::create(j > 5 ? items[ops[j - 6]].c_str() : commons[j].c_str(), "bigFont.fnt", 140.f);
            label->setScale(0.5);
            label->setTag(j);
            
            auto button = CCMenuItemSpriteExtra::create(label, this, menu_selector(AdvancedMenu::onSwitchTab));
            button->setPosition(ccp(50.f, h));
            button->setColor(j == m_tab ? ccc3(0, 255, 0) : ccc3(127, 127, 127));
            button->setTag(j);

            m_menuItems->addChild(button);
            h += 17.f;
        }
    }

    scrollerItems->m_contentLayer->setContentSize(CCSize(100.f, h - 3.f));
    m_menuItems->setContentSize(CCSize(100.f, h - 3.f));

    // scroller setup
    this->m_scrollerSetup = ScrollLayer::create(CCSize(280.f, 180.f));
    m_scrollerSetup->setPosition(ccp(130.f, 45.f));
    m_scrollerSetup->setID("setup-scroller");
    m_mainLayer->addChild(m_scrollerSetup);

    // workspace title
    // tell users what they are currently setting for
    auto lbfSetupTitle = CCLabelBMFont::create("?", "goldFont.fnt", 260.f);
    lbfSetupTitle->setPosition(ccp(135.f, 240.f));
    lbfSetupTitle->setAnchorPoint(ccp(0.f, 0.6));
    lbfSetupTitle->setScale(0.6);
    lbfSetupTitle->setID("setup-title");
    m_mainLayer->addChild(lbfSetupTitle);
    
    auto cellDefault = ModeCell::create("Default");
    cellDefault->setTag(0);
    this->m_scrollerSetup->m_contentLayer->addChild(cellDefault);

    this->m_lbfDefault = createHint("Default");
    m_lbfDefault->setTag(0);
    this->m_scrollerSetup->m_contentLayer->addChild(m_lbfDefault);

    auto cellFollow = ModeCell::create("Follow");
    cellFollow->setTag(1);
    this->m_scrollerSetup->m_contentLayer->addChild(cellFollow);

    auto lbfFollow = createHint("Following a player's picked color");
    lbfFollow->setTag(1);
    this->m_scrollerSetup->m_contentLayer->addChild(lbfFollow);

    auto cellFollowTarget = EnumCell::create(false);
    cellFollowTarget->setTag(1);
    this->m_scrollerSetup->m_contentLayer->addChild(cellFollowTarget);

    auto cellManual = ModeCell::create("Manual");
    cellManual->setTag(2);
    this->m_scrollerSetup->m_contentLayer->addChild(cellManual);

    auto lbfManual = createHint("Pick the best color by your own.");
    lbfManual->setTag(2);
    this->m_scrollerSetup->m_contentLayer->addChild(lbfManual);

    auto cellManualColor = ColorCell::create();
    cellManualColor->setTag(2);
    this->m_scrollerSetup->m_contentLayer->addChild(cellManualColor);

    auto cellChroma = ModeCell::create("Chromatic");
    cellChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(cellChroma);

    auto lbfChroma = createHint("Chromatic Rainbow like");
    lbfChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(lbfChroma);

    auto tglChroma = ToggleCell::create(false);
    tglChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(tglChroma);
    this->asyncs.push_back(tglChroma);

    auto sliSpeedChroma = SliderInputCell::create(Signal::Speed, "Speed");
    sliSpeedChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(sliSpeedChroma);
    this->speeds.push_back(sliSpeedChroma);

    auto sliPhaseChroma = SliderInputCell::create(Signal::Phase, "Phase");
    sliPhaseChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(sliPhaseChroma);
    this->phases.push_back(sliPhaseChroma);

    auto sliLengthChroma = SliderInputCell::create(Signal::Length, "Frequency");
    sliLengthChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(sliLengthChroma);

    auto sliSatuChroma = SliderInputCell::create(Signal::Satu, "Saturation");
    sliSatuChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(sliSatuChroma);

    auto sliBritChroma = SliderInputCell::create(Signal::Brit, "Brightness");
    sliBritChroma->setTag(5);
    this->m_scrollerSetup->m_contentLayer->addChild(sliBritChroma);

    auto cellProgress = ModeCell::create("Gradient");
    cellProgress->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(cellProgress);

    auto lbfProgress = createHint("Tint regarding progress percentage.");
    lbfProgress->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(lbfProgress);
    
    auto cellProgressTarget = EnumCell::create(true);
    cellProgressTarget->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(cellProgressTarget);

    auto tglProgress = ToggleCell::create(false);
    tglProgress->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(tglProgress);
    this->asyncs.push_back(tglProgress);

    auto sliSpeedProgress = SliderInputCell::create(Signal::Speed, "Speed");
    sliSpeedProgress->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(sliSpeedProgress);
    this->speeds.push_back(sliSpeedProgress);

    auto sliPhaseProgress = SliderInputCell::create(Signal::Phase, "Phase");
    sliPhaseProgress->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(sliPhaseProgress);
    this->phases.push_back(sliPhaseProgress);

    auto cellProgressColor = MultiColorCell::create();
    cellProgressColor->setTag(3);
    this->m_scrollerSetup->m_contentLayer->addChild(cellProgressColor);

    auto cellRandom = ModeCell::create("Random");
    cellRandom->setTag(4);
    this->m_scrollerSetup->m_contentLayer->addChild(cellRandom);

    auto lbfRandom = createHint("Experimental (won't crash though)");
    lbfRandom->setTag(4);
    this->m_scrollerSetup->m_contentLayer->addChild(lbfRandom);

    auto tglRandom = ToggleCell::create(true);
    tglRandom->setTag(4);
    this->m_scrollerSetup->m_contentLayer->addChild(tglRandom);

    auto cellAdvanced = ModeCell::create("Advanced");
    cellAdvanced->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(cellAdvanced);

    auto lbfAdvanced = createHint("Write the shader yourself, rename it and put them in the config folder.");
    lbfAdvanced->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(lbfAdvanced);

    auto iptAdvancedVert = InputCell::create(false);
    iptAdvancedVert->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(iptAdvancedVert);

    auto iptAdvancedFrag = InputCell::create(true);
    iptAdvancedFrag->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(iptAdvancedFrag);

    auto tglAdvanced = ToggleCell::create(false);
    tglAdvanced->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(tglAdvanced);
    this->asyncs.push_back(tglAdvanced);

    auto sliSpeedAdvanced = SliderInputCell::create(Signal::Speed, "Frequency");
    sliSpeedAdvanced->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(sliSpeedAdvanced);
    this->speeds.push_back(sliSpeedAdvanced);

    auto sliPhaseAdvanced = SliderInputCell::create(Signal::Phase, "Phase");
    sliPhaseAdvanced->setTag(6);
    this->m_scrollerSetup->m_contentLayer->addChild(sliPhaseAdvanced);
    this->phases.push_back(sliPhaseAdvanced);

    // apply
    auto sprApply = ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 0.8);
    sprApply->setScale(0.8);
    auto btnApply = CCMenuItemSpriteExtra::create(sprApply, this, menu_selector(AdvancedMenu::onClose));
    auto size = btnApply->getContentSize();
    btnApply->setPosition(ccp(405.f - size.width / 2, 15.f + size.height / 2));
    m_mainLayer->getChildByType<CCMenu>(0)->addChild(btnApply);

    // preview sprite
    // debug testing so set visible false
    this->m_previewBar = PreviewBar::create();
    m_previewBar->setPosition(ccp(240.f, 15.f + size.height / 2));
    m_previewBar->setID("preview-bar");
    //m_previewBar->setVisible(false);
    m_mainLayer->addChild(m_previewBar);

    // hint
    if (!Mod::get()->setSavedValue("showed-item-browser-hint", true)) {
        auto lbfNewHint = CCLabelBMFont::create("Scroll the left border of this frame can also browse the tabs", "chatFont.fnt", 280.f);
        lbfNewHint->setPosition(ccp(-12.f, 140.f));
        lbfNewHint->setScale(0.65f);
        lbfNewHint->setRotation(-90.f);
        m_mainLayer->addChild(lbfNewHint);
        lbfNewHint->runAction(CCRepeatForever::create(CCSequence::create(
            CCTintTo::create(0.4f, 100, 100, 100),
            CCTintTo::create(0.4f, 255, 255, 255),
            nullptr
        )));
    }

    refresh();
    initialize();
    return true;
}

void AdvancedMenu::initialize() {
    auto lbf = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("setup-title"));
    // title
    if (m_tab < 6) {
        // common tabs
        lbf->setString((commons[m_tab] + (m_tab > 2 ? " List" : "")).c_str());
        this->m_lbfDefault->setString("No any change.");
        // default key
        this->defKey = tabs[m_tab];
        // default color
        this->defColor = m_tab < 3 ? (m_tab == 1 ? defCol2 : defCol1) : (m_tab == 3 ? defCol3 : defCol4);

    } else {
        this->m_lbfDefault->setString("Apply the common setup above.");
        if (m_tab < 12)
            lbf->setString((titles[m_tab / 2 - 2] + " - " + items[m_tab % 2]).c_str());
        else if (m_tab < 15)
            lbf->setString(("Quests - " + items[m_tab - 10]).c_str());
        else if (m_tab < 21)
            lbf->setString((titles[m_tab / 3 - 1] + " - " + items[m_tab % 3 + 5]).c_str());
        else
            lbf->setString("Retry Level Drop Bar");
    }

    // default key and color
    if (m_tab == 6 || m_tab == 8 || m_tab == 10 || m_tab == 21) {
        this->defKey = "normal";
        this->defColor = defCol1;
    } else if (m_tab == 7 || m_tab == 9 || m_tab == 11) {
        this->defKey = "practice";
        this->defColor = defCol2;
    } else if (m_tab == 12 || m_tab == 13 || m_tab == 14) {
        this->defKey = "quest";
        this->defColor = defCol1;
    } else if (m_tab == 15 || m_tab == 18) {
        this->defKey = "list-todo";
        this->defColor = defCol3;
    } else if (m_tab == 16 || m_tab == 19) {
        this->defKey = "list-done";
        this->defColor = defCol4;
    } else if (m_tab == 17 || m_tab == 20) {
        this->defKey = "list-unf";
        this->defColor = defCol4;
    }

    // load current config
    this->m_currentConfig = Mod::get()->getSavedValue<BarColor>(tabs[m_tab], makeDefStruct(this->defColor, this->defKey));

    float Y = 0.f;

    // update status and calculate height
    for (auto node : CCArrayExt<CCNode*>(this->m_scrollerSetup->m_contentLayer->getChildren())) {
        // set ui status regarding current value
        if (node->getID() != "hint")
            static_cast<SetupCell*>(node)->setVal(m_currentConfig);

        // show or hide
        if (node->getTag() == (int)m_currentConfig.mode || node->getID() == "title") {
            node->setScaleY(node->getID() == "hint" ? 0.7 : 1);
            Y += node->getScaledContentHeight();   
        } else
            node->setScaleY(0.f);
    }
    Y += 5.f;
    float H = Y;

    float top;

    // place them
    for (auto node : CCArrayExt<CCNode*>(this->m_scrollerSetup->m_contentLayer->getChildren())) {
        // place content layer
        if (node->getTag() == (int)m_currentConfig.mode && node->getID() == "title")
            this->m_scrollerSetup->m_contentLayer->setPositionY(Y > 180.f ? Y-H : 0.f);          
        // show or hide
        if (node->getTag() == (int)m_currentConfig.mode || node->getID() == "title")
            Y -= node->getScaledContentHeight();
        node->setPositionY(Y);
    }

    this->m_scrollerSetup->moveToTop();
    this->m_scrollerSetup->m_contentLayer->setContentHeight(H);
    // init the preview
    this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
}

void AdvancedMenu::switchMode(Mode mode) {
    // nonsense
    if (mode == m_currentConfig.mode) {
        // toggle it back
        static_cast<ModeCell*>(this->m_scrollerSetup->m_contentLayer->getChildByTag((int)mode))->toggle(false, true);
        return;        
    }

    float Y = 0.f;
    float scale;
    bool display;
    // setup menu config
    for (auto node : CCArrayExt<CCNode*>(this->m_scrollerSetup->m_contentLayer->getChildren())) {
        scale = node->getID() == "hint" ? 0.7 : 1;
        // title
        if (node->getID() == "title") {
            display = true;
            if (node->getTag() == (int)mode)
                static_cast<ModeCell*>(node)->toggle(false, true);
            else if (node->getTag() == (int)m_currentConfig.mode)
                static_cast<ModeCell*>(node)->toggle(false, false);
        }
        // not title but current mode
        else if (node->getTag() == (int)mode) {
            display = true;
            node->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.2, scale, scale)));
        }
        // hide
        else {
            display = false;
            node->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.2, scale, 0)));
        }
        // sum height
        if (display)
            Y += node->getContentHeight() * scale;
    }

    Y += 5.f;
    float H = Y;

    // place them
    for (auto node : CCArrayExt<CCNode*>(this->m_scrollerSetup->m_contentLayer->getChildren())) {
        // show or hide
        scale = node->getID() == "hint" ? 0.7 : 1;
        if (node->getTag() == (int)mode || node->getID() == "title")
            Y -= node->getContentHeight() * scale;
        // move node
        node->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.2, ccp(node->getID() == "title" ? 0.f : 10.f, Y))));
    }

    auto contL = this->m_scrollerSetup->m_contentLayer;
    contL->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.2, ccp(0,
        contL->getContentHeight() + contL->getPositionX() > H ? 0 : contL->getContentHeight() + contL->getPositionX() - H))));
    contL->setContentHeight(H);

    // write internal mode
    m_currentConfig.mode = mode;
    Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
    log::debug("switch mode init shader: defKey = {} defColor = {} {} {}", defKey, defColor.r, defColor.g, defColor.b);
    this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
}

void AdvancedMenu::onSwitchTab(CCObject* sender) {
    int tag = sender->getTag();
    // same tab
    if (tag == this->m_tab)
        return;

    // dump
    Mod::get()->setSavedValue("current-tab", tag);

    // white the selected tab
    m_menuItems->getChildByTag(m_tab)->runAction(CCTintTo::create(0.3, 127, 127, 127));

    // switch to current config
    this->m_tab = tag;

    // green the new tab
    m_menuItems->getChildByTag(m_tab)->runAction(CCTintTo::create(0.3, 0, 255, 0));

    // setup menu
    int count = this->m_scrollerSetup->m_contentLayer->getChildrenCount();
    for (auto child : CCArrayExt<CCMenu*>(this->m_scrollerSetup->m_contentLayer->getChildren())) {
        count--;
        child->runAction(CCSequence::create(
            CCFadeOut::create(0.15),
            CallFuncExt::create([this, count] { if (!count) this->initialize(); }),
            CCFadeIn::create(0.15),
            nullptr
        ));        
    }
}

void AdvancedMenu::onClose(CCObject* sender) {
    Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
    Popup::onClose(sender);
}