#include <Geode/binding/GameManager.hpp>
#include <random>
#include <regex>
#include "head.hpp"

using namespace geode::prelude;

// global variables
ccColor3B main1, second1, glow1, main2, second2, glow2;

// load program
CCGLProgram* get_prog(const char* key, bool adv = false, bool reload = false, std::string vertfile = "", std::string fragfile = "") {
	if (!reload || !adv)
		if (auto program = shch->programForKey(key))
			return program;

	auto program = new CCGLProgram();
	std::string vert, frag;
	if (adv) {
		vert = file::readString(pathK / vertfile).unwrapOr(
			file::readString(pathR / "vert.glsl").unwrapOrDefault());
		frag = file::readString(pathK / fragfile).unwrapOr(
			file::readString(pathR / "frag.glsl").unwrapOrDefault());
	}
	else {
		vert = file::readString(pathR / "vert.glsl").unwrapOrDefault();
		frag = file::readString(pathR / "frag.glsl").unwrapOrDefault();		
	}

	if (!program->initWithVertexShaderByteArray(vert.c_str(), frag.c_str())) {
		CC_SAFE_DELETE(program);
		return nullptr;
	}

	program->addAttribute("a_position", kCCVertexAttrib_Position);
	program->addAttribute("a_texCoord", kCCVertexAttrib_TexCoords);
	program->addAttribute("a_color", kCCVertexAttrib_Color);

	if (!program->link()) {
		CC_SAFE_RELEASE(program);
		return nullptr;
	}

	CCShaderCache::sharedShaderCache()->addProgram(program, key);

	return program;
}

// init a target node (progress bar) regarding its keyname
// @param advKey key for advanced
// @param defKey key for default
// @param progress current percentage of this progress bar
// @param inCommon avoid infinite loop
void init_shader(CCSprite* target, std::string advKey, std::string defKey, BarColor &config, ccColor3B defCol, int progress, bool inCommon = false) {
	auto gm = GameManager::sharedState();
	// maginot line
	if (!target)
		return;
		
	auto key = inCommon ? defKey : advKey;
	// reference
	config = Mod::get()->getSavedValue<BarColor>(key, makeDefStruct(defCol, key));

	// skip if default
	if (config.mode == Mode::Default) {
		if (!inCommon)
			init_shader(target, defKey, defKey, config, defCol, progress, true);
		return;		
	}

	// claim the program
	CCGLProgram* prog;

	// advanced
	if (config.mode == Mode::Advanced) {
		prog = get_prog(key.c_str(), true, Mod::get()->getSettingValue<bool>("reload"), config.vert, config.frag);
		if (!prog)
			return;

		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), 0);
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("progress"), progress);

		prog->setUniformLocationWith3f(prog->getUniformLocationForName("main"), main1.r / 255.f, main1.g / 255.f, main1.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("second"), second1.r / 255.f, second1.g / 255.f, second1.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("glow"), glow1.r / 255.f, glow1.g / 255.f, glow1.b / 255.f);

		prog->setUniformLocationWith3f(prog->getUniformLocationForName("main2"), main2.r / 255.f, main2.g / 255.f, main2.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("second2"), second2.r / 255.f, second2.g / 255.f, second2.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("glow2"), glow2.r / 255.f, glow2.g / 255.f, glow2.b / 255.f);

		prog->updateUniforms();
		target->setShaderProgram(prog);
		return;		
	}

	prog = get_prog("0"_spr, false);
	if (!prog)
		return;

	if (config.mode == Mode::Follow) {
		// mode = static
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 0);
		switch (config.follower) {
		case 0:
			prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), main1.r / 255.f, main1.g / 255.f, main1.b / 255.f, 1);
			break;
		case 1:
			prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), second1.r / 255.f, second1.g / 255.f, second1.b / 255.f, 1);
			break;
		case 2:
			prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), glow1.r / 255.f, glow1.g / 255.f, glow1.b / 255.f, 1);
			break;			
		case 3:
			prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), main2.r / 255.f, main2.g / 255.f, main2.b / 255.f, 1);
			break;
		case 4:
			prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), second2.r / 255.f, second2.g / 255.f, second2.b / 255.f, 1);
			break;
		case 5:
			prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), glow2.r / 255.f, glow2.g / 255.f, glow2.b / 255.f, 1);
			break;
		default:
			break;
		}
	} else if (config.mode == Mode::Manual) {
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 0);
		prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"),
			config.color.r / 255.f, config.color.g / 255.f, config.color.b / 255.f, config.color.a / 255.f);
	}
	else if (config.mode == Mode::Chromatic) {
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 3);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), 0);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("wl"), config.length);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("satu"), config.satu / 100.f);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("brit"), config.brit / 100.f);
	} else if (config.mode == Mode::Gradient) {
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), int(config.gradType));
		prog->setUniformLocationWith4f(prog->getUniformLocationForName("colorl"),
			config.colorZero.r / 255.f, config.colorZero.g / 255.f, config.colorZero.b / 255.f, config.colorZero.a / 255.f);
		prog->setUniformLocationWith4f(prog->getUniformLocationForName("colorr"),
			config.colorHdrd.r / 255.f, config.colorHdrd.g / 255.f, config.colorHdrd.b / 255.f, config.colorHdrd.a / 255.f);	
		switch (config.gradType) {
		case GradType::Progress:
			prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), progress / 100.f);
			break;
		case GradType::Space:
			break;
		case GradType::Time:
			prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), 0);
			break;
		default:
			break;
		}
	} else if (config.mode == Mode::Random) {
		// random
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.f, 1.f);

		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 0);
		prog->setUniformLocationWith4f(prog->getUniformLocationForName("sc"), dis(gen), dis(gen), dis(gen), config.randa ? dis(gen) : 1);
	}

	prog->updateUniforms();

	target->setShaderProgram(prog);
}

