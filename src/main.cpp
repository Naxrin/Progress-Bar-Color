#include "head.hpp"

using namespace geode::prelude;

#include <Geode/modify/PauseLayer.hpp>
class $modify(MyPauseLayer, PauseLayer) {
	struct Fields {
		float delta, delta1, delta2;
		bool plat, bp_loaded;
		CCNode* nb, * pb;
		CCGLProgram* ng, * pg, * nbg, * pbg;
		BarColor nc, pc;
	};

	static void onModify(auto& self) {
        if (!self.setHookPriorityPost("PauseLayer::customSetup", Priority::Late))
            log::warn("Failed to set hook priority.");
    }

	void customSetup() override {
		PauseLayer::customSetup();
		refresh();

		auto level = PlayLayer::get()->m_level;
		m_fields->plat = level->isPlatformer();
		int progress = level->m_normalPercent.value();
		if (m_fields->plat)
			return;
			
		auto bp = Loader::get()->getLoadedMod("tpdea.betterpause-better");
		if (bp && bp->getSettingValue<int64_t>("type-pause") == 1) {
			m_fields->bp_loaded = true;
			log::warn("betterpause loaded");
			if (auto node = this->getChildByID("better-pause-node")) {
				log::warn("bp node found");
				m_fields->nb = node->getChildByID("normal-bar");
				m_fields->ng = init_shader(m_fields->nb->getChildByType<CCSprite>(1), "pause-menu-normal", "normal", "pause-menu-normal", m_fields->nc, defCol1, progress, true);
				m_fields->nbg = init_shader(m_fields->nb->getChildByType<CCSprite>(2), "pause-menu-normal", "normal", "pause-menu-normal-current", m_fields->nc, defCol1, progress, true);

				m_fields->pb = node->getChildByID("practice-bar");
				m_fields->pg = init_shader(m_fields->pb->getChildByType<CCSprite>(1), "pause-menu-practice", "practice", "pause-menu-practice", m_fields->pc, defCol2, progress, true);
				m_fields->pbg = init_shader(m_fields->pb->getChildByType<CCSprite>(2), "pause-menu-practice", "practice", "pause-menu-practice-current", m_fields->pc, defCol2, progress, true);
			}
		}
		else {
			// nodes
			m_fields->nb = this->getChildByID("normal-progress-bar");
			m_fields->pb = this->getChildByID("practice-progress-bar");

			// paint
			m_fields->ng = init_shader(m_fields->nb->getChildByType<CCSprite>(0), "pause-menu-normal", "normal", "pause-menu-normal", m_fields->nc, defCol1, progress, true);
			m_fields->pg = init_shader(m_fields->pb->getChildByType<CCSprite>(0), "pause-menu-practice", "practice", "pause-menu-practice", m_fields->pc, defCol2, progress, true);
		}

		schedule(schedule_selector(MyPauseLayer::dynamic));
	}
	void dynamic(float dt) {
		if (m_fields->plat)
			return;

		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);

