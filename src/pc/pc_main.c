#include <stdlib.h>
#include <stdio.h>

#ifdef __ANDROID__
#include <sys/stat.h>
#include "platform.h"
#endif

#ifdef TARGET_WEB
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "sm64.h"

#include "game/memory.h"
#include "audio/external.h"

#include "gfx/gfx_pc.h"

#include "gfx/gfx_opengl.h"
#include "gfx/gfx_direct3d11.h"
#include "gfx/gfx_direct3d12.h"

#include "gfx/gfx_dxgi.h"
#include "gfx/gfx_sdl.h"

#include "audio/audio_api.h"
#include "audio/audio_sdl.h"
#include "audio/audio_null.h"

#include "pc_main.h"
#include "cliopts.h"
#include "configfile.h"
#include "controller/controller_api.h"
#include "controller/controller_keyboard.h"
#include "controller/controller_touchscreen.h"
#include "fs/fs.h"

#include "game/game_init.h"
#include "game/main.h"
#include "game/thread6.h"

#ifdef DISCORDRPC
#include "pc/discord/discordrpc.h"
#endif

OSMesg D_80339BEC;
OSMesgQueue gSIEventMesgQueue;

s8 gResetTimer;
s8 D_8032C648;
s8 gDebugLevelSelect;
s8 gShowProfiler;
s8 gShowDebugText;

s32 gRumblePakPfs;
struct RumbleData gRumbleDataQueue[3];
struct StructSH8031D9B0 gCurrRumbleSettings;

static struct AudioAPI *audio_api;
static struct GfxWindowManagerAPI *wm_api;
static struct GfxRenderingAPI *rendering_api;

extern void gfx_run(Gfx *commands);
extern void thread5_game_loop(void *arg);
extern void create_next_audio_buffer(s16 *samples, u32 num_samples);
void game_loop_one_iteration(void);

void dispatch_audio_sptask(struct SPTask *spTask) {
}

void set_vblank_handler(s32 index, struct VblankHandler *handler, OSMesgQueue *queue, OSMesg *msg) {
}

static bool inited = false;

#include "game/display.h" // for gGlobalTimer
void send_display_list(struct SPTask *spTask) {
    if (!inited) return;
    gfx_run((Gfx *)spTask->task.t.data_ptr);
}

#ifdef VERSION_EU
#define SAMPLES_HIGH 656
#define SAMPLES_LOW 640
#else
#define SAMPLES_HIGH 544
#define SAMPLES_LOW 528
#endif

void produce_one_frame(void) {
    gfx_start_frame();

    const f32 master_mod = (f32)configMasterVolume / 127.0f;
    set_sequence_player_volume(SEQ_PLAYER_LEVEL, (f32)configMusicVolume / 127.0f * master_mod);
    set_sequence_player_volume(SEQ_PLAYER_SFX, (f32)configSfxVolume / 127.0f * master_mod);
    set_sequence_player_volume(SEQ_PLAYER_ENV, (f32)configEnvVolume / 127.0f * master_mod);

    game_loop_one_iteration();
    thread6_rumble_loop(NULL);

    int samples_left = audio_api->buffered();
    u32 num_audio_samples = samples_left < audio_api->get_desired_buffered() ? SAMPLES_HIGH : SAMPLES_LOW;
    //printf("Audio samples: %d %u\n", samples_left, num_audio_samples);
    s16 audio_buffer[SAMPLES_HIGH * 2 * 2];
    for (int i = 0; i < 2; i++) {
        /*if (audio_cnt-- == 0) {
            audio_cnt = 2;
        }
        u32 num_audio_samples = audio_cnt < 2 ? 528 : 544;*/
        create_next_audio_buffer(audio_buffer + i * (num_audio_samples * 2), num_audio_samples);
    }
    //printf("Audio samples before submitting: %d\n", audio_api->buffered());

    audio_api->play((u8 *)audio_buffer, 2 * num_audio_samples * 4);

    gfx_end_frame();
}

void audio_shutdown(void) {
    if (audio_api) {
        if (audio_api->shutdown) audio_api->shutdown();
        audio_api = NULL;
    }
}

void game_deinit(void) {
#ifdef DISCORDRPC
    discord_shutdown();
#endif
    configfile_save(configfile_name());
    controller_shutdown();
    audio_shutdown();
    gfx_shutdown();
    inited = false;
}

void game_exit(void) {
    game_deinit();
#ifndef TARGET_WEB
    exit(0);
#endif
}

#ifdef TARGET_WEB
static void em_main_loop(void) {
}

static void request_anim_frame(void (*func)(double time)) {
    EM_ASM(requestAnimationFrame(function(time) {
        dynCall("vd", $0, [time]);
    }), func);
}

static void on_anim_frame(double time) {
    static double target_time;

    time *= 0.03; // milliseconds to frame count (33.333 ms -> 1)

    if (time >= target_time + 10.0) {
        // We are lagging 10 frames behind, probably due to coming back after inactivity,
        // so reset, with a small margin to avoid potential jitter later.
        target_time = time - 0.010;
    }

    for (int i = 0; i < 2; i++) {
        // If refresh rate is 15 Hz or something we might need to generate two frames
        if (time >= target_time) {
            produce_one_frame();
            target_time = target_time + 1.0;
        }
    }

    if (inited) // only continue if the init flag is still set
        request_anim_frame(on_anim_frame);
}
#endif