// update a target node (progress bar) regarding its keyname
// @param advKey key for advanced
// @param defKey key for default
// @param progress current percentage of this progress bar
// @param inCommon avoid infinite loop
void update_shader(CCSprite* target, std::string advKey, std::string defKey, BarColor const &config, float const &dt, float const &ph0, float &ph, bool inCommon = false, bool tick = true) {
	// maginot line
	if (!target)
		return;
		

	if (!inCommon && tick)
		ph = fmod(ph + dt * config.speed, 1);

	// skip if default
	if (config.mode == Mode::Default) {
		if (!inCommon)
			update_shader(target, advKey, defKey, config, dt, ph0, ph, true);
		return;
	}

	auto key = inCommon ? defKey : advKey;
	// claim the program
	CCGLProgram* prog;
	auto phase = config.async ? ph : ph0;

	// advanced
	if (config.mode == Mode::Advanced) {
		prog = get_prog(key.c_str(), true, Mod::get()->getSettingValue<bool>("reload"), config.vert, config.frag);
		if (!prog)
			return;
		prog->use();
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), phase);
		prog->updateUniforms();
		return;		
	}

	prog = get_prog("0"_spr, false);
	if (!prog)
		return;
	prog->use();

	if (config.mode == Mode::Chromatic)
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), phase);
	else if (config.mode == Mode::Gradient && config.gradType == GradType::Time)
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), 1.0 - abs(2.0 * phase - 1.0));

	prog->updateUniforms();
}

#include <Geode/modify/PauseLayer.hpp>
class $modify(MyPauseLayer, PauseLayer) {
	struct Fields {
		float delta, delta1, delta2;
		bool plat, bp_loaded;
		CCNode* nb, * pb;
		BarColor nc, pc;
	};

	static void onModify(auto& self) {
        if (!self.setHookPriorityPost("PauseLayer::customSetup", Priority::Late))
            log::warn("Failed to set hook priority.");
    }

