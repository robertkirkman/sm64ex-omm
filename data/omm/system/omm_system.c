#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#include "level_commands.h"

//
// Data
//

typedef void (*OmmRoutine)(void);
static OmmArray sOmmRoutines[OMM_ROUTINE_TYPES] = { omm_array_zero, omm_array_zero, omm_array_zero };
static bool sOmmSkipIntro = false;
bool sOmmIsMainMenu = true;
bool sOmmIsLevelEntry = false;
bool sOmmIsEndingCutscene = false;
bool sOmmIsEndingCakeScreen = false;
static u32  sOmmReturnToMainMenu = 0;
static s32  sOmmWarpToLastCourseNum = COURSE_NONE;

//
// Routines
//

void omm_add_routine(s32 type, void (*func)(void)) {
    if (OMM_LIKELY(type >= 0 && type < OMM_ROUTINE_TYPES && func)) {
        if (omm_array_find(sOmmRoutines[type], ptr, func) == -1) {
            omm_array_add(sOmmRoutines[type], ptr, func);
        }
    }
}

static void omm_execute_routines(s32 type) {
    omm_array_for_each(sOmmRoutines[type], p) {
        OmmRoutine routine = (OmmRoutine) p->as_ptr;
        routine();
    }
}

//
// Main Menu
//

void omm_select_save_file(s32 fileIndex, s32 modeIndex, s32 courseNum, bool skipIntro) {
    gMarioState->health = omm_health_get_max(0);
    gCurrSaveFileNum = fileIndex + 1;
    gCurrAreaIndex = modeIndex + 1;
    sWarpDest.areaIdx = modeIndex + 1;
    sOmmWarpToLastCourseNum = courseNum;
    sOmmIsMainMenu = false;
    sOmmIsLevelEntry = false;
    sOmmIsEndingCutscene = false;
    sOmmSkipIntro = skipIntro;
}

void omm_return_to_main_menu() {
    if (optmenu_open) optmenu_toggle();
    fade_into_special_warp(-2, 0);
    gDialogBoxState = 0;
    gMenuMode = -1;
}

//
// Update
//

void omm_update() {

    // Resume save file from last course
    if (!sOmmIsMainMenu && sOmmWarpToLastCourseNum != COURSE_NONE && gMarioObject) {
        s32 levelNum = omm_level_from_course(sOmmWarpToLastCourseNum);
        initiate_warp(levelNum, gCurrAreaIndex, OMM_LEVEL_ENTRY_WARP(levelNum), 0);
        play_transition(WARP_TRANSITION_FADE_INTO_COLOR, 1, 0xFF, 0xFF, 0xFF);
        level_set_transition(0, NULL);
        warp_special(0);
        gSavedCourseNum = COURSE_NONE;
        sOmmWarpToLastCourseNum = COURSE_NONE;
        return;
    }

    // Level entry
    if (sOmmIsLevelEntry) {
        omm_execute_routines(OMM_ROUTINE_TYPE_LEVEL_ENTRY);
        sOmmIsLevelEntry = false;
    }

    // Update routines
    omm_execute_routines(OMM_ROUTINE_TYPE_UPDATE);

    // Inhibit inputs (except camera controls) during a transition
    if (omm_is_transition_active() || omm_is_warping()) {
        for (s32 i = 0; i != array_length(gControllers); ++i) {
            gControllers[i].rawStickX      = 0;
            gControllers[i].rawStickY      = 0;
            gControllers[i].stickX         = 0;
            gControllers[i].stickY         = 0;
            gControllers[i].stickMag       = 0;
            gControllers[i].buttonDown    &= (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | L_TRIG | R_TRIG);
            gControllers[i].buttonPressed &= (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | L_TRIG | R_TRIG);
        }
    }

    // Auto-update display options
    static ConfigWindow sConfigWindow;
    configWindow.settings_changed = sConfigWindow.fullscreen != configWindow.fullscreen || sConfigWindow.vsync != configWindow.vsync;
    sConfigWindow = configWindow;

    // Trigger a save after exiting the options menu
    static bool optmenu_was_open = false;
    if (optmenu_was_open && !optmenu_open) {
        omm_save_file_do_save();
    }
    optmenu_was_open = optmenu_open;

    // Misc stuff
    gPrevFrameObjectCount = 0;
#if OMM_GAME_IS_SMSR
    gStarRoadHardMode = FALSE;
#endif
}

