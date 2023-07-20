#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#include "behavior_commands.h"

static Scroll sOmmStatsBoardScroll = { 0 };
InterpData sOmmStatsBoard[1];
Gfx sOmmStatsBoardGfx[0x1000];

#define OMM_STATS_BOARD_FADE 10
#define OMM_STATS_BOARD_WIDTH 240
#define OMM_STATS_BOARD_HEIGHT 152

//
// Render
//

static void bhv_omm_stats_board_render_time(f32 t, s16 bgx, s16 bgw, s16 dx, s16 y, u64 value) {
    u64 ms = (1000llu * value) / 30llu;
    u64 s = (ms / 1000llu) % 60;
    u64 m = (ms / 60000llu) % 60;
    u64 h = (ms / 3600000llu);
    str_fmt_sa(valueStr, 32, "%llu:%02llu:%02llu", h, m, s);
    u8 *valueStr64 = omm_text_convert(valueStr, false);
    s16 x = bgx + bgw - (8 + dx + omm_render_get_string_width(valueStr64)) * t;
    omm_render_string_sized(x, y, 8 * t, 8 * t, 0xFF, 0xFF, 0x80, 0xFF * t, valueStr64, false);
    for (u8 *c = valueStr64; *c != 0xFF; ++c) {
        if (*c >= 1 && *c <= 9) {
            *c = 0xFF;
            break;
        }
    }
    omm_render_string_sized(x, y, 8 * t, 8 * t, 0x40, 0x40, 0x40, 0xFF * t, valueStr64, false);
}

