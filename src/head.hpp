#pragma once

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>


using namespace geode::prelude;

static const char* followeds[6] = {"Main", "Secondary", "Glow", "Dual-Main", "Dual-Secondary", "Dual-Glow"};
static const char* gradtypes[3] = {"Progress", "Space", "Time"};

const std::string titles[8] = {"Common", "Info Menu", "Pause Menu", "Official Menu", "Quests Page", "List Page", "List Cell", "Other"};
const std::string items[9] = {"Normal", "Practice", "Top", "Middle", "Bottom", "Unclaimed", "Claimed", "Unfeatured", "Drop Bar"};

const std::string modes[7] = {"Default", "Follow", "Manual", "Random", "Chromatic", "Gradient", "Advanced"};

const std::string commons[6] = {"Normal", "Practice", "Quest", "Unclaimed", "Claimed", "Unfeatured"};

const std::string tabs[22] = {
    "normal-default", "practice-default", "quest-default",
    "list-todo-default", "list-done-default", "list-unf-default",
    "info-menu-normal", "info-menu-practice",
    "pause-menu-normal", "pause-menu-practice",
    "official-levels-normal", "official-levels-practice",
    "quest-top", "quest-middle", "quest-bottom",
    "list-page-todo", "list-page-done", "list-page-unf",
    "list-cell-todo", "list-cell-done", "list-cell-unf",
    "drop-bar"
};

// global variables
static ccColor3B main1, second1, glow1, main2, second2, glow2;

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
    /*
    int shaderMode() const {
        if (mode == Mode::Chromatic) return 3;
        if (mode == Mode::Gradient) {
            if (gradType == GradType::Space)
                return 1;
            if (gradType == GradType::Time)
                return 2;
        }
        return 0;
    }*/
};

// normal & quest
const ccColor3B defCol1 = ccc3(0, 255, 0);
// practice
const ccColor3B defCol2 = ccc3(0, 255, 255);
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
            .satu = value.contains("satu") ? (int) value["satu"].asDouble().unwrapOr(100) % 101 : 100,
            .brit = value.contains("brit") ? (int) value["brit"].asDouble().unwrapOr(100) % 101 : 100,
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
    Mode, Follower, Color, Length, Speed, Async, Phase, Satu, Brit, GradType, Zero, Hdrd, RandA, Vert, Frag, Slider
};

template<typename T>
struct SignalEvent : public Event<SignalEvent<T>, bool(T), Signal> {
    using Event<SignalEvent<T>, bool(T), Signal>::Event;
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
        SignalEvent<int>(Signal::Mode).send(this->getTag());
    }
