#pragma once

#include <Geode/loader/SettingV3.hpp>
//#include <geode.custom-keybinds/include/OptionalAPI.hpp>

using namespace geode::prelude;
//using namespace keybinds;

static const char* followeds[6] = {"Main", "Secondary", "Glow", "Dual-Main", "Dual-Secondary", "Dual-Glow"};
static const char* gradtypes[3] = {"Progress", "Time", "Space"};

const std::string titles[7] = {"Common", "Info Menu", "Pause Menu", "Official Menu", "Quests Page", "List Page", "List Cell"};
const std::string items[8] = {"Normal", "Practice", "Top", "Middle", "Bottom", "Unclaimed", "Claimed", "Unfeatured"};

const std::string modes[10] = {"Default", "Follow", "Manual", "Random", "Chromatic", "Gradient", "Advanced"};

const std::string commons[6] = {"Normal", "Practice", "Quest", "Unclaimed", "Claimed", "Unfeatured"};

const std::string tabs[21] = {
    "normal-default", "practice-default", "quest-default",
    "list-todo-default", "list-done-default", "list-unf-default",
    "info-menu-normal", "info-menu-practice",
    "pause-menu-normal", "pause-menu-practice",
    "official-levels-normal", "official-levels-practice",
    "quest-top", "quest-middle", "quest-bottom",
    "list-page-todo", "list-page-done", "list-page-unf",
    "list-cell-todo", "list-cell-done", "list-cell-unf"
};

const auto pathR = Mod::get()->getResourcesDir();
const auto pathK = Mod::get()->getConfigDir();

const auto shch = CCShaderCache::sharedShaderCache();

enum class Mode : uint8_t {
    Default, Follow, Manual, Gradient, Random, Chromatic, Advanced
};

enum class GradType : uint8_t {
    Progress, Space, Time
};

// struct setup for a color channel
struct BarColor {
    Mode mode; // mode
    int follower; // player color channel to follow
    ccColor4B color; // static color for manual mode
    // gradient
    GradType gradType; // gradient type
    ccColor4B colorZero; // zero point color
    ccColor4B colorHdrd; // hundred point color
    int phase; // init phase
    bool async; // use custom speed
    float speed; // time varient speed
    int satu; // saturation
    int brit; // brightness
    float length; // length in space, zero for rainbow mode OFF
    bool randa; // random alpha
    std::string vert; // vert file name
    std::string frag; // frag file name

    bool operator == (const BarColor &c) const {
	    return ((mode == c.mode) && (follower == c.follower) && (color == c.color)) && (gradType == c.gradType)
            && (colorZero == c.colorZero) && (colorHdrd == c.colorHdrd) && (phase == c.phase) && (async == c.async)
            && (speed == c.speed) && (satu == c.satu) && (brit == c.brit) && (length == c.length)
            && (randa == c.randa) && (vert == c.vert) && (frag == c.frag);
    }
};

// normal & quest
const ccColor3B defCol1 = ccc3(0, 255, 0);
// practice
const ccColor3B defCol2 = ccc3(255, 255, 0);
// unclaimed
const ccColor3B defCol3 = ccc3(255, 84, 50);
// claimed
const ccColor3B defCol4 = ccc3(80, 190, 255);

inline BarColor makeDefStruct(ccColor3B color, std::string key) {
    return BarColor{
        .mode = Mode::Default, .follower = 0, .color = to4B(color),
        .gradType = GradType::Progress, .colorZero = to4B(color), .colorHdrd = to4B(color),
        .phase = 0, .async = false, .speed = 1.f, .satu = 100, .brit = 100, .length = 1.f, .randa = false,
        .vert = fmt::format("vert-{}.glsl", key), .frag = fmt::format("frag-{}.glsl", key)
    };
}