//
// Update GFX
//

static void omm_pre_render_update_stars_models() {

    // Stars number
    if (OMM_EXTRAS_SHOW_STAR_NUMBER) {
        omm_array_for_each(omm_obj_get_star_or_key_behaviors(), p) {
            const BehaviorScript *bhv = (const BehaviorScript *) p->as_ptr;
            for_each_object_with_behavior(star, bhv) {
                if (!obj_is_dormant(star) &&
                    star->behavior != bhvBowserKey &&
                    star->behavior != bhvGrandStar &&
                    star->behavior != bhvOmmSparklyStar) {
                    bool numberSpawned = false;
                    for_each_object_with_behavior(obj, bhvOmmStarNumber) {
                        if (obj->activeFlags && obj->parentObj == star) {
                            numberSpawned = true;
                            break;
                        }
                    }
                    if (!numberSpawned) {
                        omm_spawn_star_number(star);
                    }
                }
            }
        }
    }

#if !OMM_GAME_IS_SMMS
    // Colored Stars
    static struct GraphNode *sOmmStarGraphNodes[34] = { NULL };
    if (OMM_UNLIKELY(!sOmmStarGraphNodes[0])) {
        sOmmStarGraphNodes[0]  = geo_layout_to_graph_node(NULL, omm_geo_star_0_opaque);
        sOmmStarGraphNodes[1]  = geo_layout_to_graph_node(NULL, omm_geo_star_1_opaque);
        sOmmStarGraphNodes[2]  = geo_layout_to_graph_node(NULL, omm_geo_star_2_opaque);
        sOmmStarGraphNodes[3]  = geo_layout_to_graph_node(NULL, omm_geo_star_3_opaque);
        sOmmStarGraphNodes[4]  = geo_layout_to_graph_node(NULL, omm_geo_star_4_opaque);
        sOmmStarGraphNodes[5]  = geo_layout_to_graph_node(NULL, omm_geo_star_5_opaque);
        sOmmStarGraphNodes[6]  = geo_layout_to_graph_node(NULL, omm_geo_star_6_opaque);
        sOmmStarGraphNodes[7]  = geo_layout_to_graph_node(NULL, omm_geo_star_7_opaque);
        sOmmStarGraphNodes[8]  = geo_layout_to_graph_node(NULL, omm_geo_star_8_opaque);
        sOmmStarGraphNodes[9]  = geo_layout_to_graph_node(NULL, omm_geo_star_9_opaque);
        sOmmStarGraphNodes[10] = geo_layout_to_graph_node(NULL, omm_geo_star_10_opaque);
        sOmmStarGraphNodes[11] = geo_layout_to_graph_node(NULL, omm_geo_star_11_opaque);
        sOmmStarGraphNodes[12] = geo_layout_to_graph_node(NULL, omm_geo_star_12_opaque);
        sOmmStarGraphNodes[13] = geo_layout_to_graph_node(NULL, omm_geo_star_13_opaque);
        sOmmStarGraphNodes[14] = geo_layout_to_graph_node(NULL, omm_geo_star_14_opaque);
        sOmmStarGraphNodes[15] = geo_layout_to_graph_node(NULL, omm_geo_star_15_opaque);
        sOmmStarGraphNodes[16] = geo_layout_to_graph_node(NULL, omm_geo_star_16_opaque);
        sOmmStarGraphNodes[17] = geo_layout_to_graph_node(NULL, omm_geo_star_0_transparent);
        sOmmStarGraphNodes[18] = geo_layout_to_graph_node(NULL, omm_geo_star_1_transparent);
        sOmmStarGraphNodes[19] = geo_layout_to_graph_node(NULL, omm_geo_star_2_transparent);
        sOmmStarGraphNodes[20] = geo_layout_to_graph_node(NULL, omm_geo_star_3_transparent);
        sOmmStarGraphNodes[21] = geo_layout_to_graph_node(NULL, omm_geo_star_4_transparent);
        sOmmStarGraphNodes[22] = geo_layout_to_graph_node(NULL, omm_geo_star_5_transparent);
        sOmmStarGraphNodes[23] = geo_layout_to_graph_node(NULL, omm_geo_star_6_transparent);
        sOmmStarGraphNodes[24] = geo_layout_to_graph_node(NULL, omm_geo_star_7_transparent);
        sOmmStarGraphNodes[25] = geo_layout_to_graph_node(NULL, omm_geo_star_8_transparent);
        sOmmStarGraphNodes[26] = geo_layout_to_graph_node(NULL, omm_geo_star_9_transparent);
        sOmmStarGraphNodes[27] = geo_layout_to_graph_node(NULL, omm_geo_star_10_transparent);
        sOmmStarGraphNodes[28] = geo_layout_to_graph_node(NULL, omm_geo_star_11_transparent);
        sOmmStarGraphNodes[29] = geo_layout_to_graph_node(NULL, omm_geo_star_12_transparent);
        sOmmStarGraphNodes[30] = geo_layout_to_graph_node(NULL, omm_geo_star_13_transparent);
        sOmmStarGraphNodes[31] = geo_layout_to_graph_node(NULL, omm_geo_star_14_transparent);
        sOmmStarGraphNodes[32] = geo_layout_to_graph_node(NULL, omm_geo_star_15_transparent);
        sOmmStarGraphNodes[33] = geo_layout_to_graph_node(NULL, omm_geo_star_16_transparent);
    }

    s32 starColor = OMM_STAR_COLOR_[clamp_s(gCurrCourseNum, 0, 16) + OMM_STAR_COLOR_OFFSET(OMM_GAME_MODE)];
    omm_array_for_each(omm_obj_get_star_model_behaviors(), p) {
        const BehaviorScript *bhv = (const BehaviorScript *) p->as_ptr;
        for_each_object_with_behavior(obj, bhv) {
            if (OMM_EXTRAS_COLORED_STARS) {
                if (obj_check_model(obj, MODEL_STAR) || (obj->behavior == bhvCelebrationStar && !obj_check_model(obj, MODEL_BOWSER_KEY))) {
                    obj->oGraphNode = sOmmStarGraphNodes[starColor];
                } else if (obj_check_model(obj, MODEL_TRANSPARENT_STAR)) {
                    obj->oGraphNode = sOmmStarGraphNodes[starColor + 17 * !omm_is_ending_cutscene()];
                }
            } else {
                for (s32 i = 0; i != 34; ++i) {
                    if (obj_has_graph_node(obj, sOmmStarGraphNodes[i])) {
                        obj->oGraphNode = gLoadedGraphNodes[i < 17 ? MODEL_STAR : MODEL_TRANSPARENT_STAR];
                        break;
                    }
                }
            }
        }
    }
#endif
}