		if (m_fields->bp_loaded) {
			if (auto node = this->getChildByID("better-pause-node")) {
				update_shader(m_fields->nb->getChildByType<CCSprite>(1), m_fields->nc, m_fields->ng, dt, m_fields->delta, m_fields->delta1);
				update_shader(m_fields->nb->getChildByType<CCSprite>(2), m_fields->nc, m_fields->nbg, dt, m_fields->delta, m_fields->delta1);

				update_shader(m_fields->pb->getChildByType<CCSprite>(1), m_fields->pc, m_fields->pg, dt, m_fields->delta, m_fields->delta2);
				update_shader(m_fields->pb->getChildByType<CCSprite>(2), m_fields->pc, m_fields->pbg, dt, m_fields->delta, m_fields->delta2);
			}
		}
		else {
			update_shader(m_fields->nb->getChildByType<CCSprite>(0), m_fields->nc, m_fields->ng, dt, m_fields->delta, m_fields->delta1);
			update_shader(m_fields->pb->getChildByType<CCSprite>(0), m_fields->pc, m_fields->pg, dt, m_fields->delta, m_fields->delta2);
		}
	}
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
	struct Fields {
		float delta, delta1, delta2;
		CCSprite* nb, * pb;
		CCGLProgram* ng, * pg;
		BarColor nc, pc;
	};

	bool init(GJGameLevel* level, bool p) {
		if (!LevelInfoLayer::init(level, p))
			return false;
		if (level->isPlatformer())
			return true;
			
		refresh();
		// nodes
		m_fields->nb = this->getChildByID("normal-mode-bar")->getChildByType<CCSprite>(0);
		m_fields->pb = this->getChildByID("practice-mode-bar")->getChildByType<CCSprite>(0);
		// paint
		m_fields->ng = init_shader(m_fields->nb, "info-menu-normal", "normal", "info-menu-normal", m_fields->nc, defCol1, level->m_normalPercent.value());
		m_fields->pg = init_shader(m_fields->pb, "info-menu-practice", "practice", "info-menu-practice", m_fields->pc, defCol2, level->m_practicePercent);
		
		schedule(schedule_selector(MyLevelInfoLayer::dynamic));
		return true;
	}
	void dynamic(float dt) {
		if (this->m_level->isPlatformer())
			return;
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		//log::debug("phase = {}", m_fields->delta);
		update_shader(m_fields->nb, m_fields->nc, m_fields->ng, dt, m_fields->delta, m_fields->delta1);
		update_shader(m_fields->pb, m_fields->pc, m_fields->pg, dt, m_fields->delta, m_fields->delta2);
	}
};

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(DynamicOfficialLayer, LevelSelectLayer) {
	struct Fields {
		int m_scrolls;
		bool in_init = true;

		float delta, delta1, delta2;
		CCGLProgram* ng[3], *pg[3];
		BarColor nc, pc;
	};
	bool init(int p) {
		if (!LevelSelectLayer::init(p))
			return false;

		refresh();

		if (!this->m_scrollLayer->getChildByID("level-pages"))
			return true;
		
		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", (p + 2) % 3 + 1))), p == 0 ? 23 : p - 1);

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", p % 3 + 1))), p);

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", (p + 1) % 3 + 1))), p == 23 ? 0 : p + 1);

		//log::debug("{} -> {} | {} -> {} | {} -> {}",
			//(p + 2) % 3 + 1, p == 0 ? 23 : p - 1, p % 3 + 1, p, (p + 1) % 3 + 1, p == 23 ? 0 : p + 1);
		m_fields->in_init = false;

		schedule(schedule_selector(DynamicOfficialLayer::dynamic));
		return true;
	}

	void scrollLayerMoved(CCPoint pos) override {
		LevelSelectLayer::scrollLayerMoved(pos);
		// not changing
		if (m_fields->in_init || m_scrolls == m_fields->m_scrolls)
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

	void paintProgressBar(LevelPage* lvlpage, int i, float dt = 0) {
		if (i > 21 || !lvlpage)
			return;

		auto level = GameLevelManager::get()->getMainLevel(i + 1, true);
		if (dt) {
			//bool tick = fmt::format("level-page-{}", m_scrolls % 3 + 1) == lvlpage->getID(); // wtf
			// paint
			update_shader(lvlpage->m_normalProgressBar, m_fields->nc, m_fields->ng[i % 3], dt, m_fields->delta, m_fields->delta1);
			update_shader(lvlpage->m_practiceProgressBar, m_fields->pc, m_fields->pg[i % 3], dt, m_fields->delta, m_fields->delta2);
		} else {
			// paint
			m_fields->ng[i % 3] = init_shader(lvlpage->m_normalProgressBar, "official-levels-normal", "normal", fmt::format("official-levels-normal_{}"_spr, i % 3), m_fields->nc, defCol1, level->m_normalPercent.value());
			m_fields->pg[i % 3] = init_shader(lvlpage->m_practiceProgressBar, "official-levels-practice", "practice", fmt::format("official-levels-practice_{}"_spr, i % 3), m_fields->pc, defCol2, level->m_practicePercent);
		}

	}
	void dynamic(float dt) {
		if (!this->m_scrollLayer->getChildByID("level-pages"))
			return;

		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);

		//log::debug("m_scrolls = {}", m_scrolls);
		
		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", (m_scrolls + 2) % 3 + 1))), m_scrolls == 0 ? 23 : m_scrolls - 1, dt);

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", m_scrolls % 3 + 1))), m_scrolls, dt);

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", (m_scrolls + 1) % 3 + 1))), m_scrolls == 23 ? 0 : m_scrolls + 1, dt);
	}
};

