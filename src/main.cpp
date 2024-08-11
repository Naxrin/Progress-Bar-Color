#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "head.hpp"

using namespace geode::prelude;

// tell me the actual color im gonna paint regarding color setup
void paint(auto target, BarColor color, BarColor def){
	auto gm = GameManager::sharedState();
	if (!target) return;

	switch(color.mode){
	case 0:
		switch (def.mode) {
		case 1:
			switch(def.follower) {
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
		case 2:
			target->setColor(def.color);
			break;
		default:		
			break;
		}
		break;
	case 1:
		switch(def.follower) {
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
	case 2:
		target->setColor(color.color);
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
			CCSprite *normal_now, *practice_now, *normal_bar, *practice_bar;
			if (getChildByID("better-pause-node")){
				auto menu = getChildByID("better-pause-node");
				normal_now = static_cast<CCSprite*>(menu->getChildByID("normal-bar")->getChildren()->objectAtIndex(1));
				practice_now = static_cast<CCSprite*>(menu->getChildByID("practice-bar")->getChildren()->objectAtIndex(1));
				normal_bar = static_cast<CCSprite*>(menu->getChildByID("normal-bar")->getChildren()->objectAtIndex(2));
				practice_bar = static_cast<CCSprite*>(menu->getChildByID("practice-bar")->getChildren()->objectAtIndex(2));

				// normal
				paint(
					normal_bar,
					Mod::get()->getSettingValue<BarColor>("pause-menu-normal"),
					Mod::get()->getSettingValue<BarColor>("normal-default")
				);
				paint(
					normal_now,
					Mod::get()->getSettingValue<BarColor>("pause-menu-normal"),
					Mod::get()->getSettingValue<BarColor>("normal-default")
				);
				
				//practice
				paint(
					practice_bar,
					Mod::get()->getSettingValue<BarColor>("pause-menu-practice"),
					Mod::get()->getSettingValue<BarColor>("practice-default")
				);
				paint(
					practice_now,
					Mod::get()->getSettingValue<BarColor>("pause-menu-practice"),
					Mod::get()->getSettingValue<BarColor>("practice-default")
				);
			}
			else{
				normal_bar = static_cast<CCSprite*>(getChildByID("normal-progress-bar")->getChildren()->objectAtIndex(0));
				practice_bar = static_cast<CCSprite*>(getChildByID("practice-progress-bar")->getChildren()->objectAtIndex(0));
			
				// normal
				paint(
					normal_bar,
					Mod::get()->getSettingValue<BarColor>("pause-menu-normal"),
					Mod::get()->getSettingValue<BarColor>("normal-default")
				);
				
				//practice
				paint(
					practice_bar,
					Mod::get()->getSettingValue<BarColor>("pause-menu-practice"),
					Mod::get()->getSettingValue<BarColor>("practice-default")
				);
			}
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
			// normal
			paint(
				static_cast<CCSprite* >(getChildByID("normal-mode-bar")->getChildren()->objectAtIndex(0)),
				Mod::get()->getSettingValue<BarColor>("info-menu-normal"),
				Mod::get()->getSettingValue<BarColor>("normal-default")
			);
			
			//practice
			paint(
				static_cast<CCSprite* >(this->getChildByID("practice-mode-bar")->getChildren()->objectAtIndex(0)),
				Mod::get()->getSettingValue<BarColor>("info-menu-practice"),
				Mod::get()->getSettingValue<BarColor>("practice-default")
			);
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
		auto ext = static_cast<CCNode*>(scroll->getChildren()->objectAtIndex(0));
		for (auto* obj: CCArrayExt<CCObject*>(ext->getChildren())){
			auto lvlpage = static_cast<CCNode*>(obj);	

			//normal					
			nb = static_cast<CCNode*>(lvlpage->getChildren()->objectAtIndex(1));
			normal_bar = static_cast<CCSprite*>(nb->getChildren()->objectAtIndex(0));
			paint(
				normal_bar,
				Mod::get()->getSettingValue<BarColor>("official-levels-normal"),
				Mod::get()->getSettingValue<BarColor>("normal-default")
			);

			//practice		
			pb = static_cast<CCNode*>(lvlpage->getChildren()->objectAtIndex(2));
			practice_bar = static_cast<CCSprite*>(pb->getChildren()->objectAtIndex(0));
			paint(
				practice_bar,
				Mod::get()->getSettingValue<BarColor>("official-levels-practice"),
				Mod::get()->getSettingValue<BarColor>("practice-default")
			);
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
			const char* grades[4] = {"quest-default", "quest-top", "quest-middle", "quest-bottom"};

			paint(
				target,
				Mod::get()->getSettingValue<BarColor>(grades[item->m_position]),
				Mod::get()->getSettingValue<BarColor>(grades[0])
			);			
		}
		return true;
	}
};

