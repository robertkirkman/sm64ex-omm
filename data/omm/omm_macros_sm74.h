#ifndef OMM_MACROS_SM74_H
#define OMM_MACROS_SM74_H

// Game macros
#define OMM_GAME_CODE                                   "SM74"
#define OMM_GAME_NAME                                   "Super Mario 74"
#define OMM_GAME_TYPE                                   OMM_GAME_SM74
#define OMM_GAME_SAVE                                   OMM_GAME_SM74
#define OMM_GAME_MODE                                   (gCurrAreaIndex - 1)
#define OMM_GAME_MENU                                   "sm74"

// SM74 stuff
#define OMM_SM74_MODE_NORMAL                            (SM74_MODE_NORMAL - 1)
#define OMM_SM74_MODE_EXTREME                           (SM74_MODE_EXTREME - 1)
#define OMM_SM74_MODE_COUNT                             (OMM_SM74_MODE_EXTREME + 1)

// Better camera
#define BETTER_CAM_IS_PUPPY_CAM                         1
#define BETTER_CAM_IS_ENABLED                           gPuppyCam.enabled
#define BETTER_CAM_MODE                                 0
#define BETTER_CAM_YAW                                  0
#define BETTER_CAM_RAYCAST_ARGS                         __EXPAND(, UNUSED s32 flags)
#define BETTER_CAM_MOUSE_CAM                            (BETTER_CAM_IS_ENABLED && configCameraMouse)
#define CAMERA_X_THIRD_PERSON_VIEW                      (configCameraInvertX ? -1 : +1)
#define CAMERA_Y_THIRD_PERSON_VIEW                      (configCameraInvertY ? -1 : +1)
#define CAMERA_X_FIRST_PERSON_VIEW                      (configCameraInvertX ? +1 : -1)
#define CAMERA_Y_FIRST_PERSON_VIEW                      (configCameraInvertY ? -1 : +1)
#define gMouseXPos                                      gMouseX
#define gMouseYPos                                      gMouseY
#define gOldMouseXPos                                   gMouseX
#define gOldMouseYPos                                   gMouseY
#ifndef GFX_SCREEN_CONFIG_H // exclude configfile.c
#define configMouse                                     configCameraMouse
#endif

// Animation
#define AnimInfoStruct                                  AnimInfo
#define mAreaIndex                                      areaIndex
#define mActiveAreaIndex                                activeAreaIndex
#define mModel                                          model
#define mAnimInfo                                       animInfo
#define mAnimYTransDivisor                              animYTransDivisor
#define mStartFrame                                     startFrame
#define mLoopStart                                      loopStart
#define mLoopEnd                                        loopEnd

// Mario animation
#define MarioAnimationsStruct                           struct DmaHandlerList
#define MarioAnimDmaTableStruct                         struct DmaTable
#define gMarioAnimations                                gMarioState->animList
#define gMarioAnimDmaTable                              gMarioAnimations->dmaTable
#define gMarioCurrAnimAddr                              gMarioAnimations->currentAddr
#define gMarioTargetAnim                                gMarioAnimations->bufTarget
#define gMarioAnimsData                                 gMarioAnimsBuf

// Audio
#define gGlobalSoundArgs                                gGlobalSoundSource
#define sAcousticReachPerLevel                          sLevelAcousticReaches
#define audio_play_wing_cap_music()                     { play_cap_music(SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP)); }
#define audio_play_metal_cap_music()                    { play_cap_music(SEQUENCE_ARGS(4, SEQ_EVENT_METAL_CAP)); }
#define audio_play_vanish_cap_music()                   { play_cap_music(SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP)); }
#define audio_stop_cap_music()                          { stop_cap_music(); }
#define audio_play_shell_music()                        { play_shell_music(); }
#define audio_stop_shell_music()                        { stop_shell_music(); }
#define audio_play_course_clear()                       { play_course_clear(); }
#define audio_stop_course_clear()               
#define audio_play_puzzle_jingle()                      { play_puzzle_jingle(); }
#define audio_play_toads_jingle()                       { play_toads_jingle(); }
#define audio_play_star_jingle()                        { play_power_star_jingle(1); }
#define audio_play_star_fanfare()                       { play_star_fanfare(); }

// Music
#define music_pause()                                   { gSequencePlayers[0].muted = 1; }
#define music_resume()                                  { gSequencePlayers[0].muted = 0; }
#define music_stop()                            
#define audio_mute(...)                                 { set_audio_muted(__VA_ARGS__); }
#define music_fade_out(...)                             { seq_player_fade_out(__VA_ARGS__); }
#define music_lower_volume(...)                         { seq_player_lower_volume(__VA_ARGS__); }
#define music_unlower_volume(...)                       { seq_player_unlower_volume(__VA_ARGS__); }
#define music_play_sequence_define                      1
#define MUSIC_QUEUE_MAX_SIZE                            MAX_BACKGROUND_MUSIC_QUEUE_SIZE

