#if OMM_GAME_IS_SM74
#if !defined(OMM_OPTIONS_MENU_INL)
// File included from omm_options.c

//
// Warp to level (init)
//

typedef struct { s32 levelNum; s32 actNum; } ActValues;
static OmmArray sOmmWarpActValues[OMM_SM74_MODE_COUNT] = { omm_array_zero, omm_array_zero };

static const u8 **sOmmWarpLevelChoices[OMM_SM74_MODE_COUNT] = { NULL, NULL };
static const u8 **sOmmWarpActChoices[OMM_SM74_MODE_COUNT] = { NULL, NULL };

struct Option *sOmmWarpLevelOpt = NULL;
struct Option *sOmmWarpActOpt = NULL;

static u32 sOmmWarpLevelNum = 0;
static u32 sOmmWarpModeIndex = 0;
static u32 sOmmWarpActNum = 0;
#define sOmmWarpAreaIndex sOmmWarpModeIndex

static void omm_opt_init_warp_to_level() {
    for (s32 modeIndex = OMM_SM74_MODE_NORMAL; modeIndex <= OMM_SM74_MODE_EXTREME; ++modeIndex) {
        for (s32 i = 0; i != omm_level_get_count(); ++i) {
            s32 levelNum = omm_level_get_list()[i];
            s32 stars = max_s(0x1, omm_stars_get_bits_total(levelNum, modeIndex));
            for (s32 j = 0; j != OMM_NUM_ACTS_MAX_PER_COURSE; ++j) {
                if ((stars >> j) & 1) {
                    ActValues *actValues = mem_new(ActValues, 1);
                    actValues->levelNum = levelNum;
                    actValues->actNum = j + 1;
                    omm_array_add(sOmmWarpActValues[modeIndex], ptr, actValues);
                }
            }
        }
    }

    sOmmWarpLevelChoices[OMM_SM74_MODE_NORMAL] = mem_new(u8 *, omm_level_get_count() * OMM_SM74_MODE_COUNT);
    sOmmWarpLevelChoices[OMM_SM74_MODE_EXTREME] = sOmmWarpLevelChoices[OMM_SM74_MODE_NORMAL] + omm_level_get_count();
    for (s32 modeIndex = OMM_SM74_MODE_NORMAL; modeIndex <= OMM_SM74_MODE_EXTREME; ++modeIndex) {
        for (s32 i = 0; i != omm_level_get_count(); ++i) {
            const u8 *name = omm_level_get_course_name(omm_level_get_list()[i], modeIndex, true, true);
            sOmmWarpLevelChoices[modeIndex][i] = mem_dup(name, omm_text_length(name) + 1);
        }
    }

    sOmmWarpActChoices[OMM_SM74_MODE_NORMAL] = mem_new(u8 *, omm_array_count(sOmmWarpActValues[OMM_SM74_MODE_NORMAL]) + omm_array_count(sOmmWarpActValues[OMM_SM74_MODE_EXTREME]));
    sOmmWarpActChoices[OMM_SM74_MODE_EXTREME] = sOmmWarpActChoices[OMM_SM74_MODE_NORMAL] + omm_array_count(sOmmWarpActValues[OMM_SM74_MODE_NORMAL]);
    for (s32 modeIndex = OMM_SM74_MODE_NORMAL; modeIndex <= OMM_SM74_MODE_EXTREME; ++modeIndex) {
        omm_array_for_each(sOmmWarpActValues[modeIndex], p) {
            ActValues *actValues = (ActValues *) p->as_ptr;
            const u8 *name = omm_level_get_act_name(actValues->levelNum, actValues->actNum, modeIndex, true, true);
            sOmmWarpActChoices[modeIndex][i_p] = mem_dup(name, omm_text_length(name) + 1);
        }
    }
}

static struct Option omm_opt_make_choice_level(const char *label, u32 *value) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_text_convert(label, true);
    opt.uval = value;
    opt.choices = (const u8 **) 1;
    opt.numChoices = 0;
    return opt;
}

static struct Option omm_opt_make_choice_area(UNUSED const char *label, u32 *value) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_text_convert(OMM_TEXT_SM74_OPT_WARP_EDITION, true);
    opt.uval = value;
    opt.choices = mem_new(u8 *, OMM_SM74_MODE_COUNT);
    opt.numChoices = 2;
    opt.choices[OMM_SM74_MODE_NORMAL] = omm_text_convert(OMM_TEXT_SM74_OPT_WARP_NORMAL, true);
    opt.choices[OMM_SM74_MODE_EXTREME] = omm_text_convert(OMM_TEXT_SM74_OPT_WARP_EXTREME, true);
    return opt;
}

static struct Option omm_opt_make_choice_act(const char *label, u32 *value) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_text_convert(label, true);
    opt.uval = value;
    opt.choices = (const u8 **) 2;
    opt.numChoices = 0;
    return opt;
}

//
// Warp to level (update)
//

