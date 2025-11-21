#include "head.hpp"

extern std::set<std::string> have_cache;

void refresh() {
	auto gm = GameManager::sharedState();

	main1 = gm->colorForIdx(gm->getPlayerColor());
	second1 = gm->colorForIdx(gm->getPlayerColor2());
	glow1 = gm->colorForIdx(gm->getPlayerGlowColor());

	if (auto SDI = Loader::get()->getLoadedMod("weebify.separate_dual_icons")) {
		main2 = gm->colorForIdx(SDI->getSavedValue<int64_t>("color1"));
		second2 = gm->colorForIdx(SDI->getSavedValue<int64_t>("color2"));
		glow2 = gm->colorForIdx(SDI->getSavedValue<int64_t>("colorglow"));
	} else {
		main2 = gm->colorForIdx(gm->getPlayerColor());
		second2 = gm->colorForIdx(gm->getPlayerColor2());
		glow2 = gm->colorForIdx(gm->getPlayerGlowColor());
	}
}

// load program
CCGLProgram* get_prog(std::string key, bool adv = false, std::string vertfile = "", std::string fragfile = "") {
	std::string full = fmt::format("{}"_spr, key);
	
	if (have_cache.contains(key)) {
		// preview not allowed
		if (!AdvancedMenu::get() && (!adv || !Mod::get()->getSettingValue<bool>("reload")))
			if (auto program = shch->programForKey(full.c_str())) {
				log::debug("program {} is already in index", full.c_str());
				return program;		
			}
	} else
		have_cache.insert(key);
	
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

	//log::error("vert = {}\nfrag = {}", vert, frag);
	if (!program->initWithVertexShaderByteArray(vert.c_str(), frag.c_str())) {
		log::debug("shader for {} init failed!", key);
		CC_SAFE_DELETE(program);
		return nullptr;
	}

	//log::debug("shader for {} init done!", key);
	program->addAttribute("a_position", kCCVertexAttrib_Position);
	program->addAttribute("a_texCoord", kCCVertexAttrib_TexCoords);
	program->addAttribute("a_color", kCCVertexAttrib_Color);

	if (!program->link()) {
		log::debug("shader for {} link failed!", key);
		CC_SAFE_RELEASE(program);
		return nullptr;
	}

	CCShaderCache::sharedShaderCache()->addProgram(program, full.c_str());
	log::debug("shader for {} create successfully!", key);
	return program;
}