static void omm_pre_render_update_caps_models() {
    static s32 (*sCapFunctions[])(s32) = {
        omm_player_graphics_get_normal_cap,
        omm_player_graphics_get_wing_cap,
        omm_player_graphics_get_metal_cap,
        omm_player_graphics_get_winged_metal_cap,
    };
    s32 playerIndex = omm_player_get_selected_index();
    omm_array_for_each(omm_obj_get_cap_behaviors(), p) {
        const BehaviorScript *bhv = (const BehaviorScript *) p->as_ptr;
        for_each_object_with_behavior(obj, bhv) {
            for (s32 capType = 0; capType != 4; ++capType) {
                s32 playerCapModel = sCapFunctions[capType](playerIndex);
                for (s32 charIndex = 0; charIndex != OMM_NUM_PLAYABLE_CHARACTERS; ++charIndex) {
                    s32 charCapModel = sCapFunctions[capType](charIndex);
                    if (charCapModel != playerCapModel && obj_check_model(obj, charCapModel)) {
                        obj->oGraphNode = gLoadedGraphNodes[playerCapModel];
                        capType = 3; // break the other for loop
                        break;
                    }
                }
            }
            geo_preprocess_object_graph_node(obj);
        }
    }
}

void omm_pre_render() {

    // Mario model state
    if (gMarioState->marioBodyState) {
        if (OMM_EXTRAS_INVISIBLE_MODE) {
            gMarioState->marioBodyState->modelState &= ~0xFF;
            gMarioState->marioBodyState->modelState |= 0x100;
        } else if (omm_is_main_menu()) {
            gMarioState->marioBodyState->modelState &= ~0x1FF;
        }
    }

    // Sparkly Stars sparkles
    if (OMM_EXTRAS_SPARKLY_STARS_REWARD && gMarioObject && get_dialog_id() == -1 && !omm_is_game_paused()) {
        f32 vel = vec3f_dist(gMarioState->pos, gOmmMario->state.previous.pos);
        if (gGlobalTimer % (3 - clamp_s(vel / 25.f, 0, 2)) == 0) {
            omm_spawn_sparkly_star_sparkle_mario(gMarioObject, OMM_EXTRAS_SPARKLY_STARS_REWARD, 60.f, 10.f, 0.4f, 30.f);
        }
    }

    // Object models
    omm_pre_render_update_stars_models();
    omm_pre_render_update_caps_models();

    // Routines
    omm_execute_routines(OMM_ROUTINE_TYPE_PRE_RENDER);

    // Invisible mode
    omm_array_for_each(omm_obj_get_player_behaviors(), p) {
        const BehaviorScript *bhv = (const BehaviorScript *) p->as_ptr;
        for_each_object_with_behavior(obj, bhv) {
            if (OMM_EXTRAS_INVISIBLE_MODE) {
                obj->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
            } else {
                obj->oNodeFlags &= ~GRAPH_RENDER_INVISIBLE;
            }
        }
    }

    // Cut the music before resuming save file from last course
    if (!sOmmIsMainMenu && sOmmWarpToLastCourseNum != COURSE_NONE && gMarioObject) {
        music_fade_out(SEQ_PLAYER_LEVEL, 1);
        music_pause();
    }

    // Clear the spin bit
    gOmmMario->spin.pressed = false;
}

