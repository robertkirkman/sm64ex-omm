#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#include "menu/intro_geo.h"
#include "level_commands.h"
#if OMM_CODE_DYNOS
extern s32   dynos_gfx_get_mario_model_pack_index();
extern void *dynos_opt_get_action(const char *funcName);
extern void  dynos_opt_set_action(const char *funcName, void *funcPtr);
static void *dynos_opt_warp_to_level = NULL;
static void *dynos_opt_warp_to_castle = NULL;
static void *dynos_opt_restart_level = NULL;
static void *dynos_opt_exit_level = NULL;
static void *dynos_opt_return_to_main_menu = NULL;

static void omm_register_warp_functions() {
    if (!dynos_opt_warp_to_level) {
        dynos_opt_warp_to_level = dynos_opt_get_action("DynOS_Opt_WarpToLevel");
        dynos_opt_warp_to_castle = dynos_opt_get_action("DynOS_Opt_WarpToCastle");
        dynos_opt_restart_level = dynos_opt_get_action("DynOS_Opt_RestartLevel");
        dynos_opt_exit_level = dynos_opt_get_action("DynOS_Opt_ExitLevel");
        dynos_opt_return_to_main_menu = dynos_opt_get_action("DynOS_Opt_ReturnToMainMenu");
    }
}

static void omm_disable_warp_functions() {
    dynos_opt_set_action("DynOS_Opt_WarpToLevel", NULL);
    dynos_opt_set_action("DynOS_Opt_WarpToCastle", NULL);
    dynos_opt_set_action("DynOS_Opt_RestartLevel", NULL);
    dynos_opt_set_action("DynOS_Opt_ExitLevel", NULL);
    dynos_opt_set_action("DynOS_Opt_ReturnToMainMenu", NULL);
}

static void omm_enable_warp_functions() {
    dynos_opt_set_action("DynOS_Opt_WarpToLevel", dynos_opt_warp_to_level);
    dynos_opt_set_action("DynOS_Opt_WarpToCastle", dynos_opt_warp_to_castle);
    dynos_opt_set_action("DynOS_Opt_RestartLevel", dynos_opt_restart_level);
    dynos_opt_set_action("DynOS_Opt_ExitLevel", dynos_opt_exit_level);
    dynos_opt_set_action("DynOS_Opt_ReturnToMainMenu", dynos_opt_return_to_main_menu);
}
#else
static void omm_register_warp_functions() {}
static void omm_disable_warp_functions() {}
static void omm_enable_warp_functions() {}
#endif
void beh_yellow_background_menu_init(void) {}
void beh_yellow_background_menu_loop(void) {}
void bhv_menu_button_init(void) {}
void bhv_menu_button_loop(void) {}
void bhv_menu_button_manager_init(void) {}
void bhv_menu_button_manager_loop(void) {}
Gfx *geo_file_select_strings_and_menu_cursor(UNUSED s32 callContext, UNUSED struct GraphNode *node, UNUSED Mat4 mtx) { return NULL; }

static struct {
    s32 timer;
    s32 index;
} sOmmMainMenu[1];

static struct {
    bool open;
    s32 timer;
    s32 index;
    s32 mode;
} sOmmFileSelect[1];

static struct {
    bool open;
    s32 index;
} sOmmFileCopy[1];

static struct {
    bool open;
    s32 timer;
    bool coins;
} sOmmFileScore[1];

//
// Utils
//

static u32 omm_get_inputs() {
    static bool inputHold = false;
    static s32 inputTimer = 0;
    u32  inputs = (gPlayer1Controller->buttonPressed);
    bool inputU = (gPlayer1Controller->stickY > +60 || (gPlayer1Controller->buttonDown & U_JPAD));
    bool inputD = (gPlayer1Controller->stickY < -60 || (gPlayer1Controller->buttonDown & D_JPAD));
    bool inputL = (gPlayer1Controller->stickX < -60 || (gPlayer1Controller->buttonDown & L_JPAD));
    bool inputR = (gPlayer1Controller->stickX > +60 || (gPlayer1Controller->buttonDown & R_JPAD));
    if (!inputU && !inputD && !inputL && !inputR) {
        inputHold = 0;
        inputTimer = 0;
    } else if (--inputTimer <= 0) {
        inputs |= inputU * STICK_UP;
        inputs |= inputD * STICK_DOWN;
        inputs |= inputL * STICK_LEFT;
        inputs |= inputR * STICK_RIGHT;
        if (!inputHold) {
            inputTimer = 10;
            inputHold = true;
        } else {
            inputTimer = 3;
        }
    }
    return inputs;
}

//
// Complete save cheat
// Press C-Up, C-Up, C-Down, C-Down, C-Left, C-Right, C-Left, C-Right, Z, R and select a file with A
// Collects all stars, sets all flags, opens all cannons and unlocks Peach
//

static const u16 sOmmCompleteSaveButtons[] = { U_CBUTTONS, U_CBUTTONS, D_CBUTTONS, D_CBUTTONS, L_CBUTTONS, R_CBUTTONS, L_CBUTTONS, R_CBUTTONS, Z_TRIG, R_TRIG, A_BUTTON, 0xFFFF };
static s32 sOmmCompleteSaveSequenceIndex = 0;
extern void omm_set_complete_save_file(s32 fileIndex, s32 modeIndex);

static s32 omm_update_complete_save_sequence_index() {
    if (gPlayer1Controller->buttonPressed != 0) {
        u16 buttonPressed = gPlayer1Controller->buttonPressed;
        u16 buttonRequired = sOmmCompleteSaveButtons[sOmmCompleteSaveSequenceIndex];
        if ((buttonPressed & buttonRequired) == buttonRequired) {
            sOmmCompleteSaveSequenceIndex++;
            if (sOmmCompleteSaveButtons[sOmmCompleteSaveSequenceIndex] == A_BUTTON) {
                play_sound(SOUND_GENERAL2_RIGHT_ANSWER | 0xFF00, gGlobalSoundArgs);
            }
        } else {
            sOmmCompleteSaveSequenceIndex = 0;
        }
    }
    return sOmmCompleteSaveSequenceIndex;
}

static void omm_check_complete_save(s32 fileIndex, s32 modeIndex) {
    if (sOmmCompleteSaveSequenceIndex == array_length(sOmmCompleteSaveButtons) - 1) {
        omm_set_complete_save_file(fileIndex, modeIndex);
    }
    sOmmCompleteSaveSequenceIndex = 0;
}

//
// Main Menu constants
//

#define GFX_DIMENSIONS_SCREEN_WIDTH     ((s32) (GFX_DIMENSIONS_FROM_RIGHT_EDGE(0) - GFX_DIMENSIONS_FROM_LEFT_EDGE(0) + 0.5f))

#define OMM_MM_PLAY                     0
#define OMM_MM_PLAY_NO_SAVE             1
#define OMM_MM_COPY                     2
#define OMM_MM_ERASE                    3
#define OMM_MM_SCORE                    4
#define OMM_MM_OPTIONS                  5
#define OMM_MM_PALETTE_EDITOR           6

#define OMM_MM_LOGO_TEXTURE             "menu/" OMM_GAME_MENU "/logo.rgba32"
#define OMM_MM_LOGO_W                   (OMM_MM_LOGO_H * 1.6f)
#define OMM_MM_LOGO_H                   (SCREEN_HEIGHT / 3)
#define OMM_MM_LOGO_X                   GFX_DIMENSIONS_FROM_LEFT_EDGE(0)
#define OMM_MM_LOGO_Y                   (SCREEN_HEIGHT - OMM_MM_LOGO_H)

#define OMM_MM_STRINGS_COUNT            ((s32) array_length(sOmmMainMenuStrings))
#define OMM_MM_STRINGS_X                GFX_DIMENSIONS_FROM_LEFT_EDGE(48)
#define OMM_MM_STRINGS_Y                ((11 * SCREEN_HEIGHT) / 20)
#define OMM_MM_STRINGS_H                16

#define OMM_MM_BOX_X                    (OMM_MM_STRINGS_X - 20)
#define OMM_MM_BOX_Y                    (OMM_MM_STRINGS_Y - 4 - OMM_MM_STRINGS_H * sOmmMainMenu->index)
#define OMM_MM_BOX_W                    (135 * GFX_DIMENSIONS_ASPECT_RATIO)
#define OMM_MM_BOX_H                    OMM_MM_STRINGS_H
#define OMM_MM_BOX_A                    (0x60 + 0x40 * sins(sOmmMainMenu->timer * 0x800))

#define OMM_MM_CURSOR_X                 (OMM_MM_STRINGS_X - 16)
#define OMM_MM_CURSOR_Y                 (OMM_MM_STRINGS_Y - 2 - OMM_MM_STRINGS_H * sOmmMainMenu->index)
#define OMM_MM_CURSOR_W                 12
#define OMM_MM_CURSOR_H                 (OMM_MM_CURSOR_W + (OMM_MM_CURSOR_W / 3) * sins(clamp_s(sOmmMainMenu->timer % 90, 0, 16) * 0x1000))

#define OMM_MM_INFO_STRING_X            GFX_DIMENSIONS_FROM_LEFT_EDGE(4)
#define OMM_MM_INFO_STRING_Y            4
#define OMM_MM_INFO_STRING_W            5
#define OMM_MM_INFO_STRING_H            5
#define OMM_MM_INFO_STRING_C            0x80

#define OMM_MM_INFO_BOX_X               (OMM_MM_INFO_STRING_X - 2)
#define OMM_MM_INFO_BOX_Y               (OMM_MM_INFO_STRING_Y - 2)
#define OMM_MM_INFO_BOX_W               (170 * GFX_DIMENSIONS_ASPECT_RATIO)
#define OMM_MM_INFO_BOX_H               (OMM_MM_INFO_STRING_H + 4)
#define OMM_MM_INFO_BOX_A               0xC0

#define OMM_MM_SOUND_SCROLL             (SOUND_MENU_CHANGE_SELECT | 0xFE00)
#define OMM_MM_SOUND_FILE_SELECT        (SOUND_MENU_STAR_SOUND | 0xFF00)
#define OMM_MM_SOUND_PLAY_NO_SAVE       (SOUND_MENU_STAR_SOUND_OKEY_DOKEY | 0xFF00)

static struct { const char *label; const char *info[2]; } sOmmMainMenuStrings[] = {
    [OMM_MM_PLAY]           = { OMM_TEXT_MM_PLAY,           { OMM_TEXT_MM_INFO_PLAY,           OMM_TEXT_MM_INFO_PLAY             } },
    [OMM_MM_PLAY_NO_SAVE]   = { OMM_TEXT_MM_PLAY_NO_SAVE,   { OMM_TEXT_MM_INFO_PLAY_NO_SAVE,   OMM_TEXT_MM_INFO_PLAY_NO_SAVE     } },
    [OMM_MM_COPY]           = { OMM_TEXT_MM_COPY,           { OMM_TEXT_MM_INFO_COPY,           OMM_TEXT_MM_INFO_COPY             } },
    [OMM_MM_ERASE]          = { OMM_TEXT_MM_ERASE,          { OMM_TEXT_MM_INFO_ERASE,          OMM_TEXT_MM_INFO_ERASE            } },
    [OMM_MM_SCORE]          = { OMM_TEXT_MM_SCORE,          { OMM_TEXT_MM_INFO_SCORE,          OMM_TEXT_MM_INFO_SCORE            } },
    [OMM_MM_OPTIONS]        = { OMM_TEXT_MM_OPTIONS,        { OMM_TEXT_MM_INFO_OPTIONS,        OMM_TEXT_MM_INFO_OPTIONS          } },
    [OMM_MM_PALETTE_EDITOR] = { OMM_TEXT_MM_PALETTE_EDITOR, { OMM_TEXT_MM_INFO_PALETTE_EDITOR, OMM_TEXT_MM_INFO_PALETTE_EDITOR_2 } },
};

//
// Main Menu Mario animation
//

static const u16 sOmmMainMenuMarioAnimIndices[] = {
    0x0001, 0x0000, 0x005A, 0x0001, 0x005A, 0x005B, 0x0001, 0x0000, 0x0001, 0x00B5, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x0A3D,
    0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x0A3C, 0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x09E2, 0x0001, 0x09DE, 0x0001, 0x09DF, 0x0001, 0x09E0,
    0x005A, 0x021E, 0x005A, 0x0278, 0x005A, 0x02D2, 0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x01C4, 0x005A, 0x00B6, 0x005A, 0x0110, 0x005A, 0x016A,
    0x0001, 0x09DA, 0x0001, 0x09DB, 0x0001, 0x09DC, 0x005A, 0x03E0, 0x005A, 0x043A, 0x005A, 0x0494, 0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x0386,
    0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x032C, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x09DD, 0x005A, 0x08CC, 0x005A, 0x0926, 0x005A, 0x0980,
    0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x0872, 0x005A, 0x0764, 0x005A, 0x07BE, 0x005A, 0x0818, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x09E1,
    0x005A, 0x0656, 0x005A, 0x06B0, 0x005A, 0x070A, 0x0001, 0x0000, 0x0001, 0x0000, 0x005A, 0x05FC, 0x005A, 0x04EE, 0x005A, 0x0548, 0x005A, 0x05A2,
};

