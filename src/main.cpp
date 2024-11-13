#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "head.hpp"

using namespace geode::prelude;

// paint a target node (progress bar) regarding its keyname
// key = pause-menu/info-menu/official-levels/quest
// place = normal/practice/top/middle/bottom
void paint(auto target, std::string key, std::string place) {
	auto gm = GameManager::sharedState();
	// maginot line
	if (!target) return;
	std::string index = key + "-" + place;
	BarColor advcfg = Mod::get()->getSavedValue<BarColor>(index, place == "default" ? def2 : def1);
	std::string mode, follower;
	switch(advcfg.mode) {
	// common
	case 0:
		mode = Mod::get()->getSettingValue<std::string>(key + "-default-mode");
		if (mode == "Follow") {
			follower = Mod::get()->getSettingValue<std::string>(key + "-default-follower");
			if (follower == "Main")
				target->setColor(gm->colorForIdx(gm->getPlayerColor()));
			else if (follower == "Secondary")
				target->setColor(gm->colorForIdx(gm->getPlayerColor2()));
			else if (follower == "Glow")
				target->setColor(gm->colorForIdx(gm->getPlayerGlowColor()));
		} else if (mode == "Manual")
			target->setColor(Mod::get()->getSettingValue<ccColor3B>(key + "-default-color"));
		break;
	// follow
	case 1:
		switch(advcfg.follower) {
		case 0:
			target->setColor(gm->colorForIdx(gm->getPlayerColor()));
			break;
		case 1:
			target->setColor(gm->colorForIdx(gm->getPlayerColor2()));
			break;
		case 2:
			target->setColor(gm->colorForIdx(gm->getPlayerGlowColor()));
			break;
		default:
			break;
		}
		break;
	// manual
	case 2:
		target->setColor(advcfg.color);
		//log::info("paint {} {} {}", c.r, c.g, c.b);		
		break;
	default:
		break;
	}
}

class $modify(PauseLayer) {
	static void onModify(auto& self) {
        self.setHookPriority("PauseLayer::init", -100);}
    
	void customSetup() {
		PauseLayer::customSetup();

		auto playLayer = PlayLayer::get();
		bool plat = playLayer->m_level->isPlatformer();
		if (!plat){
			// don't crash plz
			if (!getChildByID("normal-progress-bar") || !getChildByID("normal-progress-bar")->getChildren()) return;
			// nodes
			auto normal_bar = static_cast<CCSprite*>(getChildByID("normal-progress-bar")->getChildren()->objectAtIndex(0));
			auto practice_bar = static_cast<CCSprite*>(getChildByID("practice-progress-bar")->getChildren()->objectAtIndex(0));
			// paint
			paint(normal_bar, "pause-menu", "normal");
			paint(practice_bar, "pause-menu", "practice");
		}
	}
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
	bool init(GJGameLevel* level, bool p) {
		if (!LevelInfoLayer::init(level, p)) {
			return false;
		}
		if (!level->isPlatformer()){
			// don't crash plz
			if (!getChildByID("normal-mode-bar") || !getChildByID("normal-mode-bar")->getChildren()) return true;
			// nodes
			auto normal_bar = static_cast<CCSprite*>(getChildByID("normal-mode-bar")->getChildren()->objectAtIndex(0));
			auto practice_bar = static_cast<CCSprite*>(getChildByID("practice-mode-bar")->getChildren()->objectAtIndex(0));
			// paint
			paint(normal_bar, "info-menu", "normal");
			paint(practice_bar, "info-menu", "practice");
		}
		return true;
	}
};

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(LevelSelectLayer) {
	bool init(int p) {
		if (!LevelSelectLayer::init(p))
			return false;
		CCNode *nb, *pb;
		CCSprite *normal_bar, *practice_bar;

		auto children = this->getChildren();
		auto scroll = getChildOfType<BoomScrollLayer>(this, 0);
		if (! scroll || !scroll->getChildren()) return true;
		auto ext = static_cast<CCNode*>(scroll->getChildren()->objectAtIndex(0));
		for (auto* obj: CCArrayExt<CCObject*>(ext->getChildren())){
			auto lvlpage = static_cast<CCNode*>(obj);

			// Overcharged Main Levels fix
			if (Loader::get()->isModLoaded("firee.overchargedlevels"))
				lvlpage = lvlpage->getChildByID("normal-progress-bar");

			// normal
			nb = static_cast<CCNode*>(lvlpage->getChildren()->objectAtIndex(1));
			normal_bar = static_cast<CCSprite*>(nb->getChildren()->objectAtIndex(0));
			// practice
			pb = static_cast<CCNode*>(lvlpage->getChildren()->objectAtIndex(2));
			practice_bar = static_cast<CCSprite*>(pb->getChildren()->objectAtIndex(0));	
			// paint
			paint(normal_bar, "official-levels", "normal");
			paint(practice_bar, "official-levels", "practice");
		}
		
		this->updateLayout();
		return true;
	}
};

#include <Geode/modify/ChallengeNode.hpp>
class $modify(ChallengeNode){
	bool init(GJChallengeItem *item, ChallengesPage *page, bool isNew){

		if(!ChallengeNode::init(item, page, isNew))
			return false;
		if (auto bar = getChildByID("progress-bar")){
			auto target = static_cast<CCSprite*>(bar->getChildren()->objectAtIndex(0));
			const char* grades[4] = {"default", "top", "middle", "bottom"};

			paint(target, "quest", grades[item->m_position]);			
		}
		return true;
	}
};


// Register setting
$execute {
    (void)Mod::get()->registerCustomSettingType("advanced-option", &AdvancedSetting::parse);
}
