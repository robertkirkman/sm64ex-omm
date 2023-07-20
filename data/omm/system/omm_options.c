#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#ifdef TOUCH_CONTROLS
#include "pc/controller/controller_touchscreen.h"
#endif
#if !OMM_GAME_IS_R96X
u32 gKeyPressed;
#endif

//
// From options_menu.c
//

#if !OMM_GAME_IS_RF14
enum OptType { OPT_INVALID = 0, OPT_TOGGLE, OPT_CHOICE, OPT_SCROLL, OPT_SUBMENU, OPT_BIND, OPT_BUTTON, };
struct SubMenu;
struct Option { enum OptType type; const u8 *label; union { u32 *uval; bool *bval; }; union { struct { const u8 **choices; s32 numChoices; }; struct { u32 scrMin; u32 scrMax; u32 scrStep; }; struct SubMenu *nextMenu; void (*actionFn)(struct Option *, s32); }; };
struct SubMenu { struct SubMenu *prev; const u8 *label; struct Option *opts; s32 numOpts; s32 select; s32 scroll; };
#endif

//
// Options definition
//

OmmOptMenu gOmmOptMenu;
#ifdef TOUCH_CONTROLS
OmmOptMenu gOmmOptTouchControls;
#endif
OmmOptMenu gOmmOptControls;
#if !OMM_GAME_IS_R96X
OmmOptMenu gOmmOptCheats;
#endif
#if !OMM_CODE_DYNOS
OmmOptMenu gOmmOptWarpToLevel;
OmmOptMenu gOmmOptReturnToMainMenu;
OmmOptMenu gOmmOptModels;
bool **gOmmOptModelsEnabled;
#endif

#define K_NONE VK_INVALID
static const struct {
    u32 *binds;                u32 b0; u32 b1; u32 b2;
} sOmmControlsDefault[] = {                              //  QW  /  AZ  | XBOne Con | Switch PC |
    { gOmmControlsButtonA,     0x0026, 0x1000, K_NONE }, //     [L]     |    (A)    |    (B)    |
    { gOmmControlsButtonB,     0x0033, 0x1001, K_NONE }, //  [,] / [;]  |    (B)    |    (A)    |
    { gOmmControlsButtonX,     0x0025, 0x1002, K_NONE }, //     [K]     |    (X)    |    (X)    |
    { gOmmControlsButtonY,     0x0032, 0x1003, K_NONE }, //  [M] / [,]  |    (Y)    |    (Y)    |
    { gOmmControlsButtonStart, 0x0039, 0x1006, K_NONE }, //   [SPACE]   |  (Start)  |    (+)    |
    { gOmmControlsButtonSpin,  0x1102, 0x1007, K_NONE }, //    (MWB)    |   (LSB)   |   (LSB)   |
    { gOmmControlsTriggerL,    0x002A, 0x1009, K_NONE }, //   [LSHFT]   |    (LB)   |    (L)    |
    { gOmmControlsTriggerR,    0x0036, 0x100A, K_NONE }, //   [RSHFT]   |    (RB)   |    (R)    |
    { gOmmControlsTriggerZ,    0x0018, 0x101B, K_NONE }, //     [O]     |    (RT)   |    (ZR)   |
    { gOmmControlsCUp,         0x0148, K_NONE, K_NONE }, //     [^]     |   (R-U)   |   (R-U)   |
    { gOmmControlsCDown,       0x0150, K_NONE, K_NONE }, //     [v]     |   (R-D)   |   (R-D)   |
    { gOmmControlsCLeft,       0x014B, K_NONE, K_NONE }, //     [<]     |   (R-L)   |   (R-L)   |
    { gOmmControlsCRight,      0x014D, K_NONE, K_NONE }, //     [>]     |   (R-R)   |   (R-R)   |
    { gOmmControlsDUp,         0x000C, 0x100B, K_NONE }, //  [-] / [)]  |   (D-U)   |   (D-U)   |
    { gOmmControlsDDown,       0x001A, 0x100C, K_NONE }, //  [{] / [^]  |   (D-D)   |   (D-D)   |
    { gOmmControlsDLeft,       0x0019, 0x100D, K_NONE }, //     [P]     |   (D-L)   |   (D-L)   |
    { gOmmControlsDRight,      0x001B, 0x100E, K_NONE }, //  [}] / [$]  |   (D-R)   |   (D-R)   |
    { gOmmControlsStickUp,     0x0011, K_NONE, K_NONE }, //  [W] / [Z]  |   (L-U)   |   (L-U)   |
    { gOmmControlsStickDown,   0x001F, K_NONE, K_NONE }, //     [S]     |   (L-D)   |   (L-D)   |
    { gOmmControlsStickLeft,   0x001E, K_NONE, K_NONE }, //  [A] / [Q]  |   (L-L)   |   (L-L)   |
    { gOmmControlsStickRight,  0x0020, K_NONE, K_NONE }, //     [D]     |   (L-R)   |   (L-R)   |
};

