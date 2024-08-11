#pragma once
#include <Geode/loader/SettingNode.hpp>

using namespace geode::prelude;

// struct setup
struct BarColor {
    int mode;
    int follower;
    ccColor3B color;
    bool operator == (const BarColor &c) const{
	    return ((mode == c.mode) && (follower == c.follower) && (color == c.color));
    };
};

template<>
struct matjson::Serialize<BarColor> {
    static BarColor from_json(matjson::Value const& value) {
        return BarColor {
            .mode = value["mode"].as_int(),
            .follower = value["follower"].as_int(),
            .color = ccColor3B(
                value["r"].as_int(),
                value["g"].as_int(),
                value["b"].as_int()
            )
        };
    }

    static matjson::Value to_json(BarColor const& value) {
        auto obj = matjson::Object();
        obj["mode"] = value.mode;
        obj["follower"] = value.follower;
        obj["r"] = value.color.r;
        obj["g"] = value.color.g;
        obj["b"] = value.color.b;
        return obj;
    }
};

// custom setting value
class BarColorValue : public SettingValue {
protected:
    BarColor bar_color;
public:
    BarColorValue(std::string const& key, std::string const& mod, BarColor b_color)
      : SettingValue(key, mod), bar_color(b_color) {}

    bool load(matjson::Value const& json) override {
        try {
            bar_color = BarColor {
                .mode = json["mode"].as_int(),
                .follower = json["follower"].as_int(),
                .color = ccColor3B(
                    json["r"].as_int(),
                    json["g"].as_int(),
                    json["b"].as_int()
                )
            };
            return true;
        } catch(...) {
            return false;
        }
    }

    bool save(matjson::Value& json) const override {
        // save the value of the setting into json,
        // returning true if saving was succesful
        json["mode"] = bar_color.mode;
        json["follower"] = bar_color.follower;
        json["r"] = bar_color.color.r;
        json["g"] = bar_color.color.g;
        json["b"] = bar_color.color.b;
        return true;
    }
    void setVal(BarColor val){
        bar_color = val;
    }
    BarColor getVal() const {
        return bar_color;
    }

    SettingNode* createNode(float width);
};
/*
class MyDelegate : public SettingNodeDelegate, public ColorPickPopupDelegate {
public:
    //virtual void updateColor(ccColor4B const &color) override;
        //ColorPickPopupDelegate::updateColor(color);
    virtual void setColor(ccColor4B const &color) = 0;

    //}
};*/

class BarColorNode : public SettingNode {
protected:
    BarColor m_current;

    const char* mode[3] = {"Default", "Sync", "Manual"};
    const char* follower[3] = {"Main", "Secondary", "Glow"};

    bool init(BarColorValue* value, float width);

    void changeVisibleState();
    ccColor3B getActualColor();

    void onModeSwitch(CCObject *sender);
    void onSyncSwitch(CCObject *sender);
    void onPickColor(CCObject *sender);
    void onDescription(CCObject *sender);

public:
    // setting
    void setColor(ccColor4B const &color);    
    void commit() override {
        static_cast<BarColorValue*>(m_value)->setVal(m_current);
        this->dispatchCommitted();
    }

    bool hasUncommittedChanges() override {
        return !(m_current == static_cast<BarColorValue*>(m_value)->getVal());
    }

    bool hasNonDefaultValue() override;

    void resetToDefault() override;

    static BarColorNode* create(BarColorValue* value, float width) {
        auto ret = new BarColorNode();
        if (ret->init(value, width)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

template<>
struct SettingValueSetter<BarColor> {
    static BarColor get(SettingValue* setup) {
        auto s = static_cast<BarColorValue*>(setup);
        struct BarColor val = s->getVal();
        return val;
    };
    static void set(SettingValue* s, BarColor const& value) {
        static_cast<BarColorValue*>(s)->setVal(value);
    };
};

class MyColorPickPopup : public ColorPickPopup {
protected:
    BarColorNode* node;
    /*
    bool init(float width, float height, const cocos2d::ccColor4B &args, bool arg, const char *bg, cocos2d::CCRect bgRect, BarColorNode *node) {
        if(!ColorPickPopup::init(width, height, args, arg, bg, bgRect)) return false;
        this->node = node;
        return true;
    }*/
    void onClose(CCObject *sender) override{
        node->setColor(m_color);
        ColorPickPopup::onClose(sender);
    }
public: 
    static MyColorPickPopup* create(ccColor4B color, bool isRGBA, BarColorNode *node) {
        auto ret = new MyColorPickPopup();
        if (ret->initAnchored(400.f, (isRGBA ? 290.f : 240.f), color, isRGBA)) {
            ret->node = node;
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};