static const s16 sOmmMainMenuMarioAnimValues[] = {
    0x0000, 0x00AC, 0x00AB, 0x00AB, 0x00A9, 0x00A8, 0x00A6, 0x00A4, 0x00A2, 0x00A0, 0x009E, 0x009D, 0x009B, 0x009A, 0x0099, 0x0099,
    0x0099, 0x009A, 0x009B, 0x009C, 0x009E, 0x00A0, 0x00A1, 0x00A3, 0x00A5, 0x00A7, 0x00A8, 0x00AA, 0x00AB, 0x00AB, 0x00AC, 0x00AB,
    0x00AB, 0x00A9, 0x00A8, 0x00A6, 0x00A4, 0x00A2, 0x00A0, 0x009E, 0x009C, 0x009B, 0x0099, 0x0098, 0x0097, 0x0097, 0x0097, 0x0098,
    0x0099, 0x009B, 0x009C, 0x009E, 0x00A0, 0x00A2, 0x00A4, 0x00A6, 0x00A8, 0x00A9, 0x00AB, 0x00AB, 0x00AC, 0x00AB, 0x00AB, 0x00A9,
    0x00A8, 0x00A6, 0x00A4, 0x00A2, 0x00A0, 0x009E, 0x009C, 0x009B, 0x0099, 0x0098, 0x0097, 0x0097, 0x0097, 0x0098, 0x0099, 0x009A,
    0x009B, 0x009C, 0x009E, 0x00A0, 0x00A1, 0x00A3, 0x00A5, 0x00A7, 0x00A8, 0x00AA, 0x00AB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
    0x0000, 0x0000, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002, 0x0002, 0x0001,
    0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002, 0x0002, 0x0001, 0x0001, 0x0000,
    0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001,
    0x0002, 0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0000,
    0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x3FFF, 0x0000, 0x0003, 0x000D, 0x001D, 0x0031, 0x0049, 0x0063, 0x007D, 0x0098, 0x00B2,
    0x00C9, 0x00DD, 0x00ED, 0x00F7, 0x00FB, 0x00F8, 0x00EF, 0x00E1, 0x00CF, 0x00BA, 0x00A2, 0x008A, 0x0071, 0x0058, 0x0041, 0x002C,
    0x001A, 0x000C, 0x0003, 0x0000, 0x0003, 0x000C, 0x001A, 0x002C, 0x0041, 0x0058, 0x0071, 0x008A, 0x00A2, 0x00BA, 0x00CF, 0x00E1,
    0x00EF, 0x00F8, 0x00FB, 0x00F8, 0x00EF, 0x00E1, 0x00CF, 0x00BA, 0x00A2, 0x008A, 0x0071, 0x0058, 0x0041, 0x002C, 0x001A, 0x000C,
    0x0003, 0x0000, 0x0003, 0x000C, 0x001A, 0x002C, 0x0041, 0x0058, 0x0071, 0x008A, 0x00A2, 0x00BA, 0x00CF, 0x00E1, 0x00EF, 0x00F8,
    0x00FB, 0x00F8, 0x00EF, 0x00E1, 0x00D0, 0x00BB, 0x00A4, 0x008B, 0x0073, 0x005A, 0x0043, 0x002E, 0x001D, 0x000F, 0x0006, 0x0003,
    0xF97C, 0xF97C, 0xF97D, 0xF97E, 0xF97F, 0xF981, 0xF983, 0xF985, 0xF987, 0xF989, 0xF98B, 0xF98C, 0xF98E, 0xF98E, 0xF98F, 0xF98E,
    0xF98E, 0xF98D, 0xF98B, 0xF98A, 0xF988, 0xF986, 0xF984, 0xF982, 0xF981, 0xF97F, 0xF97E, 0xF97D, 0xF97C, 0xF97C, 0xF97C, 0xF97D,
    0xF97E, 0xF97F, 0xF981, 0xF982, 0xF984, 0xF986, 0xF988, 0xF98A, 0xF98B, 0xF98D, 0xF98E, 0xF98E, 0xF98F, 0xF98E, 0xF98E, 0xF98D,
    0xF98B, 0xF98A, 0xF988, 0xF986, 0xF984, 0xF982, 0xF981, 0xF97F, 0xF97E, 0xF97D, 0xF97C, 0xF97C, 0xF97C, 0xF97D, 0xF97E, 0xF97F,
    0xF981, 0xF982, 0xF984, 0xF986, 0xF988, 0xF98A, 0xF98B, 0xF98D, 0xF98E, 0xF98E, 0xF98F, 0xF98E, 0xF98E, 0xF98D, 0xF98B, 0xF98A,
    0xF988, 0xF986, 0xF984, 0xF983, 0xF981, 0xF97F, 0xF97E, 0xF97D, 0xF97C, 0xF97C, 0xED8C, 0xED74, 0xED34, 0xECD0, 0xEC51, 0xEBBE,
    0xEB1C, 0xEA73, 0xE9CB, 0xE929, 0xE895, 0xE816, 0xE7B3, 0xE772, 0xE75B, 0xE76F, 0xE7A8, 0xE800, 0xE871, 0xE8F6, 0xE989, 0xEA24,
    0xEAC2, 0xEB5E, 0xEBF1, 0xEC76, 0xECE7, 0xED3F, 0xED77, 0xED8C, 0xED77, 0xED3F, 0xECE7, 0xEC76, 0xEBF1, 0xEB5E, 0xEAC2, 0xEA24,
    0xE989, 0xE8F6, 0xE871, 0xE800, 0xE7A8, 0xE76F, 0xE75B, 0xE76F, 0xE7A8, 0xE800, 0xE871, 0xE8F6, 0xE989, 0xEA24, 0xEAC2, 0xEB5E,
    0xEBF1, 0xEC76, 0xECE7, 0xED3F, 0xED77, 0xED8C, 0xED77, 0xED3F, 0xECE7, 0xEC76, 0xEBF2, 0xEB5F, 0xEAC4, 0xEA26, 0xE98A, 0xE8F7,
    0xE872, 0xE801, 0xE7A9, 0xE770, 0xE75B, 0xE76E, 0xE7A6, 0xE7FD, 0xE86C, 0xE8EF, 0xE980, 0xEA1A, 0xEAB6, 0xEB50, 0xEBE1, 0xEC64,
    0xECD4, 0xED2B, 0xED63, 0xED77, 0xCA4F, 0xCA3D, 0xCA0A, 0xC9BB, 0xC957, 0xC8E2, 0xC863, 0xC7DD, 0xC758, 0xC6D9, 0xC664, 0xC5FF,
    0xC5B1, 0xC57E, 0xC56C, 0xC57C, 0xC5A8, 0xC5EE, 0xC647, 0xC6B0, 0xC724, 0xC79F, 0xC81C, 0xC897, 0xC90B, 0xC974, 0xC9CD, 0xCA12,
    0xCA3F, 0xCA4F, 0xCA3F, 0xCA12, 0xC9CD, 0xC974, 0xC90B, 0xC897, 0xC81C, 0xC79F, 0xC724, 0xC6B0, 0xC647, 0xC5EE, 0xC5A8, 0xC57C,
    0xC56C, 0xC57C, 0xC5A8, 0xC5EE, 0xC647, 0xC6B0, 0xC724, 0xC79F, 0xC81C, 0xC897, 0xC90B, 0xC974, 0xC9CD, 0xCA12, 0xCA3F, 0xCA4F,
    0xCA3F, 0xCA12, 0xC9CD, 0xC973, 0xC90A, 0xC896, 0xC81B, 0xC79E, 0xC723, 0xC6AF, 0xC646, 0xC5ED, 0xC5A8, 0xC57B, 0xC56C, 0xC57C,
    0xC5AA, 0xC5F1, 0xC64B, 0xC6B6, 0xC72C, 0xC7A8, 0xC826, 0xC8A3, 0xC918, 0xC982, 0xC9DD, 0xCA23, 0xCA50, 0xCA61, 0xFB1D, 0xFAF0,
    0xFA72, 0xF9B0, 0xF8B8, 0xF798, 0xF65D, 0xF514, 0xF3CB, 0xF290, 0xF16F, 0xF078, 0xEFB6, 0xEF38, 0xEF0B, 0xEF32, 0xEFA1, 0xF04C,
    0xF129, 0xF22C, 0xF34A, 0xF47A, 0xF5AE, 0xF6DD, 0xF7FC, 0xF8FF, 0xF9DC, 0xFA87, 0xFAF6, 0xFB1D, 0xFAF6, 0xFA87, 0xF9DC, 0xF8FF,
    0xF7FC, 0xF6DD, 0xF5AE, 0xF47A, 0xF34A, 0xF22C, 0xF129, 0xF04C, 0xEFA1, 0xEF32, 0xEF0B, 0xEF32, 0xEFA1, 0xF04C, 0xF129, 0xF22C,
    0xF34A, 0xF47A, 0xF5AE, 0xF6DD, 0xF7FC, 0xF8FF, 0xF9DC, 0xFA87, 0xFAF6, 0xFB1D, 0xFAF6, 0xFA87, 0xF9DC, 0xF900, 0xF7FD, 0xF6DF,
    0xF5B0, 0xF47C, 0xF34D, 0xF22F, 0xF12C, 0xF04F, 0xEFA3, 0xEF33, 0xEF0B, 0xEF30, 0xEF9D, 0xF045, 0xF11F, 0xF21F, 0xF33A, 0xF465,
    0xF596, 0xF6C2, 0xF7DD, 0xF8DD, 0xF9B8, 0xFA61, 0xFACF, 0xFAF6, 0xD2DC, 0xD30B, 0xD38D, 0xD455, 0xD555, 0xD67E, 0xD7C4, 0xD918,
    0xDA6B, 0xDBB1, 0xDCDB, 0xDDDB, 0xDEA3, 0xDF25, 0xDF54, 0xDF2B, 0xDEB8, 0xDE08, 0xDD24, 0xDC18, 0xDAF0, 0xD9B7, 0xD878, 0xD73F,
    0xD617, 0xD50C, 0xD428, 0xD377, 0xD305, 0xD2DC, 0xD305, 0xD377, 0xD428, 0xD50C, 0xD617, 0xD73F, 0xD878, 0xD9B7, 0xDAF0, 0xDC18,
    0xDD24, 0xDE08, 0xDEB8, 0xDF2B, 0xDF54, 0xDF2B, 0xDEB8, 0xDE08, 0xDD24, 0xDC18, 0xDAF0, 0xD9B7, 0xD878, 0xD73F, 0xD617, 0xD50C,
    0xD428, 0xD377, 0xD305, 0xD2DC, 0xD305, 0xD377, 0xD427, 0xD50B, 0xD616, 0xD73D, 0xD876, 0xD9B4, 0xDAED, 0xDC15, 0xDD21, 0xDE05,
    0xDEB6, 0xDF2A, 0xDF54, 0xDF2D, 0xDEBC, 0xDE0E, 0xDD2E, 0xDC26, 0xDB01, 0xD9CC, 0xD891, 0xD75C, 0xD637, 0xD52E, 0xD44D, 0xD39E,
    0xD32D, 0xD305, 0xD4B4, 0xD4F2, 0xD5A1, 0xD6AF, 0xD807, 0xD997, 0xDB4E, 0xDD16, 0xDEDF, 0xE095, 0xE226, 0xE37E, 0xE48B, 0xE53B,
    0xE579, 0xE543, 0xE4A9, 0xE3BB, 0xE288, 0xE120, 0xDF92, 0xDDED, 0xDC40, 0xDA9B, 0xD90D, 0xD7A5, 0xD672, 0xD584, 0xD4EA, 0xD4B4,
    0xD4EA, 0xD584, 0xD672, 0xD7A5, 0xD90D, 0xDA9B, 0xDC40, 0xDDED, 0xDF92, 0xE120, 0xE288, 0xE3BB, 0xE4A9, 0xE543, 0xE579, 0xE543,
    0xE4A9, 0xE3BB, 0xE288, 0xE120, 0xDF92, 0xDDED, 0xDC40, 0xDA9B, 0xD90D, 0xD7A5, 0xD672, 0xD584, 0xD4EA, 0xD4B4, 0xD4EA, 0xD584,
    0xD671, 0xD7A3, 0xD90B, 0xDA98, 0xDC3D, 0xDDE9, 0xDF8E, 0xE11C, 0xE284, 0xE3B7, 0xE4A6, 0xE541, 0xE579, 0xE545, 0xE4AE, 0xE3C4,
    0xE296, 0xE132, 0xDFA9, 0xDE09, 0xDC61, 0xDAC1, 0xD937, 0xD7D3, 0xD6A4, 0xD5B9, 0xD520, 0xD4EA, 0xF549, 0xF516, 0xF485, 0xF3A7,
    0xF28A, 0xF13F, 0xEFD6, 0xEE5C, 0xECE3, 0xEB79, 0xEA2E, 0xE912, 0xE833, 0xE7A3, 0xE76F, 0xE79C, 0xE81B, 0xE8E0, 0xE9DD, 0xEB06,
    0xEC4F, 0xEDAB, 0xEF0D, 0xF069, 0xF1B2, 0xF2DB, 0xF3D8, 0xF49D, 0xF51C, 0xF549, 0xF51C, 0xF49D, 0xF3D8, 0xF2DB, 0xF1B2, 0xF069,
    0xEF0D, 0xEDAB, 0xEC4F, 0xEB06, 0xE9DD, 0xE8E0, 0xE81B, 0xE79C, 0xE76F, 0xE79C, 0xE81B, 0xE8E0, 0xE9DD, 0xEB06, 0xEC4F, 0xEDAB,
    0xEF0D, 0xF069, 0xF1B2, 0xF2DB, 0xF3D8, 0xF49D, 0xF51C, 0xF549, 0xF51C, 0xF49D, 0xF3D9, 0xF2DC, 0xF1B4, 0xF06B, 0xEF10, 0xEDAE,
    0xEC53, 0xEB0A, 0xE9E0, 0xE8E3, 0xE81E, 0xE79E, 0xE76F, 0xE79A, 0xE817, 0xE8D8, 0xE9D2, 0xEAF7, 0xEC3C, 0xED94, 0xEEF2, 0xF04A,
    0xF18F, 0xF2B5, 0xF3AF, 0xF472, 0xF4EF, 0xF51C, 0xC7DD, 0xC7E2, 0xC7EF, 0xC802, 0xC81B, 0xC839, 0xC859, 0xC87A, 0xC89B, 0xC8BB,
    0xC8D8, 0xC8F1, 0xC905, 0xC912, 0xC916, 0xC912, 0xC907, 0xC8F6, 0xC8DF, 0xC8C5, 0xC8A8, 0xC88A, 0xC86A, 0xC84C, 0xC82F, 0xC814,
    0xC7FE, 0xC7ED, 0xC7E1, 0xC7DD, 0xC7E1, 0xC7ED, 0xC7FE, 0xC814, 0xC82F, 0xC84C, 0xC86A, 0xC88A, 0xC8A8, 0xC8C5, 0xC8DF, 0xC8F6,
    0xC907, 0xC912, 0xC916, 0xC912, 0xC907, 0xC8F6, 0xC8DF, 0xC8C5, 0xC8A8, 0xC88A, 0xC86A, 0xC84C, 0xC82F, 0xC814, 0xC7FE, 0xC7ED,
    0xC7E1, 0xC7DD, 0xC7E1, 0xC7EC, 0xC7FD, 0xC813, 0xC82D, 0xC84A, 0xC868, 0xC887, 0xC8A6, 0xC8C3, 0xC8DD, 0xC8F4, 0xC905, 0xC911,
    0xC916, 0xC914, 0xC90B, 0xC8FC, 0xC8E8, 0xC8D1, 0xC8B7, 0xC89B, 0xC87F, 0xC864, 0xC849, 0xC832, 0xC81E, 0xC80E, 0xC804, 0xC800,
    0x054B, 0x057F, 0x0611, 0x06F1, 0x0810, 0x095D, 0x0ACA, 0x0C46, 0x0DC2, 0x0F2E, 0x107C, 0x119A, 0x127A, 0x130C, 0x1340, 0x1313,
    0x1292, 0x11CC, 0x10CD, 0x0FA2, 0x0E56, 0x0CF8, 0x0B93, 0x0A35, 0x08EA, 0x07BE, 0x06BF, 0x05F9, 0x0579, 0x054B, 0x0579, 0x05F9,
    0x06BF, 0x07BE, 0x08EA, 0x0A35, 0x0B93, 0x0CF8, 0x0E56, 0x0FA2, 0x10CD, 0x11CC, 0x1292, 0x1313, 0x1340, 0x1313, 0x1292, 0x11CC,
    0x10CD, 0x0FA2, 0x0E56, 0x0CF8, 0x0B93, 0x0A35, 0x08EA, 0x07BE, 0x06BF, 0x05F9, 0x0579, 0x054B, 0x0579, 0x05F9, 0x06BE, 0x07BD,
    0x08E8, 0x0A33, 0x0B91, 0x0CF5, 0x0E53, 0x0F9E, 0x10CA, 0x11CA, 0x1290, 0x1311, 0x1340, 0x1314, 0x1297, 0x11D4, 0x10D9, 0x0FB1,
    0x0E6A, 0x0D10, 0x0BAF, 0x0A55, 0x090D, 0x07E5, 0x06E8, 0x0625, 0x05A6, 0x0579, 0x2BD6, 0x2BC0, 0x2B80, 0x2B1F, 0x2AA3, 0x2A13,
    0x2975, 0x28D0, 0x282B, 0x278D, 0x26FC, 0x2680, 0x261F, 0x25E0, 0x25C9, 0x25DD, 0x2615, 0x266A, 0x26D9, 0x275B, 0x27EB, 0x2882,
    0x291D, 0x29B5, 0x2A45, 0x2AC7, 0x2B35, 0x2B8B, 0x2BC3, 0x2BD6, 0x2BC3, 0x2B8B, 0x2B35, 0x2AC7, 0x2A45, 0x29B5, 0x291D, 0x2882,
    0x27EB, 0x275B, 0x26D9, 0x266A, 0x2615, 0x25DD, 0x25C9, 0x25DD, 0x2615, 0x266A, 0x26D9, 0x275B, 0x27EB, 0x2882, 0x291D, 0x29B5,
    0x2A45, 0x2AC7, 0x2B35, 0x2B8B, 0x2BC3, 0x2BD6, 0x2BC3, 0x2B8B, 0x2B35, 0x2AC7, 0x2A45, 0x29B6, 0x291E, 0x2884, 0x27EC, 0x275C,
    0x26DA, 0x266C, 0x2616, 0x25DE, 0x25C9, 0x25DC, 0x2613, 0x2667, 0x26D4, 0x2754, 0x27E2, 0x2878, 0x2911, 0x29A7, 0x2A35, 0x2AB6,
    0x2B23, 0x2B78, 0x2BAF, 0x2BC3, 0xCCF6, 0xCD40, 0xCE0F, 0xCF4D, 0xD0E4, 0xD2BE, 0xD4C4, 0xD6E1, 0xD8FD, 0xDB03, 0xDCDD, 0xDE74,
    0xDFB2, 0xE081, 0xE0CB, 0xE08B, 0xDFD5, 0xDEBB, 0xDD51, 0xDBA7, 0xD9D0, 0xD7DE, 0xD5E3, 0xD3F1, 0xD21A, 0xD070, 0xCF06, 0xCDED,
    0xCD37, 0xCCF6, 0xCD37, 0xCDED, 0xCF06, 0xD070, 0xD21A, 0xD3F1, 0xD5E3, 0xD7DE, 0xD9D0, 0xDBA7, 0xDD51, 0xDEBB, 0xDFD5, 0xE08B,
    0xE0CB, 0xE08B, 0xDFD5, 0xDEBB, 0xDD51, 0xDBA7, 0xD9D0, 0xD7DE, 0xD5E3, 0xD3F1, 0xD21A, 0xD070, 0xCF06, 0xCDED, 0xCD37, 0xCCF6,
    0xCD36, 0xCDEC, 0xCF05, 0xD06F, 0xD218, 0xD3EE, 0xD5DF, 0xD7DA, 0xD9CB, 0xDBA2, 0xDD4C, 0xDEB7, 0xDFD1, 0xE089, 0xE0CB, 0xE08D,
    0xDFDB, 0xDEC6, 0xDD61, 0xDBBC, 0xD9EB, 0xD7FF, 0xD60A, 0xD41E, 0xD24D, 0xD0A7, 0xCF41, 0xCE2B, 0xCD77, 0xCD37, 0x0000, 0x0006,
    0x0019, 0x0036, 0x005C, 0x0087, 0x00B7, 0x00E8, 0x0119, 0x0149, 0x0174, 0x019A, 0x01B7, 0x01CA, 0x01D1, 0x01CB, 0x01BA, 0x01A0,
    0x017F, 0x0158, 0x012D, 0x00FF, 0x00D1, 0x00A3, 0x0078, 0x0051, 0x0030, 0x0016, 0x0005, 0x0000, 0x0005, 0x0016, 0x0030, 0x0051,
    0x0078, 0x00A3, 0x00D1, 0x00FF, 0x012D, 0x0158, 0x017F, 0x01A0, 0x01BA, 0x01CB, 0x01D1, 0x01CB, 0x01BA, 0x01A0, 0x017F, 0x0158,
    0x012D, 0x00FF, 0x00D1, 0x00A3, 0x0078, 0x0051, 0x0030, 0x0016, 0x0005, 0x0000, 0x0005, 0x0016, 0x0030, 0x0051, 0x0078, 0x00A3,
    0x00D0, 0x00FF, 0x012C, 0x0158, 0x017F, 0x01A0, 0x01BA, 0x01CA, 0x01D1, 0x01CD, 0x01C2, 0x01B0, 0x0198, 0x017C, 0x015B, 0x0138,
    0x0111, 0x00E9, 0x00C1, 0x0098, 0x0070, 0x004A, 0x0026, 0x0005, 0xFF30, 0xFF32, 0xFF37, 0xFF40, 0xFF4B, 0xFF58, 0xFF66, 0xFF74,
    0xFF83, 0xFF90, 0xFF9D, 0xFFA8, 0xFFB1, 0xFFB6, 0xFFB8, 0xFFB6, 0xFFB2, 0xFFAA, 0xFFA0, 0xFF95, 0xFF88, 0xFF7B, 0xFF6D, 0xFF60,
    0xFF53, 0xFF48, 0xFF3E, 0xFF37, 0xFF32, 0xFF30, 0xFF32, 0xFF37, 0xFF3E, 0xFF48, 0xFF53, 0xFF60, 0xFF6D, 0xFF7B, 0xFF88, 0xFF95,
    0xFFA0, 0xFFAA, 0xFFB2, 0xFFB6, 0xFFB8, 0xFFB6, 0xFFB2, 0xFFAA, 0xFFA0, 0xFF95, 0xFF88, 0xFF7B, 0xFF6D, 0xFF60, 0xFF53, 0xFF48,
    0xFF3E, 0xFF37, 0xFF32, 0xFF30, 0xFF32, 0xFF37, 0xFF3E, 0xFF48, 0xFF53, 0xFF60, 0xFF6D, 0xFF7B, 0xFF88, 0xFF95, 0xFFA0, 0xFFAA,
    0xFFB1, 0xFFB6, 0xFFB8, 0xFFB7, 0xFFB4, 0xFFAF, 0xFFA8, 0xFF9F, 0xFF96, 0xFF8B, 0xFF80, 0xFF74, 0xFF69, 0xFF5D, 0xFF51, 0xFF46,
    0xFF3B, 0xFF32, 0xC9D8, 0xC9A7, 0xC91F, 0xC84E, 0xC743, 0xC60C, 0xC4B7, 0xC355, 0xC1F2, 0xC09D, 0xBF66, 0xBE5B, 0xBD8A, 0xBD02,
    0xBCD1, 0xBCFC, 0xBD73, 0xBE2C, 0xBF1A, 0xC032, 0xC167, 0xC2AE, 0xC3FB, 0xC542, 0xC677, 0xC78F, 0xC87D, 0xC936, 0xC9AE, 0xC9D8,
    0xC9AE, 0xC936, 0xC87D, 0xC78F, 0xC677, 0xC542, 0xC3FB, 0xC2AE, 0xC167, 0xC032, 0xBF1A, 0xBE2C, 0xBD73, 0xBCFC, 0xBCD1, 0xBCFC,
    0xBD73, 0xBE2C, 0xBF1A, 0xC032, 0xC167, 0xC2AE, 0xC3FB, 0xC542, 0xC677, 0xC78F, 0xC87D, 0xC936, 0xC9AE, 0xC9D8, 0xC9AE, 0xC936,
    0xC87E, 0xC790, 0xC679, 0xC544, 0xC3FE, 0xC2B1, 0xC16A, 0xC035, 0xBF1D, 0xBE2F, 0xBD75, 0xBCFD, 0xBCD1, 0xBCEC, 0xBD3C, 0xBDBB,
    0xBE64, 0xBF30, 0xC019, 0xC11A, 0xC22C, 0xC34A, 0xC46F, 0xC593, 0xC6B1, 0xC7C4, 0xC8C4, 0xC9AE, 0x1E4A, 0x1E9C, 0x1F82, 0x20E3,
    0x22A6, 0x24B4, 0x26F2, 0x294A, 0x2BA1, 0x2DE0, 0x2FED, 0x31B1, 0x3312, 0x33F8, 0x344A, 0x3402, 0x3338, 0x3200, 0x306E, 0x2E96,
    0x2C8B, 0x2A63, 0x2830, 0x2608, 0x23FE, 0x2225, 0x2093, 0x1F5B, 0x1E91, 0x1E4A, 0x1E91, 0x1F5B, 0x2093, 0x2225, 0x23FE, 0x2608,
    0x2830, 0x2A63, 0x2C8B, 0x2E96, 0x306E, 0x3200, 0x3338, 0x3402, 0x344A, 0x3402, 0x3338, 0x3200, 0x306E, 0x2E96, 0x2C8B, 0x2A63,
    0x2830, 0x2608, 0x23FE, 0x2225, 0x2093, 0x1F5B, 0x1E91, 0x1E4A, 0x1E91, 0x1F5B, 0x2092, 0x2224, 0x23FB, 0x2605, 0x282C, 0x2A5E,
    0x2C86, 0x2E90, 0x3069, 0x31FB, 0x3335, 0x3400, 0x344A, 0x341C, 0x3395, 0x32BE, 0x31A2, 0x3049, 0x2EBF, 0x2D0E, 0x2B3E, 0x295B,
    0x276D, 0x2580, 0x239C, 0x21CD, 0x201B, 0x1E91, 0x1B09, 0x1AF8, 0x1AC8, 0x1A7F, 0x1A22, 0x19B5, 0x193F, 0x18C3, 0x1847, 0x17D0,
    0x1764, 0x1706, 0x16BD, 0x168E, 0x167D, 0x168C, 0x16B5, 0x16F6, 0x1749, 0x17AB, 0x1817, 0x1889, 0x18FD, 0x196F, 0x19DB, 0x1A3D,
    0x1A90, 0x1AD0, 0x1AFA, 0x1B09, 0x1AFA, 0x1AD0, 0x1A90, 0x1A3D, 0x19DB, 0x196F, 0x18FD, 0x1889, 0x1817, 0x17AB, 0x1749, 0x16F6,
    0x16B5, 0x168C, 0x167D, 0x168C, 0x16B5, 0x16F6, 0x1749, 0x17AB, 0x1817, 0x1889, 0x18FD, 0x196F, 0x19DB, 0x1A3D, 0x1A90, 0x1AD0,
    0x1AFA, 0x1B09, 0x1AFA, 0x1AD0, 0x1A90, 0x1A3D, 0x19DC, 0x1970, 0x18FE, 0x188A, 0x1818, 0x17AC, 0x174A, 0x16F7, 0x16B6, 0x168C,
    0x167D, 0x1686, 0x16A2, 0x16CF, 0x1709, 0x1751, 0x17A2, 0x17FC, 0x185B, 0x18BF, 0x1925, 0x198B, 0x19EF, 0x1A4F, 0x1AA9, 0x1AFA,
    0x1144, 0x115A, 0x1197, 0x11F5, 0x126D, 0x12F9, 0x1391, 0x1431, 0x14D0, 0x1569, 0x15F4, 0x166C, 0x16CA, 0x1707, 0x171D, 0x170A,
    0x16D4, 0x1681, 0x1616, 0x1599, 0x150E, 0x147B, 0x13E6, 0x1353, 0x12C8, 0x124B, 0x11E0, 0x118D, 0x1157, 0x1144, 0x1157, 0x118D,
    0x11E0, 0x124B, 0x12C8, 0x1353, 0x13E6, 0x147B, 0x150E, 0x1599, 0x1616, 0x1681, 0x16D4, 0x170A, 0x171D, 0x170A, 0x16D4, 0x1681,
    0x1616, 0x1599, 0x150E, 0x147B, 0x13E6, 0x1353, 0x12C8, 0x124B, 0x11E0, 0x118D, 0x1157, 0x1144, 0x1157, 0x118D, 0x11E0, 0x124A,
    0x12C8, 0x1352, 0x13E5, 0x147A, 0x150D, 0x1598, 0x1615, 0x1680, 0x16D3, 0x1709, 0x171D, 0x1711, 0x16ED, 0x16B4, 0x1668, 0x160D,
    0x15A4, 0x1531, 0x14B6, 0x1435, 0x13B2, 0x132F, 0x12AE, 0x1233, 0x11C0, 0x1157, 0xAC44, 0xAC20, 0xABBD, 0xAB25, 0xAA62, 0xA97F,
    0xA887, 0xA784, 0xA681, 0xA589, 0xA4A6, 0xA3E3, 0xA34A, 0xA2E7, 0xA2C3, 0xA2E2, 0xA33A, 0xA3C0, 0xA46E, 0xA53A, 0xA61C, 0xA70A,
    0xA7FD, 0xA8EC, 0xA9CD, 0xAA99, 0xAB47, 0xABCE, 0xAC25, 0xAC44, 0xAC25, 0xABCE, 0xAB47, 0xAA99, 0xA9CD, 0xA8EC, 0xA7FD, 0xA70A,
    0xA61C, 0xA53A, 0xA46E, 0xA3C0, 0xA33A, 0xA2E2, 0xA2C3, 0xA2E2, 0xA33A, 0xA3C0, 0xA46E, 0xA53A, 0xA61C, 0xA70A, 0xA7FD, 0xA8EC,
    0xA9CD, 0xAA99, 0xAB47, 0xABCE, 0xAC25, 0xAC44, 0xAC25, 0xABCE, 0xAB47, 0xAA9A, 0xA9CE, 0xA8ED, 0xA7FF, 0xA70C, 0xA61E, 0xA53C,
    0xA470, 0xA3C2, 0xA33B, 0xA2E3, 0xA2C3, 0xA2D7, 0xA312, 0xA36E, 0xA3E9, 0xA47E, 0xA528, 0xA5E3, 0xA6AC, 0xA77C, 0xA851, 0xA926,
    0xA9F7, 0xAABF, 0xAB7B, 0xAC25, 0x0000, 0xFFFB, 0xFFEA, 0xFFD1, 0xFFB0, 0xFF8B, 0xFF61, 0xFF36, 0xFF0B, 0xFEE2, 0xFEBC, 0xFE9C,
    0xFE83, 0xFE72, 0xFE6C, 0xFE71, 0xFE80, 0xFE96, 0xFEB3, 0xFED5, 0xFEFB, 0xFF22, 0xFF4B, 0xFF72, 0xFF98, 0xFFBA, 0xFFD6, 0xFFED,
    0xFFFB, 0x0000, 0xFFFB, 0xFFED, 0xFFD6, 0xFFBA, 0xFF98, 0xFF72, 0xFF4B, 0xFF22, 0xFEFB, 0xFED5, 0xFEB3, 0xFE96, 0xFE80, 0xFE71,
    0xFE6C, 0xFE71, 0xFE80, 0xFE96, 0xFEB3, 0xFED5, 0xFEFB, 0xFF22, 0xFF4B, 0xFF72, 0xFF98, 0xFFBA, 0xFFD6, 0xFFED, 0xFFFB, 0x0000,
    0xFFFB, 0xFFED, 0xFFD7, 0xFFBA, 0xFF98, 0xFF72, 0xFF4B, 0xFF23, 0xFEFB, 0xFED5, 0xFEB4, 0xFE97, 0xFE80, 0xFE72, 0xFE6C, 0xFE70,
    0xFE79, 0xFE89, 0xFE9D, 0xFEB6, 0xFED2, 0xFEF1, 0xFF12, 0xFF35, 0xFF59, 0xFF7C, 0xFF9F, 0xFFC0, 0xFFDF, 0xFFFB, 0x0000, 0xFFFF,
    0xFFFB, 0xFFF4, 0xFFEC, 0xFFE2, 0xFFD8, 0xFFCD, 0xFFC2, 0xFFB7, 0xFFAE, 0xFFA5, 0xFF9F, 0xFF9B, 0xFF99, 0xFF9B, 0xFF9E, 0xFFA4,
    0xFFAB, 0xFFB4, 0xFFBE, 0xFFC8, 0xFFD2, 0xFFDC, 0xFFE6, 0xFFEE, 0xFFF6, 0xFFFB, 0xFFFF, 0x0000, 0xFFFF, 0xFFFB, 0xFFF6, 0xFFEE,
    0xFFE6, 0xFFDC, 0xFFD2, 0xFFC8, 0xFFBE, 0xFFB4, 0xFFAB, 0xFFA4, 0xFF9E, 0xFF9B, 0xFF99, 0xFF9B, 0xFF9E, 0xFFA4, 0xFFAB, 0xFFB4,
    0xFFBE, 0xFFC8, 0xFFD2, 0xFFDC, 0xFFE6, 0xFFEE, 0xFFF6, 0xFFFB, 0xFFFF, 0x0000, 0xFFFF, 0xFFFB, 0xFFF6, 0xFFEE, 0xFFE6, 0xFFDC,
    0xFFD2, 0xFFC8, 0xFFBE, 0xFFB4, 0xFFAC, 0xFFA4, 0xFF9E, 0xFF9B, 0xFF99, 0xFF9A, 0xFF9D, 0xFFA1, 0xFFA6, 0xFFAC, 0xFFB3, 0xFFBB,
    0xFFC4, 0xFFCD, 0xFFD6, 0xFFDF, 0xFFE8, 0xFFF0, 0xFFF8, 0xFFFF, 0xBF40, 0xBF1A, 0xBEB0, 0xBE0D, 0xBD3C, 0xBC49, 0xBB3F, 0xBA2A,
    0xB914, 0xB80A, 0xB717, 0xB646, 0xB5A3, 0xB539, 0xB513, 0xB534, 0xB591, 0xB622, 0xB6DC, 0xB7B6, 0xB8A8, 0xB9A7, 0xBAAC, 0xBBAB,
    0xBC9D, 0xBD77, 0xBE31, 0xBEC2, 0xBF1F, 0xBF40, 0xBF1F, 0xBEC2, 0xBE31, 0xBD77, 0xBC9D, 0xBBAB, 0xBAAC, 0xB9A7, 0xB8A8, 0xB7B6,
    0xB6DC, 0xB622, 0xB591, 0xB534, 0xB513, 0xB534, 0xB591, 0xB622, 0xB6DC, 0xB7B6, 0xB8A8, 0xB9A7, 0xBAAC, 0xBBAB, 0xBC9D, 0xBD77,
    0xBE31, 0xBEC2, 0xBF1F, 0xBF40, 0xBF1F, 0xBEC2, 0xBE32, 0xBD78, 0xBC9E, 0xBBAD, 0xBAAE, 0xB9AA, 0xB8AA, 0xB7B9, 0xB6DE, 0xB624,
    0xB593, 0xB535, 0xB513, 0xB528, 0xB566, 0xB5CA, 0xB64D, 0xB6ED, 0xB7A3, 0xB86C, 0xB942, 0xBA22, 0xBB06, 0xBBEA, 0xBCCA, 0xBDA0,
    0xBE69, 0xBF1F, 0x2537, 0x257A, 0x2635, 0x2755, 0x28C4, 0x2A71, 0x2C45, 0x2E2D, 0x3016, 0x31EA, 0x3396, 0x3506, 0x3626, 0x36E1,
    0x3724, 0x36E9, 0x3645, 0x3547, 0x33FF, 0x327E, 0x30D4, 0x2F12, 0x2D48, 0x2B86, 0x29DC, 0x285C, 0x2714, 0x2616, 0x2571, 0x2537,
    0x2571, 0x2616, 0x2714, 0x285C, 0x29DC, 0x2B86, 0x2D48, 0x2F12, 0x30D4, 0x327E, 0x33FF, 0x3547, 0x3645, 0x36E9, 0x3724, 0x36E9,
    0x3645, 0x3547, 0x33FF, 0x327E, 0x30D4, 0x2F12, 0x2D48, 0x2B86, 0x29DC, 0x285C, 0x2714, 0x2616, 0x2571, 0x2537, 0x2571, 0x2615,
    0x2713, 0x285A, 0x29DA, 0x2B83, 0x2D45, 0x2F0F, 0x30D0, 0x327A, 0x33FB, 0x3543, 0x3642, 0x36E8, 0x3724, 0x36FF, 0x3690, 0x35E1,
    0x34FA, 0x33E1, 0x32A0, 0x313F, 0x2FC5, 0x2E3B, 0x2CA9, 0x2B17, 0x298D, 0x2813, 0x26B2, 0x2571, 0xD64A, 0xD656, 0xD676, 0xD6A7,
    0xD6E5, 0xD72E, 0xD77E, 0xD7D1, 0xD824, 0xD874, 0xD8BD, 0xD8FC, 0xD92D, 0xD94D, 0xD958, 0xD94E, 0xD932, 0xD907, 0xD8CF, 0xD88D,
    0xD845, 0xD7F8, 0xD7AA, 0xD75D, 0xD715, 0xD6D3, 0xD69C, 0xD670, 0xD654, 0xD64A, 0xD654, 0xD670, 0xD69C, 0xD6D3, 0xD715, 0xD75D,
    0xD7AA, 0xD7F8, 0xD845, 0xD88D, 0xD8CF, 0xD907, 0xD932, 0xD94E, 0xD958, 0xD94E, 0xD932, 0xD907, 0xD8CF, 0xD88D, 0xD845, 0xD7F8,
    0xD7AA, 0xD75D, 0xD715, 0xD6D3, 0xD69C, 0xD670, 0xD654, 0xD64A, 0xD654, 0xD670, 0xD69B, 0xD6D3, 0xD715, 0xD75D, 0xD7AA, 0xD7F8,
    0xD844, 0xD88D, 0xD8CE, 0xD906, 0xD932, 0xD94E, 0xD958, 0xD952, 0xD93F, 0xD921, 0xD8FA, 0xD8CA, 0xD893, 0xD857, 0xD817, 0xD7D4,
    0xD78F, 0xD74B, 0xD707, 0xD6C7, 0xD68B, 0xD654, 0xEC41, 0xEC26, 0xEBDB, 0xEB69, 0xEAD6, 0xEA2B, 0xE971, 0xE8AE, 0xE7EB, 0xE730,
    0xE686, 0xE5F3, 0xE580, 0xE535, 0xE51B, 0xE532, 0xE574, 0xE5D9, 0xE65C, 0xE6F5, 0xE79F, 0xE852, 0xE909, 0xE9BD, 0xEA66, 0xEB00,
    0xEB82, 0xEBE8, 0xEC29, 0xEC41, 0xEC29, 0xEBE8, 0xEB82, 0xEB00, 0xEA66, 0xE9BD, 0xE909, 0xE852, 0xE79F, 0xE6F5, 0xE65C, 0xE5D9,
    0xE574, 0xE532, 0xE51B, 0xE532, 0xE574, 0xE5D9, 0xE65C, 0xE6F5, 0xE79F, 0xE852, 0xE909, 0xE9BD, 0xEA66, 0xEB00, 0xEB82, 0xEBE8,
    0xEC29, 0xEC41, 0xEC2A, 0xEBE8, 0xEB83, 0xEB00, 0xEA67, 0xE9BE, 0xE90B, 0xE854, 0xE7A1, 0xE6F7, 0xE65D, 0xE5DB, 0xE575, 0xE533,
    0xE51B, 0xE52A, 0xE556, 0xE59B, 0xE5F8, 0xE668, 0xE6E8, 0xE775, 0xE80B, 0xE8A8, 0xE949, 0xE9E9, 0xEA86, 0xEB1D, 0xEBA9, 0xEC29,
    0xB59E, 0xB589, 0xB54F, 0xB4F5, 0xB481, 0xB3FB, 0xB369, 0xB2D0, 0xB237, 0xB1A5, 0xB11F, 0xB0AC, 0xB052, 0xB017, 0xB002, 0xB014,
    0xB048, 0xB097, 0xB0FE, 0xB176, 0xB1FC, 0xB288, 0xB318, 0xB3A5, 0xB42A, 0xB4A2, 0xB509, 0xB558, 0xB58C, 0xB59E, 0xB58C, 0xB558,
    0xB509, 0xB4A2, 0xB42A, 0xB3A5, 0xB318, 0xB288, 0xB1FC, 0xB176, 0xB0FE, 0xB097, 0xB048, 0xB014, 0xB002, 0xB014, 0xB048, 0xB097,
    0xB0FE, 0xB176, 0xB1FC, 0xB288, 0xB318, 0xB3A5, 0xB42A, 0xB4A2, 0xB509, 0xB558, 0xB58C, 0xB59E, 0xB58C, 0xB559, 0xB509, 0xB4A3,
    0xB42B, 0xB3A6, 0xB319, 0xB28A, 0xB1FD, 0xB178, 0xB0FF, 0xB099, 0xB049, 0xB015, 0xB002, 0xB00E, 0xB030, 0xB067, 0xB0AF, 0xB107,
    0xB16C, 0xB1DA, 0xB250, 0xB2CC, 0xB34A, 0xB3C7, 0xB443, 0xB4B9, 0xB527, 0xB58C, 0x7FFF, 0x7FFF, 0x4171, 0xBF5F, 0x7FFF, 0x7FFF,
    0x4171, 0xBF5F, 0x0215, 0x01F5, 0x019A, 0x010E, 0x005C, 0xFF8D, 0xFEAA, 0xFDBD, 0xFCD0, 0xFBED, 0xFB1D, 0xFA6B, 0xF9DF, 0xF984,
    0xF964, 0xF980, 0xF9D0, 0xFA4B, 0xFAEA, 0xFBA5, 0xFC73, 0xFD4E, 0xFE2C, 0xFF06, 0xFFD5, 0x008E, 0x012D, 0x01A9, 0x01F9, 0x0215,
    0x01F9, 0x01A9, 0x012D, 0x008E, 0xFFD5, 0xFF06, 0xFE2C, 0xFD4E, 0xFC73, 0xFBA5, 0xFAEA, 0xFA4B, 0xF9D0, 0xF980, 0xF964, 0xF980,
    0xF9D0, 0xFA4B, 0xFAEA, 0xFBA5, 0xFC73, 0xFD4E, 0xFE2C, 0xFF06, 0xFFD5, 0x008E, 0x012D, 0x01A9, 0x01F9, 0x0215, 0x01F9, 0x01A9,
    0x012D, 0x008E, 0xFFD5, 0xFF06, 0xFE2C, 0xFD4E, 0xFC73, 0xFBA5, 0xFAEA, 0xFA4B, 0xF9D0, 0xF980, 0xF964, 0xF980, 0xF9D0, 0xFA4B,
    0xFAEA, 0xFBA5, 0xFC73, 0xFD4E, 0xFE2C, 0xFF06, 0xFFD5, 0x008E, 0x012D, 0x01A9, 0x01F9, 0x0215, 0x081A, 0x40A1, 0x40A7, 0x40B8,
    0x40D2, 0x40F4, 0x411B, 0x4145, 0x4172, 0x419E, 0x41C9, 0x41F0, 0x4211, 0x422B, 0x423C, 0x4242, 0x423D, 0x422E, 0x4217, 0x41F9,
    0x41D6, 0x41AF, 0x4186, 0x415D, 0x4134, 0x410D, 0x40EA, 0x40CC, 0x40B5, 0x40A6, 0x40A1, 0x40A6, 0x40B5, 0x40CC, 0x40EA, 0x410D,
    0x4134, 0x415D, 0x4186, 0x41AF, 0x41D6, 0x41F9, 0x4217, 0x422E, 0x423D, 0x4242, 0x423D, 0x422E, 0x4217, 0x41F9, 0x41D6, 0x41AF,
    0x4186, 0x415D, 0x4134, 0x410D, 0x40EA, 0x40CC, 0x40B5, 0x40A6, 0x40A1, 0x40A6, 0x40B5, 0x40CC, 0x40EA, 0x410D, 0x4134, 0x415C,
    0x4186, 0x41AF, 0x41D6, 0x41F9, 0x4217, 0x422E, 0x423D, 0x4242, 0x423F, 0x4235, 0x4225, 0x4210, 0x41F6, 0x41D9, 0x41B9, 0x4197,
    0x4173, 0x414E, 0x412A, 0x4106, 0x40E4, 0x40C4, 0x40A6,
};