DEFINE_KBINDS(gOmmControlsButtonA,     0x0026, 0x1000, K_NONE);
DEFINE_KBINDS(gOmmControlsButtonB,     0x0033, 0x1001, K_NONE);
DEFINE_KBINDS(gOmmControlsButtonX,     0x0025, 0x1002, K_NONE);
DEFINE_KBINDS(gOmmControlsButtonY,     0x0032, 0x1003, K_NONE);
DEFINE_KBINDS(gOmmControlsButtonStart, 0x0039, 0x1006, K_NONE);
DEFINE_KBINDS(gOmmControlsButtonSpin,  0x1102, 0x1007, K_NONE);
DEFINE_KBINDS(gOmmControlsTriggerL,    0x002A, 0x1009, K_NONE);
DEFINE_KBINDS(gOmmControlsTriggerR,    0x0036, 0x100A, K_NONE);
DEFINE_KBINDS(gOmmControlsTriggerZ,    0x0018, 0x101B, K_NONE);
DEFINE_KBINDS(gOmmControlsCUp,         0x0148, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsCDown,       0x0150, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsCLeft,       0x014B, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsCRight,      0x014D, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsDUp,         0x000C, 0x100B, K_NONE);
DEFINE_KBINDS(gOmmControlsDDown,       0x001A, 0x100C, K_NONE);
DEFINE_KBINDS(gOmmControlsDLeft,       0x0019, 0x100D, K_NONE);
DEFINE_KBINDS(gOmmControlsDRight,      0x001B, 0x100E, K_NONE);
DEFINE_KBINDS(gOmmControlsStickUp,     0x0011, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsStickDown,   0x001F, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsStickLeft,   0x001E, K_NONE, K_NONE);
DEFINE_KBINDS(gOmmControlsStickRight,  0x0020, K_NONE, K_NONE);
#if !OMM_GAME_IS_R96X
DEFINE_TOGGLE(gOmmCheatEnable, 0);                                          // Disabled
DEFINE_TOGGLE(gOmmCheatMoonJump, 0);                                        // Disabled
DEFINE_TOGGLE(gOmmCheatGodMode, 0);                                         // Disabled
DEFINE_TOGGLE(gOmmCheatInvincible, 0);                                      // Disabled
DEFINE_TOGGLE(gOmmCheatSuperSpeed, 0);                                      // Disabled
DEFINE_TOGGLE(gOmmCheatSuperResponsive, 0);                                 // Disabled
DEFINE_TOGGLE(gOmmCheatNoFallDamage, 0);                                    // Disabled
DEFINE_TOGGLE(gOmmCheatCapModifier, 0);                                     // Disabled
DEFINE_TOGGLE(gOmmCheatWalkOnLava, 0);                                      // Disabled
DEFINE_TOGGLE(gOmmCheatWalkOnQuicksand, 0);                                 // Disabled
DEFINE_TOGGLE(gOmmCheatWalkOnWater, 0);                                     // Disabled
DEFINE_TOGGLE(gOmmCheatWalkOnGas, 0);                                       // Disabled
DEFINE_TOGGLE(gOmmCheatWalkOnSlope, 0);                                     // Disabled
DEFINE_TOGGLE(gOmmCheatWalkOnDeathBarrier, 0);                              // Disabled
DEFINE_TOGGLE(gOmmCheatBljAnywhere, 0);                                     // Disabled
#endif
DEFINE_CHOICE(gOmmFrameRate, OMM_FPS_30, 4);                                // 30 FPS
DEFINE_TOGGLE(gOmmShowFPS, 0);                                              // Disabled
DEFINE_CHOICE(gOmmTextureCaching, 2, 3);                                    // Permanent
DEFINE_CHOICE_SC(gOmmCharacter, 0, OMM_NUM_PLAYABLE_CHARACTERS);            // Mario
DEFINE_CHOICE_SC(gOmmMovesetType, 1, 4);                                    // Odyssey (3-Health)
DEFINE_CHOICE_SC(gOmmCapType, 2, 4);                                        // Cappy (Capture - Press)
DEFINE_CHOICE_SC(gOmmStarsMode, 1, 2);                                      // Non-Stop
DEFINE_CHOICE_SC(gOmmPowerUpsType, 1, 2);                                   // Improved
DEFINE_CHOICE_SC(gOmmCameraMode, 0, 3);                                     // Classic
DEFINE_CHOICE_SC(gOmmSparklyStarsMode, 0, OMM_SPARKLY_MODE_COUNT);          // Disabled
DEFINE_CHOICE_SC(gOmmSparklyStarsHintAtLevelEntry, 0, 3);                   // Always
DEFINE_TOGGLE(gOmmCheatUnlimitedCappyBounces, 0);                           // Disabled
DEFINE_TOGGLE(gOmmCheatCappyStaysForever, 0);                               // Disabled
DEFINE_TOGGLE(gOmmCheatHomingAttackGlobalRange, 0);                         // Disabled
DEFINE_TOGGLE(gOmmCheatMarioTeleportsToCappy, 0);                           // Disabled
DEFINE_TOGGLE(gOmmCheatCappyCanCollectStars, 0);                            // Disabled
DEFINE_TOGGLE(gOmmCheatPlayAsCappy, 0);                                     // Disabled
DEFINE_TOGGLE(gOmmCheatPeachEndlessVibeGauge, 0);                           // Disabled
DEFINE_TOGGLE(gOmmCheatShadowMario, 0);                                     // Disabled
DEFINE_CHOICE(gOmmExtrasMarioColors, 0, /*omm_mario_colors_count()*/ 32);   // Default
DEFINE_CHOICE(gOmmExtrasPeachColors, 0, /*omm_mario_colors_count()*/ 32);   // Default
DEFINE_TOGGLE_SC(gOmmExtrasSMOAnimations, 1);                               // Enabled
DEFINE_TOGGLE_SC(gOmmExtrasCappyAndTiara, 1);                               // Enabled
DEFINE_TOGGLE_SC(gOmmExtrasColoredStars, 1);                                // Enabled
DEFINE_TOGGLE_SC(gOmmExtrasVanishingHUD, 1);                                // Enabled
DEFINE_TOGGLE_SC(gOmmExtrasRevealSecrets, 0);                               // Disabled
DEFINE_TOGGLE_SC(gOmmExtrasRedCoinsRadar, 1);                               // Enabled
DEFINE_TOGGLE_SC(gOmmExtrasShowStarNumber, 1);                              // Enabled
DEFINE_TOGGLE_SC(gOmmExtrasInvisibleMode, 0);                               // Disabled
DEFINE_CHOICE_SC(gOmmExtrasSparklyStarsReward, 0, OMM_SPARKLY_MODE_COUNT);  // Disabled
#if OMM_CODE_DEBUG
DEFINE_TOGGLE_SC(gOmmDebugHitbox, 0);                                       // Disabled
DEFINE_TOGGLE_SC(gOmmDebugHurtbox, 0);                                      // Disabled
DEFINE_TOGGLE_SC(gOmmDebugWallbox, 0);                                      // Disabled
DEFINE_TOGGLE_SC(gOmmDebugSurface, 0);                                      // Disabled
DEFINE_TOGGLE_SC(gOmmDebugMario, 0);                                        // Disabled
DEFINE_TOGGLE_SC(gOmmDebugCappy, 0);                                        // Disabled
#endif
#if OMM_CODE_DEV
#include "data/omm/dev/omm_dev_opt_define.inl"
#endif

//
// Shortcuts
//

typedef struct {
    s32 type;
    u32 *binds;
    const char *label;
    union {
        struct {
            bool *option;
        } toggle;
        struct {
            u32 *option;
            const u32 *numChoices;
            const char *choices[8];
        } choice;
    };
} OmmOptShortcut;

#define DEFINE_SHORTCUT_TOGGLE(opt, lbl)        { .type = OPT_TOGGLE, .binds = opt##Shortcuts, .label = lbl, .toggle.option = &opt }
#define DEFINE_SHORTCUT_CHOICE(opt, lbl, ...)   { .type = OPT_CHOICE, .binds = opt##Shortcuts, .label = lbl, .choice.option = &opt, .choice.numChoices = &opt##Count, .choice.choices = { __VA_ARGS__ } }