#include <Geode/modify/RetryLevelLayer.hpp>
class $modify(DynamicRetryLevelDropBar, RetryLevelLayer) {
	struct Fields {
		float delta, deltac;
		CCSprite* bar;
		CCGLProgram* prog;
		BarColor c;
	};

	void showLayer(bool p) override {
		RetryLevelLayer::showLayer(p);
		log::debug("hook RetryLevelLayer::showLayer({})", p);
		auto target = m_mainLayer->getChildByID("chain-left");
		auto pl = PlayLayer::get();
		log::debug("target = {}, {}", bool(target), bool(pl));
		if (!target || !pl)
			return;
		if (pl->m_level->isPlatformer() && Mod::get()->getSettingValue<bool>("drop-bar-fix")) {
			target->setVisible(false);
			if (auto plabel = m_mainLayer->getChildByType<CCLabelBMFont>(3))
				plabel->setVisible(false);
			return;
		}

		refresh();
		m_fields->bar = target->getChildByType<CCSprite>(0);
		m_fields->prog = init_shader(
			m_fields->bar, "drop-bar", "normal", "drop-bar",
			m_fields->c, defCol1, pl->getCurrentPercentInt());

		log::debug("percent = {}", pl->getCurrentPercentInt());

		schedule(schedule_selector(DynamicRetryLevelDropBar::dynamic));
	}

	void dynamic(float dt) {
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		update_shader(m_fields->bar, m_fields->c, m_fields->prog, dt, m_fields->delta, m_fields->deltac);
	}
};

#include <Geode/modify/ChallengeNode.hpp>
class $modify(DynamicChallengeNode, ChallengeNode) {
	struct Fields {
		std::string grade;
		float delta, deltac;
		CCSprite* bar;
		CCGLProgram* prog;
		BarColor c;
	};

	bool init(GJChallengeItem *item, ChallengesPage *page, bool isNew){

		if (!ChallengeNode::init(item, page, isNew))
			return false;

		refresh();

		if (auto bar = this->getChildByID("progress-bar")) {
			std::string grades[4] = {"default", "top", "middle", "bottom"};
			m_fields->bar = bar->getChildByType<CCSprite>(0);
			m_fields->grade = "quest-" + grades[item->m_position];
			m_fields->prog = init_shader(
				m_fields->bar, m_fields->grade, "quest", m_fields->grade, 
				m_fields->c, defCol1, 100.f * item->m_count.value() / item->m_goal.value()
			);
		}
		schedule(schedule_selector(DynamicChallengeNode::dynamic));
		return true;
	}