static u32 omm_opt_get_level_index(s32 levelNum) {
    for (s32 i = 0; i != omm_level_get_count(); ++i) {
        if (omm_level_get_list()[i] == levelNum) {
            return i;
        }
    }
    return 0;
}

static u32 omm_opt_get_first_act_index(s32 levelNum) {
    omm_array_for_each(sOmmWarpActValues[sOmmWarpModeIndex], p) {
        ActValues *actValues = (ActValues *) p->as_ptr;
        if (actValues->levelNum == levelNum) {
            return i_p;
        }
    }
    return 0;
}

static void omm_opt_update_warp_to_level() {
    static u32 sOmmWarpLevelNumPrev = (u32) -1;
    static u32 sOmmWarpModeIndexPrev = (u32) -1;
    static u32 sOmmWarpActNumPrev = (u32) -1;
    if (OMM_UNLIKELY(!sOmmWarpLevelOpt || !sOmmWarpActOpt)) {
        return;
    }

    // Mode changed
    if (sOmmWarpModeIndexPrev != sOmmWarpModeIndex) {
        s32 levelNum = omm_level_get_list()[sOmmWarpLevelNum];
        sOmmWarpLevelOpt->choices = sOmmWarpLevelChoices[sOmmWarpModeIndex];
        sOmmWarpLevelOpt->numChoices = omm_level_get_count();
        sOmmWarpActOpt->choices = sOmmWarpActChoices[sOmmWarpModeIndex];
        sOmmWarpActOpt->numChoices = omm_array_count(sOmmWarpActValues[sOmmWarpModeIndex]);
        sOmmWarpActNum = omm_opt_get_first_act_index(levelNum);
    }

    // Level changed
    else if (sOmmWarpLevelNumPrev != sOmmWarpLevelNum) {
        s32 levelNum = omm_level_get_list()[sOmmWarpLevelNum];
        sOmmWarpActNum = omm_opt_get_first_act_index(levelNum);
    }

    // Act changed
    else if (sOmmWarpActNumPrev != sOmmWarpActNum) {
        s32 levelNum = ((ActValues *) omm_array_get(sOmmWarpActValues[sOmmWarpModeIndex], ptr, sOmmWarpActNum))->levelNum;
        sOmmWarpLevelNum = omm_opt_get_level_index(levelNum);
    }

    // Update values
    sOmmWarpLevelNumPrev = sOmmWarpLevelNum;
    sOmmWarpModeIndexPrev = sOmmWarpModeIndex;
    sOmmWarpActNumPrev = sOmmWarpActNum;
}

static void omm_opt_warp_to_level(UNUSED void *opt, s32 arg) {
    if (!arg) {
        s32 levelNum = omm_level_get_list()[sOmmWarpLevelNum];
        s32 actNum = ((ActValues *) omm_array_get(sOmmWarpActValues[sOmmWarpModeIndex], ptr, sOmmWarpActNum))->actNum;
        if (omm_is_main_menu() || !omm_warp_to_level(levelNum, sOmmWarpModeIndex + 1, actNum)) {
            play_sound(SOUND_MENU_CAMERA_BUZZ | 0xFF00, gGlobalSoundArgs);
        }
    }
}

void omm_opt_sm74_change_mode(UNUSED void *opt, s32 arg) {
    if (!arg) {
        if (!omm_is_main_menu()) {
            if (optmenu_open) optmenu_toggle();
            gCurrAreaIndex = sWarpDest.areaIdx = 1 + ((OMM_GAME_MODE + 1) % OMM_SM74_MODE_COUNT);
            initiate_warp(gCurrLevelNum, gCurrAreaIndex, 0x0A, 0);
            fade_into_special_warp(0, 0);
            gSavedCourseNum = COURSE_NONE;
            gDialogBoxState = 0;
            gMenuMode = -1;
        } else {
            play_sound(SOUND_MENU_CAMERA_BUZZ | 0xFF00, gGlobalSoundArgs);
        }
    }
}

#else
// File included from omm_options_menu.inl

extern struct Option *sOmmWarpLevelOpt;
extern struct Option *sOmmWarpActOpt;

static void omm_sm74_opt_locate_warp_to_level_options(struct SubMenu *subMenu) {
    if (subMenu) {
        for (s32 i = 0; i != subMenu->numOpts; ++i) {
            struct Option *opt = &subMenu->opts[i];
            if (opt->type == OPT_SUBMENU) {
                omm_sm74_opt_locate_warp_to_level_options(opt->nextMenu);
            } else if (opt->type == OPT_CHOICE) {
                switch ((uintptr_t) opt->choices) {
                    case 1: sOmmWarpLevelOpt = opt; break;
                    case 2: sOmmWarpActOpt = opt; break;
                }
            }
        }
    }
}

OMM_ROUTINE_UPDATE(omm_sm74_opt_update) {
    if (OMM_UNLIKELY(!sOmmWarpLevelOpt || !sOmmWarpActOpt)) {
        omm_sm74_opt_locate_warp_to_level_options(&menuMain);
    }
}

#endif
#endif
