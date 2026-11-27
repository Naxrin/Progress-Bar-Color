#include "head.hpp"

using namespace geode::prelude;

#include <Geode/modify/PauseLayer.hpp>
class $modify(MyPauseLayer, PauseLayer) {
	struct Fields {
		float delta = 0.f, delta1 = 0.f, delta2 = 0.f, delta3 = 0.f;
		bool plat = false;
		// pre-2.2 layout: separate normal / practice bar fills
		CCSprite* nb = nullptr, * pb = nullptr;
		CCGLProgram* ng = nullptr, * pg = nullptr;
		BarColor nc{}, pc{};
		// 2.2+ layout: only the current run fill is customized; the best run
		// indicator is left untouched so it keeps the game's default look
		CCSprite* current = nullptr;
		CCGLProgram* currentg = nullptr;
		BarColor cc{};
	};

	static void onModify(auto& self) {
        if (!self.setHookPriorityPost("PauseLayer::customSetup", Priority::Late))
            log::warn("Failed to set hook priority.");
    }

	void customSetup() override {
		PauseLayer::customSetup();
		refresh();

		auto pl = PlayLayer::get();
		if (!pl || !pl->m_level)
			return;

		auto level = pl->m_level;
		m_fields->plat = level->isPlatformer();
		if (m_fields->plat)
			return;

		// GD 2.2+ puts all pause menu content into a "main-layer" child, and
		// node-ids assigns the progress bar IDs on that layer. Fall back to
		// the layer itself in case the ID is missing.
		auto root = this->getChildByID("main-layer");
		if (!root)
			root = this;

		// old layout (Legacy Pause Progress): separate normal / practice bars
		if (auto bar = root->getChildByID("normal-progress-bar")) {
			m_fields->nb = bar->getChildByType<CCSprite>(0);
			m_fields->ng = init_shader(
				m_fields->nb, "pause-menu-normal", "normal", "pause-menu-normal",
				m_fields->nc, defCol1, level->m_normalPercent.value()
			);
		}
		if (auto bar = root->getChildByID("practice-progress-bar")) {
			m_fields->pb = bar->getChildByType<CCSprite>(0);
			m_fields->pg = init_shader(
				m_fields->pb, "pause-menu-practice", "practice", "pause-menu-practice",
				m_fields->pc, defCol2, level->m_practicePercent
			);
		}

		// new layout: a single bar showing the best run and the current run
		if (auto bar = root->getChildByID("progress-bar")) {
			bool practice = pl->m_isPracticeMode;
			auto advKey = practice ? "pause-menu-practice" : "pause-menu-normal";
			auto defKey = practice ? "practice" : "normal";
			auto defCol = practice ? defCol2 : defCol1;

			m_fields->current = typeinfo_cast<CCSprite*>(bar->getChildByID("current-bar"));
			if (!m_fields->current)
				m_fields->current = bar->getChildByType<CCSprite>(1);

			// best-bar is deliberately skipped: it should keep the game's
			// default gray "best progress" appearance
			m_fields->currentg = init_shader(
				m_fields->current, advKey, defKey, "pause-menu-current",
				m_fields->cc, defCol, pl->getCurrentPercentInt()
			);
		}

		schedule(schedule_selector(MyPauseLayer::dynamic));
	}
	void dynamic(float dt) {
		if (m_fields->plat)
			return;

		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);

		if (m_fields->ng)
			update_shader(m_fields->nb, m_fields->nc, m_fields->ng, dt, m_fields->delta, m_fields->delta1);
		if (m_fields->pg)
			update_shader(m_fields->pb, m_fields->pc, m_fields->pg, dt, m_fields->delta, m_fields->delta2);
		if (m_fields->currentg)
			update_shader(m_fields->current, m_fields->cc, m_fields->currentg, dt, m_fields->delta, m_fields->delta3);
	}
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
	struct Fields {
		bool plat = false;
		bool scheduled = false;
		float delta = 0.f, delta1 = 0.f, delta2 = 0.f;
		// the painted bar fills; retained so the pointers stay valid even when
		// the layer (or another mod) rebuilds the layout later on
		CCSprite* nb = nullptr, * pb = nullptr;
		CCGLProgram* ng = nullptr, * pg = nullptr;
		BarColor nc{}, pc{};

		~Fields() {
			if (nb)
				nb->release();
			if (pb)
				pb->release();
		}
	};

	static void onModify(auto& self) {
		// The bar IDs come from node-ids' `setupProgressBars` hook and are
		// only valid around that call: the nodes may be moved around or
		// re-identified by the rest of `init`. Grab them right afterwards.
		(void) self.setHookPriority("LevelInfoLayer::setupProgressBars", Priority::Early);
	}

	bool init(GJGameLevel* level, bool p) {
		if (!LevelInfoLayer::init(level, p))
			return false;
		if (!level)
			return true;

		m_fields->plat = level->isPlatformer();
		if (m_fields->plat)
			return true;

		// Normally already done by the setupProgressBars hook; retried here in
		// case the layer was built without (or before) the progress bars.
		this->paintBars(level);
		this->startPainting();
		return true;
	}

	// the only place where the two progress bars are created
	void setupProgressBars() {
		LevelInfoLayer::setupProgressBars();

		auto level = m_level;
		if (!level || level->isPlatformer())
			return;

		m_fields->plat = false;
		this->paintBars(level);
		this->startPainting();
	}

	void dynamic(float dt) {
		if (m_fields->plat || !m_level || m_level->isPlatformer())
			return;

		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);

		if (m_fields->ng)
			update_shader(m_fields->nb, m_fields->nc, m_fields->ng, dt, m_fields->delta, m_fields->delta1);
		if (m_fields->pg)
			update_shader(m_fields->pb, m_fields->pc, m_fields->pg, dt, m_fields->delta, m_fields->delta2);
	}