static const struct Animation sOmmMainMenuMarioAnim = {
    0, 0, 0, 0, 90, 0, sOmmMainMenuMarioAnimValues, sOmmMainMenuMarioAnimIndices, 0
};

static const struct Animation *sOmmMainMenuMarioAnims[] = {
    &sOmmMainMenuMarioAnim
};

//
// Main Menu update
//

static f32 omm_main_menu_get_mario_cappy_pos_y(f32 t, f32 value_at_0, f32 peak, f32 peak_t) {
    f32 a = (value_at_0 - peak) / sqr_f(peak_t);
    f32 b = -2 * (value_at_0 - peak) / peak_t;
    f32 c = value_at_0;
    return a * t * t + b * t + c;
}

static s32 omm_main_menu_update() {
    if (optmenu_open) return 0;
    sOmmMainMenu->timer++;

    // Update Mario
    struct Object *mario = obj_get_first_with_behavior(bhvOmmMainMenuMario);
    if (mario) {
        mario->oAnimations = (struct Animation **) sOmmMainMenuMarioAnims;
        f32 t = relerp_0_1_f(mario->oTimer, 0, 20, 1, 0);
        f32 posX = 175 * GFX_DIMENSIONS_ASPECT_RATIO;
        f32 posY = omm_main_menu_get_mario_cappy_pos_y(t, -950, -850, 0.2f);
        obj_scale(mario, 7);
        obj_set_pos(mario, posX, posY, 0);
        obj_set_angle(mario, -0x800, -6 * posX, 0);
        obj_update_gfx(mario);
        obj_anim_play(mario, 0, 1.f);
        obj_anim_set_frame(mario, gMarioState->actionTimer);
        gMarioState->actionTimer = (gMarioState->actionTimer + 1) * !obj_anim_is_at_end(mario);
        mario->oGraphNode = gLoadedGraphNodes[MODEL_MARIO] = geo_layout_to_graph_node(NULL, mario_geo);
        mario->oAction = 0;
        mario->oTimer++;
    }

    // Update Cappy
    struct Object *cappy = obj_get_first_with_behavior(bhvOmmMainMenuCappy);
    if (cappy) {
        f32 t = relerp_0_1_f(cappy->oTimer, 0, 30, 1, 0);
        f32 posX = 175 * GFX_DIMENSIONS_ASPECT_RATIO;
        f32 posY = omm_main_menu_get_mario_cappy_pos_y(t, 0, 100, 0.225f);
        obj_scale(cappy, 6);
        obj_set_pos(cappy, posX, posY + relerp_0_1_f(sins(cappy->oTimer * 0x400), -1, 1, 20, 60), 0);
        obj_set_angle(cappy, -0x400, -6 * posX, relerp_0_1_f(coss(cappy->oTimer * 0x6B7), -1, 1, 0, -0x400));
        obj_update_gfx(cappy);
        cappy->oOpacity = 0xFF;
        cappy->oGraphNode = gLoadedGraphNodes[MODEL_MARIOS_CAP] = geo_layout_to_graph_node(NULL, marios_cap_geo);
        cappy->oTimer++;
    }

    // Update inputs
    u32 inputs = omm_get_inputs();

    // Transition to File select screen
    if (sOmmFileSelect->timer) {
        sOmmFileSelect->open = (--sOmmFileSelect->timer == 0);
        mario->oAction = (sOmmFileSelect->open && sOmmMainMenu->index == OMM_MM_PLAY);
    }
    
    // Option above
    else if (inputs & STICK_UP) {
        play_sound(OMM_MM_SOUND_SCROLL, gGlobalSoundArgs);
        sOmmMainMenu->index = (sOmmMainMenu->index + OMM_MM_STRINGS_COUNT - 1) % OMM_MM_STRINGS_COUNT;
        sOmmMainMenu->timer = 8;
    }
    
    // Option below
    else if (inputs & STICK_DOWN) {
        play_sound(OMM_MM_SOUND_SCROLL, gGlobalSoundArgs);
        sOmmMainMenu->index = (sOmmMainMenu->index + 1) % OMM_MM_STRINGS_COUNT;
        sOmmMainMenu->timer = 8;
    }
    
    // Next menu (A)
    else if (inputs & A_BUTTON) {
        switch (sOmmMainMenu->index) {

            // Open the file select screen
            case OMM_MM_PLAY:
            case OMM_MM_COPY:
            case OMM_MM_ERASE:
            case OMM_MM_SCORE: {
                play_sound(OMM_MM_SOUND_FILE_SELECT, gGlobalSoundArgs);
                sOmmFileSelect->timer = 30;
                sOmmFileSelect->index = 0;
                sOmmFileSelect->mode = 0;
            } break;

            // Start a new no-save game
            case OMM_MM_PLAY_NO_SAVE: {
                play_sound(OMM_MM_SOUND_PLAY_NO_SAVE, gGlobalSoundArgs);
#if OMM_GAME_IS_SM74
                // Normal Edition
                omm_save_file_erase(NUM_SAVE_FILES, OMM_SM74_MODE_NORMAL);
                omm_select_save_file(NUM_SAVE_FILES, OMM_SM74_MODE_NORMAL, COURSE_NONE, false);
#else
                // Play intro cutscene
                omm_save_file_erase(NUM_SAVE_FILES, OMM_GAME_MODE);
                omm_select_save_file(NUM_SAVE_FILES, OMM_GAME_MODE, COURSE_NONE, false);
#endif
                return NUM_SAVE_FILES + 1;
            } break;

            // Open the options menu
            case OMM_MM_OPTIONS: {
#if OMM_CODE_DYNOS
                gPlayer1Controller->buttonPressed = R_TRIG;
                optmenu_draw_prompt();
#else
                optmenu_toggle();
#endif
                gPlayer1Controller->buttonPressed = 0;
                gPlayer1Controller->rawStickX = 0;
                gPlayer1Controller->rawStickY = 0;
                gPlayer1Controller->stickX = 0;
                gPlayer1Controller->stickY = 0;
                gPlayer1Controller->stickMag = 0;
                gPlayer1Controller->extStickX = 0;
                gPlayer1Controller->extStickY = 0;
            } break;

            // Open the palette editor
            case OMM_MM_PALETTE_EDITOR: {
                omm_palette_editor_open();
            } break;
        }
    }

    // Next menu (Start)
    else if (inputs & START_BUTTON) {
        switch (sOmmMainMenu->index) {

            // Start a new no-save game
            case OMM_MM_PLAY_NO_SAVE: {
                play_sound(OMM_MM_SOUND_PLAY_NO_SAVE, gGlobalSoundArgs);
#if OMM_GAME_IS_SM74
                // Extreme Edition
                omm_save_file_erase(NUM_SAVE_FILES, OMM_SM74_MODE_EXTREME);
                omm_select_save_file(NUM_SAVE_FILES, OMM_SM74_MODE_EXTREME, COURSE_NONE, false);
#else
                // Skip intro cutscene
                omm_save_file_erase(NUM_SAVE_FILES, OMM_GAME_MODE);
                omm_select_save_file(NUM_SAVE_FILES, OMM_GAME_MODE, COURSE_NONE, true);
#endif
                return NUM_SAVE_FILES + 1;
            } break;
        }
    }
    return 0;
}