static const OmmOptShortcut sOmmOptShortcuts[] = {
DEFINE_SHORTCUT_CHOICE(gOmmCharacter, OMM_TEXT_OPT_CHARACTER_LABEL, OMM_TEXT_MARIO, OMM_TEXT_PEACH, OMM_TEXT_LUIGI, OMM_TEXT_WARIO),
DEFINE_SHORTCUT_CHOICE(gOmmMovesetType, OMM_TEXT_OPT_MOVESET_LABEL, OMM_TEXT_OPT_MOVESET_CLASSIC, OMM_TEXT_OPT_MOVESET_ODYSSEY_3H, OMM_TEXT_OPT_MOVESET_ODYSSEY_6H, OMM_TEXT_OPT_MOVESET_ODYSSEY_1H),
DEFINE_SHORTCUT_CHOICE(gOmmCapType, OMM_TEXT_OPT_CAP_LABEL, OMM_TEXT_OPT_CAP_CLASSIC, OMM_TEXT_OPT_CAP_NO_CAPTURE, OMM_TEXT_OPT_CAP_CAPTURE_PRESS, OMM_TEXT_OPT_CAP_CAPTURE_HOLD),
DEFINE_SHORTCUT_CHOICE(gOmmStarsMode, OMM_TEXT_OPT_STARS_LABEL, OMM_TEXT_OPT_STARS_CLASSIC, OMM_TEXT_OPT_STARS_NON_STOP),
DEFINE_SHORTCUT_CHOICE(gOmmPowerUpsType, OMM_TEXT_OPT_POWER_UPS_LABEL, OMM_TEXT_OPT_POWER_UPS_CLASSIC, OMM_TEXT_OPT_POWER_UPS_IMPROVED),
DEFINE_SHORTCUT_CHOICE(gOmmCameraMode, OMM_TEXT_OPT_CAMERA_LABEL, OMM_TEXT_OPT_CAMERA_CLASSIC, OMM_TEXT_OPT_CAMERA_8_DIR, OMM_TEXT_OPT_CAMERA_16_DIR),
DEFINE_SHORTCUT_CHOICE(gOmmSparklyStarsMode, OMM_TEXT_OPT_SPARKLY_STARS_MODE, OMM_TEXT_OPT_SPARKLY_STARS_MODE_DISABLED, OMM_TEXT_OPT_SPARKLY_STARS_MODE_NORMAL, OMM_TEXT_OPT_SPARKLY_STARS_MODE_HARD, OMM_TEXT_OPT_SPARKLY_STARS_MODE_LUNATIC),
DEFINE_SHORTCUT_CHOICE(gOmmSparklyStarsHintAtLevelEntry, OMM_TEXT_OPT_SPARKLY_STARS_HINT, OMM_TEXT_OPT_SPARKLY_STARS_HINT_ALWAYS, OMM_TEXT_OPT_SPARKLY_STARS_HINT_NOT_COLLECTED, OMM_TEXT_OPT_SPARKLY_STARS_HINT_NEVER),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasSMOAnimations, OMM_TEXT_OPT_SMO_ANIMATIONS),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasCappyAndTiara, OMM_TEXT_OPT_CAPPY_AND_TIARA),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasColoredStars, OMM_TEXT_OPT_COLORED_STARS),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasVanishingHUD, OMM_TEXT_OPT_VANISHING_HUD),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasRevealSecrets, OMM_TEXT_OPT_REVEAL_SECRETS),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasRedCoinsRadar, OMM_TEXT_OPT_RED_COINS_RADAR),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasShowStarNumber, OMM_TEXT_OPT_SHOW_STAR_NUMBER),
DEFINE_SHORTCUT_TOGGLE(gOmmExtrasInvisibleMode, OMM_TEXT_OPT_INVISIBLE_MODE),
#if OMM_CODE_DEBUG
DEFINE_SHORTCUT_TOGGLE(gOmmDebugHitbox, OMM_TEXT_OPT_DEBUG_HITBOX),
DEFINE_SHORTCUT_TOGGLE(gOmmDebugHurtbox, OMM_TEXT_OPT_DEBUG_HURTBOX),
DEFINE_SHORTCUT_TOGGLE(gOmmDebugWallbox, OMM_TEXT_OPT_DEBUG_WALLBOX),
DEFINE_SHORTCUT_TOGGLE(gOmmDebugSurface, OMM_TEXT_OPT_DEBUG_SURFACE),
DEFINE_SHORTCUT_TOGGLE(gOmmDebugMario, OMM_TEXT_OPT_DEBUG_MARIO),
DEFINE_SHORTCUT_TOGGLE(gOmmDebugCappy, OMM_TEXT_OPT_DEBUG_CAPPY),
#endif
DEFINE_SHORTCUT_CHOICE(gOmmExtrasSparklyStarsReward, OMM_TEXT_OPT_SPARKLY_STARS_REWARD, OMM_TEXT_OPT_SPARKLY_STARS_REWARD_DISABLED, OMM_TEXT_OPT_SPARKLY_STARS_REWARD_NORMAL, OMM_TEXT_OPT_SPARKLY_STARS_REWARD_HARD, OMM_TEXT_OPT_SPARKLY_STARS_REWARD_LUNATIC), // Must be last
};

//
// Option wrappers
//

#if OMM_GAME_IS_R96X
#define omm_opt_text(str, ...)      (const u8 *) str
#define omm_opt_text_length(str)    (s32) strlen((const char *) str)
#else
#define omm_opt_text(str, ...)      omm_text_convert(str, __VA_ARGS__)
#define omm_opt_text_length(str)    omm_text_length(str)
#endif

static struct Option omm_opt_make_toggle(const char *label, bool *value) {
    struct Option opt = { 0 };
    opt.type = OPT_TOGGLE;
    opt.label = omm_opt_text(label, true);
    opt.bval = value;
    return opt;
}

static struct Option omm_opt_make_choice(const char *label, u32 *value, const char **choices, s32 numChoices) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_opt_text(label, true);
    opt.uval = value;
    opt.choices = mem_new(u8 *, numChoices);
    opt.numChoices = numChoices;
    for (s32 i = 0; i != numChoices; ++i) {
        opt.choices[i] = omm_opt_text(choices[i], true);
    }
    return opt;
}

static struct Option omm_opt_make_scroll(const char *label, u32 *value, u32 min, u32 max, u32 step) {
    struct Option opt = { 0 };
    opt.type = OPT_SCROLL;
    opt.label = omm_opt_text(label, true);
    opt.uval = value;
    opt.scrMin = min;
    opt.scrMax = max;
    opt.scrStep = step;
    return opt;
}

static struct Option omm_opt_make_bind(const char *label, u32 *binds) {
    struct Option opt = { 0 };
    opt.type = OPT_BIND;
    opt.label = omm_opt_text(label, true);
    opt.uval = binds;
    return opt;
}

static struct Option omm_opt_make_button(const char *label, void (*actionFn)(struct Option *, s32)) {
    struct Option opt = { 0 };
    opt.type = OPT_BUTTON;
    opt.label = omm_opt_text(label, true);
    opt.actionFn = actionFn;
    return opt;
}

static struct Option omm_opt_make_submenu(const char *label, const char *title, struct Option *options, s32 numOptions) {
    struct Option opt = { 0 };
    opt.type = OPT_SUBMENU;
    opt.label = omm_opt_text(label, true);
    opt.nextMenu = mem_new(struct SubMenu, 1);
    opt.nextMenu->label = omm_opt_text(title, true);
    opt.nextMenu->opts = mem_new(struct Option, numOptions);
    opt.nextMenu->numOpts = numOptions;
    for (s32 i = 0; i != numOptions; ++i) {
        opt.nextMenu->opts[i] = options[i];
    }
    return opt;
}

static struct Option omm_opt_make_shortcuts_submenu(const char *label, const char *title, s32 numShortcuts) {
    struct Option opt = { 0 };
    opt.type = OPT_SUBMENU;
    opt.label = omm_opt_text(label, true);
    opt.nextMenu = mem_new(struct SubMenu, 1);
    opt.nextMenu->label = omm_opt_text(title, true);
    opt.nextMenu->opts = mem_new(struct Option, numShortcuts);
    opt.nextMenu->numOpts = numShortcuts;
    for (s32 i = 0; i != numShortcuts; ++i) {
        opt.nextMenu->opts[i] = omm_opt_make_bind(sOmmOptShortcuts[i].label, sOmmOptShortcuts[i].binds);
    }
    return opt;
}

#if OMM_GAME_IS_SM74

//
// Super Mario 74 code
//

#define OMM_OPTIONS_C
#include "data/omm/system/omm_sm74.inl"
#undef OMM_OPTIONS_C

#else

//
// Warp to level (init)
//

static u32 sOmmWarpLevelNum = 0;

typedef struct { s32 levelNum; s32 areaIndex; } AreaValues;
static OmmArray sOmmWarpAreaValues = omm_array_zero;
static u32 sOmmWarpAreaIndex = 0;

