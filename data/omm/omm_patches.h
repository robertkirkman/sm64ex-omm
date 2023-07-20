#ifndef OMM_PATCHES_H
#define OMM_PATCHES_H

//
// Patches
//

#if defined(DYNOS) || defined(R96X)
#define OMM_CODE_DYNOS 1
#else
#define OMM_CODE_DYNOS 0
#endif

#if defined(TIME_TRIALS)
#ifdef OMM_ALL_HEADERS
#include "game/time_trials.h"
#endif
#define OMM_CODE_TIME_TRIALS 1
#else
#define OMM_CODE_TIME_TRIALS 0
#define time_trials_enabled() false
#define time_trials_render_star_select_time(...)
#define time_trials_render_time_table(...) false
#endif

//
// Code patches
//

#define omm_patch__play_sound__play_character_sound                             OMM_RETURN_IF_TRUE(omm_sound_play_character_sound_n64(soundBits, pos),,);
#define omm_patch__stop_background_music__fix_boss_music                        stop_background_music_fix_boss_music();
#define omm_patch__find_in_bounds_yaw_wdw_bob_thi__return_yaw                   return yaw;
#define omm_patch__render_dialog_entries__fix_dialog_box_text_lower_bound       fix_dialog_box_text_lower_bound();
#define omm_patch__check_death_barrier__cheat_walk_on_death_barrier             OMM_RETURN_IF_TRUE(OMM_CHEAT_WALK_ON_DEATH_BARRIER,,);
#define omm_patch__check_lava_boost__cheat_walk_on_lava                         OMM_RETURN_IF_TRUE(OMM_CHEAT_WALK_ON_LAVA,,);
#define omm_patch__level_trigger_warp__check_death_warp                         OMM_RETURN_IF_TRUE(omm_mario_check_death_warp(m, warpOp), 0,);
#define omm_patch__update_air_with_turn__update_air_with_turn                   OMM_RETURN_IF_TRUE(OMM_LIKELY(omm_mario_update_air_with_turn(m)),,);
#define omm_patch__update_air_without_turn__update_air_without_turn             OMM_RETURN_IF_TRUE(OMM_LIKELY(omm_mario_update_air_without_turn(m)),,);
#define omm_patch__common_air_action_step__wall_slide_check                     OMM_RETURN_IF_TRUE(omm_mario_try_to_perform_wall_slide(m), AIR_STEP_NONE, set_mario_animation(m, animation););
#define omm_patch__set_mario_npc_dialog__skip_if_capture                        OMM_RETURN_IF_TRUE(omm_mario_is_capture(gMarioState), 2, gDialogResponse = 1; gCamera->cutscene = 0; extern u8 sObjectCutscene; sObjectCutscene = 0; extern u8 sCutsceneDialogResponse; sCutsceneDialogResponse = 1; gRecentCutscene = CUTSCENE_DIALOG;);
#define omm_patch__check_for_instant_quicksand__fix_downwarp                    OMM_RETURN_IF_TRUE(OMM_MOVESET_ODYSSEY, (find_floor_height(m->pos[0], m->pos[1], m->pos[2]) >= m->pos[1]) && mario_update_quicksand(m, 0.f),);
#define omm_patch__apply_slope_accel__not_peach_vibe_gloom                      if (!omm_peach_vibe_is_gloom())
#define omm_patch__update_walking_speed__update_walking_speed                   OMM_RETURN_IF_TRUE(OMM_LIKELY(omm_mario_update_walking_speed(m)),,);
#define omm_patch__mario_get_floor_class__cheat_walk_on_slope                   OMM_RETURN_IF_TRUE(OMM_CHEAT_WALK_ON_SLOPE, SURFACE_CLASS_NOT_SLIPPERY,);
#define omm_patch__mario_floor_is_slippery__cheat_walk_on_slope                 OMM_RETURN_IF_TRUE(OMM_CHEAT_WALK_ON_SLOPE, FALSE,);
#define omm_patch__cur_obj_update_dialog__skip_if_capture                       OMM_RETURN_IF_TRUE(omm_mario_is_capture(gMarioState), TRUE, gDialogLineIndex = 1; handle_special_dialog_text(dialogID); o->oDialogResponse = TRUE;);
#define omm_patch__cur_obj_update_dialog_with_cutscene__skip_if_capture         OMM_RETURN_IF_TRUE(omm_mario_is_capture(gMarioState), TRUE, gDialogLineIndex = 1; handle_special_dialog_text(dialogID); o->oDialogResponse = TRUE;);
#define omm_patch__render_painting__interpolate_painting                        extern void gfx_interpolate_painting(Vtx *, s32); gfx_interpolate_painting(verts, numVtx);
#define omm_patch__geo_painting_update__fix_floor_pointer                       geo_painting_update_fix_floor();
#define omm_patch__r96_get_intended_level_music__bowser_4_music                 OMM_RETURN_IF_TRUE(omm_sparkly_is_bowser_4_battle(), R96_LEVEL_BOWSER_3,);
#define omm_patch__cheats_play_as__disable                                      return FALSE;
#define omm_patch__dynos_sound_play__play_character_sound                       OMM_RETURN_IF_TRUE(omm_sound_play_character_sound_r96(name, pos),,);
#define omm_patch__memory__main_pool_state                                      u8 *gMainPoolStatePtr = (u8 *) &gMainPoolState;
#if defined(DYNOS)
#define omm_patch__cur_obj_has_model__check_georef
#else
#define omm_patch__cur_obj_has_model__check_georef                              if (o->oGraphNode && gLoadedGraphNodes[modelID] && o->oGraphNode->georef == gLoadedGraphNodes[modelID]->georef) { return TRUE; } else
#endif