CCGLProgram* init_shader(
	CCSprite* target, std::string advKey, std::string defKey, std::string progKey,
	BarColor &config, ccColor3B defCol, int progress, bool inCommon) {
	// maginot line
	if (!target)
		return nullptr;
		
	auto key = inCommon ? defKey + "-default" : advKey;
	// reference
	config = Mod::get()->getSavedValue<BarColor>(key, makeDefStruct(defCol, key));

	// skip if default
	if (config.mode == Mode::Default) {
		//log::debug("sdv = {} def = {}", advKey, defKey)	;	
		if (!inCommon)
			return init_shader(target, advKey, defKey, progKey, config, defCol, progress, true);
	}

	// claim the program
	CCGLProgram* prog;

	// advanced
	if (config.mode == Mode::Advanced) {
		prog = get_prog(progKey, true, config.vert, config.frag);
		log::debug("custom shader for {} init {}! vert = {} frag = {}", progKey, prog ? "done" : "failed", config.vert, config.frag);
		if (!prog)
			return nullptr;

		prog->use();

		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), config.phase / 360.f);
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("progress"), progress);

		prog->setUniformLocationWith3f(prog->getUniformLocationForName("main"), main1.r / 255.f, main1.g / 255.f, main1.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("second"), second1.r / 255.f, second1.g / 255.f, second1.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("glow"), glow1.r / 255.f, glow1.g / 255.f, glow1.b / 255.f);

		prog->setUniformLocationWith3f(prog->getUniformLocationForName("main2"), main2.r / 255.f, main2.g / 255.f, main2.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("second2"), second2.r / 255.f, second2.g / 255.f, second2.b / 255.f);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("glow2"), glow2.r / 255.f, glow2.g / 255.f, glow2.b / 255.f);

		prog->updateUniforms();
		target->setShaderProgram(prog);
		return prog;		
	}

	prog = get_prog(progKey, false);
	log::debug("common shader for {} init {}!", progKey, prog ? "done" : "failed");
	if (!prog)
		return nullptr;

	prog->use();

	if (config.mode == Mode::Follow) {
		// mode = static
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 0);
        prog->setUniformLocationWith1f(prog->getUniformLocationForName("alpha"), 1.f);
		switch (config.follower) {
		case 0:
			prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), main1.r / 255.f, main1.g / 255.f, main1.b / 255.f);
			break;
		case 1:
			prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), second1.r / 255.f, second1.g / 255.f, second1.b / 255.f);
			break;
		case 2:
			prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), glow1.r / 255.f, glow1.g / 255.f, glow1.b / 255.f);
			break;			
		case 3:
			prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), main2.r / 255.f, main2.g / 255.f, main2.b / 255.f);
			break;
		case 4:
			prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), second2.r / 255.f, second2.g / 255.f, second2.b / 255.f);
			break;
		case 5:
			prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), glow2.r / 255.f, glow2.g / 255.f, glow2.b / 255.f);
			break;
		default:
			break;
		}
	} else if (config.mode == Mode::Manual) {
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 0);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"),
			config.color.r / 255.f, config.color.g / 255.f, config.color.b / 255.f);
        prog->setUniformLocationWith1f(prog->getUniformLocationForName("alpha"), config.color.a / 255.f);
	}
	else if (config.mode == Mode::Chromatic) {
		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 3);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), config.phase / 360.f);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("freq"), config.length);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("satu"), config.satu / 100.f);
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("brit"), config.brit / 100.f);
	} else if (config.mode == Mode::Gradient) {
		switch (config.gradType) {
		case GradType::Progress:
            prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 4);
			prog->setUniformLocationWith1f(prog->getUniformLocationForName("progress"), progress);
			break;
		case GradType::Space:
            prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 1);
			break;
		case GradType::Time:
            prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 2);
			prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), 1.0 - abs(config.phase / 180.f - 1.0));
			break;
		default:
			break;
		}		

		prog->setUniformLocationWith4f(prog->getUniformLocationForName("colorl"),
			config.colorZero.r / 255.f, config.colorZero.g / 255.f, config.colorZero.b / 255.f, config.colorZero.a / 255.f);
		prog->setUniformLocationWith4f(prog->getUniformLocationForName("colorr"),
			config.colorHdrd.r / 255.f, config.colorHdrd.g / 255.f, config.colorHdrd.b / 255.f, config.colorHdrd.a / 255.f);	

	} else if (config.mode == Mode::Random) {
		// random
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.f, 1.f);

		prog->setUniformLocationWith1i(prog->getUniformLocationForName("mode"), 0);
		prog->setUniformLocationWith3f(prog->getUniformLocationForName("sc"), dis(gen), dis(gen), dis(gen));
        prog->setUniformLocationWith1f(prog->getUniformLocationForName("alpha"), config.randa ? dis(gen) : 1);
	}

	prog->updateUniforms();

	target->setShaderProgram(prog);

	return prog;
}

void update_shader(
	CCSprite* target, BarColor const &config, CCGLProgram* prog, 
	float const &dt, float const &ph0, float &ph, bool inCommon) {
	// maginot line
	if (!target || !prog)
		return;
		

	if (!inCommon)
		ph = fmod(ph + dt * config.speed, 1);


	// skip if default
	if (config.mode == Mode::Default) {
		if (!inCommon)
			update_shader(target, config, prog, dt, ph0, ph, true);
		return;
	}

	prog->use();

	auto phase = config.async ? ph : ph0;

	// advanced
	if (config.mode == Mode::Advanced)
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), config.phase / 360.f + phase);
	else if (config.mode == Mode::Chromatic)
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), config.phase / 360.f + phase);
	else if (config.mode == Mode::Gradient && config.gradType == GradType::Time)
		prog->setUniformLocationWith1f(prog->getUniformLocationForName("phase"), 1.0 - abs(config.phase / 180.f + phase * 2 - 1.0));

	prog->updateUniforms();
}