//
// Main Menu render
//

static void omm_main_menu_render_box(s16 x, s16 y, s16 w, s16 h, u8 r, u8 g, u8 b, u8 a0, u8 a1) {
    Vtx *vtx = (Vtx *) alloc_display_list(sizeof(Vtx) * 4);
    vtx[0] = (Vtx) { { { x,     y,     0 }, 0, { 0, 0 }, { r, g, b, a0 } } };
    vtx[1] = (Vtx) { { { x,     y + h, 0 }, 0, { 0, 0 }, { r, g, b, a0 } } };
    vtx[2] = (Vtx) { { { x + w, y,     0 }, 0, { 0, 0 }, { r, g, b, a1 } } };
    vtx[3] = (Vtx) { { { x + w, y + h, 0 }, 0, { 0, 0 }, { r, g, b, a1 } } };
    omm_render_create_dl_ortho_matrix();
    OMM_RENDER_ENABLE_ALPHA(gDisplayListHead++);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    gSPVertex(gDisplayListHead++, vtx, 4, 0);
    gSP2Triangles(gDisplayListHead++, 2, 1, 0, 0, 1, 2, 3, 0);
}

static void omm_main_menu_render() {

    // Update Mario eyes
    gMarioState->marioBodyState->eyeState = ((OMM_MK_MARIO_COLORS && omm_models_get_mario_model_pack_index() == -1) ? MARIO_EYES_LOOK_UP : MARIO_EYES_OPEN);

    // Game logo
    omm_render_texrect(OMM_MM_LOGO_X, OMM_MM_LOGO_Y, OMM_MM_LOGO_W, OMM_MM_LOGO_H, 1024, 640, 0xFF, 0xFF, 0xFF, 0xFF, OMM_MM_LOGO_TEXTURE, false);

    // Main strings
    omm_main_menu_render_box(OMM_MM_BOX_X, OMM_MM_BOX_Y, OMM_MM_BOX_W, OMM_MM_BOX_H, 0x00, 0xFF, 0xFF, OMM_MM_BOX_A, 0x00);
    omm_render_texrect(OMM_MM_CURSOR_X, OMM_MM_CURSOR_Y, OMM_MM_CURSOR_W, OMM_MM_CURSOR_H, 256, 256, 0xFF, 0xFF, 0xFF, 0xFF, OMM_TEXTURE_MENU_CURSOR, false);
    for (s32 i = 0; i != OMM_MM_STRINGS_COUNT; ++i) {
        const u8 *str = omm_text_convert(sOmmMainMenuStrings[i].label, false);
        omm_render_string(OMM_MM_STRINGS_X, OMM_MM_STRINGS_Y - OMM_MM_STRINGS_H * i, 0xFF, 0xFF, 0xFF, 0xFF, str, true);
    }

    // Info string
    omm_main_menu_render_box(OMM_MM_INFO_BOX_X, OMM_MM_INFO_BOX_Y, OMM_MM_INFO_BOX_W, OMM_MM_INFO_BOX_H, 0x00, 0x00, 0x00, OMM_MM_INFO_BOX_A, 0x00);
    const u8 *info = omm_text_convert(sOmmMainMenuStrings[sOmmMainMenu->index].info[OMM_SPARKLY_IS_PEACH_UNLOCKED], false);
    omm_render_string_sized(OMM_MM_INFO_STRING_X, OMM_MM_INFO_STRING_Y, OMM_MM_INFO_STRING_W, OMM_MM_INFO_STRING_H, OMM_MM_INFO_STRING_C, OMM_MM_INFO_STRING_C, OMM_MM_INFO_STRING_C, 0xFF, info, false);

    // Transition to file select
    if (sOmmFileSelect->timer) {
        omm_render_texrect(
            GFX_DIMENSIONS_FROM_LEFT_EDGE(0), 0, GFX_DIMENSIONS_SCREEN_WIDTH, SCREEN_HEIGHT,
            32, 32, 0xFF, 0xFF, 0xFF, 0xFF * relerp_0_1_f(sOmmFileSelect->timer, 30, 15, 0, 1), OMM_TEXTURE_MISC_WHITE, false
        );
    }

    // Options menu
    else if (optmenu_open) {
        omm_render_create_dl_ortho_matrix();
        omm_render_shade_screen(0xF0);
#if OMM_CODE_DYNOS
        optmenu_draw_prompt();
#endif
        optmenu_check_buttons();
        optmenu_draw();
    }
}

