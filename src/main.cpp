#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>

#include <random>

#include "head.hpp"

using namespace geode::prelude;

inline ccColor3B getGradient(float p, ccColor3B const &c1, ccColor3B const &c2) {
    return ccc3(
        c1.r + (p / 100) * (c2.r - c1.r),
        c1.g + (p / 100) * (c2.g - c1.g),
        c1.b + (p / 100) * (c2.b - c1.b)
    );
}

// paint a target node (progress bar) regarding its keyname
// @param advKey key for advanced
// @param defKey key for default
// @param progress current percentage of this progress bar
// @param inCommon avoid infinite stack
void paint(CCSprite* target, std::string advKey, std::string defKey, int progress, bool inCommon = false) {
	auto gm = GameManager::sharedState();
	// maginot line
	if (!target)
		return;
	// reference
	BarColor config = BarColor();

	// random
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(0, 255);

	if (inCommon) {
		for (auto mode : modes) {
			if (mode == Mod::get()->getSettingValue<std::string>(defKey + "-default-mode"))
				break;
			config.mode++;
		}
		for (auto followed : followeds) {
			if (followed == Mod::get()->getSettingValue<std::string>(defKey + "-default-follower"))
				break;
			config.follower++;
		}
		config.color = config.colorZero = Mod::get()->getSettingValue<ccColor3B>(defKey + "-default-color");
		config.colorHdrd = Mod::get()->getSettingValue<ccColor3B>(defKey + "-default-another");
	} else
		config = Mod::get()->getSavedValue<BarColor>(advKey, advKey.find("practice") ? def2 : def1);

	switch(config.mode) {
	// common
	case 0:
		if (!inCommon)
			paint(target, advKey, defKey, progress, true);
		return;
	// follow
	case 1:
		// SDI
		if (auto SDI = Loader::get()->getLoadedMod("weebify.separate_dual_icons")) {
			switch (config.follower) {
			case 3:
				target->setColor(gm->colorForIdx(SDI->getSavedValue<int64_t>("color1")));
				return;
			case 4:
				target->setColor(gm->colorForIdx(SDI->getSavedValue<int64_t>("color2")));
				return;
			case 5:
				target->setColor(gm->colorForIdx(SDI->getSavedValue<int64_t>("colorglow")));
				return;
			default:
				break;
			}
		}
		// Robtop's raw game
		config.follower = config.follower % 3;
		switch (config.follower) {
		case 0:
			target->setColor(gm->colorForIdx(gm->getPlayerColor()));
			return;
		case 1:
			target->setColor(gm->colorForIdx(gm->getPlayerColor2()));
			return;
		case 2:
			target->setColor(gm->colorForIdx(gm->getPlayerGlowColor()));
			return;
		}
	// manual
	case 2:
		target->setColor(config.color);
		return;
	// progress
	case 3:
		target->setColor(getGradient(progress, config.colorZero, config.colorHdrd));
		return;
	// random
	case 4:
		target->setColor(ccc3(distrib(gen), distrib(gen), distrib(gen)));
		return;
	default:
		return;
	}
}

#include <Geode/modify/PauseLayer.hpp>
class $modify(PauseLayer) {

	void customSetup() {
		PauseLayer::customSetup();

		auto level = PlayLayer::get()->m_level;
		bool plat = level->isPlatformer();
		int progress = level->m_normalPercent.value();
		if (plat)
			return;

		// nodes
		auto normal_bar = this->getChildByID("normal-progress-bar")->getChildByType<CCSprite>(0);
		auto practice_bar = this->getChildByID("practice-progress-bar")->getChildByType<CCSprite>(0);

		// paint
		paint(normal_bar, "pause-menu-normal", "normal", progress);
		paint(practice_bar, "pause-menu-practice", "practice", progress);
	}
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
	bool init(GJGameLevel* level, bool p) {
		if (!LevelInfoLayer::init(level, p))
			return false;
		if (level->isPlatformer())
			return true;

		int progress = level->m_normalPercent.value();
		// nodes
		auto normal_bar = this->getChildByID("normal-mode-bar")->getChildByType<CCSprite>(0);
		auto practice_bar = this->getChildByID("practice-mode-bar")->getChildByType<CCSprite>(0);
		// paint
		paint(normal_bar, "info-menu-normal", "normal", level->m_normalPercent.value());
		paint(practice_bar, "info-menu-practice", "practice", level->m_practicePercent);
		
		return true;
	}
};

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(LevelSelectLayer) {
	struct Fields {
		int m_scrolls;
	};
	bool init(int p) {
		if (!LevelSelectLayer::init(p))
			return false;

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID("level-page-1")), 0);
		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID("level-page-2")), 1);
		return true;
	}

	void scrollLayerMoved(CCPoint pos) override {
		LevelSelectLayer::scrollLayerMoved(pos);
		// not changing
		if (m_scrolls == m_fields->m_scrolls)
			return;

		const int forward = (2 * m_scrolls - m_fields->m_scrolls + 24) % 24;
		// what the fuck is this
		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", forward % 3 + 1))),
			forward
		);
		m_fields->m_scrolls = m_scrolls;
	}

	void paintProgressBar(LevelPage* lvlpage, int i) {
		if (i > 21)
			return;

		auto level = GameLevelManager::get()->getMainLevel(i + 1, true);
		// paint
		if (lvlpage->m_normalProgressBar)
			//m_normalProgressBar->setID(level->m_levelName);
			paint(lvlpage->m_normalProgressBar, "official-levels-normal", "normal", level->m_normalPercent.value());
		if (lvlpage->m_practiceProgressBar)
			//m_practiceProgressBar->setID(level->m_levelName);
			paint(lvlpage->m_practiceProgressBar, "official-levels-practice", "practice", level->m_practicePercent);		
	}
};

#include <Geode/modify/ChallengeNode.hpp>
class $modify(ChallengeNode){
	bool init(GJChallengeItem *item, ChallengesPage *page, bool isNew){

		if (!ChallengeNode::init(item, page, isNew))
			return false;
		if (auto bar = this->getChildByID("progress-bar")){
			std::string grades[4] = {"default", "top", "middle", "bottom"};
			paint(
				bar->getChildByType<CCSprite>(0),
				"quest-" + grades[item->m_position], "quest", (100.f * item->m_count.value() / item->m_goal.value()));
		}
		return true;
	}
};


// Register setting

$execute {
    (void)Mod::get()->registerCustomSettingType("advanced-option", &AdvancedSetting::parse);
}