Gfx *omm_render_stats_board(Gfx *pos, f32 t) {
    gSPDisplayList(pos, pos + 3);
    gSPDisplayList(pos + 1, sOmmStatsBoardGfx);
    gSPEndDisplayList(pos + 2);
    pos += 3;

    // Render board
    Gfx *displayListHead = gDisplayListHead;
    gDisplayListHead = sOmmStatsBoardGfx;
    if (t > 0) {

        // Black background
        s16 bgw = OMM_STATS_BOARD_WIDTH * t;
        s16 bgh = OMM_STATS_BOARD_HEIGHT * t;
        s16 bgx = (SCREEN_WIDTH - bgw) / 2;
        s16 bgy = (SCREEN_HEIGHT - bgh) / 2;
        omm_render_texrect(bgx, bgy, bgw, bgh, 32, 32, 0x00, 0x00, 0x00, 0xC0 * t, OMM_TEXTURE_MISC_WHITE, false);

        // White arrows
        s16 aw = 8 * t;
        s16 ah = 8 * t;
        s16 axl = (SCREEN_WIDTH) / 2 - aw;
        s16 axr = (SCREEN_WIDTH) / 2;
        s16 ayu = (SCREEN_HEIGHT + bgh) / 2 + 4 * t;
        s16 ayd = (SCREEN_HEIGHT - bgh) / 2 - 4 * t - ah;
        omm_render_texrect(axl, ayu, aw, ah, 32, 32, 0xFF, 0xFF, 0xFF, 0xFF * t, OMM_TEXTURE_MISC_WHITE_DOWN_RIGHT, false);
        omm_render_texrect(axr, ayu, aw, ah, 32, 32, 0xFF, 0xFF, 0xFF, 0xFF * t, OMM_TEXTURE_MISC_WHITE_DOWN_LEFT, false);
        omm_render_texrect(axl, ayd, aw, ah, 32, 32, 0xFF, 0xFF, 0xFF, 0xFF * t, OMM_TEXTURE_MISC_WHITE_UP_RIGHT, false);
        omm_render_texrect(axr, ayd, aw, ah, 32, 32, 0xFF, 0xFF, 0xFF, 0xFF * t, OMM_TEXTURE_MISC_WHITE_UP_LEFT, false);

        // Current page
        switch (sOmmStatsBoardScroll.idx) {
            case 0:
            case 1: {
                const struct { const char *name; u64 value; } sOmmStatsBoardSinglePages[2][9] = {{
                    { OMM_TEXT_STATS_STARS, gOmmStats->starsCollected },
                    { OMM_TEXT_STATS_SPARKLY_STARS, gOmmStats->sparklyStarsCollected },
                    { OMM_TEXT_STATS_COINS, gOmmStats->coinsCollected },
                    { OMM_TEXT_STATS_CAPS, gOmmStats->capsCollected },
                    { OMM_TEXT_STATS_MUSHROOMS_1UP, gOmmStats->mushrooms1upCollected },
                    { OMM_TEXT_STATS_SECRETS, gOmmStats->secretsCollected },
                    { OMM_TEXT_STATS_EXCLAMATION_BOXES, gOmmStats->exclamationBoxesBroken },
                    { OMM_TEXT_STATS_ENEMIES, gOmmStats->enemiesDefeated },
                    { OMM_TEXT_STATS_BOWSERS, gOmmStats->bowsersDefeated },
                }, {
                    { OMM_TEXT_STATS_A_PRESSES, gOmmStats->aPresses },
                    { OMM_TEXT_STATS_JUMPS, gOmmStats->jumps },
                    { OMM_TEXT_STATS_ATTACKS, gOmmStats->attacks },
                    { OMM_TEXT_STATS_CAPPY_THROWS, gOmmStats->cappyThrows },
                    { OMM_TEXT_STATS_CAPPY_BOUNCES, gOmmStats->cappyBounces },
                    { OMM_TEXT_STATS_CAPTURES, gOmmStats->captures },
                    { OMM_TEXT_STATS_HITS_TAKEN, gOmmStats->hitsTaken },
                    { OMM_TEXT_STATS_RESTARTS, gOmmStats->restarts },
                    { OMM_TEXT_STATS_DEATHS, gOmmStats->deaths },
                }};
                for (s32 i = 0; i != 9; ++i) {
                    str_fmt_sa(valueStr, 32, "%llu", sOmmStatsBoardSinglePages[sOmmStatsBoardScroll.idx][i].value);
                    const u8 *nameStr64 = omm_text_convert(sOmmStatsBoardSinglePages[sOmmStatsBoardScroll.idx][i].name, false);
                    const u8 *valueStr64 = omm_text_convert(valueStr, false);
                    s16 xl = bgx + 8 * t;
                    s16 xr = bgx + bgw - (8 + omm_render_get_string_width(valueStr64)) * t;
                    s16 y = bgy + bgh - (i + 1) * 16 * t;
                    omm_render_string_sized(xl, y, 8 * t, 8 * t, 0xFF, 0xFF, 0xFF, 0xFF * t, nameStr64, false);
                    omm_render_string_sized(xr, y, 8 * t, 8 * t, 0xFF, 0xFF, 0x80, 0xFF * t, valueStr64, false);
                }
            } break;

            case 2:
            case 3: {
                const struct { const char *name; u64 *values; } sOmmStatsBoardDoublePages[2][7] = {{
                    { OMM_TEXT_STATS_TOTAL, gOmmStats->distanceTotal },
                    { OMM_TEXT_STATS_ON_GROUND, gOmmStats->distanceOnGround },
                    { OMM_TEXT_STATS_AIRBORNE, gOmmStats->distanceAirborne },
                    { OMM_TEXT_STATS_UNDERWATER, gOmmStats->distanceUnderwater },
                    { OMM_TEXT_STATS_WING_CAP, gOmmStats->distanceWingCap },
                    { OMM_TEXT_STATS_METAL_CAP, gOmmStats->distanceMetalCap },
                    { OMM_TEXT_STATS_VANISH_CAP, gOmmStats->distanceVanishCap },
                }, {
                    { OMM_TEXT_STATS_TOTAL, gOmmStats->timeTotal },
                    { OMM_TEXT_STATS_ON_GROUND, gOmmStats->timeOnGround },
                    { OMM_TEXT_STATS_AIRBORNE, gOmmStats->timeAirborne },
                    { OMM_TEXT_STATS_UNDERWATER, gOmmStats->timeUnderwater },
                    { OMM_TEXT_STATS_WING_CAP, gOmmStats->timeWingCap },
                    { OMM_TEXT_STATS_METAL_CAP, gOmmStats->timeMetalCap },
                    { OMM_TEXT_STATS_VANISH_CAP, gOmmStats->timeVanishCap },
                }};
                bool isTime = (sOmmStatsBoardScroll.idx == 3);
                const u8 *titleStr64 = omm_text_convert(isTime ? OMM_TEXT_STATS_TIME : OMM_TEXT_STATS_DISTANCE, false);
                const u8 *column0Str64 = omm_text_convert(OMM_TEXT_STATS_MARIO, false);
                const u8 *column1Str64 = omm_text_convert(OMM_TEXT_STATS_CAPTURE, false);
                s16 xl = bgx + 8 * t;
                s16 xr0 = bgx + bgw - (8 + OMM_STATS_BOARD_WIDTH / 3 + omm_render_get_string_width(column0Str64)) * t;
                s16 xr1 = bgx + bgw - (8 + omm_render_get_string_width(column1Str64)) * t;
                s16 yt0 = bgy + bgh - 16 * t;
                s16 yt1 = bgy + bgh - 32 * t;
                omm_render_string_sized(xl, yt0, 8 * t, 8 * t, 0xFF, 0xFF, 0xFF, 0xFF * t, titleStr64, false);
                omm_render_string_sized(xr0, yt1, 8 * t, 8 * t, 0xFF, 0xFF, 0xFF, 0xFF * t, column0Str64, false);
                omm_render_string_sized(xr1, yt1, 8 * t, 8 * t, 0xFF, 0xFF, 0xFF, 0xFF * t, column1Str64, false);
                for (s32 i = 0; i != 7; ++i) {
                    const u8 *nameStr64 = omm_text_convert(sOmmStatsBoardDoublePages[isTime][i].name, false);
                    s16 xl = bgx + 8 * t;
                    s16 y = bgy + bgh - (i + 3) * 16 * t;
                    omm_render_string_sized(xl, y, 8 * t, 8 * t, 0xFF, 0xFF, 0xFF, 0xFF * t, nameStr64, false);
                    u64 value0 = sOmmStatsBoardDoublePages[isTime][i].values[0];
                    u64 value1 = sOmmStatsBoardDoublePages[isTime][i].values[1];
                    if (isTime) {
                        bhv_omm_stats_board_render_time(t, bgx, bgw, OMM_STATS_BOARD_WIDTH / 3, y, value0);
                        if (i < 4) { // Don't display cap values for captures
                            bhv_omm_stats_board_render_time(t, bgx, bgw, 0, y, value1);
                        }
                    } else {
                        f64 toMeters = 1.55 / 120.0; // Mario's human height (155 cm) / Mario's model height (120 u)
                        str_fmt_sa(value0Str, 32, "%llu", (u64) (value0 * toMeters));
                        str_fmt_sa(value1Str, 32, "%llu", (u64) (value1 * toMeters));
                        const u8 *value0Str64 = omm_text_convert(value0Str, false);
                        const u8 *value1Str64 = omm_text_convert(value1Str, false);
                        s16 xr0 = bgx + bgw - (8 + OMM_STATS_BOARD_WIDTH / 3 + omm_render_get_string_width(value0Str64)) * t;
                        s16 xr1 = bgx + bgw - (8 + omm_render_get_string_width(value1Str64)) * t;
                        omm_render_string_sized(xr0, y, 8 * t, 8 * t, 0xFF, 0xFF, 0x80, 0xFF * t, value0Str64, false);
                        if (i < 4) { // Don't display cap values for captures
                            omm_render_string_sized(xr1, y, 8 * t, 8 * t, 0xFF, 0xFF, 0x80, 0xFF * t, value1Str64, false);
                        }
                    }
                }
            } break;
        }
    }
    gSPEndDisplayList(gDisplayListHead++);
    gDisplayListHead = displayListHead;
    return pos;
}