// Sound
#define sound_stop(...)                                 { stop_sound(__VA_ARGS__); }
#define sound_stop_from_source(...)                     { stop_sounds_from_source(__VA_ARGS__); }
#define sound_stop_continuous()                         { stop_sounds_in_continuous_banks(); }
#define sound_set_moving_speed(...)                     { set_sound_moving_speed(__VA_ARGS__); }
#define SOUND_OBJ_WHOMP_SLAM                            SOUND_OBJ_WHOMP

// Object fields
#define oSnowmansBodyScale                              oSnowmansBottomScale
#define oBitsPlatformBowserObject                       oBitsPlatformBowser
#define oBowserCameraState                              oBowserCamAct
#define oBowserFlags                                    oBowserStatus
#define oBowserActionTimer                              oBowserTimer
#define oBowserBitsJump                                 oBowserBitsJustJump
#define oBowserSpitFireNumFlames                        oBowserRandSplitFloor
#define oBowserGrabbedAction                            oBowserGrabbedStatus
#define oBowserActionSelected                           oBowserIsReacting
#define oBowserOpacityTarget                            oBowserTargetOpacity
#define oBowserBlinkTimer                               oBowserEyesTimer
#define oBowserRainbowLightEffect                       oBowserRainbowLight
#define oBowserShockwaveScale                           oBowserShockWaveScale

// Tables, dialogs and menus
#define gCourseNameTable(mode)                          (array_of(const u8 **) { (const u8 **) seg2_course_name_table, (const u8 **) seg2_course_name_table_EE })[mode]
#define gActNameTable(mode)                             (array_of(const u8 **) { (const u8 **) seg2_act_name_table, (const u8 **) seg2_act_name_table_EE })[mode]
#define gDialogTable(mode)                              (array_of(struct DialogEntry **) { (struct DialogEntry **) seg2_dialog_table, (struct DialogEntry **) seg2_dialog_table_EE })[mode]
#define gDialogBoxAngle                                 gDialogBoxOpenTimer
#define gDialogLineIndex                                gDialogLineNum
#define gPauseScreenMode                                gMenuOptSelectIndex
#define GLOBAL_CHAR_TERMINATOR                          GLOBAR_CHAR_TERMINATOR

// Level scripts
#define LEVEL_SCRIPT_PRESSED_START                      0
#define level_script_entry_point                        level_script_entry
#define level_script_splash_screen                      level_intro_splash_screen
#define level_script_goddard_regular                    level_intro_mario_head_regular
#define level_script_goddard_game_over                  level_intro_mario_head_dizzy
#define level_script_debug_level_select                 level_intro_entry_4
#define level_script_to_file_select                     script_intro_L1
#define level_script_to_debug_level_select              script_intro_L2
#define level_script_to_star_select_1                   script_intro_L3
#define level_script_to_star_select_2                   script_intro_L4
#define level_script_to_splash_screen                   script_intro_L5
#define level_script_file_select                        level_main_menu_entry_1
#define level_script_star_select                        level_main_menu_entry_2
#define level_script_cake_ending                        level_ending_entry

// Geo layouts
#define amp_geo                                         dAmpGeo
#define bowser2_geo                                     bowser_geo_no_shadow
#define geo_intro_backdrop                              geo_intro_regular_backdrop
#define MODEL_BOWSER2                                   MODEL_BOWSER_NO_SHADOW

// Misc
#define check_surface_collisions_for_camera()           (gCheckingSurfaceCollisionsForCamera)
#define enable_surface_collisions_for_camera()          { gCheckingSurfaceCollisionsForCamera = 1; }
#define disable_surface_collisions_for_camera()         { gCheckingSurfaceCollisionsForCamera = 0; }
#define check_surface_intangible_find_floor()           (gFindFloorIncludeSurfaceIntangible)
#define enable_surface_intangible_find_floor()          { gFindFloorIncludeSurfaceIntangible = 1; }
#define disable_surface_intangible_find_floor()         { gFindFloorIncludeSurfaceIntangible = 0; }
#define find_static_floor(...)                          find_floor(__VA_ARGS__)
#define find_dynamic_floor(...)                         find_floor(__VA_ARGS__)
#define load_gfx_memory_pool()                          select_gfx_pool()
#define init_scene_rendering()                          init_rcp()
#define clear_framebuffer(...)                          clear_frame_buffer(__VA_ARGS__)
#define INPUT_BOUNCE                                    INPUT_STOMPED

// OMM
#define OMM_STAR_COLORS                                 0, 1, 4, 3, 2, 5, 6, 10, 8, 14, 11, 9, 12, 7, 13, 15, 16, 17, 18, 19, \
                                                        0, 14, 2, 3, 5, 9, 6, 10, 1, 7, 4, 11, 12, 13, 8, 15, 16, 17, 18, 19,