Mode refer_mode(std::string val) {
    int i = 0;
    for (auto mode : modes) {
        if (val == mode)
            return Mode(i);
        i ++;
    }
    return Mode::Default;
}

int refer_follow(std::string val) {
    int i = 0;
    for (auto followed : followeds) {
        if (val == followed)
            return i;
        i ++;
    }
    return 0;
}

$on_mod(Loaded) {
    // makes sure DevTools is loaded before registering
    devtools::waitForDevTools([] {
        AdvancedMenu::registerDevTools();
    });
    
    // port setttings to saved
    if (Mod::get()->setSavedValue("ported-3.6.0", true))
        return;

    auto set = Mod::get()->getSavedSettingsData();

    std::string keys[6] = {"normal", "practice", "quest", "list-todo", "list-done", "list-unf"};

    for (auto item : keys) {
        auto c1 = set.contains(item + "-default-color") ? ccc4(
                    GLubyte(set[item + "-default-color"].contains("r") ? (int) set[item + "-default-color"]["r"].asInt().unwrapOr(255) : 255),
                    GLubyte(set[item + "-default-color"].contains("g") ? (int) set[item + "-default-color"]["g"].asInt().unwrapOr(255) : 255),
                    GLubyte(set[item + "-default-color"].contains("b") ? (int) set[item + "-default-color"]["b"].asInt().unwrapOr(255) : 255),
                    GLubyte(set[item + "-default-color"].contains("a") ? (int) set[item + "-default-color"]["a"].asInt().unwrapOr(255) : 255)
                ) : to4B(ccWHITE);

        auto c2 = set.contains(item + "-default-another") ? ccc4(
                    GLubyte(set[item + "-default-another"].contains("r") ? (int) set[item + "-default-another"]["r"].asInt().unwrapOr(255) : 255),
                    GLubyte(set[item + "-default-another"].contains("g") ? (int) set[item + "-default-another"]["g"].asInt().unwrapOr(255) : 255),
                    GLubyte(set[item + "-default-another"].contains("b") ? (int) set[item + "-default-another"]["b"].asInt().unwrapOr(255) : 255),
                    GLubyte(set[item + "-default-another"].contains("a") ? (int) set[item + "-default-another"]["a"].asInt().unwrapOr(255) : 255)
                ) : to4B(ccWHITE);

        Mod::get()->setSavedValue(item + "-default", BarColor{
            .mode = set.contains(item + "-default-mode") ? refer_mode(set[item + "-default-mode"].asString().unwrapOrDefault()) : Mode::Default,
            .follower = set.contains(item + "-default-follower") ? refer_follow(set[item + "-default-follower"].asString().unwrapOrDefault()) : 0,
            .color = c1,
            .gradType = GradType::Progress,
            .colorZero = c1,
            .colorHdrd = c2,
            .phase = 0,
            .async = false,
            .speed = 1.f,
            .satu = 100,
            .brit = 100,
            .length = 1.f,
            .randa = false,
            .vert = fmt::format("vert-{}-default.glsl", item),
            .frag = fmt::format("frag-{}-default.glsl", item)
        });            
    }

    //for (auto k : tabs)
    auto& val = Mod::get()->getSaveContainer();
    for (auto& [k, v] : val) {
        if (!v.isObject())
            continue;
        v["vert"] = fmt::format("vert-{}.glsl", k);
        v["frag"] = fmt::format("frag-{}.glsl", k);
    }
    
}

$execute {
    // Register setting
    (void)Mod::get()->registerCustomSettingType("advanced-option", &AdvancedSetting::parse);
}

/*
// register keybinds
(void)[&]() -> Result<> {
    GEODE_UNWRAP(BindManagerV2::registerBindable(GEODE_UNWRAP(BindableActionV2::create(
        // ID, should be prefixed with mod ID
        "settings-menu"_spr,
        // Name
        "Settings Menu",
        // Description, leave empty for none
        "Popup The Settings Menu of Progress Bars",
        // Default binds
        { GEODE_UNWRAP(KeybindV2::create(KEY_Q, ModifierV2::None)) },
        // Category; use slashes for specifying subcategories. See the
        // Category class for default categories
        GEODE_UNWRAP(CategoryV2::create("Progress Bar Color/Settings"))
    ))));
    return Ok();
}();*/