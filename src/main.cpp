#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

ccColor3B paint(bool practice){
	ccColor3B c;
	const char *follow, *manual;
	if (practice){
		follow = "practice-follow";
		manual = "practice-manual";
	}else{
		follow = "normal-follow";
		manual = "normal-manual";
	}
	auto gm = GameManager::sharedState();

	switch(Mod::get()->getSettingValue<int64_t>(follow)){
		case 1:
			c = gm->colorForIdx(gm->getPlayerColor());
			break;
		case 2:
			c = gm->colorForIdx(gm->getPlayerColor2());
			break;
		case 3:
			c = gm->colorForIdx(gm->getPlayerGlowColor());
			break;
		default:
			c = Mod::get()->getSettingValue<ccColor3B>(manual);
			break;
	}
	return c;
}

class $modify(PauseLayer) {
	static void onModify(auto& self) {
        self.setHookPriority("PauseLayer::create", -100);}
    
	static PauseLayer* create(bool p0) {
		auto pl = PauseLayer::create(p0);

		auto playLayer = PlayLayer::get();
		bool plat = playLayer->m_level->isPlatformer();
		if ((!plat)&&(Mod::get()->getSettingValue<bool>("pause-layer"))){

			CCSprite *normal_now, *practice_now, *normal_bar, *practice_bar;
			if (pl->getChildByID("better-pause-node")){
				auto menu = pl->getChildByID("better-pause-node");
				normal_now = static_cast<CCSprite*>(menu->getChildByID("normal-bar")->getChildren()->objectAtIndex(1));
				practice_now = static_cast<CCSprite*>(menu->getChildByID("practice-bar")->getChildren()->objectAtIndex(1));
				normal_bar = static_cast<CCSprite*>(menu->getChildByID("normal-bar")->getChildren()->objectAtIndex(2));
				practice_bar = static_cast<CCSprite*>(menu->getChildByID("practice-bar")->getChildren()->objectAtIndex(2));

				// normal
				ccColor3B cb1 = paint(false);
				normal_bar->setColor(cb1);
				normal_now->setColor(cb1);
				
				//practice
				ccColor3B cb2 = paint(true);
				practice_bar->setColor(cb2);
				practice_now->setColor(cb2);
			}
			else{
				normal_bar = static_cast<CCSprite*>(pl->getChildByID("normal-progress-bar")->getChildren()->objectAtIndex(0));
				practice_bar = static_cast<CCSprite*>(pl->getChildByID("practice-progress-bar")->getChildren()->objectAtIndex(0));
			
				// normal
				ccColor3B cb1 = paint(false);
				normal_bar->setColor(cb1);
				
				//practice
				ccColor3B cb2 = paint(true);
				practice_bar->setColor(cb2);
			}
		}
		return pl;
	}
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
	bool init(GJGameLevel* level, bool p) {
		if (!LevelInfoLayer::init(level, p)) {
			return false;
		}
		if ((!level->isPlatformer())&&Mod::get()->getSettingValue<bool>("level-info-layer")){
		
			// normal
			ccColor3B cb1 = paint(false);
			auto normal_bar = static_cast<CCSprite* >(this->getChildByID("normal-mode-bar")->getChildren()->objectAtIndex(0));
			normal_bar->setColor(cb1);

			//practice
			ccColor3B cb2 = paint(true);
			auto practice_bar = static_cast<CCSprite* >(this->getChildByID("practice-mode-bar")->getChildren()->objectAtIndex(0));
			practice_bar->setColor(cb2);
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

		ccColor3B cb1 = paint(false);
		ccColor3B cb2 = paint(true);

		auto children = this->getChildren();
		auto scroll = getChildOfType<BoomScrollLayer>(this, 0);
		auto ext = static_cast<CCNode*>(scroll->getChildren()->objectAtIndex(0));
		if (Mod::get()->getSettingValue<bool>("official-level")){
			for (int i=0; i<3; i++){
				auto lvlpage = static_cast<CCNode*>(ext->getChildren()->objectAtIndex(i));	

				//normal					
				nb = static_cast<CCNode*>(lvlpage->getChildren()->objectAtIndex(1));
				normal_bar = static_cast<CCSprite*>(nb->getChildren()->objectAtIndex(0));
				normal_bar->setColor(cb1);
				
				//practice		
				pb = static_cast<CCNode*>(lvlpage->getChildren()->objectAtIndex(2));
				practice_bar = static_cast<CCSprite*>(pb->getChildren()->objectAtIndex(0));
				practice_bar->setColor(cb2);
			}
		}
		this->updateLayout();
		return true;
	}
};
