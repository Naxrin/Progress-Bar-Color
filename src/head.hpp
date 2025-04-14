#pragma once

#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCMenuItemToggler.hpp>

using namespace geode::prelude;

static const char* followeds[6] = {"Main", "Secondary", "Glow", "Dual-Main", "Dual-Secondary", "Dual-Glow"};
const std::string titles[6] = {"Level Info Menu", "Pause Menu", "Official Levels", "Quests Page", "List Page", "List Cell"};
const std::string items[7] = {"Normal", "Practice", "Top", "Middle", "Bottom", "Unclaimed", "Claimed"};
const std::string modes[5] = {"Default", "Follow", "Manual", "Progress", "Random"};

const std::string tabs[13] = {
    "info-menu-normal", "info-menu-practice",
    "pause-menu-normal", "pause-menu-practice", 
    "official-levels-normal", "official-levels-practice",
    "quest-top", "quest-middle", "quest-bottom",
    "list-page-todo", "list-page-done",
    "list-cell-todo", "list-cell-done"
};

// struct setup for a color channel
struct BarColor {
    int mode;
    int follower;
    ccColor3B color;
    ccColor3B colorZero;
    ccColor3B colorHdrd;
    bool operator == (const BarColor &c) const {
	    return ((mode == c.mode) && (follower == c.follower) && (color == c.color));
    };
};

// normal & quest
const ccColor3B defCol1 = ccc3(0, 255, 0);
// practice
const ccColor3B defCol2 = ccc3(255, 255, 0);
// unclaimed
const ccColor3B defCol3 = ccc3(255, 84, 50);
// claimed
const ccColor3B defCol4 = ccc3(80, 190, 255);

inline BarColor makeDefStruct(ccColor3B col) {
    return BarColor{
        .mode = 0, .follower = 0,
        .color = col, .colorZero = col, .colorHdrd = col      
    };

}

template<>
struct matjson::Serialize<BarColor> {
    static Result<BarColor> fromJson(matjson::Value const& value) {
        return Ok(BarColor{
            .mode = (int) value["mode"].asInt().unwrap() % 5,
            .follower = (int) value["follower"].asInt().unwrap() % 6,
            .color = ccc3(
                (int) value["r"].asInt().unwrap(),
                (int) value["g"].asInt().unwrap(),
                (int) value["b"].asInt().unwrap()
            ),
            .colorZero = value.contains("zero") ? value["zero"].asString().andThen([](auto str) { return cc3bFromHexString(str, true); })
                .unwrapOr(ccc3(255,255,255)) : ccc3(255, 255, 255),
            .colorHdrd = value.contains("hdrd") ? value["hdrd"].asString().andThen([](auto str) { return cc3bFromHexString(str, true); })
                .unwrapOr(ccc3(255,255,255)) : ccc3(255, 255, 255)
        });
    }

    static matjson::Value toJson(BarColor const& value) {
        return matjson::makeObject({
            {"mode", value.mode},
            {"follower", value.follower},
            {"r", value.color.r},
            {"g", value.color.g},
            {"b", value.color.b},
            {"zero", cc3bToHexString(value.colorZero)},
            {"hdrd", cc3bToHexString(value.colorHdrd)}
        });
    }
    static bool isJson(matjson::Value const& json) {
        if (!json.isObject()) return false;
        if (!json.contains("mode") || !json["mode"].isNumber()) return false;
        if (!json.contains("follower") || !json["follower"].isNumber()) return false;
        if (!json.contains("r") || !json["r"].isNumber()) return false;
        if (!json.contains("g") || !json["g"].isNumber()) return false;
        if (!json.contains("b") || !json["b"].isNumber()) return false;
        if (!json.contains("zero") || !json["zero"].isString()) return false;
        if (!json.contains("hdrd") || !json["hdrd"].isString()) return false;

        return true;
    }
};

enum class Signal {
    Mode, Follower, Color, Zero, Hdrd
};

template<typename T>
class SignalEvent : public Event {
public:
    Signal signal;
    T value;
    SignalEvent(Signal signal, T value) : Event() {
        this->signal = signal;
        this->value = value;
    }
};

class SetupCell : public CCMenu {
protected:
    virtual bool setup() = 0;
    bool init() {
        if (!CCMenu::init())
            return false;
            this->setPositionX(10.f);
            this->setAnchorPoint(CCPoint(0.5, 0.f));
        return this->setup();
    }
public:
    virtual void setVal(BarColor const& setup) = 0;
};