#define OMM_STAR_COLOR_OFFSET(modeIndex)                (modeIndex * 20)
#define OMM_STAR_COLOR_COUNT                            40
#define OMM_LEVEL_ENTRY_WARP(levelNum)                  0x0A
#define OMM_LEVEL_EXIT_DISTANCE                         150
#define OMM_LEVEL_SLIDE                                 LEVEL_PSS
#define OMM_LEVEL_ENTRY_POINT                           LEVEL_COURT
#define OMM_LEVEL_RETURN_TO_CASTLE                      LEVEL_COURT, OMM_GAME_MODE + 1, 0x40, 0
#define OMM_LEVEL_END                                   LEVEL_ENDING
#define OMM_SEQ_MAIN_MENU                               SEQ_MENU_TITLE_SCREEN
#define OMM_SEQ_FILE_SELECT                             SEQ_MENU_FILE_SELECT
#define OMM_SEQ_STAR_SELECT                             SEQ_MENU_STAR_SELECT
#define OMM_SEQ_PALETTE_EDITOR                          SEQ_MENU_TITLE_SCREEN
#define OMM_STATS_BOARD_LEVEL                           (OMM_GAME_MODE == OMM_SM74_MODE_NORMAL ? LEVEL_COURT : LEVEL_COURT)
#define OMM_STATS_BOARD_AREA                            (OMM_GAME_MODE == OMM_SM74_MODE_NORMAL ? 1 : 2)
#define OMM_STATS_BOARD_X                               (OMM_GAME_MODE == OMM_SM74_MODE_NORMAL ? 3000 : -2700)
#define OMM_STATS_BOARD_Y                               (OMM_GAME_MODE == OMM_SM74_MODE_NORMAL ? -861 : -900)
#define OMM_STATS_BOARD_Z                               (OMM_GAME_MODE == OMM_SM74_MODE_NORMAL ? 6000 : 7000)
#define OMM_STATS_BOARD_ANGLE                           (OMM_GAME_MODE == OMM_SM74_MODE_NORMAL ? 0xC000 : 0x4000)
#define OMM_CAMERA_LOOK_UP_WARP_STARS                   10
#define OMM_CAMERA_IS_BOWSER_FIGHT                      omm_camera_is_bowser_fight()
#define OMM_NUM_PLAYABLE_CHARACTERS                     2
#define OMM_NUM_SAVE_MODES                              OMM_SM74_MODE_COUNT
#define OMM_NUM_STARS_MAX_PER_COURSE                    7
#define OMM_NUM_ACTS_MAX_PER_COURSE                     (OMM_NUM_STARS_MAX_PER_COURSE - 1)
#define OMM_TEXT_FORMAT(id, str)                        str
#define STAR                                            "STAR"
#define Star                                            "Star"

// Sparkly stars
#define OMM_SPARKLY_REQUIREMENT                         (array_of(s32) { 151, 157 })[OMM_GAME_MODE]
#define OMM_SPARKLY_BLOCK_LEVEL                         LEVEL_COURT
#define OMM_SPARKLY_BLOCK_AREA                          (OMM_GAME_MODE + 1)
#define OMM_SPARKLY_BLOCK_COUNT                         1
#define OMM_SPARKLY_BLOCK_AVAILABLE                     { 0, 0, 1 }
#define OMM_SPARKLY_BLOCK_X                             (array_of(f32) { -5665, 6464 })[OMM_GAME_MODE]
#define OMM_SPARKLY_BLOCK_Y                             (array_of(f32) {  -860, -920 })[OMM_GAME_MODE]
#define OMM_SPARKLY_BLOCK_Z                             (array_of(f32) {  5365, 6300 })[OMM_GAME_MODE]
#define OMM_SPARKLY_BLOCK_ANGLE                         0x0000

// Extra text
#define OMM_TEXT_FS_PLAY                                "SELECT FILE - 74", "SELECT FILE - EE", NULL, NULL
#define OMM_TEXT_FS_COPY                                "COPY FILE - 74", "COPY FILE - EE", NULL, NULL
#define OMM_TEXT_FS_ERASE                               "ERASE FILE - 74", "ERASE FILE - EE", "SPARKLY STARS", NULL
#define OMM_TEXT_FS_SCORE                               "SCORES - 74", "SCORES - EE", "SPARKLY STARS", NULL

// Files
#define FILE_MACRO_PRESETS_H                            "macro_preset_names.h"
#define FILE_SPECIAL_PRESETS_H                          "special_preset_names.h"
#define FILE_BETTERCAM_H                                "extras/bettercamera.h"
#define FILE_OPTIONS_H                                  "extras/options_menu.h"
#define FILE_SOUNDS_H                                   "sounds.h"
#define FILE_CHEATS_H                                   "extras/cheats.h"
#define FILE_MARIO_CHEATS_H                             "extras/cheats.h"
#define FILE_TITLE_H                                    "menu/title_screen.h"
#define FILE_TXT_CONV_H                                 "types.h"
#define FILE_TEXT_LOADER_H                              "types.h"
#define FILE_R96_SYSTEM_H                               "types.h"

#endif