typedef struct { s32 levelNum; s32 actNum; } ActValues;
static OmmArray sOmmWarpActValues = omm_array_zero;
static u32 sOmmWarpActNum = 0;

static void omm_opt_init_warp_to_level() {

    // Areas
    for (s32 i = 0; i != omm_level_get_count(); ++i) {
        s32 levelNum = omm_level_get_list()[i];
        s32 areas = omm_level_get_areas(levelNum);
        for (s32 j = 0; j != 32; ++j) {
            if ((areas >> j) & 1) {
                AreaValues *value = mem_new(AreaValues, 1);
                value->levelNum = levelNum;
                value->areaIndex = j;
                omm_array_add(sOmmWarpAreaValues, ptr, value);
            }
        }
    }

    // Acts
    for (s32 i = 0; i != omm_level_get_count(); ++i) {
        s32 levelNum = omm_level_get_list()[i];
        s32 stars = omm_stars_get_bits_total(levelNum, OMM_GAME_MODE);
        if (stars != 0) {
            for (s32 j = 0; j != OMM_NUM_ACTS_MAX_PER_COURSE; ++j) {
                if ((stars >> j) & 1) {
                    ActValues *value = mem_new(ActValues, 1);
                    value->levelNum = levelNum;
                    value->actNum = j + 1;
                    omm_array_add(sOmmWarpActValues, ptr, value);
                }
            }
        } else {
            ActValues *value = mem_new(ActValues, 1);
            value->levelNum = levelNum;
            value->actNum = 1;
            omm_array_add(sOmmWarpActValues, ptr, value);
        }
    }
}

static struct Option omm_opt_make_choice_level(const char *label, u32 *value) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_opt_text(label, true);
    opt.uval = value;
    opt.choices = mem_new(u8 *, omm_level_get_count());
    opt.numChoices = omm_level_get_count();
    for (s32 i = 0; i != (s32) opt.numChoices; ++i) {
        const u8 *name = omm_level_get_course_name(omm_level_get_list()[i], OMM_GAME_MODE, true, true);
        opt.choices[i] = mem_dup(name, omm_opt_text_length(name) + 1);
    }
    return opt;
}

static struct Option omm_opt_make_choice_area(const char *label, u32 *value) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_opt_text(label, true);
    opt.uval = value;
    opt.choices = mem_new(u8 *, omm_array_count(sOmmWarpAreaValues));
    opt.numChoices = omm_array_count(sOmmWarpAreaValues);
    for (s32 i = 0; i != (s32) opt.numChoices; ++i) {
        str_fmt_sa(name, 256, "Area %d", ((AreaValues *) omm_array_get(sOmmWarpAreaValues, ptr, i))->areaIndex);
        opt.choices[i] = omm_opt_text(name, true);
    }
    return opt;
}

static struct Option omm_opt_make_choice_act(const char *label, u32 *value) {
    struct Option opt = { 0 };
    opt.type = OPT_CHOICE;
    opt.label = omm_opt_text(label, true);
    opt.uval = value;
    opt.choices = mem_new(u8 *, omm_array_count(sOmmWarpActValues));
    opt.numChoices = omm_array_count(sOmmWarpActValues);
    for (s32 i = 0; i != (s32) opt.numChoices; ++i) {
        s32 levelNum = ((ActValues *) omm_array_get(sOmmWarpActValues, ptr, i))->levelNum;
        s32 actNum = ((ActValues *) omm_array_get(sOmmWarpActValues, ptr, i))->actNum;
        const u8 *name = omm_level_get_act_name(levelNum, actNum, OMM_GAME_MODE, true, true);
        opt.choices[i] = mem_dup(name, omm_opt_text_length(name) + 1);
    }
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

static u32 omm_opt_get_first_area_index(s32 levelNum) {
    omm_array_for_each(sOmmWarpAreaValues, p) {
        AreaValues *areaValues = (AreaValues *) p->as_ptr;
        if (areaValues->levelNum == levelNum) {
            return i_p;
        }
    }
    return 0;
}

static u32 omm_opt_get_first_act_index(s32 levelNum) {
    omm_array_for_each(sOmmWarpActValues, p) {
        ActValues *actValues = (ActValues *) p->as_ptr;
        if (actValues->levelNum == levelNum) {
            return i_p;
        }
    }
    return 0;
}

static void omm_opt_update_warp_to_level() {
    static u32 sOmmWarpLevelNumPrev = 0;
    static u32 sOmmWarpAreaIndexPrev = 0;
    static u32 sOmmWarpActNumPrev = 0;

    // Level changed
    if (sOmmWarpLevelNumPrev != sOmmWarpLevelNum) {
        s32 levelNum = omm_level_get_list()[sOmmWarpLevelNum];
        sOmmWarpAreaIndex = omm_opt_get_first_area_index(levelNum);
        sOmmWarpActNum = omm_opt_get_first_act_index(levelNum);
    }

    // Area changed
    else if (sOmmWarpAreaIndexPrev != sOmmWarpAreaIndex) {
        s32 numAreas       = omm_array_count(sOmmWarpAreaValues);
        u32 areaIndexLeft  = (sOmmWarpAreaIndexPrev + numAreas - 1) % numAreas;
        u32 areaIndexRight = (sOmmWarpAreaIndexPrev + numAreas + 1) % numAreas;
        s32 areaIndexPrev  = ((AreaValues *) omm_array_get(sOmmWarpAreaValues, ptr, sOmmWarpAreaIndexPrev))->areaIndex;
        s32 areaIndexCurr  = ((AreaValues *) omm_array_get(sOmmWarpAreaValues, ptr, sOmmWarpAreaIndex))->areaIndex;
        s32 levelNum       = ((AreaValues *) omm_array_get(sOmmWarpAreaValues, ptr, sOmmWarpAreaIndex))->levelNum;
        if ((sOmmWarpAreaIndex == areaIndexLeft && areaIndexCurr >= areaIndexPrev) || (sOmmWarpAreaIndex == areaIndexRight && areaIndexCurr <= areaIndexPrev))  {
            sOmmWarpActNum = omm_opt_get_first_act_index(levelNum);
        }
        sOmmWarpLevelNum = omm_opt_get_level_index(levelNum);
    }

    // Act changed
    else if (sOmmWarpActNumPrev != sOmmWarpActNum) {
        s32 numActs     = omm_array_count(sOmmWarpActValues);
        u32 actNumLeft  = (sOmmWarpActNumPrev + numActs - 1) % numActs;
        u32 actNumRight = (sOmmWarpActNumPrev + numActs + 1) % numActs;
        s32 actNumPrev  = ((ActValues *) omm_array_get(sOmmWarpActValues, ptr, sOmmWarpActNumPrev))->actNum;
        s32 actNumCurr  = ((ActValues *) omm_array_get(sOmmWarpActValues, ptr, sOmmWarpActNum))->actNum;
        s32 levelNum    = ((ActValues *) omm_array_get(sOmmWarpActValues, ptr, sOmmWarpActNum))->levelNum;
        if ((sOmmWarpActNum == actNumLeft && actNumCurr >= actNumPrev) || (sOmmWarpActNum == actNumRight && actNumCurr <= actNumPrev))  {
            sOmmWarpAreaIndex = omm_opt_get_first_area_index(levelNum);
        }
        sOmmWarpLevelNum = omm_opt_get_level_index(levelNum);
    }

    // Update values
    sOmmWarpLevelNumPrev = sOmmWarpLevelNum;
    sOmmWarpAreaIndexPrev = sOmmWarpAreaIndex;
    sOmmWarpActNumPrev = sOmmWarpActNum;
}

extern bool sOmmIsMainMenu;
extern bool sOmmIsLevelEntry;
extern bool sOmmIsEndingCutscene;
extern bool sOmmIsEndingCakeScreen;
s32 omm_level_main_menu_end(UNUSED s32 arg, UNUSED s32 unused);
void omm_stars_init_bits();
s32 omm_health_get_max(s32 health);

static void omm_opt_warp_to_level(UNUSED void *opt, s32 arg) {
    if (!arg) {
        s32 levelNum = omm_level_get_list()[sOmmWarpLevelNum];
        s32 areaIndex = ((AreaValues *) omm_array_get(sOmmWarpAreaValues, ptr, sOmmWarpAreaIndex))->areaIndex;
        s32 actNum = ((ActValues *) omm_array_get(sOmmWarpActValues, ptr, sOmmWarpActNum))->actNum;
        //if (omm_is_main_menu() || !omm_warp_to_level(levelNum, areaIndex, actNum)) {
        if (omm_warp_to_level(levelNum, areaIndex, actNum)) {        
            if (omm_is_main_menu()) {
                omm_level_main_menu_end(0, 0);
                omm_stars_init_bits();
                gMarioState->health = omm_health_get_max(0);
                sOmmIsMainMenu = false;
                sOmmIsLevelEntry = true;
                sOmmIsEndingCutscene = false;
                sOmmIsEndingCakeScreen = false;
            }
        } else {
            play_sound(SOUND_MENU_CAMERA_BUZZ | 0xFF00, gGlobalSoundArgs);
        }
    }
}

#endif

//
// Buttons
//

void omm_opt_return_to_main_menu(UNUSED void *opt, s32 arg) {
    if (!arg) {
        if (!omm_is_main_menu()) {
            omm_return_to_main_menu();
        } else {
            play_sound(SOUND_MENU_CAMERA_BUZZ | 0xFF00, gGlobalSoundArgs);
        }
    }
}

void omm_opt_reset_binds(u32 *binds) {
    for (s32 i = 0; i != array_length(sOmmControlsDefault); ++i) {
        if (binds   == sOmmControlsDefault[i].binds) {
            binds[0] = sOmmControlsDefault[i].b0;
            binds[1] = sOmmControlsDefault[i].b1;
            binds[2] = sOmmControlsDefault[i].b2;
            return;
        }
    }
}

static void omm_opt_reset_controls(UNUSED void *opt, s32 arg) {
    if (!arg) {
        for (s32 i = 0; i != array_length(sOmmControlsDefault); ++i) {
            sOmmControlsDefault[i].binds[0] = sOmmControlsDefault[i].b0;
            sOmmControlsDefault[i].binds[1] = sOmmControlsDefault[i].b1;
            sOmmControlsDefault[i].binds[2] = sOmmControlsDefault[i].b2;
        }
    }
}

static void omm_opt_enter_touch_control_config(UNUSED void *opt, s32 arg) {
    if (!arg) gInTouchConfig = true;
}

//
// Init
//

#define choices(...) (array_of(const char *) { __VA_ARGS__ })
#define options(...) (array_of(struct Option) { __VA_ARGS__ })

enum OmmOptState {
    OMM_OPT_STATE_SPARKLY_STARS_LOCKED,
    OMM_OPT_STATE_SPARKLY_STARS_UNLOCKED,
    OMM_OPT_STATE_PEACH_UNLOCKED,
    OMM_OPT_STATE_COUNT
};
static struct Option sOmmOptMenus[OMM_OPT_STATE_COUNT];

static s32 omm_opt_get_state() {
    if (OMM_SPARKLY_IS_PEACH_UNLOCKED) return OMM_OPT_STATE_PEACH_UNLOCKED;
    if (OMM_SPARKLY_IS_GAMEMODE_UNLOCKED) return OMM_OPT_STATE_SPARKLY_STARS_UNLOCKED;
    return OMM_OPT_STATE_SPARKLY_STARS_LOCKED;
}

static struct Option omm_opt_make_main_menu(s32 state) {