class ModeCell : public SetupCell {
protected:
    // label
    CCLabelBMFont* m_label;
    // toggler
    CCMenuItemToggler* m_toggler;
    // init
    bool setup() override;
    // on toggle
    void onToggle(CCObject* sender) {
        SignalEvent(Signal::Mode, this->getTag()).post();
    }
    void setTitle(const char* name) {
        this->m_label->setString(name);
    }
public:
    // toggle
    void toggle() {
        if (m_toggler->isToggled())
            m_label->runAction(CCTintTo::create(0.2, 127, 127, 127));
        else
            m_label->runAction(CCTintTo::create(0.2, 0, 255, 0));
        this->m_toggler->toggle(false);      
    }
    void setVal(BarColor const& setup) override {
        bool on = setup.mode == this->getTag();
        if (on == m_toggler->isToggled())
            return;
        this->m_toggler->toggle(on);
        if (on)
            m_label->setColor(ccc3(0, 255, 0));
        else
            m_label->setColor(ccc3(127, 127, 127));
    }
    static ModeCell* create(const char* name) {
        auto node = new ModeCell();
        if (node && node->init()) {
            node->setTitle(name);
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class FollowerCell : public SetupCell {
protected:
    // current follower index
    int index;
    // label
    CCLabelBMFont* m_label;
    // init
    bool setup() override;
    // on toggle
    void onArrow(CCObject* sender) {
        this->index = (index + (sender->getTag() == 2 ? 7 : 5)) % 6;
        m_label->setString(followeds[this->index]);
        SignalEvent(Signal::Follower, this->index).post();
    }
public:
    // set current index
    void setVal(BarColor const& setup) override {
        this->index = setup.follower;
        m_label->setString(followeds[this->index]);
    }
    static FollowerCell* create() {
        auto node = new FollowerCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class ColorCell : public SetupCell, public ColorPickPopupDelegate {
protected:
    // node
    CCMenuItemSpriteExtra* m_btn;
    // init
    bool setup() override;
    // on button
    void onButton(CCObject* sender) {
        auto popup = ColorPickPopup::create(m_btn->getColor());
        popup->setDelegate(this);
        popup->show();
    }
    // set current index
    void updateColor(ccColor4B const& color) override {
        this->m_btn->setColor(to3B(color));
        SignalEvent(Signal::Color, to3B(color)).post();
    }
public:
    void setVal(BarColor const& setup) override {
        m_btn->setColor(setup.color);
    }
    static ColorCell* create() {
        auto node = new ColorCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class MultiColorCell : public SetupCell, public ColorPickPopupDelegate {
protected:
    // which
    bool hd;
    // node
    CCMenuItemSpriteExtra* m_btnZero;
    CCMenuItemSpriteExtra* m_btnHdrd;
    // init
    bool setup() override;
    // on button
    void onButton(CCObject* sender) {
        this->hd = sender->getTag() == 2;
        auto popup = ColorPickPopup::create(( this->hd ? m_btnHdrd : m_btnZero)->getColor());
        popup->setDelegate(this);
        popup->show();
    }
    // set current index
    void updateColor(ccColor4B const& color) override {
        (this->hd ? m_btnHdrd : m_btnZero)->setColor(to3B(color));
        SignalEvent(this->hd ? Signal::Hdrd : Signal::Zero, to3B(color)).post();
    }
public:
    void setVal(BarColor const& setup) override {
        m_btnZero->setColor(setup.colorZero);
        m_btnHdrd->setColor(setup.colorHdrd);
    }
    static MultiColorCell* create() {
        auto node = new MultiColorCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

/*
class ToggleCell : public SetupCell {
protected:
    // toggler
    CCMenuItemToggler* m_toggler;
    // init
    bool setup();
    // on toggle
    void onToggle(CCObject* sender) {
        SignalEvent(Signal::Instant, !m_toggler->isToggled()).post();
    }
public:
    // toggle
    void setVal(BarColor const& setup) {
        this->m_toggler->toggle(setup.instant);
    }
    static ToggleCell* create() {
        auto node = new ToggleCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};
*/

// menu for advanced settings
class AdvancedMenu : public Popup<> {
private:
    // left menu item item (from buttom to top)
    const std::vector<bool> types = {
        false, false, true, false, false, true, 
        false, false, false, true,
        false, false, true, false, false, true, false, false, true
    };
    // reversed 
    const std::vector<short> ops = {0, 1, 0, 1, 0, 1, 2, 3, 4, 5, 6, 5, 6};
protected:

    // save the current selected tab in the left
    int m_tab;
    // current config setup
    BarColor m_currentConfig;
    // items menu
    CCMenu* m_menuItems;
    // setup menu
    CCMenu* m_menuSetup;

    EventListener<EventFilter<SignalEvent<int>>> listenerInt
        = EventListener<EventFilter<SignalEvent<int>>>(
            [this] (SignalEvent<int>* event) -> ListenerResult {
                if (event->signal == Signal::Mode)
                    this->switchMode(event->value);
                else if (event->signal == Signal::Follower)
                    this->m_currentConfig.follower = event->value;
                return ListenerResult::Stop;
            });

    EventListener<EventFilter<SignalEvent<ccColor3B>>> listenerColor
        = EventListener<EventFilter<SignalEvent<ccColor3B>>>(
            [this] (SignalEvent<ccColor3B>* event) -> ListenerResult {
                if (event->signal == Signal::Color)
                    this->m_currentConfig.color = event->value;
                if (event->signal == Signal::Zero)
                    this->m_currentConfig.colorZero = event->value;
                if (event->signal == Signal::Hdrd)
                    this->m_currentConfig.colorHdrd = event->value;
                return ListenerResult::Stop;
            });

    // rewrite setup ui
    bool setup() override;
    // initialize menu status in the right
    void initialize();
    // run animation switch mode
    void switchMode(int mode);
    // on switch to another tab
    void onSwitchTab(CCObject*);
    // close rewrite
    void onClose(CCObject*) override;
public:
    // create
    static AdvancedMenu* create() {
        auto ret = new AdvancedMenu();
        if (ret && ret->initAnchored(420.f, 280.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// the advanced setting node
class AdvancedSetting : public SettingBaseValueV3<BarColor> {
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<AdvancedSetting>();
        auto root = checkJson(json, "AdvancedSetting");
        res->init(key, modID, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }
    bool load(matjson::Value const&) override {
        return true;
    }
    bool save(matjson::Value &) const override {
        return true;
    }
    bool isDefaultValue() const override {
        return true;
    }
    void reset() override {};

    SettingNodeV3* createNode(float width) override;
};

class AdvancedSettingNode : public SettingNodeV3 {
protected:
    ButtonSprite* m_buttonSprite;
    CCMenuItemSpriteExtra* m_button;

    bool init(std::shared_ptr<AdvancedSetting> setting, float width) {
        if (!SettingNodeV3::init(setting, width))
            return false;
        
        m_buttonSprite = ButtonSprite::create("Config", "goldFont.fnt", "GJ_button_01.png", .8f);
        m_buttonSprite->setScale(.6f);
        m_button = CCMenuItemSpriteExtra::create(
            m_buttonSprite, this, menu_selector(AdvancedSettingNode::onButton)
        );
        this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
        this->getButtonMenu()->setContentWidth(60);
        this->getButtonMenu()->updateLayout();

        this->updateState(nullptr);
        
        return true;
    }
    void updateState(CCNode* invoker) override {
        SettingNodeV3::updateState(invoker);
        auto shouldEnable = this->getSetting()->shouldEnable();
        m_button->setEnabled(shouldEnable);
        m_buttonSprite->setCascadeColorEnabled(true);
        m_buttonSprite->setCascadeOpacityEnabled(true);
        m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
        m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
    }
    void onButton(CCObject*) {
        AdvancedMenu::create()->show();
    }
    void onCommit() override {}
    void onResetToDefault() override {}

public:
    static AdvancedSettingNode* create(std::shared_ptr<AdvancedSetting> setting, float width) {
        auto ret = new AdvancedSettingNode();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    bool hasUncommittedChanges() const override {
        return false;
    }
    bool hasNonDefaultValue() const override {
        return false;
    }

    std::shared_ptr<AdvancedSetting> getSetting() const {
        return std::static_pointer_cast<AdvancedSetting>(SettingNodeV3::getSetting());
    }
};