//
// File Select constants
//

#define OMM_FS_BACKGROUND_MARGIN                16
#define OMM_FS_BACKGROUND_BORDER                12

#define OMM_FS_TITLE_H                          24
#define OMM_FS_TITLE_Y                          (SCREEN_HEIGHT - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER + OMM_FS_TITLE_H))

#define OMM_FS_FILE_BUTTON_MARGIN_OUT           16
#define OMM_FS_FILE_BUTTON_MARGIN_IN            8
#define OMM_FS_FILE_BUTTON_WIDTH                48
#define OMM_FS_FILE_BUTTON_HEIGHT               36
#define OMM_FS_FILE_BUTTON_BORDER               4

#define OMM_FS_BOX_X                            (x - OMM_FS_FILE_BUTTON_WIDTH / 2 - 8)
#define OMM_FS_BOX_Y                            (y - OMM_FS_FILE_BUTTON_HEIGHT / 2 - 8 - 8 * !OMM_FS_MODE_IS_SPARKLY_STARS)
#define OMM_FS_BOX_W                            (w - 8)
#define OMM_FS_BOX_H                            (OMM_FS_FILE_BUTTON_HEIGHT + 16 + 8 * !OMM_FS_MODE_IS_SPARKLY_STARS)
#define OMM_FS_BOX_A                            (0x60 + 0x40 * sins(sOmmFileSelect->timer * 0x800))

#define OMM_FS_SOUND_SCROLL                     (SOUND_MENU_CHANGE_SELECT | 0xFE00)
#define OMM_FS_SOUND_RETURN                     (SOUND_MENU_HAND_DISAPPEAR | 0xFF00)
#define OMM_FS_SOUND_CHANGE_MODE                (SOUND_MENU_MARIO_CASTLE_WARP | 0xFF00)
#define OMM_FS_SOUND_PLAY                       (SOUND_MENU_STAR_SOUND_OKEY_DOKEY | 0xFF00)
#define OMM_FS_SOUND_COPY                       (SOUND_MENU_STAR_SOUND | 0xFF00)
#define OMM_FS_SOUND_ERASE                      (SOUND_MARIO_WAAAOOOW | 0xFF00)
#define OMM_FS_SOUND_SCORE_OPEN                 (SOUND_MENU_CAMERA_ZOOM_IN | 0xFF00)
#define OMM_FS_SOUND_SCORE_CLOSE                (SOUND_MENU_CAMERA_ZOOM_OUT | 0xFF00)
#define OMM_FS_SOUND_SCORE_COINS                (SOUND_MENU_CLICK_FILE_SELECT | 0xFE00)
#define OMM_FS_SOUND_INVALID                    (SOUND_MENU_CAMERA_BUZZ | 0xFF00)

#define OMM_FS_MODE_SPARKLY_STARS_AVAILABLE     (sOmmFileSelectStrings[sOmmMainMenu->index][OMM_NUM_SAVE_MODES] != NULL)
#define OMM_FS_MODE_IS_SPARKLY_STARS            (sOmmFileSelect->mode == OMM_NUM_SAVE_MODES)

#define OMM_FS_SCORE_COURSE_FONT_SIZE           5
#define OMM_FS_SCORE_COURSE_FONT_SIZE_RATIO     (OMM_FS_SCORE_COURSE_FONT_SIZE / 8.f)
#define OMM_FS_SCORE_COURSE_DATA_GAP            (12 * (1.f - (sl / 24.f)))
#define OMM_FS_SCORE_COURSE_LINE_Y              (SCREEN_HEIGHT - 64 - 10 * lineIndex)
#define OMM_FS_SCORE_COURSE_LEVEL_X_LEFT        (sl + 10)
#define OMM_FS_SCORE_COURSE_LEVEL_X_RIGHT       (sr - 10 - OMM_FS_SCORE_COURSE_DATA_GAP - maxWidth * OMM_FS_SCORE_COURSE_FONT_SIZE_RATIO - (OMM_FS_SCORE_COURSE_FONT_SIZE + 1) * 7)
#define OMM_FS_SCORE_COURSE_COINS_X_LEFT        (sl + 10 + OMM_FS_SCORE_COURSE_DATA_GAP + maxWidth * OMM_FS_SCORE_COURSE_FONT_SIZE_RATIO + (OMM_FS_SCORE_COURSE_FONT_SIZE + 1))
#define OMM_FS_SCORE_COURSE_COINS_X_RIGHT       (sr - 10 - (OMM_FS_SCORE_COURSE_FONT_SIZE + 1) * 6)
#define OMM_FS_SCORE_COURSE_STARS_X_LEFT        (sl + 10 + OMM_FS_SCORE_COURSE_DATA_GAP + maxWidth * OMM_FS_SCORE_COURSE_FONT_SIZE_RATIO + (OMM_FS_SCORE_COURSE_FONT_SIZE + 1) * k)
#define OMM_FS_SCORE_COURSE_STARS_X_RIGHT       (sr - 10 - (OMM_FS_SCORE_COURSE_FONT_SIZE + 1) * (7 - k))

#define OMM_FS_SCORE_SPARKLY_LINE_Y             (SCREEN_HEIGHT - 52 - 12 * j)
#define OMM_FS_SCORE_SPARKLY_LEVEL_X_LEFT       (sl + 10)
#define OMM_FS_SCORE_SPARKLY_LEVEL_X_RIGHT      (sr - 10 - sparklyLevels[i].maxWidth)

#define OMM_FS_COURSE_TEXTURE(mode, course)     "menu/" OMM_GAME_MENU "/" STRINGIFY(mode) "/" STRINGIFY(course)  ".rgba32"
#define OMM_FS_COURSE_TEXTURES(course)          OMM_FS_COURSE_TEXTURE(0, course), OMM_FS_COURSE_TEXTURE(1, course), OMM_FS_COURSE_TEXTURE(2, course), OMM_FS_COURSE_TEXTURE(3, course)

static const char *sOmmFileSelectCourses[COURSE_MAX][4] = {
    [COURSE_NONE]  = { OMM_FS_COURSE_TEXTURES(COURSE_NONE)  },
    [COURSE_BOB]   = { OMM_FS_COURSE_TEXTURES(COURSE_BOB)   },
    [COURSE_WF]    = { OMM_FS_COURSE_TEXTURES(COURSE_WF)    },
    [COURSE_JRB]   = { OMM_FS_COURSE_TEXTURES(COURSE_JRB)   },
    [COURSE_CCM]   = { OMM_FS_COURSE_TEXTURES(COURSE_CCM)   },
    [COURSE_BBH]   = { OMM_FS_COURSE_TEXTURES(COURSE_BBH)   },
    [COURSE_HMC]   = { OMM_FS_COURSE_TEXTURES(COURSE_HMC)   },
    [COURSE_LLL]   = { OMM_FS_COURSE_TEXTURES(COURSE_LLL)   },
    [COURSE_SSL]   = { OMM_FS_COURSE_TEXTURES(COURSE_SSL)   },
    [COURSE_DDD]   = { OMM_FS_COURSE_TEXTURES(COURSE_DDD)   },
    [COURSE_SL]    = { OMM_FS_COURSE_TEXTURES(COURSE_SL)    },
    [COURSE_WDW]   = { OMM_FS_COURSE_TEXTURES(COURSE_WDW)   },
    [COURSE_TTM]   = { OMM_FS_COURSE_TEXTURES(COURSE_TTM)   },
    [COURSE_THI]   = { OMM_FS_COURSE_TEXTURES(COURSE_THI)   },
    [COURSE_TTC]   = { OMM_FS_COURSE_TEXTURES(COURSE_TTC)   },
    [COURSE_RR]    = { OMM_FS_COURSE_TEXTURES(COURSE_RR)    },
    [COURSE_BITDW] = { OMM_FS_COURSE_TEXTURES(COURSE_BITDW) },
    [COURSE_BITFS] = { OMM_FS_COURSE_TEXTURES(COURSE_BITFS) },
    [COURSE_BITS]  = { OMM_FS_COURSE_TEXTURES(COURSE_BITS)  },
    [COURSE_PSS]   = { OMM_FS_COURSE_TEXTURES(COURSE_PSS)   },
    [COURSE_COTMC] = { OMM_FS_COURSE_TEXTURES(COURSE_COTMC) },
    [COURSE_TOTWC] = { OMM_FS_COURSE_TEXTURES(COURSE_TOTWC) },
    [COURSE_VCUTM] = { OMM_FS_COURSE_TEXTURES(COURSE_VCUTM) },
    [COURSE_WMOTR] = { OMM_FS_COURSE_TEXTURES(COURSE_WMOTR) },
    [COURSE_SA]    = { OMM_FS_COURSE_TEXTURES(COURSE_SA)    },
};

static const char *sOmmFileSelectStrings[][4] = {
    [OMM_MM_PLAY]  = { OMM_TEXT_FS_PLAY  },
    [OMM_MM_COPY]  = { OMM_TEXT_FS_COPY  },
    [OMM_MM_ERASE] = { OMM_TEXT_FS_ERASE },
    [OMM_MM_SCORE] = { OMM_TEXT_FS_SCORE },
};

static const char *sOmmFileSelectBackgrounds[][2] = {
    [OMM_MM_PLAY]  = { OMM_ASSET_MENU_SELECT_BUTTON, NULL                                      },
    [OMM_MM_COPY]  = { OMM_ASSET_MENU_COPY_BUTTON,   NULL                                      },
    [OMM_MM_ERASE] = { OMM_ASSET_MENU_ERASE_BUTTON,  OMM_ASSET_MENU_ERASE_BUTTON               },
    [OMM_MM_SCORE] = { OMM_ASSET_MENU_SCORE_BUTTON,  OMM_ASSET_MENU_SCORE_SPARKLY_STARS_BUTTON },
};

static const u8 sOmmFileSelectButtonColors[NUM_SAVE_FILES][3][3] = {
    { { 0xFF, 0xFF, 0xFF }, { 0x80, 0x80, 0x80 }, { 0xFF, 0xE0, 0x80 } },
    { { 0xFF, 0xFF, 0xFF }, { 0x80, 0x80, 0x80 }, { 0xC0, 0xE0, 0xFF } },
    { { 0xFF, 0xFF, 0xFF }, { 0x80, 0x80, 0x80 }, { 0xFF, 0x40, 0x80 } },
    { { 0xFF, 0xFF, 0xFF }, { 0x80, 0x80, 0x80 }, { 0xFF, 0xFF, 0xFF } },
};

static const char *sOmmFileSelectFiles[NUM_SAVE_FILES][3] = {
    { OMM_TEXT_FS_MARIO_A, OMM_TEXT_LEVEL_UNKNOWN, OMM_TEXT_SPARKLY_1 },
    { OMM_TEXT_FS_MARIO_B, OMM_TEXT_LEVEL_UNKNOWN, OMM_TEXT_SPARKLY_2 },
    { OMM_TEXT_FS_MARIO_C, OMM_TEXT_LEVEL_UNKNOWN, OMM_TEXT_SPARKLY_3 },
    { OMM_TEXT_FS_MARIO_D, OMM_TEXT_LEVEL_UNKNOWN, OMM_TEXT_LEVEL_UNKNOWN },
};