void main_func(void) {
#ifdef __ANDROID__
    char gamedir[SYS_MAX_PATH] = { 0 };
    const char *basedir = get_gamedir();
    snprintf(gamedir, sizeof(gamedir), "%s/%s", 
             basedir, gCLIOpts.GameDir[0] ? gCLIOpts.GameDir : FS_BASEDIR);
    if (stat(gamedir, NULL) == -1) {
        mkdir(gamedir, 0770);
    }
    // Extract res
    SDL_AndroidCopyAssetFilesToDir(basedir);
#else
    const char *gamedir = gCLIOpts.GameDir[0] ? gCLIOpts.GameDir : FS_BASEDIR;
#endif
    const char *userpath = gCLIOpts.SavePath[0] ? gCLIOpts.SavePath : sys_user_path();
    fs_init(sys_ropaths, gamedir, userpath);

    configfile_load(configfile_name());

    if (gCLIOpts.FullScreen == 1)
        configWindow.fullscreen = true;
    else if (gCLIOpts.FullScreen == 2)
        configWindow.fullscreen = false;

    const size_t poolsize = gCLIOpts.PoolSize ? gCLIOpts.PoolSize : DEFAULT_POOL_SIZE;
    u64 *pool = malloc(poolsize);
    if (!pool) sys_fatal("Could not alloc %u bytes for main pool.\n", poolsize);
    main_pool_init(pool, pool + poolsize / sizeof(pool[0]));
    gEffectsMemoryPool = mem_pool_init(0x4000, MEMORY_POOL_LEFT);

    #if defined(WAPI_SDL1) || defined(WAPI_SDL2)
    wm_api = &gfx_sdl;
    #elif defined(WAPI_DXGI)
    wm_api = &gfx_dxgi;
    #else
    #error No window API!
    #endif

    #if defined(RAPI_D3D11)
    rendering_api = &gfx_direct3d11_api;
    # define RAPI_NAME "DirectX 11"
    #elif defined(RAPI_D3D12)
    rendering_api = &gfx_direct3d12_api;
    # define RAPI_NAME "DirectX 12"
    #elif defined(RAPI_GL) || defined(RAPI_GL_LEGACY)
    rendering_api = &gfx_opengl_api;
    # ifdef USE_GLES
    #  define RAPI_NAME "OpenGL ES"
    # else
    #  define RAPI_NAME "OpenGL"
    # endif
    #else
    #error No rendering API!
    #endif

    char window_title[96] =
    "Super Mario 64 EX (" RAPI_NAME ")"
    #ifdef NIGHTLY
    " nightly " GIT_HASH
    #endif
    ;

    gfx_init(wm_api, rendering_api, window_title);
    wm_api->set_keyboard_callbacks(keyboard_on_key_down, keyboard_on_key_up, keyboard_on_all_keys_up);

#ifdef TOUCH_CONTROLS
    wm_api->set_touchscreen_callbacks((void *)touch_down, (void *)touch_motion, (void *)touch_up);
#endif

    #if defined(AAPI_SDL1) || defined(AAPI_SDL2)
    if (audio_api == NULL && audio_sdl.init()) 
        audio_api = &audio_sdl;
    #endif

    if (audio_api == NULL) {
        audio_api = &audio_null;
    }

    audio_init();
    sound_init();

    thread5_game_loop(NULL);

    inited = true;

#ifdef EXTERNAL_DATA
    // precache data if needed
    if (configPrecacheRes) {
        fprintf(stdout, "precaching data\n");
        fflush(stdout);
        gfx_precache_textures();
    }
#endif

#ifdef DISCORDRPC
    discord_init();
#endif

#ifdef TARGET_WEB
    emscripten_set_main_loop(em_main_loop, 0, 0);
    request_anim_frame(on_anim_frame);
#else
    while (true) {
        wm_api->main_loop(produce_one_frame);
#ifdef DISCORDRPC
        discord_update_rich_presence();
#endif
    }
#endif
}