template<>
struct matjson::Serialize<BarColor> {
    static Result<BarColor> fromJson(matjson::Value const& value) {
        return Ok(BarColor{
            .mode = value.contains("mode") ? Mode(value["mode"].asInt().unwrapOrDefault() % 7) : Mode::Default,
            .follower = value.contains("follower") ? (int) value["follower"].asInt().unwrapOrDefault() % 6 : 0,
            .color = ccc4(
                GLubyte(value.contains("r") ? (int) value["r"].asInt().unwrapOr(255) : 255),
                GLubyte(value.contains("g") ? (int) value["g"].asInt().unwrapOr(255) : 255),
                GLubyte(value.contains("b") ? (int) value["b"].asInt().unwrapOr(255) : 255),
                GLubyte(value.contains("a") ? (int) value["a"].asInt().unwrapOr(255) : 255)
            ),
            .gradType = value.contains("grad-type") ? GradType((int)value["grad-type"].asInt().unwrapOrDefault() % 3) : GradType::Progress,
            .colorZero = value.contains("zero") ? value["zero"].asString().andThen([](auto str) { return cc4bFromHexString(str, false, true); })
                .unwrapOr(ccc4(255, 255, 255, 255)) : ccc4(255, 255, 255, 255),
            .colorHdrd = value.contains("hdrd") ? value["hdrd"].asString().andThen([](auto str) { return cc4bFromHexString(str, false, true); })
                .unwrapOr(ccc4(255, 255, 255, 255)) : ccc4(255, 255, 255, 255),
            .phase = value.contains("phase") ? (int) value["phase"].asInt().unwrapOrDefault() % 360 : 0,
            .async = value.contains("async") ? (bool) value["async"].asBool().unwrapOrDefault() : false,
            .speed = value.contains("speed") ? (float) value["speed"].asDouble().unwrapOr(1.f) : 1.f,
            .satu = value.contains("satu") ? (int) value["satu"].asDouble().unwrapOrDefault() % 100 : 100,
            .brit = value.contains("brit") ? (int) value["brit"].asDouble().unwrapOrDefault() % 100 : 100,
            .length = value.contains("length") ? (float) value["length"].asDouble().unwrapOr(1.f) : 1.f,
            .randa = value.contains("randa") ? (bool) value["randa"].asBool().unwrapOrDefault() : false,
            .vert = value.contains("vert") ? value["vert"].asString().unwrapOrDefault() : "",
            .frag = value.contains("frag") ? value["frag"].asString().unwrapOrDefault() : ""
        });
    }

    static matjson::Value toJson(BarColor const& value) {
        return matjson::makeObject({
            {"mode", (int)value.mode},
            {"follower", value.follower},
            {"r", value.color.r},
            {"g", value.color.g},
            {"b", value.color.b},
            {"a", value.color.a},
            {"grad-type", (int)value.gradType},
            {"zero", cc4bToHexString(value.colorZero)},
            {"hdrd", cc4bToHexString(value.colorHdrd)},
            {"phase", value.phase},
            {"async", value.async},
            {"speed", value.speed},
            {"satu", value.satu},
            {"brit", value.brit},
            {"length", value.length},
            {"randa", value.randa},
            {"vert", value.vert},
            {"frag", value.frag}
        });
    }
    static bool isJson(matjson::Value const& json) {
        return true;
    }
};