static const u32 sOmmFileSelectScoreFlags[2][16] = { {
    SAVE_FLAG_HAVE_WING_CAP, 0xFF, 0x00, 0x00,
    SAVE_FLAG_HAVE_METAL_CAP, 0x00, 0xD0, 0x00,
    SAVE_FLAG_HAVE_VANISH_CAP, 0x00, 0x80, 0xFF,
0 }, {
    SAVE_FLAG_UNLOCKED_BASEMENT_DOOR | SAVE_FLAG_HAVE_KEY_1, 0xFF, 0xFF, 0x00,
    SAVE_FLAG_UNLOCKED_UPSTAIRS_DOOR | SAVE_FLAG_HAVE_KEY_2, 0xFF, 0xFF, 0x00,
0 } };

static const char *sOmmFileSelectScoreSparklyTimerGlyphs[] = {
    OMM_TEXTURE_HUD_0,
    OMM_TEXTURE_HUD_1,
    OMM_TEXTURE_HUD_2,
    OMM_TEXTURE_HUD_3,
    OMM_TEXTURE_HUD_4,
    OMM_TEXTURE_HUD_5,
    OMM_TEXTURE_HUD_6,
    OMM_TEXTURE_HUD_7,
    OMM_TEXTURE_HUD_8,
    OMM_TEXTURE_HUD_9,
    OMM_TEXTURE_HUD_DOTS,
};

//
// File Select update
//

static s32 omm_file_select_update() {
    if (sOmmFileSelect->timer++ < 15) return 0;

    // Update inputs
    u32 inputs = omm_get_inputs();

    // Score board
    if (sOmmFileScore->open) {
        sOmmFileScore->timer++;
        sOmmFileScore->open = (sOmmFileScore->timer != 0);
        if (sOmmFileScore->timer < 10) return 0;

        // Switch between stars/coins score
        if (!OMM_FS_MODE_IS_SPARKLY_STARS && (inputs & A_BUTTON)) {
            play_sound(OMM_FS_SOUND_SCORE_COINS, gGlobalSoundArgs);
            sOmmFileScore->coins = !sOmmFileScore->coins;
        }

        // Close score board
        else if (inputs & (A_BUTTON | B_BUTTON)) {
            play_sound(OMM_FS_SOUND_SCORE_CLOSE, gGlobalSoundArgs);
            sOmmFileScore->timer = -10;
        }
    }

    // Next save file (vertically)
    else if (inputs & (STICK_UP | STICK_DOWN)) {
        play_sound(OMM_FS_SOUND_SCROLL, gGlobalSoundArgs);
        sOmmFileSelect->index = (sOmmFileSelect->index + 2) % 4;
        sOmmFileSelect->timer = 40;
    }
    
    // Next save file (horizontally)
    else if (inputs & (STICK_LEFT | STICK_RIGHT)) {
        play_sound(OMM_FS_SOUND_SCROLL, gGlobalSoundArgs);
        sOmmFileSelect->index = ((sOmmFileSelect->index + 1) % 2) + 2 * (sOmmFileSelect->index / 2);
        sOmmFileSelect->timer = 40;
    }
    
    // Change mode
    else if (inputs & L_TRIG) {
        s32 prevMode = sOmmFileSelect->mode;
        sOmmFileSelect->mode = (sOmmFileSelect->mode + 1) % (OMM_NUM_SAVE_MODES + (OMM_FS_MODE_SPARKLY_STARS_AVAILABLE && OMM_SPARKLY_IS_GAMEMODE_UNLOCKED));
        if (prevMode != sOmmFileSelect->mode) play_sound(OMM_FS_SOUND_CHANGE_MODE, gGlobalSoundArgs);
    }
    
    // Return to main menu or cancel copy
    else if (inputs & B_BUTTON) {
        play_sound(OMM_FS_SOUND_RETURN, gGlobalSoundArgs);
        if (sOmmFileCopy->open) {
            sOmmFileCopy->open = false;
        } else {
            return -1;
        }
    }
    
    // Advance (A)
    else if (inputs & A_BUTTON) {
        switch (sOmmMainMenu->index) {

            // Start a new save or resume
            case OMM_MM_PLAY: {
                play_sound(OMM_FS_SOUND_PLAY, gGlobalSoundArgs);
                omm_check_complete_save(sOmmFileSelect->index, sOmmFileSelect->mode);
                omm_select_save_file(sOmmFileSelect->index, sOmmFileSelect->mode, COURSE_NONE, false);
                return sOmmFileSelect->index + 1;
            } break;

            // Copy an existing file
            case OMM_MM_COPY: {
                
                // Enter copy mode
                if (!sOmmFileCopy->open && omm_save_file_exists(sOmmFileSelect->index, sOmmFileSelect->mode)) {
                    play_sound(OMM_FS_SOUND_SCROLL, gGlobalSoundArgs);
                    sOmmFileCopy->index = sOmmFileSelect->index;
                    sOmmFileCopy->open = true;
                }
                
                // Do the file copy
                else if (sOmmFileCopy->open && sOmmFileCopy->index != sOmmFileSelect->index && !omm_save_file_exists(sOmmFileSelect->index, sOmmFileSelect->mode)) {
                    play_sound(OMM_FS_SOUND_COPY, gGlobalSoundArgs);
                    omm_save_file_copy(sOmmFileCopy->index, sOmmFileSelect->mode, sOmmFileSelect->index);
                    sOmmFileCopy->open = false;
                }
                
                // Invalid
                else {
                    play_sound(OMM_FS_SOUND_INVALID, gGlobalSoundArgs);
                }
            } break;

            // Erase an existing file
            case OMM_MM_ERASE: {

                // Erase a save file
                if (!OMM_FS_MODE_IS_SPARKLY_STARS && omm_save_file_exists(sOmmFileSelect->index, sOmmFileSelect->mode)) {
                    play_sound(OMM_FS_SOUND_ERASE, gGlobalSoundArgs);
                    omm_save_file_erase(sOmmFileSelect->index, sOmmFileSelect->mode);
                }
                
                // Clear Sparkly stars mode
                else if (OMM_FS_MODE_IS_SPARKLY_STARS && omm_sparkly_is_timer_started(sOmmFileSelect->index + 1)) {
                    play_sound(OMM_FS_SOUND_ERASE, gGlobalSoundArgs);
                    omm_sparkly_clear_mode(sOmmFileSelect->index + 1);
                }
                
                // Invalid
                else {
                    play_sound(OMM_FS_SOUND_INVALID, gGlobalSoundArgs);
                }
            } break;

            // Open score board
            case OMM_MM_SCORE: {
                
                // Save file or Sparkly stars
                if ((!OMM_FS_MODE_IS_SPARKLY_STARS && omm_save_file_exists(sOmmFileSelect->index, sOmmFileSelect->mode)) ||
                    ( OMM_FS_MODE_IS_SPARKLY_STARS && omm_sparkly_is_timer_started(sOmmFileSelect->index + 1))) {
                    play_sound(OMM_FS_SOUND_SCORE_OPEN, gGlobalSoundArgs);
                    sOmmFileScore->open = true;
                    sOmmFileScore->timer = 0;
                    sOmmFileScore->coins = false;
                }

                // Invalid
                else {
                    play_sound(OMM_FS_SOUND_INVALID, gGlobalSoundArgs);
                }
            } break;
        }
    }

    // Advance (Start)
    else if (inputs & START_BUTTON) {
        switch (sOmmMainMenu->index) {

            // Start a new save but skip intro or resume to last course
            case OMM_MM_PLAY: {
                play_sound(OMM_FS_SOUND_PLAY, gGlobalSoundArgs);
                omm_check_complete_save(sOmmFileSelect->index, sOmmFileSelect->mode);
                omm_select_save_file(sOmmFileSelect->index, sOmmFileSelect->mode, omm_save_file_get_last_course(sOmmFileSelect->index, sOmmFileSelect->mode), true);
                return sOmmFileSelect->index + 1;
            } break;
        }
    }
    return 0;
}

//
// File Select render
//