    // Character
    struct Option optCharacter = omm_opt_make_choice(OMM_TEXT_OPT_CHARACTER_LABEL, &gOmmCharacter, choices(
        OMM_TEXT_MARIO,
        OMM_TEXT_PEACH,
        OMM_TEXT_LUIGI,
        OMM_TEXT_WARIO),
    gOmmCharacterCount);

    // Moveset
    struct Option optMoveset = omm_opt_make_choice(OMM_TEXT_OPT_MOVESET_LABEL, &gOmmMovesetType, choices(
        OMM_TEXT_OPT_MOVESET_CLASSIC,
        OMM_TEXT_OPT_MOVESET_ODYSSEY_3H,
        OMM_TEXT_OPT_MOVESET_ODYSSEY_6H,
        OMM_TEXT_OPT_MOVESET_ODYSSEY_1H),
    gOmmMovesetTypeCount);

    // Cap
    struct Option optCap = omm_opt_make_choice(OMM_TEXT_OPT_CAP_LABEL, &gOmmCapType, choices(
        OMM_TEXT_OPT_CAP_CLASSIC,
        OMM_TEXT_OPT_CAP_NO_CAPTURE,
        OMM_TEXT_OPT_CAP_CAPTURE_PRESS,
        OMM_TEXT_OPT_CAP_CAPTURE_HOLD),
    gOmmCapTypeCount);

    // Stars
    struct Option optStars = omm_opt_make_choice(OMM_TEXT_OPT_STARS_LABEL, &gOmmStarsMode, choices(
        OMM_TEXT_OPT_STARS_CLASSIC,
        OMM_TEXT_OPT_STARS_NON_STOP),
    gOmmStarsModeCount);

    // Power-ups
    struct Option optPowerUps = omm_opt_make_choice(OMM_TEXT_OPT_POWER_UPS_LABEL, &gOmmPowerUpsType, choices(
        OMM_TEXT_OPT_POWER_UPS_CLASSIC,
        OMM_TEXT_OPT_POWER_UPS_IMPROVED),
    gOmmPowerUpsTypeCount);

    // Camera
    struct Option optCamera = omm_opt_make_choice(OMM_TEXT_OPT_CAMERA_LABEL, &gOmmCameraMode, choices(
        OMM_TEXT_OPT_CAMERA_CLASSIC,
        OMM_TEXT_OPT_CAMERA_8_DIR,
        OMM_TEXT_OPT_CAMERA_16_DIR),
    gOmmCameraModeCount);

    // Sparkly Stars
    struct Option optSparklyStars = omm_opt_make_submenu(OMM_TEXT_OPT_SPARKLY_STARS_LABEL, OMM_TEXT_OPT_SPARKLY_STARS_TITLE, options(
        omm_opt_make_choice(OMM_TEXT_OPT_SPARKLY_STARS_MODE, &gOmmSparklyStarsMode, choices(
            OMM_TEXT_OPT_SPARKLY_STARS_MODE_DISABLED,
            OMM_TEXT_OPT_SPARKLY_STARS_MODE_NORMAL,
            OMM_TEXT_OPT_SPARKLY_STARS_MODE_HARD,
            OMM_TEXT_OPT_SPARKLY_STARS_MODE_LUNATIC),
        gOmmSparklyStarsModeCount),
        omm_opt_make_choice(OMM_TEXT_OPT_SPARKLY_STARS_HINT, &gOmmSparklyStarsHintAtLevelEntry, choices(
            OMM_TEXT_OPT_SPARKLY_STARS_HINT_ALWAYS,
            OMM_TEXT_OPT_SPARKLY_STARS_HINT_NOT_COLLECTED,
            OMM_TEXT_OPT_SPARKLY_STARS_HINT_NEVER),
        gOmmSparklyStarsHintAtLevelEntryCount),
    ), 2);