//
// Some preprocessor magic
// That's a really awful way of coding, but it avoids patching files for a couple diffs :)
//

// Prevent the game from playing the demo and inhibiting X and Y buttons
#define run_demo_inputs_0 ;
#define run_demo_inputs_1 run_demo_inputs_unused()
#define run_demo_inputs(...) CAT(run_demo_inputs_, N_ARGS(__VA_ARGS__))

// Prevent the game from applying the old 60 FPS interpolation
#define patch_interpolations_0 ;
#define patch_interpolations_1 patch_interpolations()
#define patch_interpolations(...) CAT(patch_interpolations_, N_ARGS(__VA_ARGS__))

// render_hud is replaced by omm_render_hud
#define render_hud_0 omm_render_hud()
#define render_hud_1 render_hud()
#define render_hud(...) CAT(render_hud_, N_ARGS(__VA_ARGS__))

// render_pause_courses_and_castle is replaced by omm_render_pause
#define render_pause_courses_and_castle_0 omm_render_pause()
#define render_pause_courses_and_castle_1 render_pause_courses_and_castle()
#define render_pause_courses_and_castle(...) CAT(render_pause_courses_and_castle_, N_ARGS(__VA_ARGS__))
#define render_pause_screen_0 omm_render_pause()
#define render_pause_screen_1 render_pause_screen()
#define render_pause_screen(...) CAT(render_pause_screen_, N_ARGS(__VA_ARGS__))

// render_course_complete_screen is replaced by omm_render_course_complete
#define render_course_complete_screen_0 omm_render_course_complete()
#define render_course_complete_screen_1 render_course_complete_screen()
#define render_course_complete_screen(...) CAT(render_course_complete_screen_, N_ARGS(__VA_ARGS__))

// SM64: do_cutscene_handler is replaced by omm_sparkly_ending_dialog
#if OMM_GAME_IS_SM64
#define do_cutscene_handler_0 omm_sparkly_ending_dialog()
#define do_cutscene_handler_1 do_cutscene_handler()
#define do_cutscene_handler(...) CAT(do_cutscene_handler_, N_ARGS(__VA_ARGS__))
#endif

// bhv_mario_update uses OMM Mario update
#define bhv_mario_update_0 bhv_mario_update()
#define bhv_mario_update_1 bhv_mario_update_unused(void)
#define bhv_mario_update(...) CAT(bhv_mario_update_, N_ARGS(__VA_ARGS__))

// mario_ready_to_speak is replaced by omm_mario_is_ready_to_speak
#define mario_ready_to_speak_0 omm_mario_is_ready_to_speak(gMarioState)
#define mario_ready_to_speak_1 mario_ready_to_speak()
#define mario_ready_to_speak(...) CAT(mario_ready_to_speak_, N_ARGS(__VA_ARGS__))

// cur_obj_within_12k_bounds always returns TRUE
#define cur_obj_within_12k_bounds_0 TRUE
#define cur_obj_within_12k_bounds_1 cur_obj_within_12k_bounds_unused()
#define cur_obj_within_12k_bounds(...) CAT(cur_obj_within_12k_bounds_, N_ARGS(__VA_ARGS__))

// cur_obj_is_mario_ground_pounding_platform now works with any type of ground pound
#define cur_obj_is_mario_ground_pounding_platform_0 (gMarioObject->platform == o && omm_mario_is_ground_pound_landing(gMarioState))
#define cur_obj_is_mario_ground_pounding_platform_1 cur_obj_is_mario_ground_pounding_platform(void)
#define cur_obj_is_mario_ground_pounding_platform(...) CAT(cur_obj_is_mario_ground_pounding_platform_, N_ARGS(__VA_ARGS__))