public:
    // toggle
    void toggle(bool on, bool lbl) {
        if (lbl)
            m_label->runAction(CCTintTo::create(0.2, 0, 255, 0));
        else
            m_label->runAction(CCTintTo::create(0.2, 127, 127, 127));
        this->m_toggler->toggle(on);
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
            SignalEvent<int>(Signal::GradType).send(this->index);
        } else {
            this->index = (index + (sender->getTag() == 2 ? 7 : 5)) % 6;
            m_label->setString(followeds[this->index]);
            SignalEvent<int>(Signal::Follower).send(this->index);
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

class ColorCell : public SetupCell, public ColorPickerDelegate {
protected:
    // node
    C4Button* m_btn;
    // init
    bool init() override;
    // on button
    void onButton(CCObject* sender) {
        auto popup = ColorPickPopup::create(m_btn->getValue());
        popup->setCallback([this](ccColor4B const& color) {
            this->m_btn->setValue(color);
            SignalEvent<ccColor4B>(Signal::Color).send(color);            
        });
        popup->show();
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
class SliderInputCell : public SetupCell, public TextInputDelegate, public SliderDelegate {
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
    // slider began
    void sliderBegan(Slider* p) override;
    // slider ended
    void sliderEnded(Slider* p) override;
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

class MultiColorCell : public SetupCell {
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
        popup->setCallback([this](ccColor4B const& color) {
            (this->hd ? m_btnHdrd : m_btnZero)->setValue(color);
            SignalEvent<ccColor4B>(this->hd ? Signal::Hdrd : Signal::Zero).send(color);            
        });
        popup->show();
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
        SignalEvent<bool>(this->randa ? Signal::RandA : Signal::Async).send(!m_toggler->isToggled());
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
        SignalEvent<std::string>(this->frag ? Signal::Frag : Signal::Vert).send(p->getString());
    }
public:
    // toggle
    void setVal(BarColor const& setup) override {
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

// init a target node (progress bar) regarding its keyname
// @param advKey key for advanced
// @param defKey key for default
// @param progress current percentage of this progress bar
// @param inCommon avoid infinite loop
CCGLProgram* init_shader(
    CCSprite* target, std::string advKey, std::string defKey, std::string progKey,
    BarColor &config, ccColor3B defCol, int progress, bool inCommon = false);

// update a target node (progress bar) regarding its keyname
// @param advKey key for advanced
// @param defKey key for default
// @param progress current percentage of this progress bar
// @param inCommon avoid infinite loop CCGLProgram *prog, 
void update_shader(
    CCSprite* target, BarColor const &config, CCGLProgram* prog,
    float const &dt, float const &ph0, float &ph, bool inCommon = false);

// refresh player colors
void refresh();

class PreviewBar : public CCLayer {
protected:
    // progress simulator
    float progress;
    // max target size
    CCSize size;
    // phase
    float delta, deltac;
    // the bar taken effect on
    CCSprite* base, * target;
    // init
    bool init() override;
    // update
    void update(float dt) override;
    // decide it's touched or not
    bool ccTouchBegan(CCTouch *touch, CCEvent* event) override;
    // move touch to update progress
    void ccTouchMoved(CCTouch *touch, CCEvent* event) override;
    // set progress
    void setProgress(float p);    
public:
    BarColor config;
    // target program
    CCGLProgram* program;
    // update a uniform
    void updateUniform(const char* name, int val) {
        this->program->setUniformLocationWith1i(this->program->getUniformLocationForName(name), val);
        this->program->updateUniforms();
    }

    void updateUniform(const char* name, float val) {
        this->program->setUniformLocationWith1f(this->program->getUniformLocationForName(name), val);
        this->program->updateUniforms();
    }

    void updateUniform(const char* name, ccColor3B val) {
        this->program->setUniformLocationWith3f(this->program->getUniformLocationForName(name), val.r / 255.f, val.g / 255.f, val.b / 255.f);
        this->program->updateUniforms();
    }
    void updateUniform(const char* name, ccColor4B val) {
        this->program->setUniformLocationWith4f(this->program->getUniformLocationForName(name), val.r / 255.f, val.g / 255.f, val.b / 255.f, val.a / 255.f);
        this->program->updateUniforms();
    }
    // init shader
    void initShader(std::string advKey, std::string defKey, ccColor3B defCol);
    // create
    static PreviewBar* create() {
        auto node = new PreviewBar();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// sorry zincoid
class AdvancedMenu : public Popup {
private:
    // left menu item is a title or not (from bottom to top)
    const std::vector<bool> types = {
        false, true, false, false, false, true, false, false, false, true,
        false, false, false, true,
        false, false, true, false, false, true, false, false, true,
        false, false, false, false, false, false, true
    };

    const std::vector<short> ops = {0, 1, 0, 1, 0, 1, 2, 3, 4, 5, 6, 7, 5, 6, 7, 8};
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
    // preview bar
    PreviewBar* m_previewBar;
    // default key for preview
    std::string defKey;
    // default color for this tab
    ccColor3B defColor;
    // on slider dragging
    bool drag_slider;

    // repeated menu lines
    std::vector<ToggleCell*> asyncs;
    std::vector<SliderInputCell*> speeds;
    std::vector<SliderInputCell*> phases;

    ListenerHandle radioSlider = SignalEvent<bool>(Signal::Slider).listen([this] (bool value) {
        this->drag_slider = value;
        return ListenerResult::Stop;
    });

    ListenerHandle radioAsync = SignalEvent<bool>(Signal::Async).listen([this] (bool value) {
        this->m_currentConfig.async = value;
        // sync async togglers elsewhere
        for (auto line : asyncs)
            if (line->getTag() != (int)m_currentConfig.mode)
                line->setVal(m_currentConfig);
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioRandA = SignalEvent<bool>(Signal::RandA).listen([this] (bool value) {
        this->m_currentConfig.randa = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioMode = SignalEvent<int>(Signal::Mode).listen([this] (int value) {
        this->switchMode(Mode(value));
        return ListenerResult::Stop;
    });

    ListenerHandle radioFollower = SignalEvent<int>(Signal::Follower).listen([this] (int value) {
        this->m_currentConfig.follower = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioGradType = SignalEvent<int>(Signal::GradType).listen([this] (int value) {
        this->m_currentConfig.gradType = GradType(value);
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioSpeed = SignalEvent<float>(Signal::Speed).listen([this] (int value) {
        this->m_currentConfig.speed = value;
        for (auto line : speeds)
            if (line->getTag() != (int)m_currentConfig.mode)
                line->setVal(m_currentConfig);
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioLength = SignalEvent<float>(Signal::Length).listen([this] (float value) {
        this->m_currentConfig.length = value;
        this->m_previewBar->updateUniform("freq", value);
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioPhase = SignalEvent<float>(Signal::Phase).listen([this] (float value) {
        this->m_currentConfig.phase = (int)value;
        for (auto line : phases)
            if (line->getTag() != (int)m_currentConfig.mode)
                line->setVal(m_currentConfig);
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioSatu = SignalEvent<float>(Signal::Satu).listen([this] (float value) {
        this->m_currentConfig.satu = (int)value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioBrit = SignalEvent<float>(Signal::Brit).listen([this] (float value) {
        this->m_currentConfig.brit = (int)value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioVert = SignalEvent<std::string>(Signal::Vert).listen([this] (std::string value) {
        this->m_currentConfig.vert = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioFrag = SignalEvent<std::string>(Signal::Frag).listen([this] (std::string value) {
        this->m_currentConfig.frag = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioColor = SignalEvent<ccColor4B>(Signal::Color).listen([this] (ccColor4B value) {
        this->m_currentConfig.color = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioZero = SignalEvent<ccColor4B>(Signal::Zero).listen([this] (ccColor4B value) {
        this->m_currentConfig.colorZero = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    ListenerHandle radioHdrd = SignalEvent<ccColor4B>(Signal::Hdrd).listen([this] (ccColor4B value) {
        this->m_currentConfig.colorHdrd = value;
        Mod::get()->setSavedValue(tabs[m_tab], m_currentConfig);
        this->m_previewBar->initShader(tabs[m_tab], this->defKey, this->defColor);
        return ListenerResult::Stop;
    });

    // rewrite setup ui
    bool init() override;

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

    // get
    static AdvancedMenu* get() {
        return CCScene::get()->getChildByType<AdvancedMenu>(0);
    }
    // create
    static AdvancedMenu* create() {
        auto ret = new AdvancedMenu();
        if (ret && ret->init()) {
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