    // Cheats
    struct Option optCheats = omm_opt_make_submenu(OMM_TEXT_OPT_CHEATS_LABEL, OMM_TEXT_OPT_CHEATS_TITLE, options(
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_UNLIMITED_CAPPY_BOUNCES, &gOmmCheatUnlimitedCappyBounces),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_CAPPY_STAYS_FOREVER, &gOmmCheatCappyStaysForever),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_GLOBAL_HOMING_ATTACK_RANGE, &gOmmCheatHomingAttackGlobalRange),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_MARIO_TELEPORTS_TO_CAPPY, &gOmmCheatMarioTeleportsToCappy),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_CAPPY_CAN_COLLECT_STARS, &gOmmCheatCappyCanCollectStars),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_PLAY_AS_CAPPY, &gOmmCheatPlayAsCappy),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_PEACH_ENDLESS_VIBE_GAUGE, &gOmmCheatPeachEndlessVibeGauge),
        omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_SHADOW_MARIO, &gOmmCheatShadowMario),
    ), 6 + 2 * (state >= OMM_OPT_STATE_PEACH_UNLOCKED));

    // Extras
    struct Option optExtras = omm_opt_make_submenu(OMM_TEXT_OPT_EXTRAS_LABEL, OMM_TEXT_OPT_EXTRAS_TITLE, options(
#if OMM_MK_MARIO_COLORS
        omm_opt_make_choice(OMM_TEXT_OPT_MARIO_COLORS, &gOmmExtrasMarioColors, omm_mario_colors_choices(false), gOmmExtrasMarioColorsCount),
#endif
        omm_opt_make_choice(OMM_TEXT_OPT_PEACH_COLORS, &gOmmExtrasPeachColors, omm_mario_colors_choices(true), gOmmExtrasPeachColorsCount),
        omm_opt_make_toggle(OMM_TEXT_OPT_SMO_ANIMATIONS, &gOmmExtrasSMOAnimations),
        omm_opt_make_toggle(OMM_TEXT_OPT_CAPPY_AND_TIARA, &gOmmExtrasCappyAndTiara),
#if !OMM_GAME_IS_SMMS
        omm_opt_make_toggle(OMM_TEXT_OPT_COLORED_STARS, &gOmmExtrasColoredStars),
#endif
        omm_opt_make_toggle(OMM_TEXT_OPT_VANISHING_HUD, &gOmmExtrasVanishingHUD),
#if !OMM_GAME_IS_SMGS
        omm_opt_make_toggle(OMM_TEXT_OPT_REVEAL_SECRETS, &gOmmExtrasRevealSecrets),
#endif
        omm_opt_make_toggle(OMM_TEXT_OPT_RED_COINS_RADAR, &gOmmExtrasRedCoinsRadar),
        omm_opt_make_toggle(OMM_TEXT_OPT_SHOW_STAR_NUMBER, &gOmmExtrasShowStarNumber),
        omm_opt_make_toggle(OMM_TEXT_OPT_INVISIBLE_MODE, &gOmmExtrasInvisibleMode),
        omm_opt_make_choice(OMM_TEXT_OPT_SPARKLY_STARS_REWARD, &gOmmExtrasSparklyStarsReward, choices(
            OMM_TEXT_OPT_SPARKLY_STARS_REWARD_DISABLED,
            OMM_TEXT_OPT_SPARKLY_STARS_REWARD_NORMAL,
            OMM_TEXT_OPT_SPARKLY_STARS_REWARD_HARD,
            OMM_TEXT_OPT_SPARKLY_STARS_REWARD_LUNATIC),
        gOmmExtrasSparklyStarsRewardCount),
    ), 7 + OMM_MK_MARIO_COLORS + !OMM_GAME_IS_SMMS + !OMM_GAME_IS_SMGS + (state >= OMM_OPT_STATE_PEACH_UNLOCKED));

    // Shortcuts
    struct Option optShortcuts = omm_opt_make_shortcuts_submenu(OMM_TEXT_OPT_SHORTCUTS_LABEL, OMM_TEXT_OPT_SHORTCUTS_TITLE,
        array_length(sOmmOptShortcuts) - (state < OMM_OPT_STATE_PEACH_UNLOCKED)
    );

#if OMM_CODE_DEBUG
    // Debug
    struct Option optDebug = omm_opt_make_submenu(OMM_TEXT_OPT_DEBUG_LABEL, OMM_TEXT_OPT_DEBUG_TITLE, options(
        omm_opt_make_toggle(OMM_TEXT_OPT_DEBUG_HITBOX, &gOmmDebugHitbox),
        omm_opt_make_toggle(OMM_TEXT_OPT_DEBUG_HURTBOX, &gOmmDebugHurtbox),
        omm_opt_make_toggle(OMM_TEXT_OPT_DEBUG_WALLBOX, &gOmmDebugWallbox),
        omm_opt_make_toggle(OMM_TEXT_OPT_DEBUG_SURFACE, &gOmmDebugSurface),
        omm_opt_make_toggle(OMM_TEXT_OPT_DEBUG_MARIO, &gOmmDebugMario),
        omm_opt_make_toggle(OMM_TEXT_OPT_DEBUG_CAPPY, &gOmmDebugCappy),
    ), 6);
#endif

#if OMM_CODE_DEV
#include "data/omm/dev/omm_dev_opt_make.inl"
#endif

    // OMM menu
    struct Option optOmmMenu;
    if (state >= OMM_OPT_STATE_SPARKLY_STARS_UNLOCKED) {
        optOmmMenu = omm_opt_make_submenu(OMM_TEXT_OPT_MENU_LABEL, OMM_TEXT_OPT_MENU_TITLE, options(
            optCharacter,
            optMoveset,
            optCap,
            optStars,
            optPowerUps,
            optCamera,
            optSparklyStars,
            optCheats,
            optExtras,
            optShortcuts,
#if OMM_CODE_DEBUG
            optDebug,
#endif
#if OMM_CODE_DEV
            optDev,
#endif
        ), 10 + OMM_CODE_DEBUG + OMM_CODE_DEV);
    } else {
        optOmmMenu = omm_opt_make_submenu(OMM_TEXT_OPT_MENU_LABEL, OMM_TEXT_OPT_MENU_TITLE, options(
            optCharacter,
            optMoveset,
            optCap,
            optStars,
            optPowerUps,
            optCamera,
            optCheats,
            optExtras,
            optShortcuts,
#if OMM_CODE_DEBUG
            optDebug,
#endif
#if OMM_CODE_DEV
            optDev,
#endif
        ), 9 + OMM_CODE_DEBUG + OMM_CODE_DEV);
    }
    return optOmmMenu;
}

