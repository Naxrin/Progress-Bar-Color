#pragma once
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCMenuItemToggler.hpp>
using namespace geode::prelude;

// struct setup for a color channel
/**/
struct BarColor {
    int mode;
    int follower;
    ccColor3B color;
    bool operator == (const BarColor &c) const{
	    return ((mode == c.mode) && (follower == c.follower) && (color == c.color));
    };
};

static BarColor def1 = BarColor{.mode = 0, .follower = 0, .color=ccColor3B(0, 255, 0)};
static BarColor def2 = BarColor{.mode = 0, .follower = 0, .color=ccColor3B(255, 255, 0)};

template<>
struct matjson::Serialize<BarColor> {
    static Result<BarColor> fromJson(matjson::Value const& value) {
        return Ok(BarColor{
            .mode = (int) value["mode"].asInt().unwrap(),
            .follower = (int) value["follower"].asInt().unwrap(),
            .color = ccColor3B(
                (int) value["r"].asInt().unwrap(),
                (int) value["g"].asInt().unwrap(),
                (int) value["b"].asInt().unwrap()
            )
        });
    }

    static matjson::Value toJson(BarColor const& value) {
        return matjson::makeObject({
            {"mode", value.mode},
            {"follower", value.follower},
            {"r", value.color.r},
            {"g", value.color.g},
            {"b", value.color.b}
        });
    }
    static bool isJson(matjson::Value const& json) {
        if (!json.isObject()) return false;
        if (!(json.contains("mode") && json["mode"].isNumber() && (int) json["mode"].asInt().unwrap() < 3 && (int) json["mode"].asInt().unwrap() >= 0)) return false;
        if (!(json.contains("follower") && json["follower"].isNumber() && (int) json["follower"].asInt().unwrap() < 3 && (int) json["follower"].asInt().unwrap() >= 0)) return false;
        if (!(json.contains("r") && json["r"].isNumber() && (int)json["r"].asInt().unwrap() < 256 && (int)json["r"].asInt().unwrap() >= 0)) return false;
        if (!(json.contains("g") && json["g"].isNumber() && (int)json["g"].asInt().unwrap() < 256 && (int)json["g"].asInt().unwrap() >= 0)) return false;
        if (!(json.contains("b") && json["b"].isNumber() && (int)json["b"].asInt().unwrap() < 256 && (int)json["b"].asInt().unwrap() >= 0)) return false;
        return true;
    }
};

class AdvancedMenu : public Popup<>, public ColorPickPopupDelegate {
protected:
    int currentTab;
    BarColor currentConfig;
    bool setup() override;
    void initialize();
    void onSwitchTab(CCObject*);
    void onSetMode(CCObject*);
    void onSetFollower(CCObject*);
    void onPickColor(CCObject*);
    void updateColor(ccColor4B const& color) override;
    void onClose(CCObject*) override;
public:
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
    bool load(matjson::Value const& json) override {
        return true;
    }
    bool save(matjson::Value& json) const override {
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