	void customSetup() override {
		PauseLayer::customSetup();

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
				init_shader(m_fields->nb->getChildByType<CCSprite>(1), "pause-menu-normal", "normal", m_fields->nc, defCol1, progress, true);
				init_shader(m_fields->nb->getChildByType<CCSprite>(2), "pause-menu-normal", "normal", m_fields->nc, defCol1, progress, true);

				m_fields->pb = node->getChildByID("practice-bar");
				init_shader(m_fields->pb->getChildByType<CCSprite>(1), "pause-menu-practice", "practice", m_fields->pc, defCol2, progress, true);
				init_shader(m_fields->pb->getChildByType<CCSprite>(2), "pause-menu-practice", "practice", m_fields->pc, defCol2, progress, true);
			}
		}
		else {
			// nodes
			m_fields->nb = this->getChildByID("normal-progress-bar");
			m_fields->pb = this->getChildByID("practice-progress-bar");

			// paint
			init_shader(m_fields->nb->getChildByType<CCSprite>(0), "pause-menu-normal", "normal", m_fields->nc, defCol1, progress, true);
			init_shader(m_fields->pb->getChildByType<CCSprite>(0), "pause-menu-practice", "practice", m_fields->pc, defCol2, progress, true);
		}

		schedule(schedule_selector(MyPauseLayer::dynamic));
	}
	void dynamic(float dt) {
		if (m_fields->plat)
			return;

		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);

		if (m_fields->bp_loaded) {
			if (auto node = this->getChildByID("better-pause-node")) {
				update_shader(m_fields->nb->getChildByType<CCSprite>(1), "pause-menu-normal", "normal", m_fields->nc, dt, m_fields->delta, m_fields->delta1);
				update_shader(m_fields->nb->getChildByType<CCSprite>(2), "pause-menu-normal", "normal", m_fields->nc, dt, m_fields->delta, m_fields->delta1, false, false);

				update_shader(m_fields->pb->getChildByType<CCSprite>(1), "pause-menu-practice", "practice", m_fields->pc, dt, m_fields->delta, m_fields->delta2);
				update_shader(m_fields->pb->getChildByType<CCSprite>(2), "pause-menu-practice", "practice", m_fields->pc, dt, m_fields->delta, m_fields->delta2, false, false);
			}
		}
		else {
			update_shader(m_fields->nb->getChildByType<CCSprite>(0), "pause-menu-normal", "normal", m_fields->nc, dt, m_fields->delta, m_fields->delta1);
			update_shader(m_fields->pb->getChildByType<CCSprite>(0), "pause-menu-practice", "practice", m_fields->pc, dt, m_fields->delta, m_fields->delta2);
		}
	}
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
	struct Fields {
		float delta, delta1, delta2;
		CCSprite* nb, * pb;
		BarColor nc, pc;
	};

	bool init(GJGameLevel* level, bool p) {
		if (!LevelInfoLayer::init(level, p))
			return false;
		if (level->isPlatformer())
			return true;

		int progress = level->m_normalPercent.value();
		// nodes
		m_fields->nb = this->getChildByID("normal-mode-bar")->getChildByType<CCSprite>(0);
		m_fields->pb = this->getChildByID("practice-mode-bar")->getChildByType<CCSprite>(0);
		// paint
		init_shader(m_fields->nb, "info-menu-normal", "normal", m_fields->nc, defCol1, level->m_normalPercent.value());
		init_shader(m_fields->pb, "info-menu-practice", "practice", m_fields->pc, defCol2, level->m_practicePercent);
		
		schedule(schedule_selector(MyLevelInfoLayer::dynamic));
		return true;
	}
	void dynamic(float dt) {
		if (this->m_level->isPlatformer())
			return;
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		update_shader(m_fields->nb, "info-menu-normal", "normal", m_fields->nc, dt, m_fields->delta, m_fields->delta1);
		update_shader(m_fields->pb, "info-menu-practice", "practice", m_fields->pc, dt, m_fields->delta, m_fields->delta2);
	}
};

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(DynamicOfficialLayer, LevelSelectLayer) {
	struct Fields {
		int m_scrolls;
		bool in_init = true;

		float delta, delta1, delta2;
		BarColor nc, pc;
	};
	bool init(int p) {
		if (!LevelSelectLayer::init(p))
			return false;

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
		bool tick = fmt::format("level-page-{}", m_scrolls % 3 + 1) == lvlpage->getID(); // wtf
		if (dt) {
			// paint
			if (lvlpage->m_normalProgressBar)
				update_shader(lvlpage->m_normalProgressBar, "official-levels-normal", "normal", m_fields->nc, dt, m_fields->delta, m_fields->delta1, false, tick);
			if (lvlpage->m_practiceProgressBar)
				update_shader(lvlpage->m_practiceProgressBar, "official-levels-practice", "practice", m_fields->pc, dt, m_fields->delta, m_fields->delta2, false, tick);
		} else {
			// paint
			if (lvlpage->m_normalProgressBar)
				init_shader(lvlpage->m_normalProgressBar, "official-levels-normal", "normal", m_fields->nc, defCol1, level->m_normalPercent.value());
			if (lvlpage->m_practiceProgressBar)
				init_shader(lvlpage->m_practiceProgressBar, "official-levels-practice", "practice", m_fields->pc, defCol2, level->m_practicePercent);
		}

	}
	void dynamic(float dt) {
		if (!this->m_scrollLayer->getChildByID("level-pages"))
			return;

		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		
		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", (m_scrolls + 2) % 3 + 1))), m_scrolls == 0 ? 23 : m_scrolls - 1, true);

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", m_scrolls % 3 + 1))), m_scrolls, true);

		this->paintProgressBar(
			static_cast<LevelPage*>(this->m_scrollLayer->getChildByID("level-pages")
				->getChildByID(fmt::format("level-page-{}", (m_scrolls + 1) % 3 + 1))), m_scrolls == 23 ? 0 : m_scrolls + 1, true);
	}
};