static void omm_file_select_render_background(const void *texture, u8 intensity) {
    static Vtx vtx[4];
    vtx[0] = (Vtx) { { { GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0),              0, 0 }, 0, { 0x000, 0x400 }, { intensity, intensity, intensity, 0xFF } } };
    vtx[1] = (Vtx) { { { GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0),             0, 0 }, 0, { 0x400, 0x400 }, { intensity, intensity, intensity, 0xFF } } };
    vtx[2] = (Vtx) { { { GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), SCREEN_HEIGHT, 0 }, 0, { 0x400, 0x000 }, { intensity, intensity, intensity, 0xFF } } };
    vtx[3] = (Vtx) { { { GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0),  SCREEN_HEIGHT, 0 }, 0, { 0x000, 0x000 }, { intensity, intensity, intensity, 0xFF } } };

    omm_render_create_dl_ortho_matrix();
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0, 0, 0, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gDisplayListHead++, vtx, 4, 0);
    gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 0, 2, 3, 0);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gSPSetGeometryMode(gDisplayListHead++, G_LIGHTING);

    s32 SW = (s32) (GFX_DIMENSIONS_SCREEN_WIDTH);
    s32 X0 = (s32) (GFX_DIMENSIONS_FROM_LEFT_EDGE(0));
    omm_render_texrect(X0 + OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER, SCREEN_HEIGHT - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), SW - (OMM_FS_BACKGROUND_MARGIN * 2 + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_BORDER, 32, 32, 0x00, 0x00, 0x00, 0xA0, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(X0 + SW - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER, OMM_FS_BACKGROUND_BORDER, SCREEN_HEIGHT - 2 * (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), 32, 32, 0x00, 0x00, 0x00, 0xA0, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(X0 + OMM_FS_BACKGROUND_MARGIN, OMM_FS_BACKGROUND_MARGIN, SW - (OMM_FS_BACKGROUND_MARGIN * 2 + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_BORDER, 32, 32, 0xFF, 0xFF, 0xFF, 0x60, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(X0 + OMM_FS_BACKGROUND_MARGIN, OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER, OMM_FS_BACKGROUND_BORDER, SCREEN_HEIGHT - 2 * (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), 32, 32, 0xFF, 0xFF, 0xFF, 0x60, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(X0 + OMM_FS_BACKGROUND_MARGIN, SCREEN_HEIGHT - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_BORDER, OMM_FS_BACKGROUND_BORDER, 32, 32, 0x00, 0x00, 0x00, 0xA0, OMM_TEXTURE_MISC_WHITE_UP_RIGHT, false);
    omm_render_texrect(X0 + OMM_FS_BACKGROUND_MARGIN, SCREEN_HEIGHT - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_BORDER, OMM_FS_BACKGROUND_BORDER, 32, 32, 0xFF, 0xFF, 0xFF, 0x60, OMM_TEXTURE_MISC_WHITE_DOWN_LEFT, false);
    omm_render_texrect(X0 + SW - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_MARGIN, OMM_FS_BACKGROUND_BORDER, OMM_FS_BACKGROUND_BORDER, 32, 32, 0x00, 0x00, 0x00, 0xA0, OMM_TEXTURE_MISC_WHITE_UP_RIGHT, false);
    omm_render_texrect(X0 + SW - (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER), OMM_FS_BACKGROUND_MARGIN, OMM_FS_BACKGROUND_BORDER, OMM_FS_BACKGROUND_BORDER, 32, 32, 0xFF, 0xFF, 0xFF, 0x60, OMM_TEXTURE_MISC_WHITE_DOWN_LEFT, false);
}

static void omm_file_select_render_button(s16 x, s16 y, u8 r, u8 g, u8 b) {
    s16 x0 = x - OMM_FS_FILE_BUTTON_WIDTH / 2;
    s16 y0 = y - OMM_FS_FILE_BUTTON_HEIGHT / 2;
    s16 x1 = x + OMM_FS_FILE_BUTTON_WIDTH / 2;
    s16 y1 = y + OMM_FS_FILE_BUTTON_HEIGHT / 2;

    Vtx *vtx = alloc_display_list(sizeof(Vtx) * 4);
    vtx[0] = (Vtx) { { { x0, y0, 0 }, 0, { 0x000, 0x400 }, { r, g, b, 0xFF } } };
    vtx[1] = (Vtx) { { { x1, y0, 0 }, 0, { 0x400, 0x400 }, { r, g, b, 0xFF } } };
    vtx[2] = (Vtx) { { { x1, y1, 0 }, 0, { 0x400, 0x000 }, { r, g, b, 0xFF } } };
    vtx[3] = (Vtx) { { { x0, y1, 0 }, 0, { 0x000, 0x000 }, { r, g, b, 0xFF } } };

    omm_render_create_dl_ortho_matrix();
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(gDisplayListHead++, OMM_ASSET_MENU_FILE, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0, 0, 0, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gDisplayListHead++, vtx, 4, 0);
    gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 0, 2, 3, 0);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gSPSetGeometryMode(gDisplayListHead++, G_LIGHTING);

    omm_render_texrect(x0 + OMM_FS_FILE_BUTTON_BORDER, y1 - OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_WIDTH - OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, 32, 32, 0xFF, 0xFF, 0xFF, 0x80, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(x1 - OMM_FS_FILE_BUTTON_BORDER, y0 + OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_HEIGHT - OMM_FS_FILE_BUTTON_BORDER * 2, 32, 32, 0xFF, 0xFF, 0xFF, 0x80, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(x0, y0, OMM_FS_FILE_BUTTON_WIDTH - OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, 32, 32, 0x00, 0x00, 0x00, 0x80, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(x0, y0 + OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_HEIGHT - OMM_FS_FILE_BUTTON_BORDER * 2, 32, 32, 0x00, 0x00, 0x00, 0x80, OMM_TEXTURE_MISC_WHITE, false);
    omm_render_texrect(x0, y1 - OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, 32, 32, 0xFF, 0xFF, 0xFF, 0x80, OMM_TEXTURE_MISC_WHITE_UP_RIGHT, false);
    omm_render_texrect(x0, y1 - OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, 32, 32, 0x00, 0x00, 0x00, 0x80, OMM_TEXTURE_MISC_WHITE_DOWN_LEFT, false);
    omm_render_texrect(x1 - OMM_FS_FILE_BUTTON_BORDER, y0, OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, 32, 32, 0xFF, 0xFF, 0xFF, 0x80, OMM_TEXTURE_MISC_WHITE_UP_RIGHT, false);
    omm_render_texrect(x1 - OMM_FS_FILE_BUTTON_BORDER, y0, OMM_FS_FILE_BUTTON_BORDER, OMM_FS_FILE_BUTTON_BORDER, 32, 32, 0x00, 0x00, 0x00, 0x80, OMM_TEXTURE_MISC_WHITE_DOWN_LEFT, false);
}

static void omm_file_select_render_icon(s16 x, s16 y, s16 w, s16 h, s16 texw, s16 texh, u8 r, u8 g, u8 b, u8 a, const void *texture) {
    Vtx *vtx = alloc_display_list(sizeof(Vtx) * 4);
    vtx[0] = (Vtx) { { { x - w / 2, y - h / 2, 0 }, 0, {               0, (texh - 1) << 5 }, { r, g, b, a } } };
    vtx[1] = (Vtx) { { { x + w / 2, y - h / 2, 0 }, 0, { (texw - 1) << 5, (texh - 1) << 5 }, { r, g, b, a } } };
    vtx[2] = (Vtx) { { { x + w / 2, y + h / 2, 0 }, 0, { (texw - 1) << 5,               0 }, { r, g, b, a } } };
    vtx[3] = (Vtx) { { { x - w / 2, y + h / 2, 0 }, 0, {               0,               0 }, { r, g, b, a } } };

    omm_render_create_dl_ortho_matrix();
    OMM_RENDER_ENABLE_ALPHA(gDisplayListHead++);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, texw, texh, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gDisplayListHead++, vtx, 4, 0);
    gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 0, 2, 3, 0);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gSPSetGeometryMode(gDisplayListHead++, G_LIGHTING);
}

static void omm_file_select_render_course_data(f32 scale, s32 sl, s32 sr, s32 x0, s32 y0, u8 alpha, const u8 *courseName, s32 courseNum, s32 lineIndex, s32 maxWidth, bool leftAlign) {
    s32 levelKnown = (courseNum == COURSE_NONE || (omm_stars_get_bits_total(omm_level_from_course(courseNum), sOmmFileSelect->mode) != 0 && omm_save_file_get_star_flags(sOmmFileSelect->index, sOmmFileSelect->mode, courseNum - 1) != 0));
    const u8 *levelStr = (levelKnown ? courseName : omm_text_convert(OMM_TEXT_LEVEL_UNKNOWN, false));
    s32 levelX = lerp_f(scale, x0, (leftAlign ? OMM_FS_SCORE_COURSE_LEVEL_X_LEFT : OMM_FS_SCORE_COURSE_LEVEL_X_RIGHT));
    s32 levelY = lerp_f(scale, y0, OMM_FS_SCORE_COURSE_LINE_Y);
    s32 levelW = lerp_f(scale,  0, OMM_FS_SCORE_COURSE_FONT_SIZE);
    s32 levelC = (levelKnown ? 0xFF : 0x80);
    omm_render_string_sized(levelX, levelY, levelW, levelW, levelC, levelC, levelC, alpha, levelStr, false);

    if (sOmmFileScore->coins && courseNum != COURSE_NONE) {
        s32 coins = omm_save_file_get_course_coin_score(sOmmFileSelect->index, sOmmFileSelect->mode, courseNum - 1);
        u8 coinsStr[] = { 0xF9, 0xFB, DIALOG_CHAR_SPACE, (coins >= 100 ? (coins / 100) % 10 : 0xE0), (coins >= 10 ? (coins / 10) % 10 : 0xE0), coins % 10, 0xFF };
        s32 coinsX = lerp_f(scale, x0, (leftAlign ? OMM_FS_SCORE_COURSE_COINS_X_LEFT : OMM_FS_SCORE_COURSE_COINS_X_RIGHT));
        s32 coinsY = lerp_f(scale, y0, OMM_FS_SCORE_COURSE_LINE_Y);
        s32 coinsW = lerp_f(scale,  0, OMM_FS_SCORE_COURSE_FONT_SIZE);
        omm_render_string_sized(coinsX, coinsY, coinsW, coinsW, 0xFF, 0xFF, 0xFF, alpha, coinsStr, false);
    } else {
        u8 starBits = omm_stars_get_bits_total(omm_level_from_course(courseNum), sOmmFileSelect->mode);
        u8 starFlags = omm_save_file_get_star_flags(sOmmFileSelect->index, sOmmFileSelect->mode, courseNum - 1);
        u8 starGlyph[] = { DIALOG_CHAR_STAR_FILLED, 0xFF };
        for (s32 j = 0, k = 0; j != OMM_NUM_STARS_MAX_PER_COURSE; ++j) {
            if (starBits & (1 << j)) {
                s32 starX = lerp_f(scale, x0, (leftAlign ? OMM_FS_SCORE_COURSE_STARS_X_LEFT : OMM_FS_SCORE_COURSE_STARS_X_RIGHT));
                s32 starY = lerp_f(scale, y0, OMM_FS_SCORE_COURSE_LINE_Y);
                s32 starW = lerp_f(scale,  0, OMM_FS_SCORE_COURSE_FONT_SIZE);
                s32 starC = (starFlags & (1 << j) ? 0xFF : 0x40);
                omm_render_string_sized(starX, starY, starW, starW, starC, starC, starC, alpha, starGlyph, false);
                k++;
            }
        }
    }
}

static void omm_file_select_render_flags_data(f32 scale, s32 sl, s32 sr, s32 x0, s32 y0, u8 alpha, const u8 *courseName, s32 lineIndex, s32 maxWidth, const u32 *flags) {
    s32 levelX = lerp_f(scale, x0, OMM_FS_SCORE_COURSE_LEVEL_X_RIGHT);
    s32 levelY = lerp_f(scale, y0, OMM_FS_SCORE_COURSE_LINE_Y);
    s32 levelW = lerp_f(scale,  0, OMM_FS_SCORE_COURSE_FONT_SIZE);
    omm_render_string_sized(levelX, levelY, levelW, levelW, 0xFF, 0xFF, 0xFF, alpha, courseName, false);
    u8 flagGlyph[] = { DIALOG_CHAR_STAR_FILLED, 0xFF };
    for (s32 i = 0, k = 0; flags[i]; i += 4, ++k) {
        s32 hasFlag = (omm_save_file_get_flags(sOmmFileSelect->index, sOmmFileSelect->mode) & flags[i]) != 0;
        s32 flagX = lerp_f(scale, x0, OMM_FS_SCORE_COURSE_STARS_X_RIGHT);
        s32 flagY = lerp_f(scale, y0, OMM_FS_SCORE_COURSE_LINE_Y);
        s32 flagW = lerp_f(scale,  0, OMM_FS_SCORE_COURSE_FONT_SIZE);
        s32 flagR = (hasFlag ? flags[i + 1] : 0x40);
        s32 flagG = (hasFlag ? flags[i + 2] : 0x40);
        s32 flagB = (hasFlag ? flags[i + 3] : 0x40);
        omm_render_string_sized(flagX, flagY, flagW, flagW, flagR, flagG, flagB, alpha, flagGlyph, false);
    }
}

static void omm_file_select_render_score_board(f32 scale) {
    s32 sl = min_s(0, GFX_DIMENSIONS_FROM_LEFT_EDGE(0) / 4);
    s32 sr = SCREEN_WIDTH + max_s(0, (GFX_DIMENSIONS_FROM_RIGHT_EDGE(0) - SCREEN_WIDTH) / 4);
    s32 x0 = SCREEN_WIDTH / 2;
    s32 y0 = SCREEN_HEIGHT / 2;
    s32 alpha = 0xFF * scale;

    // Background
    s32 bgw = GFX_DIMENSIONS_SCREEN_WIDTH * scale + 2;
    s32 bgh = SCREEN_HEIGHT * scale;
    s32 bgx = (SCREEN_WIDTH - bgw) / 2 - 1;
    s32 bgy = (SCREEN_HEIGHT - bgh) / 2;
    omm_render_texrect(bgx, bgy, bgw, bgh, 32, 32, 0x00, 0x00, 0x00, 0xF0, OMM_TEXTURE_MISC_WHITE, false);

    // Sparkly stars
    if (OMM_FS_MODE_IS_SPARKLY_STARS) {
        s32 sparklyMode = sOmmFileSelect->index + 1;
        s32 allStars = (omm_sparkly_get_collected_count(sparklyMode) == omm_sparkly_get_bowser_4_index(sparklyMode) + 1);
        s32 timer = omm_sparkly_get_timer(sparklyMode);

        // Collected count
        s32 starX  = lerp_f(scale, x0, sl + 10);
        s32 starY  = lerp_f(scale, y0, SCREEN_HEIGHT - 36);
        s32 starW  = lerp_f(scale,  0, 16);
        s32 countX = lerp_f(scale, x0, sl + 30);
        s32 countY = lerp_f(scale, y0, SCREEN_HEIGHT - 36);
        s32 countW = lerp_f(scale,  0, 16);
        s32 countS = lerp_f(scale,  0, 12);
        omm_render_glyph(starX, starY, starW, starW, 0xFF, 0xFF, 0xFF, alpha, OMM_SPARKLY_HUD_GLYPH[sparklyMode], false);
        omm_render_number(countX, countY, countW, countW, countS, alpha, omm_sparkly_get_collected_count(sparklyMode), 2, true, false);

        // Elapsed time
        s32 timerGlyphs[] = {
            ((timer / 1080000) % 10), ((timer / 108000) % 10), 10,
            ((timer /   18000) %  6), ((timer /   1800) % 10), 10,
            ((timer /     300) %  6), ((timer /     30) % 10),
        };
        for (s32 i = 0; i != 8; ++i) {
            s32 glyphW = lerp_f(scale,  1, 14);
            s32 glyphX = lerp_f(scale, x0, sr - (20 + (7 - i) * OMM_RENDER_SPARKLY_TIMER_OFFSET_X(14)));
            s32 glyphY = lerp_f(scale, y0, SCREEN_HEIGHT - 36);
            omm_render_glyph(glyphX, glyphY, glyphW, glyphW, 0xFF, 0xFF, 0xFF, alpha, sOmmFileSelectScoreSparklyTimerGlyphs[timerGlyphs[i]], false);
        }

        // List of stars
        // Needs some preprocessing: compute the length of each column to perfectly balance the display
        struct { struct { u8 name[0x100]; s32 width; s32 index; } levels[15]; s32 count; s32 maxWidth; } sparklyLevels[2] = { 0 };
        for (s32 i = 0, n = omm_sparkly_get_bowser_4_index(sparklyMode); i <= n; ++i) {
            const u8 *levelName = omm_sparkly_get_level_name(sparklyMode, i);
            if (levelName) {
                s32 courseNum = omm_level_get_course(gOmmSparklyData[sparklyMode][i].levelNum);
                s32 mainCourse = COURSE_IS_MAIN_COURSE(courseNum);
                s32 nameLength = omm_text_length(levelName);
                s32 nameWidth = omm_render_get_string_width(levelName);
                mem_cpy(sparklyLevels[mainCourse].levels[sparklyLevels[mainCourse].count].name, levelName, nameLength + 1);
                sparklyLevels[mainCourse].levels[sparklyLevels[mainCourse].count].width = nameWidth;
                sparklyLevels[mainCourse].levels[sparklyLevels[mainCourse].count].index = i;
                sparklyLevels[mainCourse].maxWidth = max_s(sparklyLevels[mainCourse].maxWidth, nameWidth);
                sparklyLevels[mainCourse].count++;
            }
        }
        u8 r = OMM_SPARKLY_HUD_COLOR[sparklyMode][0];
        u8 g = OMM_SPARKLY_HUD_COLOR[sparklyMode][1];
        u8 b = OMM_SPARKLY_HUD_COLOR[sparklyMode][2];
        for (s32 i = 0; i != 2; ++i)
        for (s32 j = 0; j != sparklyLevels[i].count; ++j) {
            s32 levelIndex = sparklyLevels[i].levels[j].index;
            s32 levelLast = (levelIndex == omm_sparkly_get_bowser_4_index(sparklyMode));
            const u8 *levelStr = (levelLast && !omm_sparkly_is_bowser_4_unlocked(sparklyMode) ? omm_text_convert(OMM_TEXT_LEVEL_UNKNOWN, false) : sparklyLevels[i].levels[j].name);
            s32 levelX = lerp_f(scale, x0, (i ? OMM_FS_SCORE_SPARKLY_LEVEL_X_LEFT : OMM_FS_SCORE_SPARKLY_LEVEL_X_RIGHT));
            s32 levelY = lerp_f(scale, y0, OMM_FS_SCORE_SPARKLY_LINE_Y);
            s32 levelW = lerp_f(scale,  0, 8);
            s32 levelD = ((levelLast ? allStars : omm_sparkly_is_star_collected(sparklyMode, levelIndex)) ? 1 : 3);
            omm_render_string_sized(levelX, levelY, levelW, levelW, r / levelD, g / levelD, b / levelD, alpha, levelStr, false);
        }
    }

    // Save file
    else {
        mem_new1(const u8 *, OMM_TEXT_LEVEL_CASTLE_STR, omm_text_convert(OMM_TEXT_LEVEL_CASTLE, true));
        mem_new1(const u8 *, OMM_TEXT_FS_CAPS_STR, omm_text_convert(OMM_TEXT_FS_CAPS, true));
        mem_new1(const u8 *, OMM_TEXT_FS_KEYS_STR, omm_text_convert(OMM_TEXT_FS_KEYS, true));
        struct { struct { const u8 *name; s32 width; s32 index; } levels[15]; s32 count; s32 maxWidth; } levels[2] = { { {
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_BOB   - 1] + 3, 0, COURSE_BOB   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_WF    - 1] + 3, 0, COURSE_WF    },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_JRB   - 1] + 3, 0, COURSE_JRB   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_CCM   - 1] + 3, 0, COURSE_CCM   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_BBH   - 1] + 3, 0, COURSE_BBH   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_HMC   - 1] + 3, 0, COURSE_HMC   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_LLL   - 1] + 3, 0, COURSE_LLL   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_SSL   - 1] + 3, 0, COURSE_SSL   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_DDD   - 1] + 3, 0, COURSE_DDD   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_SL    - 1] + 3, 0, COURSE_SL    },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_WDW   - 1] + 3, 0, COURSE_WDW   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_TTM   - 1] + 3, 0, COURSE_TTM   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_THI   - 1] + 3, 0, COURSE_THI   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_TTC   - 1] + 3, 0, COURSE_TTC   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_RR    - 1] + 3, 0, COURSE_RR    },
        }, 15, 0 }, { {
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_BITDW - 1] + 3, 0, COURSE_BITDW },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_BITFS - 1] + 3, 0, COURSE_BITFS },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_BITS  - 1] + 3, 0, COURSE_BITS  },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_TOTWC - 1] + 3, 0, COURSE_TOTWC },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_COTMC - 1] + 3, 0, COURSE_COTMC },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_VCUTM - 1] + 3, 0, COURSE_VCUTM },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_PSS   - 1] + 3, 0, COURSE_PSS   },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_SA    - 1] + 3, 0, COURSE_SA    },
            { gCourseNameTable(sOmmFileSelect->mode)[COURSE_WMOTR - 1] + 3, 0, COURSE_WMOTR },
            { OMM_TEXT_LEVEL_CASTLE_STR,                                    0, COURSE_NONE  },
            { OMM_TEXT_FS_CAPS_STR,                                         0, COURSE_NONE  },
            { OMM_TEXT_FS_KEYS_STR,                                         0, COURSE_NONE  },
        }, 12, 0 } };

        // Compute width and maxWidth
        for (s32 i = 0; i != 2; ++i)
        for (s32 j = 0; j != levels[i].count; ++j) {
            levels[i].levels[j].width = omm_render_get_string_width(levels[i].levels[j].name);
            levels[i].maxWidth = max_s(levels[i].maxWidth, levels[i].levels[j].width);
        }

        // Collected stars
        s32 numStars  = omm_save_file_get_total_star_count(sOmmFileSelect->index, sOmmFileSelect->mode, COURSE_MIN - 1, COURSE_MAX - 1);
        s32 starIconX = lerp_f(scale, x0, sl + 10);
        s32 starIconY = lerp_f(scale, y0, SCREEN_HEIGHT - 48);
        s32 starIconW = lerp_f(scale,  0, 16);
        s32 numStarsX = lerp_f(scale, x0, sl + 30);
        s32 numStarsY = lerp_f(scale, y0, SCREEN_HEIGHT - 48);
        s32 numStarsW = lerp_f(scale,  0, 16);
        s32 numStarsS = lerp_f(scale,  0, 12);
        omm_render_glyph(starIconX, starIconY, starIconW, starIconW, 0xFF, 0xFF, 0xFF, alpha, omm_render_get_star_glyph(0, OMM_EXTRAS_COLORED_STARS), false);
        omm_render_number(numStarsX, numStarsY, numStarsW, numStarsW, numStarsS, alpha, numStars, 3, true, false);

        // Collected coins
        s32 numCoins  = omm_save_file_get_total_coin_score(sOmmFileSelect->index, sOmmFileSelect->mode, COURSE_MIN - 1, COURSE_MAX - 1);
        s32 numCoinsS = lerp_f(scale,  0, 12);
        s32 coinIconX = lerp_f(scale, x0, sr - (42 + 4 * numCoinsS));
        s32 coinIconY = lerp_f(scale, y0, SCREEN_HEIGHT - 48);
        s32 coinIconW = lerp_f(scale,  0, 16);
        s32 numCoinsX = lerp_f(scale, x0, sr - (22 + 4 * numCoinsS));
        s32 numCoinsY = lerp_f(scale, y0, SCREEN_HEIGHT - 48);
        s32 numCoinsW = lerp_f(scale,  0, 16);
        omm_render_glyph(coinIconX, coinIconY, coinIconW, coinIconW, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_COIN, false);
        omm_render_number(numCoinsX, numCoinsY, numCoinsW, numCoinsW, numCoinsS, alpha, numCoins, 5, true, false);

        // Main courses
        for (s32 i = 0; i != 15; ++i) {
            omm_file_select_render_course_data(
                scale, sl, sr, x0, y0, alpha,
                levels[0].levels[i].name,
                levels[0].levels[i].index, i,
                levels[0].maxWidth, true
            );
        }

        // Bowser, Cap, Bonus levels and Castle
        for (s32 i = 0; i != 10; ++i) {
            omm_file_select_render_course_data(
                scale, sl, sr, x0, y0, alpha,
                levels[1].levels[i].name,
                levels[1].levels[i].index, i + (i / 3),
                levels[1].maxWidth, false
            );
        }

        // Flags (caps and Bowser keys)
        for (s32 i = 10; i != 12; ++i) {
            omm_file_select_render_flags_data(
                scale, sl, sr, x0, y0, alpha,
                levels[1].levels[i].name, i + 3,
                levels[1].maxWidth,
                sOmmFileSelectScoreFlags[i - 10]
            );
        }
    }
}