	void dynamic(float dt) {
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		update_shader(m_fields->bar, m_fields->c, m_fields->prog, dt, m_fields->delta, m_fields->deltac);
	}
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(DynamicLevelListLayer, LevelListLayer){
	struct Fields {
		float delta, deltac;
		CCSprite* bar;
		CCGLProgram* prog;
		BarColor c;
	};

	bool init(GJLevelList *p){
		if (!LevelListLayer::init(p))
			return false;

		refresh();

		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(p);
		auto goal = isClaimed || !p->m_featured ? p->m_levels.size() : p->m_levelsToClaim;
		auto progress = goal ? 100.f * p->completedLevels() / goal : 0;
		//log::debug("debug claim = {} size = {} goal = {} completed = {}", isClaimed, p->m_levels.size(), p->m_levelsToClaim, p->completedLevels());
		if (progress > 100)
			progress = 100;
		
		if (auto bar = this->getChildByID("progress-bar")) {
			m_fields->bar = bar->getChildByType<CCSprite>(0);
			m_fields->prog = init_shader(
				m_fields->bar,
				fmt::format("list-page-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				fmt::format("list-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				fmt::format("list-page-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				m_fields->c,
				isClaimed || !p->m_featured ? defCol4 : defCol3,
				progress
			);			
		}

		schedule(schedule_selector(DynamicLevelListLayer::dynamic));
		return true;
	}

	void onClaimReward(CCObject* sender) {
		LevelListLayer::onClaimReward(sender);
		if (!GameStatsManager::sharedState()->hasClaimedListReward(m_levelList))
			return;
			
		//log::error("claimed = {}", GameStatsManager::sharedState()->hasClaimedListReward(m_levelList));
		// hide diamond icon
		this->getChildByID("small-diamonds-icon")->setVisible(false);

		auto goal = m_levelList->m_levels.size();
		auto progress = goal ? 100.f * m_levelList->completedLevels() / goal : 0;
		if (progress > 100)
			progress = 100;
		// set text label
		static_cast<CCLabelBMFont*>(this->getChildByID("progress-bar-label"))->setString(
			fmt::format("{}/{}", m_levelList->completedLevels(), goal).c_str());

		// set progress bar status
		if (auto bar = this->getChildByID("progress-bar")) {
			auto s = bar->getContentSize();
			m_fields->bar->setTextureRect(CCRect(0, 0, s.width * progress / 100, s.height));
			
			// paint double default manually
			m_fields->prog = init_shader(m_fields->bar, "list-page-done", "list-done", "list-page-done", m_fields->c, defCol4, progress);
		}
	}

	void dynamic(float dt) {
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(m_levelList);
		update_shader(m_fields->bar, m_fields->c, m_fields->prog, dt, m_fields->delta, m_fields->deltac);
	}
};

#include <Geode/modify/LevelListCell.hpp>
class $modify(DynamicListCell, LevelListCell) {
	struct Fields {
		float delta, deltac;
		CCSprite* bar;
		CCGLProgram* prog;
		BarColor c;
	};

	void loadFromList(GJLevelList *p){
		LevelListCell::loadFromList(p);

		refresh();

		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(p);
		auto goal = isClaimed || !p->m_featured ? p->m_levels.size() : p->m_levelsToClaim;
		auto progress = goal ? 100.f * p->completedLevels() / goal : 0;
		//log::debug("debug claim = {} size = {} goal = {} completed = {}", isClaimed, p->m_levels.size(), p->m_levelsToClaim, p->completedLevels());
		if (progress > 100)
			progress = 100;
		if (auto bar = m_mainLayer->getChildByID("progress-bar")) {
			m_fields->bar = bar->getChildByType<CCSprite>(0);
			m_fields->prog = init_shader(
				m_fields->bar,
				fmt::format("list-cell-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				fmt::format("list-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				fmt::format("list-cell-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				m_fields->c,
				isClaimed || !p->m_featured ? defCol4 : defCol3,
				progress
			);			
		}
		schedule(schedule_selector(DynamicListCell::dynamic));
	}
	void dynamic(float dt) {
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(m_levelList);
		update_shader(m_fields->bar,  m_fields->c, m_fields->prog, dt, m_fields->delta, m_fields->deltac);
	}
};

// Register setting

$execute {
    //(void)Mod::get()->registerCustomSettingType("advanced-option", &AdvancedSetting::parse);

	//shch->addProgram();
}