//
// Level commands
//

void *omm_update_cmd(void *cmd, s32 reg) {

    // Main menu
    if (cmd == level_script_entry_point ||
        cmd == level_script_splash_screen ||
        cmd == level_script_goddard_regular ||
        cmd == level_script_goddard_game_over ||
        cmd == level_script_debug_level_select ||
        cmd == level_script_to_file_select ||
        cmd == level_script_to_debug_level_select ||
        cmd == level_script_to_star_select_1 ||
        cmd == level_script_to_star_select_2 ||
        cmd == level_script_to_splash_screen ||
        cmd == level_script_file_select) {
        gMarioState->action = 0;
        configSkipIntro = false;
        sOmmSkipIntro = false;
        sOmmIsMainMenu = true;
        sOmmIsLevelEntry = false;
        sOmmIsEndingCutscene = false;
        sOmmIsEndingCakeScreen = false;
        sOmmReturnToMainMenu = 0;
    }

    // Loading screen
    if (cmd == level_script_entry_point) {
        extern void omm_loading_screen_start();
        omm_loading_screen_start();
        return NULL;
    }

    // Palette editor
    if (sOmmIsMainMenu && !omm_is_transition_active()) {
        switch (gOmmPaletteEditorState) {
            case OMM_PALETTE_EDITOR_STATE_OPENING: { gOmmPaletteEditorState = OMM_PALETTE_EDITOR_STATE_OPEN;   return (void *) omm_level_palette_editor; }
            case OMM_PALETTE_EDITOR_STATE_CLOSING: { gOmmPaletteEditorState = OMM_PALETTE_EDITOR_STATE_CLOSED; return (void *) level_script_file_select; }
        }
    }

#if OMM_GAME_IS_SM64
    // Skip intro cutscene (NOT Lakitu and Bowser's laugh)
    // Oddly enough, the check for intro Lakitu and Bowser's laugh occurs BEFORE the check for the intro cutscene
    // Meaning we can enable the former, then set configSkipIntro to skip the latter 
    static const uintptr_t cmd_lvl_init_from_save_file[] = { CALL(0, lvl_init_from_save_file) };
    if (mem_eq(cmd, cmd_lvl_init_from_save_file, sizeof(cmd_lvl_init_from_save_file))) {
        configSkipIntro = sOmmSkipIntro;
    }
#endif

    // Level entry
    static const uintptr_t cmd_level_entry[] = { CALL(0, lvl_init_or_update) };
    if (mem_eq(cmd, cmd_level_entry, sizeof(cmd_level_entry))) {
        sOmmIsMainMenu = false;
        sOmmIsLevelEntry = true;
        sOmmIsEndingCakeScreen = false;
        omm_stars_init_bits();
    }

#if OMM_GAME_IS_SM64 || OMM_GAME_IS_SM74
    if (!sOmmIsMainMenu) {

        // Ending cutscene
        if (gMarioState->action == ACT_END_PEACH_CUTSCENE ||
            gMarioState->action == ACT_CREDITS_CUTSCENE ||
            gMarioState->action == ACT_END_WAVING_CUTSCENE) {
            sOmmIsEndingCutscene = true;
        }

        // Ending cake screen
        if (cmd == level_script_cake_ending) {
            sOmmIsEndingCutscene = true;
            sOmmIsEndingCakeScreen = true;
        }

        // Skip ending
        if (sOmmIsEndingCutscene && !sOmmReturnToMainMenu && (gPlayer1Controller->buttonPressed & START_BUTTON)) {
            sOmmReturnToMainMenu = gGlobalTimer;
            play_transition(WARP_TRANSITION_FADE_INTO_STAR, 30, 0, 0, 0);
            music_fade_out(SEQ_PLAYER_LEVEL, 190);
            return NULL;
        }

        // Return to main menu
        if (sOmmReturnToMainMenu && (gGlobalTimer - sOmmReturnToMainMenu) > 45) {
            gHudDisplay.flags = 0;
            sOmmIsEndingCutscene = false;
            sOmmReturnToMainMenu = 0;

            // Level stack is already cleared by the jump to cake ending, so just jump to the main menu entry
            if (sOmmIsEndingCakeScreen) {
                return (void *) level_script_goddard_regular;
            }

            // The current level must be properly cleared before jumping to the main menu
            // Unset current transition, set warp to main menu, clear and exit
            static const uintptr_t cmd_return_to_main_menu[] = {
                CALL_LOOP(1, lvl_init_or_update),
                CLEAR_LEVEL(),
                SLEEP_BEFORE_EXIT(1),
                EXIT()
            };
            level_set_transition(0, NULL);
            warp_special(-2);
            return (void *) cmd_return_to_main_menu;
        }
    }
#endif

    // Warp update
    return omm_update_warp(cmd, sOmmIsLevelEntry);
}

//
// Game states
//

bool omm_is_main_menu() {
    return sOmmIsMainMenu;
}

bool omm_is_game_paused() {
    return gMenuMode != -1;
}

bool omm_is_transition_active() {
    static s16 sOmmTransitionTimer = 0;
    if (sTransitionTimer > 0) sOmmTransitionTimer = sTransitionTimer;
    bool isTransitionActive = (gWarpTransition.isActive || sOmmTransitionTimer > 0);
    sOmmTransitionTimer = sTransitionTimer;
    return isTransitionActive;
}

bool omm_is_ending_cutscene() {
    return sOmmIsEndingCutscene;
}

bool omm_is_ending_cake_screen() {
    return sOmmIsEndingCakeScreen;
}