static void omm_file_select_render() {
    s32 SW = SCREEN_WIDTH + max_s(0, (GFX_DIMENSIONS_SCREEN_WIDTH - SCREEN_WIDTH) / 3);

    // Background
    omm_file_select_render_background(sOmmFileSelectBackgrounds[sOmmMainMenu->index][OMM_FS_MODE_IS_SPARKLY_STARS], (sOmmMainMenu->index == OMM_MM_PLAY ? 0xD2 : 0xFF));

    // Title
    const u8 *titleStr = omm_text_convert(sOmmFileSelectStrings[sOmmMainMenu->index][sOmmFileSelect->mode], false);
    omm_render_string_hud_centered(OMM_FS_TITLE_Y, 0xFF, 0xFF, 0xFF, 0xFF, titleStr, false);

    // Files
    for (s32 i = 0; i != 4; ++i) {
        s32 u = omm_sparkly_is_unlocked(i + 1);
        s32 j = OMM_FS_MODE_IS_SPARKLY_STARS * (u + 1);
        s32 w = (SW - 2 * (OMM_FS_BACKGROUND_MARGIN + OMM_FS_BACKGROUND_BORDER)) / 2;
        s32 x = (SCREEN_WIDTH / 2) + (w + OMM_FS_FILE_BUTTON_MARGIN_IN) * (i % 2) + OMM_FS_FILE_BUTTON_MARGIN_OUT * ((i + 1) % 2) + (OMM_FS_FILE_BUTTON_WIDTH / 2) - w;
        s32 y = ((SCREEN_HEIGHT - OMM_FS_TITLE_H) / 2) + ((i / 2) == 0 ? +1 : -1) * (OMM_FS_FILE_BUTTON_HEIGHT / 2 + OMM_FS_FILE_BUTTON_MARGIN_OUT);

        // Selection box (copy)
        if (sOmmFileCopy->open && sOmmFileCopy->index == i) {
            omm_render_texrect(OMM_FS_BOX_X, OMM_FS_BOX_Y, OMM_FS_BOX_W, OMM_FS_BOX_H, 32, 32, 0xFF, 0xFF, 0x00, 0x80, OMM_TEXTURE_MISC_WHITE, false);
        }

        // Selection box
        if (sOmmFileSelect->index == i) {
            omm_render_texrect(OMM_FS_BOX_X, OMM_FS_BOX_Y, OMM_FS_BOX_W, OMM_FS_BOX_H, 32, 32, 0x00, 0xFF, 0xFF, OMM_FS_BOX_A, OMM_TEXTURE_MISC_WHITE, false);
        }

        // Generic button
        const u8 *colors = sOmmFileSelectButtonColors[i][j];
        omm_file_select_render_button(x, y, colors[0], colors[1], colors[2]);

        // File
        u8 *fileStr = omm_text_convert(sOmmFileSelectFiles[i][j], false);
        omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 9, y + 10, 0x00, 0x00, 0x00, 0xFF, fileStr, false);
        omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 7, y + 10, 0x00, 0x00, 0x00, 0xFF, fileStr, false);
        omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 8, y + 11, 0x00, 0x00, 0x00, 0xFF, fileStr, false);
        omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 8, y +  9, 0x00, 0x00, 0x00, 0xFF, fileStr, false);
        omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 8, y + 10, 0xFF, 0xFF, 0xFF, 0xFF, fileStr, false);

        // Data
        if (OMM_FS_MODE_IS_SPARKLY_STARS) {

            // Sparkly mode
            s32 sparklyMode = i + 1;
            s32 numStars = omm_sparkly_get_collected_count(sparklyMode);
            s32 timer = omm_sparkly_get_timer(sparklyMode);
            if (u) {

                // Num stars
                str_fmt_sa(numStarsBuf, 8, "%d", numStars);
                u8 *numStarsStr = omm_text_convert(numStarsBuf, false);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 +  8, y - 4, 0xFF, 0xFF, 0xFF, 0xFF, array_of(u8) { 0xFA, 0xFF }, true);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 18, y - 4, 0xFF, 0xFF, 0xFF, 0xFF, array_of(u8) { 0xFB, 0xFF }, true);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 28, y - 4, 0xFF, 0xFF, 0xFF, 0xFF, numStarsStr, true);

                // Timer
                str_fmt_sa(timerBuf, 16, "%02d:%02d:%02d", (timer / 108000), (timer / 1800) % 60, (timer / 30) % 60);
                u8 *timerStr = omm_text_convert(timerBuf, false);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 8, y - 18, 0xFF, 0xFF, 0xFF, 0xFF, timerStr, true);
            }
            
            // Star icon
            s32 k = omm_sparkly_is_completed(sparklyMode);
            s32 c = (u && k ? 0xFF : 0x00);
            s32 a = (u && k ? 0xFF : 0x80);
            s32 s = ((OMM_FS_FILE_BUTTON_HEIGHT - 2 * OMM_FS_FILE_BUTTON_BORDER) * 3) / 4;
            omm_file_select_render_icon(x, y, s, s, 128, 128, c, c, c, a, OMM_SPARKLY_HUD_GLYPH[clamp_s(sparklyMode, 0, OMM_SPARKLY_MODE_COUNT - 1)]);
        } else {

            // Save file
            if (omm_save_file_exists(i, sOmmFileSelect->mode)) {
                s32 numStars = omm_save_file_get_total_star_count(i, sOmmFileSelect->mode, COURSE_MIN - 1, COURSE_MAX - 1);
                s32 numCoins = omm_save_file_get_total_coin_score(i, sOmmFileSelect->mode, COURSE_MIN - 1, COURSE_MAX - 1);
                s32 lastCourseNum = omm_save_file_get_last_course(i, sOmmFileSelect->mode);

                // Num stars
                str_fmt_sa(numStarsBuf, 8, "%d", numStars);
                u8 *numStarsStr = omm_text_convert(numStarsBuf, false);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 +  8, y - 4, 0xFF, 0xFF, 0xFF, 0xFF, array_of(u8) { 0xFA, 0xFF }, true);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 18, y - 4, 0xFF, 0xFF, 0xFF, 0xFF, array_of(u8) { 0xFB, 0xFF }, true);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 28, y - 4, 0xFF, 0xFF, 0xFF, 0xFF, numStarsStr, true);

                // Num coins
                str_fmt_sa(numCoinsBuf, 8, "%d", numCoins);
                u8 *numCoinsStr = omm_text_convert(numCoinsBuf, false);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 +  8, y - 18, 0xFF, 0xFF, 0xFF, 0xFF, array_of(u8) { 0xF9, 0xFF }, true);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 18, y - 18, 0xFF, 0xFF, 0xFF, 0xFF, array_of(u8) { 0xFB, 0xFF }, true);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 28, y - 18, 0xFF, 0xFF, 0xFF, 0xFF, numCoinsStr, true);

                // Last course icon
                s32 wi = (OMM_FS_FILE_BUTTON_WIDTH - 2 * OMM_FS_FILE_BUTTON_BORDER);
                s32 hi = (OMM_FS_FILE_BUTTON_HEIGHT - 2 * OMM_FS_FILE_BUTTON_BORDER);
                omm_file_select_render_icon(x, y, wi, hi, 320, 240, 0xFF, 0xFF, 0xFF, 0xFF, sOmmFileSelectCourses[lastCourseNum][sOmmFileSelect->mode]);

                // Last course
                u8 *courseStr = omm_level_get_course_name(omm_level_from_course(lastCourseNum), sOmmFileSelect->mode, false, false);
                omm_render_string_sized(x - OMM_FS_FILE_BUTTON_WIDTH / 2, y - (OMM_FS_FILE_BUTTON_HEIGHT / 2) - 9, 5, 5, 0xFF, 0xFF, 0xFF, 0xFF, courseStr, true);
            } else {

                // Empty icon
                s32 wi = (OMM_FS_FILE_BUTTON_WIDTH - 2 * OMM_FS_FILE_BUTTON_BORDER);
                s32 hi = (OMM_FS_FILE_BUTTON_HEIGHT - 2 * OMM_FS_FILE_BUTTON_BORDER);
                omm_file_select_render_icon(x, y, wi, hi, 64, 32, 0xFF, 0xFF, 0xFF, 0xFF, OMM_ASSET_MENU_EMPTY_FILE);

                // Empty
                u8 *emptyStr = omm_text_convert(OMM_TEXT_FS_EMPTY, false);
                omm_render_string(x + OMM_FS_FILE_BUTTON_WIDTH / 2 + 8, y - 4, 0x00, 0x00, 0x00, 0x40, emptyStr, false);
            }
        }
    }

    // Score board
    if (sOmmFileScore->open && sOmmFileScore->timer) {
        omm_file_select_render_score_board(sqr_f(relerp_0_1_f(abs_f(sOmmFileScore->timer), 0, 10, 0, 1)));
    }

    // Transition
    if (sOmmFileSelect->timer < 15) {
        omm_render_texrect(
            GFX_DIMENSIONS_FROM_LEFT_EDGE(0), 0, GFX_DIMENSIONS_SCREEN_WIDTH, SCREEN_HEIGHT,
            32, 32, 0xFF, 0xFF, 0xFF, 0xFF * relerp_0_1_f(sOmmFileSelect->timer, 0, 15, 1, 0), OMM_TEXTURE_MISC_WHITE, false
        );
    }
}

//
// Level functions
//

static s32 omm_level_main_menu_init(UNUSED s32 arg, UNUSED s32 unused) {
    mem_clr(sOmmMainMenu, sizeof(*sOmmMainMenu));
    mem_clr(sOmmFileSelect, sizeof(*sOmmFileSelect));
    mem_clr(sOmmFileCopy, sizeof(*sOmmFileCopy));
    mem_clr(sOmmFileScore, sizeof(*sOmmFileScore));
    mem_clr(gPlayerSpawnInfos, sizeof(*gPlayerSpawnInfos));
    mem_clr(gPlayerCameraState, sizeof(*gPlayerCameraState));
    mem_clr(gBodyStates, sizeof(*gBodyStates));
    mem_clr(gMarioState, sizeof(*gMarioState));
    gMarioState->controller = gControllers;
    gMarioState->spawnInfo = gPlayerSpawnInfos;
    gMarioState->statusForCamera = gPlayerCameraState;
    gMarioState->marioBodyState = gBodyStates;
    gMarioState->marioBodyState->capState = MARIO_HAS_DEFAULT_CAP_OFF;
    gMarioState->marioBodyState->handState = MARIO_HAND_FISTS;
    gMarioState->unkB0 = 0xBD;
    sOmmCompleteSaveSequenceIndex = 0;
    gMarioAnimations = &gMarioAnimsData;
    omm_register_warp_functions();
    omm_disable_warp_functions();
    return 0;
}

static s32 omm_level_main_menu_update(UNUSED s32 arg, UNUSED s32 unused) {
    if (sOmmFileSelect->open) {
#if OMM_GAME_IS_R96X
        r96_play_menu_jingle(R96_MENU_FILE_SELECT, 1.0, 1.0, 1500);
#else
        set_background_music(0, OMM_SEQ_FILE_SELECT, 0);
#endif
        sOmmCompleteSaveSequenceIndex = (sOmmMainMenu->index == OMM_MM_PLAY ? omm_update_complete_save_sequence_index() : 0);
        return omm_file_select_update();
    } else {
#if OMM_GAME_IS_R96X
        r96_play_menu_jingle(R96_EVENT_TITLE_SCREEN, 1.0, 1.0, 1500);
#else
        set_background_music(0, OMM_SEQ_MAIN_MENU, 0);
#endif
        sOmmCompleteSaveSequenceIndex = 0;
        return omm_main_menu_update();
    }
    return 0;
}

s32 omm_level_main_menu_end(UNUSED s32 arg, UNUSED s32 unused) {
    omm_enable_warp_functions();
    omm_player_select(gOmmCharacter);
    sOmmCompleteSaveSequenceIndex = 0;
    return 0;
}

static Gfx *omm_level_main_menu_render(s32 callContext, UNUSED struct GraphNode *node, UNUSED void *context) {
    if (callContext == GEO_CONTEXT_RENDER) {
        if (sOmmFileSelect->open) {
            omm_file_select_render();
        } else {
            omm_main_menu_render();
        }
    }
    return NULL;
}

//
// Level geolayout
//

const GeoLayout omm_level_main_menu_geo[] = {
    GEO_NODE_SCREEN_AREA(0, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, SCREEN_HEIGHT/2),
    GEO_OPEN_NODE(),
        GEO_ZBUFFER(0),
        GEO_OPEN_NODE(),
            GEO_NODE_ORTHO(100),
            GEO_OPEN_NODE(),
                GEO_ASM(0, geo_intro_backdrop),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
        GEO_ZBUFFER(1),
        GEO_OPEN_NODE(),
            GEO_CAMERA_FRUSTUM(45, 100, 25000),
            GEO_OPEN_NODE(),
                GEO_CAMERA(0, 0, 0, 1000, 0, 0, 0, 0),
                GEO_OPEN_NODE(),
                    GEO_RENDER_OBJ(),
                GEO_CLOSE_NODE(),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
        GEO_ZBUFFER(0),
        GEO_OPEN_NODE(),
            GEO_ASM(0, omm_level_main_menu_render),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};

//
// Level scripts
//

static const LevelScript omm_level_main_menu_reset[] = {
    STOP_MUSIC(190),
    TRANSITION(WARP_TRANSITION_FADE_INTO_COLOR, 15, 0x00, 0x00, 0x00),
    SLEEP(15),
    CALL(0, omm_level_main_menu_end),
    SLEEP(15),
    CLEAR_LEVEL(),
    JUMP(level_script_file_select),
};

const LevelScript level_script_file_select[] = {
    CALL(0, omm_level_main_menu_init),
    INIT_LEVEL(),
    ALLOC_LEVEL_POOL(),
    AREA(1, omm_level_main_menu_geo),
        OBJECT(0, 0, 0, 0, 0, 0, 0, 0, bhvOmmMainMenuMario),
        OBJECT(0, 0, 0, 0, 0, 0, 0, 0, bhvOmmMainMenuCappy),
    END_AREA(),
    FREE_LEVEL_POOL(),
    LOAD_AREA(1),
    TRANSITION(WARP_TRANSITION_FADE_FROM_COLOR, 15, 0xFF, 0xFF, 0xFF),
    CALL_LOOP(0, omm_level_main_menu_update),
    JUMP_IF(OP_EQ, -1, omm_level_main_menu_reset),
    GET_OR_SET(OP_SET, VAR_CURR_SAVE_FILE_NUM),
    STOP_MUSIC(190),
    TRANSITION(WARP_TRANSITION_FADE_INTO_COLOR, 15, 0xFF, 0xFF, 0xFF),
    SLEEP(15),
    CALL(0, omm_level_main_menu_end),
    SLEEP(15),
    CLEAR_LEVEL(),
    SLEEP_BEFORE_EXIT(1),
    SET_REG(OMM_LEVEL_ENTRY_POINT),
    EXIT_AND_EXECUTE(0x15, 0, 0, level_main_scripts_entry),
};