extern void omm_opt_init();
extern void omm_setup_behavior_update_functions_map();
extern void gfx_init_patch_display_lists();
//extern void omm_mario_colors_init();
extern void omm_peach_colors_init();
extern void omm_behavior_data_init();
extern void omm_bowser_mad_aura_init_vertices_and_triangles();
extern void bhv_omm_wing_glow_init();
extern void bhv_omm_wing_trail_init();
extern void omm_data_init();
extern void omm_memory_init_pools();
extern void omm_speedrun_init();
extern void cappy_mad_piano_reset_seq_current_id_init();
extern void omm_clear_collision_buffers_init();
extern void omm_data_reset_fields_init();
extern void omm_obj_init_perry_attacks_init();
extern void omm_sparkly_context_init_init();
extern void omm_camera_init_from_level_entry_init();
extern void omm_player_init_init();
extern void omm_render_at_level_entry_init();
extern void omm_cappy_update_play_as_init();
extern void gfx_texture_preload_opt_update_init();
extern void omm_save_file_auto_save_init();
extern void omm_level_bowser_4_entry_init();
extern void omm_spawn_bowser_init();
extern void omm_spawn_grab_init();
extern void omm_spawn_perry_charge_init();
extern void omm_spawn_perry_trail_init();
extern void omm_spawn_perry_init();
extern void omm_data_stats_update_init();
extern void omm_stars_update_init();
extern void omm_sparkly_update_save_data_init();
extern void omm_sparkly_update_init();
extern void omm_update_crash_handler_init();
extern void omm_health_state_update_init();
extern void omm_opt_update_num_options_init();
extern void omm_opt_update_menu_init();
extern void omm_palette_editor_update_init();
extern void omm_player_update_init();
extern void omm_profiler_update_init();
extern void omm_speedrun_update_init();
extern void omm_load_dialog_entries_init();
extern void dialog_box_update_init();
//extern void omm_mario_colors_update_init();
extern void omm_peach_colors_update_init();
extern void omm_shadow_mario_update_init();
extern void omm_goomba_stack_update_init();
extern void bhv_omm_stats_board_render_init();
extern void omm_obj_update_perry_attacks_init();
extern void omm_peach_vibe_update_music_init();
extern void omm_act_peach_perry_charge_update_init();
extern void omm_opt_update_shortcuts_init();
extern void omm_player_update_gfx_init();
extern void omm_render_pause_update_init();
extern void omm_update_dialogs_init();
extern void geo_register_object_effects__omm_cappy_process_graph_node();
extern void geo_register_object_effects__geo_process_object_transparency();
extern void geo_register_object_effects__omm_sparkly_bowser_4_process_graph_node();
extern void omm_level_bowser_4__create_branch();
extern void omm_level_fish__create_branch();
extern void omm_level_peachy_room__create_branch();
extern void omm_level_ttm_area_2__create_branch();
extern void omm_surface_register_collision_jump_1();
extern void omm_surface_register_collision_jump_2();
extern void omm_surface_register_collision_jump_3();

#ifdef __ANDROID__
int SDL_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
    parse_cli_opts(argc, argv);
    omm_opt_init();
    omm_setup_behavior_update_functions_map();
    gfx_init_patch_display_lists();
    //omm_mario_colors_init();
    omm_peach_colors_init();
    omm_behavior_data_init();
    omm_bowser_mad_aura_init_vertices_and_triangles();
    bhv_omm_wing_glow_init();
    bhv_omm_wing_trail_init();
    omm_data_init();
    omm_memory_init_pools();
    omm_speedrun_init();
    cappy_mad_piano_reset_seq_current_id_init();
    omm_clear_collision_buffers_init();
    omm_data_reset_fields_init();
    omm_obj_init_perry_attacks_init();
    omm_sparkly_context_init_init();
    omm_camera_init_from_level_entry_init();
    omm_player_init_init();
    omm_render_at_level_entry_init();
    omm_cappy_update_play_as_init();
    gfx_texture_preload_opt_update_init();
    omm_save_file_auto_save_init();
    omm_level_bowser_4_entry_init();
    omm_spawn_bowser_init();
    omm_spawn_grab_init();
    omm_spawn_perry_charge_init();
    omm_spawn_perry_trail_init();
    omm_spawn_perry_init();
    omm_data_stats_update_init();
    omm_stars_update_init();
    omm_sparkly_update_save_data_init();
    omm_sparkly_update_init();
    omm_update_crash_handler_init();
    omm_health_state_update_init();
    omm_opt_update_num_options_init();
    omm_opt_update_menu_init();
    omm_palette_editor_update_init();
    omm_player_update_init();
    omm_profiler_update_init();
    omm_speedrun_update_init();
    omm_load_dialog_entries_init();
    dialog_box_update_init();
    //omm_mario_colors_update_init();
    omm_peach_colors_update_init();
    omm_shadow_mario_update_init();
    omm_goomba_stack_update_init();
    bhv_omm_stats_board_render_init();
    omm_obj_update_perry_attacks_init();
    omm_peach_vibe_update_music_init();
    omm_act_peach_perry_charge_update_init();
    omm_opt_update_shortcuts_init();
    omm_player_update_gfx_init();
    omm_render_pause_update_init();
    omm_update_dialogs_init();
    geo_register_object_effects__omm_cappy_process_graph_node();
    geo_register_object_effects__geo_process_object_transparency();
    geo_register_object_effects__omm_sparkly_bowser_4_process_graph_node();
    omm_level_bowser_4__create_branch();
    omm_level_fish__create_branch();
    omm_level_peachy_room__create_branch();
    omm_level_ttm_area_2__create_branch();
    omm_surface_register_collision_jump_1();
    omm_surface_register_collision_jump_2();
    omm_surface_register_collision_jump_3();
    main_func();
    return 0;
}