private:
	void startPainting() {
		if (m_fields->scheduled || (!m_fields->ng && !m_fields->pg))
			return;
		m_fields->scheduled = true;
		schedule(schedule_selector(MyLevelInfoLayer::dynamic));
	}

	void holdBar(CCSprite*& slot, CCSprite* fill) {
		if (slot == fill)
			return;
		if (slot)
			slot->release();
		slot = fill;
		if (slot)
			slot->retain();
	}

	// safe lookup: the node IDs are provided by node-ids and are not
	// guaranteed to exist (missing dependency, different node layout...)
	static CCSprite* findBarFill(CCNode* root, const char* id) {
		CCNode* bar = root->getChildByID(id);
		if (!bar)
			bar = root->getChildByIDRecursive(id);
		if (!bar) {
			log::warn("LevelInfoLayer: no \"{}\" node found ({} children), skipping", id, root->getChildrenCount());
			return nullptr;
		}
		auto fill = bar->getChildByType<CCSprite>(0);
		if (!fill)
			log::warn("LevelInfoLayer: \"{}\" has no fill sprite, skipping", id);
		return fill;
	}

	void paintBars(GJGameLevel* level) {
		if (!level || this->m_challenge)
			return;

		refresh();

		// look the bars up again every time: `setupProgressBars` gives us the
		// freshest nodes, and the retry from `init` catches layers whose bars
		// (or node IDs) only appeared after that call
		if (auto fill = findBarFill(this, "normal-mode-bar")) {
			if (fill != m_fields->nb || !m_fields->ng) {
				holdBar(m_fields->nb, fill);
				m_fields->ng = init_shader(
					fill, "info-menu-normal", "normal", "info-menu-normal",
					m_fields->nc, defCol1, level->m_normalPercent.value()
				);
			}
		}
		if (auto fill = findBarFill(this, "practice-mode-bar")) {
			if (fill != m_fields->pb || !m_fields->pg) {
				holdBar(m_fields->pb, fill);
				m_fields->pg = init_shader(
					fill, "info-menu-practice", "practice", "info-menu-practice",
					m_fields->pc, defCol2, level->m_practicePercent
				);
			}
		}
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
		auto target = m_mainLayer->getChildByID("chain-left");
		auto pl = PlayLayer::get();
		if (!target || !pl)
			return;
		if (pl->m_level->isPlatformer() && Mod::get()->getSettingValue<bool>("drop-bar-fix")) {
			target->setVisible(false);
			if (auto plabel = m_mainLayer->getChildByType<CCLabelBMFont>(5))
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
		if (auto icon = this->getChildByID("small-diamonds-icon"))
			icon->setVisible(false);

		auto goal = m_levelList->m_levels.size();
		auto progress = goal ? 100.f * m_levelList->completedLevels() / goal : 0;
		if (progress > 100)
			progress = 100;
		// set text label
		if (auto label = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("progress-bar-label")))
			label->setString(
				fmt::format("{}/{}", m_levelList->completedLevels(), goal).c_str());

		// set progress bar status
		if (auto bar = this->getChildByID("progress-bar")) {
			auto s = bar->getContentSize();
			if (m_fields->bar) {
				m_fields->bar->setTextureRect(CCRect(0, 0, s.width * progress / 100, s.height));

				// paint double default manually
				m_fields->prog = init_shader(m_fields->bar, "list-page-done", "list-done", "list-page-done", m_fields->c, defCol4, progress);
			}
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