#if !OMM_CODE_DYNOS
//OMM_AT_STARTUP static
#endif
void omm_opt_init() {
    static bool inited = false;
    if (!inited) {
        extern void omm_data_init();
        omm_data_init();
        omm_save_file_load_all();
        omm_player_select(gOmmCharacter);

        // OMM menu
        for (s32 state = 0; state != OMM_OPT_STATE_COUNT; ++state) {
            sOmmOptMenus[state] = omm_opt_make_main_menu(state);
            if (state == omm_opt_get_state()) {
                gOmmOptMenu.label = mem_dup(sOmmOptMenus[state].label, omm_opt_text_length(sOmmOptMenus[state].label) + 1);
                gOmmOptMenu.subMenu = mem_dup(sOmmOptMenus[state].nextMenu, sizeof(struct SubMenu));
            }
        }

#if !OMM_CODE_DYNOS
        // Warp to level
        omm_opt_init_warp_to_level();
        struct Option optWarpToLevel =
            omm_opt_make_submenu(OMM_TEXT_OPT_WARP_TO_LEVEL_LABEL, OMM_TEXT_OPT_WARP_TO_LEVEL_TITLE, options(
                omm_opt_make_choice_level(OMM_TEXT_OPT_WARP_TO_LEVEL_LEVEL, &sOmmWarpLevelNum),
                omm_opt_make_choice_area(OMM_TEXT_OPT_WARP_TO_LEVEL_AREA, &sOmmWarpAreaIndex),
                omm_opt_make_choice_act(OMM_TEXT_OPT_WARP_TO_LEVEL_ACT, &sOmmWarpActNum),
                omm_opt_make_button(OMM_TEXT_OPT_WARP_TO_LEVEL_WARP, (void (*)(struct Option *, s32)) omm_opt_warp_to_level),
            ), 4);
        gOmmOptWarpToLevel.label = mem_dup(optWarpToLevel.label, omm_opt_text_length(optWarpToLevel.label) + 1);
        gOmmOptWarpToLevel.subMenu = mem_dup(optWarpToLevel.nextMenu, sizeof(struct SubMenu));
        omm_add_routine(OMM_ROUTINE_TYPE_UPDATE, omm_opt_update_warp_to_level);

        // Return to main menu
        u8 *optReturnToMainMenuLabel = omm_opt_text(OMM_TEXT_OPT_RETURN_TO_MAIN_MENU_LABEL, false);
        gOmmOptReturnToMainMenu.label = mem_dup(optReturnToMainMenuLabel, omm_opt_text_length(optReturnToMainMenuLabel) + 1);
        gOmmOptReturnToMainMenu.subMenu = NULL;

        // Model packs
        const char **packs = omm_models_init();
        if (packs) {
            s32 numPacks = 0; for (const char **p = packs; *p; ++p, ++numPacks);
            struct Option *optPacks = mem_new(struct Option, numPacks);
            gOmmOptModelsEnabled = mem_new(bool *, numPacks + 1);
            for (s32 i = 0; i != numPacks; ++i) {
                gOmmOptModelsEnabled[i] = mem_new(bool, 1);
                optPacks[i] = omm_opt_make_toggle(packs[i], gOmmOptModelsEnabled[i]);
            }
            struct Option optModels = omm_opt_make_submenu(OMM_TEXT_OPT_MODELS_LABEL, OMM_TEXT_OPT_MODELS_TITLE, optPacks, numPacks);
            gOmmOptModels.label = mem_dup(optModels.label, omm_opt_text_length(optModels.label) + 1);
            gOmmOptModels.subMenu = mem_dup(optModels.nextMenu, sizeof(struct SubMenu));
            gOmmOptModelsEnabled[numPacks] = NULL;
        } else {
            gOmmOptModels.label = NULL;
            gOmmOptModels.subMenu = NULL;
            gOmmOptModelsEnabled = NULL;
        }
#endif

        // Controls
#ifdef TOUCH_CONTROLS
        struct Option optTouchControls = omm_opt_make_submenu(OMM_TEXT_OPT_TOUCH_CONTROLS_TITLE, OMM_TEXT_OPT_TOUCH_CONTROLS_TITLE, options(
                omm_opt_make_button(OMM_TEXT_OPT_TOUCH_CONTROLS_TOUCH_BINDS, (void (*)(struct Option *, s32)) omm_opt_enter_touch_control_config),
                omm_opt_make_toggle(OMM_TEXT_OPT_TOUCH_CONTROLS_SLIDE_TOUCH, &configSlideTouch),
            ), 2);
        gOmmOptTouchControls.label = mem_dup(optTouchControls.label, omm_opt_text_length(optTouchControls.label) + 1);
        gOmmOptTouchControls.subMenu = mem_dup(optTouchControls.nextMenu, sizeof(struct SubMenu));
#endif
        struct Option optControls =
            omm_opt_make_submenu(OMM_TEXT_OPT_CONTROLS_TITLE, OMM_TEXT_OPT_CONTROLS_TITLE, options(
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_A_BUTTON, gOmmControlsButtonA),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_B_BUTTON, gOmmControlsButtonB),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_X_BUTTON, gOmmControlsButtonX),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_Y_BUTTON, gOmmControlsButtonY),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_START_BUTTON, gOmmControlsButtonStart),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_SPIN_BUTTON, gOmmControlsButtonSpin),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_L_TRIGGER, gOmmControlsTriggerL),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_R_TRIGGER, gOmmControlsTriggerR),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_Z_TRIGGER, gOmmControlsTriggerZ),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_C_UP, gOmmControlsCUp),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_C_DOWN, gOmmControlsCDown),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_C_LEFT, gOmmControlsCLeft),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_C_RIGHT, gOmmControlsCRight),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_D_UP, gOmmControlsDUp),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_D_DOWN, gOmmControlsDDown),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_D_LEFT, gOmmControlsDLeft),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_D_RIGHT, gOmmControlsDRight),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_STICK_UP, gOmmControlsStickUp),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_STICK_DOWN, gOmmControlsStickDown),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_STICK_LEFT, gOmmControlsStickLeft),
                omm_opt_make_bind(OMM_TEXT_OPT_CONTROLS_STICK_RIGHT, gOmmControlsStickRight),
                omm_opt_make_scroll(OMM_TEXT_OPT_CONTROLS_STICK_DEADZONE, &configStickDeadzone, 0, 100, 1),
                omm_opt_make_button(OMM_TEXT_OPT_CONTROLS_RESET, (void (*)(struct Option *, s32)) omm_opt_reset_controls),
            ), 23);
        gOmmOptControls.label = mem_dup(optControls.label, omm_opt_text_length(optControls.label) + 1);
        gOmmOptControls.subMenu = mem_dup(optControls.nextMenu, sizeof(struct SubMenu));

        // Cheats