// Update the number of defeated enemies
#define unload_deactivated_objects_0 omm_stats_update_defeated_enemies()
#define unload_deactivated_objects_1 \
omm_stats_update_defeated_enemies() { \
    for_each_object_in_interaction_lists(obj) { \
        if (omm_obj_is_enemy_defeated(obj)) { \
            gOmmStats->enemiesDefeated++; \
        } \
    } \
    extern void _unload_deactivated_objects(); \
    _unload_deactivated_objects(); \
} void _unload_deactivated_objects()
#define unload_deactivated_objects(...) CAT(unload_deactivated_objects_, N_ARGS(__VA_ARGS__))

// Star Road: Don't print the Hard Mode strings
#if OMM_GAME_IS_SMSR
#define print_hard_mode_strings_0 
#define print_hard_mode_strings_1 print_hard_mode_strings(void)
#define print_hard_mode_strings(...) CAT(print_hard_mode_strings_, N_ARGS(__VA_ARGS__))
#endif

// DynOS init makes a call to omm_opt_init before initializing itself
#define DynOS_Init \
DynOS_Init_Startup_() { extern void DynOS_Init_Omm_(); DynOS_Init_Omm_(); } \
extern "C" { extern void omm_opt_init(); } \
void DynOS_Init_Omm_() { extern void DynOS_Init_(); omm_opt_init(); DynOS_Init_(); } \
void DynOS_Init_

// Redefine the play_sequence function if it was replaced by the static seq_player_play_sequence
#if music_play_sequence_define
#define unused_8031E4F0 \
placeholder_func() {} \
static void seq_player_play_sequence(u8 player, u8 seqId, u16 arg2); \
void play_sequence(u8 player, u8 seqId, u16 arg2) { \
    seq_player_play_sequence(player, seqId, arg2); \
} void unused_8031E4F0
#endif

// If the boss sequence is missing, the level music is not going to restart...
// So, add the boss music in the queue to force the stop_background_music
// function to remove it from the queue and restart the level music.
#define stop_background_music_fix_boss_music() \
if ((seqId & 0xFF) == SEQ_EVENT_BOSS) { \
    for (u8 i = 0; i <= sBackgroundMusicQueueSize; ++i) { \
        if (i == sBackgroundMusicQueueSize) { \
            sBackgroundMusicQueueSize = min_s(sBackgroundMusicQueueSize + 1, MUSIC_QUEUE_MAX_SIZE); \
            mem_mov(sBackgroundMusicQueue + 1, sBackgroundMusicQueue, sizeof(sBackgroundMusicQueue[0]) * (MUSIC_QUEUE_MAX_SIZE - 1)); \
            sBackgroundMusicQueue[0].seqId = SEQ_EVENT_BOSS; \
            sBackgroundMusicQueue[0].priority = 4; \
            break; \
        } else if (sBackgroundMusicQueue[i].seqId == SEQ_EVENT_BOSS) { \
            break; \
        } \
    } \
}

// Compute the lower bound before resetting gDialogScrollOffsetY to not break frame interpolation
#define fix_dialog_box_text_lower_bound() \
lowerBound = (gDialogScrollOffsetY / 16) + 1; \
if (gDialogScrollOffsetY >= dialog->linesPerBox * DIAG_VAL1) { \
    gDialogTextPos = gLastDialogPageStrPos; \
    gDialogBoxState = DIALOG_STATE_VERTICAL; \
    gDialogScrollOffsetY = 0; \
} \
break

// Fix annoying crashes that can occur in rooms with paintings
#define geo_painting_update_fix_floor() \
gLastPaintingUpdateCounter = gPaintingUpdateCounter; \
gPaintingUpdateCounter = gAreaUpdateCounter; \
extern struct MarioState *gMarioState; \
if (gMarioState->floor) { \
    surface = gMarioState->floor; \
} else { \
    find_floor(gMarioObject->oPosX, gMarioObject->oPosY, gMarioObject->oPosZ, &surface); \
} \
if (surface) { \
    gPaintingMarioFloorType = surface->type; \
    gPaintingMarioXPos = gMarioObject->oPosX; \
    gPaintingMarioYPos = gMarioObject->oPosY; \
    gPaintingMarioZPos = gMarioObject->oPosZ; \
} \
return NULL;

// Call omm_camera_update and return if TRUE
#ifdef INCLUDED_FROM_CAMERA_C
#define update_camera \
update_camera(struct Camera *c) { \
    OMM_RETURN_IF_TRUE(omm_camera_update(c, gMarioState), , ); \
    extern void update_camera_(struct Camera *c); \
    update_camera_(c); \
} \
void update_camera_
#endif

#endif