OMM_ROUTINE_PRE_RENDER(bhv_omm_stats_board_render) {
    struct Object *activeBoard = obj_get_first_with_behavior_and_field_s32(bhvOmmStatsBoard, _FIELD(oAction), 1);
    if (activeBoard) {

        // Interpolate and render
        f32 t = sqr_f(relerp_0_1_f(abs_f(activeBoard->oSubAction), 0, OMM_STATS_BOARD_FADE, 0, 1));
        interp_data_update(sOmmStatsBoard, t != 0, gDisplayListHead, 0, 0, 0, 0, 0, t);
        gDisplayListHead = omm_render_stats_board(gDisplayListHead, sOmmStatsBoard->t0);

        // Inputs
        if (t == 1) {

            // Change the current page
            omm_render_update_scroll(&sOmmStatsBoardScroll, 4, -gPlayer1Controller->stickY);

            // Close the board
            if (gPlayer1Controller->buttonPressed & (A_BUTTON | B_BUTTON)) {
                activeBoard->oSubAction = -OMM_STATS_BOARD_FADE;
                return;
            }
        }
    }
}

//
// Behavior
//

static void bhv_omm_stats_board_update() {
    struct Object *o = gCurrentObject;
    o->activeFlags |= ACTIVE_FLAG_INITIATED_TIME_STOP;
    if (o->oAction == 1 && ++(o->oSubAction) == 0) {
        o->oAction = 0;
        mem_clr(&sOmmStatsBoardScroll, sizeof(sOmmStatsBoardScroll));
    }
}

const BehaviorScript bhvOmmStatsBoard[] = {
    OBJ_TYPE_SURFACE,
    BHV_OR_INT(oFlags, OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE),
    BHV_LOAD_COLLISION_DATA(wooden_signpost_seg3_collision_0302DD80),
    BHV_SET_INTERACT_TYPE(INTERACT_TEXT),
    BHV_SET_INT(oInteractionSubtype, INT_SUBTYPE_SIGN),
    BHV_DROP_TO_FLOOR(),
    BHV_SET_HITBOX(150, 80),
    BHV_BEGIN_LOOP(),
        BHV_SET_INT(oIntangibleTimer, 0),
        BHV_CALL_NATIVE(load_object_collision_model),
        BHV_CALL_NATIVE(bhv_omm_stats_board_update),
        BHV_SET_INT(oInteractStatus, 0),
    BHV_END_LOOP(),
};

//
// Spawner
//

struct Object *omm_spawn_stats_board(struct Object *o, f32 x, f32 y, f32 z, s16 yaw) {
    struct Object *board = obj_spawn_from_geo(o, wooden_signpost_geo, bhvOmmStatsBoard);
    board->activeFlags |= ACTIVE_FLAG_INITIATED_TIME_STOP;
    obj_set_pos(board, x, y, z);
    obj_set_angle(board, 0, yaw, 0);
    obj_set_scale(board, 1, 1, 1);
    board->oAction = 0;
    board->oSubAction = 0;
    return board;
}