#if !OMM_GAME_IS_R96X
        struct Option optCheats =
            omm_opt_make_submenu(OMM_TEXT_OPT_CHEATS_TITLE, OMM_TEXT_OPT_CHEATS_TITLE, options(
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_ENABLE, &gOmmCheatEnable),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_MOON_JUMP, &gOmmCheatMoonJump),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_GOD_MODE, &gOmmCheatGodMode),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_INVINCIBLE, &gOmmCheatInvincible),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_SUPER_SPEED, &gOmmCheatSuperSpeed),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_SUPER_RESPONSIVE, &gOmmCheatSuperResponsive),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_NO_FALL_DAMAGE, &gOmmCheatNoFallDamage),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_CAP_MODIFIER, &gOmmCheatCapModifier),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_WALK_ON_LAVA, &gOmmCheatWalkOnLava),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_WALK_ON_QUICKSAND, &gOmmCheatWalkOnQuicksand),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_WALK_ON_WATER, &gOmmCheatWalkOnWater),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_WALK_ON_GAS, &gOmmCheatWalkOnGas),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_WALK_ON_SLOPE, &gOmmCheatWalkOnSlope),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_WALK_ON_DEATH_BARRIER, &gOmmCheatWalkOnDeathBarrier),
                omm_opt_make_toggle(OMM_TEXT_OPT_CHEAT_BLJ_ANYWHERE, &gOmmCheatBljAnywhere),
            ), 15);
        gOmmOptCheats.label = mem_dup(optCheats.label, omm_opt_text_length(optCheats.label) + 1);
        gOmmOptCheats.subMenu = mem_dup(optCheats.nextMenu, sizeof(struct SubMenu));
#endif

        // Edit the options menu to include OMM sub-menus
        extern void omm_opt_init_main_menu();
        omm_opt_init_main_menu();
        inited = true;
    }
}

//
// Options shortcuts
//

#if !OMM_CODE_DYNOS
// This code updates the OMM menu each time something new is unlocked (Sparkly Stars, Peach...)
// It has no effect on DynOS, so there is no point to run this code if DynOS is installed
OMM_ROUTINE_UPDATE(omm_opt_update_menu) {
    static s32 sPrevState = -1;
    if (!omm_is_game_paused()) {
        s32 state = omm_opt_get_state();
        if (state != sPrevState) {
            mem_cpy(gOmmOptMenu.subMenu, sOmmOptMenus[state].nextMenu, sizeof(struct SubMenu));
            sPrevState = state;
        }
    }
}
#endif

OMM_ROUTINE_PRE_RENDER(omm_opt_update_shortcuts) {
    static const char *sToggleStrings[] = { OMM_TEXT_OPT_DISABLED, OMM_TEXT_OPT_ENABLED };
    static const char *sOptionLabel = NULL;
    static const char **sOptionStrings = NULL;
    static uintptr_t sOption = 0;
    static u8 *sDisplayStrings[4] = { NULL, NULL, NULL, NULL };
    static s32 sDisplayColor = 0;
    static s32 sDisplayTimer = 0;

    // Check shortcuts and change the corresponding option if pressed
    if (!omm_is_main_menu() && !optmenu_open && gMarioObject) {
        bool changed = false;
#if !OMM_GAME_IS_R96X
        gKeyPressed = controller_get_raw_key();
#endif
        if (!omm_is_game_paused() && gKeyPressed != VK_INVALID) {
            for (s32 i = 0; i != array_length(sOmmOptShortcuts); ++i) {
                const OmmOptShortcut *sc = &sOmmOptShortcuts[i];
                for (s32 j = 0; j != MAX_BINDS; ++j) {
                    if (gKeyPressed == sc->binds[j]) {
                        switch (sc->type) {
                            case OPT_TOGGLE: {
                                *sc->toggle.option = !(*sc->toggle.option);
                                sOptionLabel = sc->label;
                                sOptionStrings = (const char **) sToggleStrings;
                                sOption = 0 + (((uintptr_t) sc->toggle.option) << 1);
                            } break;

                            case OPT_CHOICE: {
                                *sc->choice.option = (*sc->choice.option + 1) % (*sc->choice.numChoices);
                                sOptionLabel = sc->label;
                                sOptionStrings = (const char **) sc->choice.choices;
                                sOption = 1 + (((uintptr_t) sc->choice.option) << 1);
                            } break;
                        }
                        changed = true;
                        break;
                    }
                }
            }
        }

        // Must return now if the values changed, to let them update properly before displaying the message box
        if (changed) {
            return;
        }
    }

    // Create the strings to display and reset the timer
    if (sOption && sOptionLabel && sOptionStrings) {
        omm_save_file_do_save();
        play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gGlobalSoundArgs);
        bool isChoice = (sOption & 1);
        u32 opt = (isChoice ? *((u32 *) (sOption >> 1)) : (u32) *((bool *) (sOption >> 1)));
        mem_del(sDisplayStrings[0]);
        mem_del(sDisplayStrings[1]);
        mem_del(sDisplayStrings[2]);
        mem_del(sDisplayStrings[3]);
        sDisplayStrings[0] = omm_text_convert(OMM_TEXT_OPT_SHORTCUTS_OPTION, true);
        sDisplayStrings[1] = omm_text_convert(sOptionLabel, true);
        sDisplayStrings[2] = omm_text_convert(OMM_TEXT_OPT_SHORTCUTS_SET_TO, true);
        sDisplayStrings[3] = omm_text_convert(sOptionStrings[opt], true);
        sDisplayColor = (isChoice ? 1 : (opt ? 2 : 3));
        sDisplayTimer = 60;
        sOptionLabel = NULL;
        sOptionStrings = NULL;
        sOption = 0;
    }

    // Display the strings
    if (sDisplayTimer-- > 0) {
        Gfx *start = gDisplayListHead;
        gSPDisplayList(gDisplayListHead++, NULL);
        s32 w = omm_render_get_string_width(sDisplayStrings[0]) +
                omm_render_get_string_width(sDisplayStrings[1]) +
                omm_render_get_string_width(sDisplayStrings[2]) +
                omm_render_get_string_width(sDisplayStrings[3]);
        s32 x = (SCREEN_WIDTH - w) / 2;

        // Black box
        static Vtx sOmmHudBlackBoxVertices[4];
        sOmmHudBlackBoxVertices[0] = (Vtx) { { { -(w + 8) / 2, -8, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x60 } } };
        sOmmHudBlackBoxVertices[1] = (Vtx) { { { -(w + 8) / 2, +8, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x60 } } };
        sOmmHudBlackBoxVertices[2] = (Vtx) { { { +(w + 8) / 2, -8, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x60 } } };
        sOmmHudBlackBoxVertices[3] = (Vtx) { { { +(w + 8) / 2, +8, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x60 } } };
        create_dl_translation_matrix(MENU_MTX_PUSH, SCREEN_WIDTH / 2, 27, 0);
        gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gSPVertex(gDisplayListHead++, sOmmHudBlackBoxVertices, 4, 0);
        gSP2Triangles(gDisplayListHead++, 2, 1, 0, 0, 1, 2, 3, 0);
        gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

        // Strings
        s32 colors[4] = { 0, 1, 0, sDisplayColor };
        for (s32 i = 0; i != 4; ++i) {
            u8 r, g, b;
            switch (colors[i]) {
                case 0: r = 0x00, g = 0xE0, b = 0xFF; break;
                case 1: r = 0xFF, g = 0xFF, b = 0xFF; break;
                case 2: r = 0x20, g = 0xE0, b = 0x20; break;
                case 3: r = 0xFF, g = 0x20, b = 0x20; break;
            }
            omm_render_string(x + 1, 22, r / 4, g / 4, b / 4, 0xFF, sDisplayStrings[i], 0);
            omm_render_string(x, 23, r, g, b, 0xFF, sDisplayStrings[i], 0); 
            x += omm_render_get_string_width(sDisplayStrings[i]);
        }
        gSPEndDisplayList(gDisplayListHead++);
        gSPDisplayList(start, gDisplayListHead);
    }
}