enum class Signal {
    Mode, Follower, Color, Length, Speed, Async, Phase, Satu, Brit, GradType, Zero, Hdrd, RandA, Vert, Frag
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

class C4Button : public CCMenuItemSpriteExtra {
protected:
    CCLabelBMFont* label;
    ColorChannelSprite* spr;
    // init
    bool init(CCObject* target, SEL_MenuHandler callback);
public:
    // set value
    void setValue(ccColor4B const& color) {
        this->spr->setColor(to3B(color));
        this->label->setString(numToString<float>(color.a / 255.f, 2).c_str());
    }
    // get value
    ccColor4B getValue() {
        return to4B(this->spr->getColor(), numFromString<float>(fmt::format("{}", this->label->getString())).unwrapOr(1.00) * 255);
    }
    // create
    static C4Button* create(CCObject* target, SEL_MenuHandler callback) {
        auto node = new C4Button();
        if (node && node->init(target, callback)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class SetupCell : public CCMenu {
protected:
    bool init() {
        if (!CCMenu::init())
            return false;
        this->setContentSize(ccp(280.f, 20.f));
        this->setPositionX(10.f);
        this->setAnchorPoint(CCPoint(0.5f, 0.f));
        return true;
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
    bool init(const char* name);
    // on toggle
    void onToggle(CCObject* sender) {
        SignalEvent(Signal::Mode, this->getTag()).post();
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
        bool on = setup.mode == Mode(this->getTag());
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
        if (node && node->init(name)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class EnumCell : public SetupCell {
protected:
    // means gradient type or not
    bool type;
    // current follower index
    int index;
    // label
    CCLabelBMFont* m_label;
    // init
    bool init(bool type);
    // on toggle
    void onArrow(CCObject* sender) {
        if (type) {
            this->index = (index + (sender->getTag() == 2 ? 4 : 2)) % 3;
            m_label->setString(gradtypes[this->index]);
            SignalEvent(Signal::GradType, this->index).post();
        } else {
            this->index = (index + (sender->getTag() == 2 ? 7 : 5)) % 6;
            m_label->setString(followeds[this->index]);
            SignalEvent(Signal::Follower, this->index).post();
        }
    }
public:
    // set current index
    void setVal(BarColor const& setup) override {
        if (type) {
            this->index = int(setup.gradType);
            m_label->setString(gradtypes[this->index]);
        } else {
            this->index = setup.follower;
            m_label->setString(followeds[this->index]);
        }
    }
    static EnumCell* create(bool type) {
        auto node = new EnumCell();
        if (node && node->init(type)) {
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
    C4Button* m_btn;
    // init
    bool init() override;
    // on button
    void onButton(CCObject* sender) {
        auto popup = ColorPickPopup::create(m_btn->getValue());
        popup->setDelegate(this);
        popup->show();
    }
    // set current index
    void updateColor(ccColor4B const& color) override {
        this->m_btn->setValue(color);
        SignalEvent(Signal::Color, color).post();
    }
public:
    void setVal(BarColor const& setup) override {
        m_btn->setValue(setup.color);
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

// linear slider input bundle
class SliderInputCell : public SetupCell, public TextInputDelegate {
protected:
    // value
    float val;
    // signal
    Signal target;
    // slider
    Slider* m_slider;
    // inputer
    TextInput* m_inputer;
    // init
    bool init(Signal target, const char* text);
    // on slider
    void onSlider(CCObject* sender);
    // text changed override
    void textChanged(CCTextInputNode* p) override;
public:
    void setVal(BarColor const &val) override;
    static SliderInputCell* create(Signal target, const char* text) {
        auto node = new SliderInputCell();
        if (node && node->init(target, text)) {
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
    C4Button* m_btnZero;
    C4Button* m_btnHdrd;
    // init
    bool init() override;
    // on button
    void onButton(CCObject* sender) {
        this->hd = sender->getTag() == 2;
        auto popup = ColorPickPopup::create(( this->hd ? m_btnHdrd : m_btnZero)->getValue());
        popup->setDelegate(this);
        popup->show();
    }
    // set current index
    void updateColor(ccColor4B const& color) override {
        (this->hd ? m_btnHdrd : m_btnZero)->setValue(color);
        SignalEvent(this->hd ? Signal::Hdrd : Signal::Zero, color).post();
    }
public:
    void setVal(BarColor const& setup) override {
        m_btnZero->setValue(setup.colorZero);
        m_btnHdrd->setValue(setup.colorHdrd);
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

// toggler
class ToggleCell : public SetupCell {
protected:
    // randA or async
    bool randa;
    // toggler
    CCMenuItemToggler* m_toggler;
    // init
    bool init(bool randa);
    // on toggle
    void onToggle(CCObject* sender) {
        SignalEvent(this->randa ? Signal::RandA : Signal::Async, !m_toggler->isToggled()).post();
    }
    void onDesc(CCObject* sender) {
        if (this->randa)
            FLAlertLayer::create("Random Alpha", "Let alpha channel also acts randomly.", "OK")->show();
        else
            FLAlertLayer::create("Async Frequency", "Speed value will not sync with your common mod settings but follow its own value, set it below this checkbox.", "OK")->show();
    }
public:
    // toggle
    void setVal(BarColor const& setup) {
        this->m_toggler->toggle(this->randa ? setup.randa : setup.async);
    }
    static ToggleCell* create(bool randa) {
        auto node = new ToggleCell();
        if (node && node->init(randa)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// shader file name
class InputCell : public SetupCell, public TextInputDelegate {
protected:
    // false = vert
    bool frag;
    // toggler
    TextInput* m_inputer;
    // init
    bool init(bool frag);
    // on toggle
    void textChanged(CCTextInputNode* p) override {
        log::debug("post {}", p->getString());
        SignalEvent(this->frag ? Signal::Frag : Signal::Vert, p->getString()).post();
    }
public:
    // toggle
    void setVal(BarColor const& setup) {
        this->m_inputer->setString(this->frag ? setup.frag : setup.vert);
    }
    static InputCell* create(bool frag) {
        auto node = new InputCell();
        if (node && node->init(frag)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// menu for advanced settings
class AdvancedMenu : public Popup<> {
private:
    // left menu item is a title or not (from bottom to top)
    const std::vector<bool> types = {
        false, false, false, true, false, false, false, true,
        false, false, false, true,
        false, false, true, false, false, true, false, false, true,
        false, false, false, false, false, false, true
    };

    const std::vector<short> ops = {0, 1, 0, 1, 0, 1, 2, 3, 4, 5, 6, 7, 5, 6, 7};
protected:

    // save the current selected tab in the left
    int m_tab;
    // current config setup
    BarColor m_currentConfig;
    // items menu
    CCMenu* m_menuItems;
    // preview sprite
    CCSprite* m_sprPrev;
    // setup menu
    ScrollLayer* m_scrollerSetup;
    // default hint
    CCLabelBMFont* m_lbfDefault;

    EventListener<EventFilter<SignalEvent<bool>>> listenerBool
        = EventListener<EventFilter<SignalEvent<bool>>>(
            [this] (SignalEvent<bool>* event) -> ListenerResult {
                if (event->signal == Signal::Async)
                    this->m_currentConfig.async = event->value;
                if (event->signal == Signal::RandA)
                    this->m_currentConfig.randa = event->value; 
                return ListenerResult::Stop;
            });

    EventListener<EventFilter<SignalEvent<int>>> listenerInt
        = EventListener<EventFilter<SignalEvent<int>>>(
            [this] (SignalEvent<int>* event) -> ListenerResult {
                if (event->signal == Signal::Mode)
                    this->switchMode(Mode(event->value));
                else if (event->signal == Signal::Follower)
                    this->m_currentConfig.follower = event->value;
                else if (event->signal == Signal::GradType)
                    this->m_currentConfig.gradType = GradType(event->value);
                else if (event->signal == Signal::Phase)
                    this->m_currentConfig.phase = event->value;
                else if (event->signal == Signal::Satu)
                    this->m_currentConfig.satu = event->value;
                else if (event->signal == Signal::Brit)
                    this->m_currentConfig.brit = event->value;
                return ListenerResult::Stop;
            });

    EventListener<EventFilter<SignalEvent<float>>> listenerFloat
        = EventListener<EventFilter<SignalEvent<float>>>(
            [this] (SignalEvent<float>* event) -> ListenerResult {
                if (event->signal == Signal::Speed)
                    this->m_currentConfig.speed = event->value;
                if (event->signal == Signal::Length)
                    this->m_currentConfig.length = event->value;
                return ListenerResult::Stop;
            });

    EventListener<EventFilter<SignalEvent<std::string>>> listenerString
        = EventListener<EventFilter<SignalEvent<std::string>>>(
            [this] (SignalEvent<std::string>* event) -> ListenerResult {
                log::debug("{}", event->value);
                if (event->signal == Signal::Vert)
                    this->m_currentConfig.vert = event->value;
                if (event->signal == Signal::Frag)
                    this->m_currentConfig.frag = event->value;
                return ListenerResult::Stop;
            });

    EventListener<EventFilter<SignalEvent<ccColor4B>>> listenerColor
        = EventListener<EventFilter<SignalEvent<ccColor4B>>>(
            [this] (SignalEvent<ccColor4B>* event) -> ListenerResult {
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
    void switchMode(Mode mode);
    // on switch to another tab
    void onSwitchTab(CCObject*);
    // close rewrite
    void onClose(CCObject*) override;
public:
    // register devtools
    static void registerDevTools();
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