#include <Geode/modify/ChallengeNode.hpp>
class $modify(DynamicChallengeNode, ChallengeNode) {
	struct Fields {
		std::string grade;
		float delta, deltac;
		CCSprite* bar;
		BarColor c;
	};

	bool init(GJChallengeItem *item, ChallengesPage *page, bool isNew){

		if (!ChallengeNode::init(item, page, isNew))
			return false;
		if (auto bar = this->getChildByID("progress-bar")) {
			std::string grades[4] = {"default", "top", "middle", "bottom"};
			m_fields->bar = bar->getChildByType<CCSprite>(0);
			m_fields->grade = "quest-" + grades[item->m_position];
			init_shader(
				m_fields->bar, m_fields->grade, "quest", m_fields->c, defCol1, 100.f * item->m_count.value() / item->m_goal.value());
		}
		schedule(schedule_selector(DynamicChallengeNode::dynamic));
		return true;
	}

	void dynamic(float dt) {
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		update_shader(m_fields->bar, m_fields->grade, "quest", m_fields->c, dt, m_fields->delta, m_fields->deltac);
	}
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(DynamicLevelListLayer, LevelListLayer){
	struct Fields {
		float delta, deltac;
		CCSprite* bar;
		BarColor c;
	};

	bool init(GJLevelList *p){

		if (!LevelListLayer::init(p))
			return false;
		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(p);
		auto goal = isClaimed || !p->m_featured ? p->m_levels.size() : p->m_levelsToClaim;
		auto progress = goal ? 100.f * p->completedLevels() / goal : 0;
		log::debug("debug claim = {} size = {} goal = {} completed = {}", isClaimed, p->m_levels.size(), p->m_levelsToClaim, p->completedLevels());
		if (progress > 100)
			progress = 100;
		
		if (auto bar = this->getChildByID("progress-bar")) {
			m_fields->bar = bar->getChildByType<CCSprite>(0);
			init_shader(
				m_fields->bar,
				fmt::format("list-page-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				fmt::format("list-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
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
			init_shader(m_fields->bar, "list-page-done", "list-done", m_fields->c, defCol4, progress);
		}
	}

	void dynamic(float dt) {
		m_fields->delta = fmod(m_fields->delta + dt * Mod::get()->getSettingValue<float>("speed"), 1);
		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(m_levelList);
		update_shader(
			m_fields->bar,
			fmt::format("list-page-{}", m_levelList->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
			fmt::format("list-{}", m_levelList->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
			m_fields->c,
			dt,
			m_fields->delta,
			m_fields->deltac
		);
	}
};

#include <Geode/modify/LevelListCell.hpp>
class $modify(DynamicListCell, LevelListCell) {
	struct Fields {
		float delta, deltac;
		CCSprite* bar;
		BarColor c;
	};

	void loadFromList(GJLevelList *p){

		LevelListCell::loadFromList(p);
		bool isClaimed = GameStatsManager::sharedState()->hasClaimedListReward(p);
		auto goal = isClaimed || !p->m_featured ? p->m_levels.size() : p->m_levelsToClaim;
		auto progress = goal ? 100.f * p->completedLevels() / goal : 0;
		log::debug("debug claim = {} size = {} goal = {} completed = {}", isClaimed, p->m_levels.size(), p->m_levelsToClaim, p->completedLevels());
		if (progress > 100)
			progress = 100;
		if (auto bar = m_mainLayer->getChildByID("progress-bar")) {
			m_fields->bar = bar->getChildByType<CCSprite>(0);
			init_shader(
				m_fields->bar,
				fmt::format("list-cell-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
				fmt::format("list-{}", p->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
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
		update_shader(
			m_fields->bar,
			fmt::format("list-cell-{}", m_levelList->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
			fmt::format("list-{}", m_levelList->m_featured ? (isClaimed ? "done" : "todo") : "unf"),
			m_fields->c,
			dt,
			m_fields->delta,
			m_fields->deltac
		);
	}
};

// Register setting

$execute {
    //(void)Mod::get()->registerCustomSettingType("advanced-option", &AdvancedSetting::parse);

	//shch->addProgram();
}
