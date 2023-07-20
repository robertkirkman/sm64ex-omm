#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#include "level_commands.h"
void bhv_act_selector_init(void) {}
void bhv_act_selector_loop(void) {}
void bhv_act_selector_star_type_loop(void) {}
Gfx *geo_act_selector_strings(UNUSED s16 a, UNUSED struct GraphNode *b, UNUSED void *c) { return NULL; }

//
// Data
//

struct Object *sOmmStarSelectStars[OMM_NUM_ACTS_MAX_PER_COURSE];
Scroll sOmmStarSelectScrollV = { 0 };
Scroll sOmmStarSelectScrollH = { 0 };
f32 sOmmStarSelectCurrent = 0;
s32 sOmmStarSelectTarget = 0;
s32 sOmmStarSelectCourse = 0;
s32 sOmmStarSelectTimer = 0;
s32 sOmmStarSelectAlpha = 0;

//
// Render
//

static s32 omm_star_select_get_string_width(const u8 *str64, s32 glyphSize, s32 spacing) {
    s32 width = -spacing;
    for (; *str64 != 0xFF; ++str64) {
        width += spacing;
        switch (*str64) {
            case DIALOG_CHAR_SPACE: width +=  8; break;
            case DIALOG_CHAR_COMMA: width +=  8; break;
            case 0x3E:              width +=  8; break;
            default:                width += 12; break;
        }
    }
    return ((width * glyphSize) / 16);
}

static void omm_star_select_render_background_rect(s32 x0, s32 y0, u8 r0, u8 g0, u8 b0, u8 a0, s32 x1, s32 y1, u8 r1, u8 g1, u8 b1, u8 a1, bool vrt) {
    Vtx *vtx = alloc_display_list(sizeof(Vtx) * 4);
    vtx[0] = (Vtx) { { { x0, y0, 0 }, 0, { 0, 0 }, { r0, g0, b0, (a0              * sOmmStarSelectAlpha) / 0xFF } } };
    vtx[1] = (Vtx) { { { x1, y0, 0 }, 0, { 0, 0 }, { r0, g0, b0, ((vrt ? a0 : a1) * sOmmStarSelectAlpha) / 0xFF } } };
    vtx[2] = (Vtx) { { { x1, y1, 0 }, 0, { 0, 0 }, { r1, g1, b1, (a1              * sOmmStarSelectAlpha) / 0xFF } } };
    vtx[3] = (Vtx) { { { x0, y1, 0 }, 0, { 0, 0 }, { r1, g1, b1, ((vrt ? a1 : a0) * sOmmStarSelectAlpha) / 0xFF } } };
    omm_render_create_dl_ortho_matrix();
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);;
    gSPVertex(gDisplayListHead++, vtx, 4, 0);
    gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0x0, 0, 2, 3, 0x0);
    gSPSetGeometryMode(gDisplayListHead++, G_LIGHTING);
}

static void omm_star_select_render_string(const u8 *str64, s32 x, s32 y, s32 glyphSize, s32 spacing, bool outline, bool shadow) {
    for (; *str64 != 0xFF; ++str64) {
        s32 dw = 0;
        s32 dx = 0;
        s32 dy = 0;
        const void *texture = NULL;
        switch (*str64) {
            case DIALOG_CHAR_SPACE: { dw =  8; dx =  0; dy =  0; texture = NULL; } break;
            case DIALOG_CHAR_COMMA: { dw =  8; dx = -2; dy = -6; texture = gOmmFontHud[GLYPH_APOSTROPHE]; } break;
            case 0x3E:              { dw =  8; dx = -2; dy = +6; texture = gOmmFontHud[GLYPH_APOSTROPHE]; } break;
            default:                { dw = 12; dx =  0; dy =  0; texture = gOmmFontHud[*str64]; } break;
        }
        dw += spacing;
        dw = ((dw * glyphSize) / 16);
        dx = ((dx * glyphSize) / 16);
        dy = ((dy * glyphSize) / 16);
        if (texture) {
            if (shadow) {
            omm_render_glyph(x + dx + 3, y + dy - 3, glyphSize, glyphSize, 0x20, 0x20, 0x20, sOmmStarSelectAlpha / 2, texture, false);
            }
            if (outline) {
            omm_render_glyph(x + dx + 1, y + dy,     glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, texture, false);
            omm_render_glyph(x + dx - 1, y + dy,     glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, texture, false);
            omm_render_glyph(x + dx,     y + dy + 1, glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, texture, false);
            omm_render_glyph(x + dx,     y + dy - 1, glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, texture, false);
            }
            omm_render_glyph(x + dx,     y + dy,     glyphSize, glyphSize, 0xFF, 0xFF, 0xFF, sOmmStarSelectAlpha, texture, false);
        }
        x += dw;
    }
}

static void omm_star_select_render() {
    sOmmStarSelectAlpha = (clamp_s(sOmmStarSelectTimer, 0, 15) * 17);
    u8 starFlags = omm_save_file_get_star_flags(gCurrSaveFileNum - 1, OMM_GAME_MODE, gCurrCourseNum - 1);
    s32 selectedIndex = ((sOmmStarSelectTarget + OMM_NUM_ACTS_MAX_PER_COURSE) % OMM_NUM_ACTS_MAX_PER_COURSE);
    u32 color = (OMM_GAME_IS_SMMS ? 0x00F000FF : (OMM_EXTRAS_COLORED_STARS ? omm_stars_get_color(gCurrLevelNum, OMM_GAME_MODE) : 0xFFFFFFFF));
    u8 r = (color >> 24) & 0xFF;
    u8 g = (color >> 16) & 0xFF;
    u8 b = (color >>  8) & 0xFF;

    // Background
    omm_star_select_render_background_rect(GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), 0, r, g, b, 0xFF, SCREEN_WIDTH / 2, SCREEN_HEIGHT, r, g, b, 0, false);
    omm_star_select_render_background_rect(SCREEN_WIDTH / 2, 0, r, g, b, 0, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), SCREEN_HEIGHT, r, g, b, 0xFF, false);
    omm_star_select_render_background_rect(GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y0, r, g, b, 0xFF, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1, r / 2, g / 2, b / 2, 0xFF, true);
    omm_star_select_render_background_rect(GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_TOP_Y0, r / 2, g / 2, b / 2, 0xFF, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_TOP_Y1, r, g, b, 0xFF, true);

    // Course name
    const u8 *courseName = omm_level_get_course_name(gCurrLevelNum, OMM_GAME_MODE, false, false);
    s32 length = omm_text_length(courseName);
    s32 courseNum = omm_level_get_course(gCurrLevelNum);
    u8 buffer[64];
    if (courseNum < 10) {
        buffer[0] = courseNum;
        buffer[1] = DIALOG_CHAR_SPACE;
        buffer[2] = 0x9F;
        buffer[3] = DIALOG_CHAR_SPACE;
        mem_cpy(&buffer[4], courseName, length + 1);
    } else {
        buffer[0] = courseNum / 10;
        buffer[1] = courseNum % 10;
        buffer[2] = DIALOG_CHAR_SPACE;
        buffer[3] = 0x9F;
        buffer[4] = DIALOG_CHAR_SPACE;
        mem_cpy(&buffer[5], courseName, length + 1);
    }
    omm_text_capitalize(buffer);
    omm_star_select_render_string(buffer, (SCREEN_WIDTH - omm_star_select_get_string_width(buffer, 16, 1)) / 2, OMM_RENDER_STAR_SELECT_COURSE_NAME_Y, 16, 1, true, true);
    
    // Act text
    u8 *actText = omm_text_convert(OMM_TEXT_ACT, false);
    s32 actTextW = omm_star_select_get_string_width(actText, 12, 2);
    s32 actTextX = (SCREEN_WIDTH - (actTextW + 18)) / 2;
    s32 actTextY = OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1 - 6;
    omm_star_select_render_string(actText, actTextX, actTextY, 12, 2, true, true);

    // Act number
    u8 actNumber[] = { selectedIndex + 1, 0xFF };
    s32 actNumberX = actTextX + actTextW + 6;
    s32 actNumberY = OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1 - 8;
    omm_star_select_render_string(actNumber, actNumberX, actNumberY, 16, 0, true, true);

    // Act name
    u8 *actName = omm_level_get_act_name(gCurrLevelNum, selectedIndex + 1, OMM_GAME_MODE, false, false);
    s32 actNameX = (SCREEN_WIDTH - omm_render_get_string_width(actName)) / 2;
    omm_render_string(actNameX - 1, OMM_RENDER_STAR_SELECT_ACT_NAME_Y,     0x00, 0x00, 0x00, sOmmStarSelectAlpha, actName, false);
    omm_render_string(actNameX + 1, OMM_RENDER_STAR_SELECT_ACT_NAME_Y,     0x00, 0x00, 0x00, sOmmStarSelectAlpha, actName, false);
    omm_render_string(actNameX,     OMM_RENDER_STAR_SELECT_ACT_NAME_Y - 1, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, actName, false);
    omm_render_string(actNameX,     OMM_RENDER_STAR_SELECT_ACT_NAME_Y + 1, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, actName, false);
    omm_render_string(actNameX,     OMM_RENDER_STAR_SELECT_ACT_NAME_Y,     0xFF, 0xFF, 0xFF, sOmmStarSelectAlpha, actName, false);

    // Score text
    u8 *scoreText = omm_text_convert(OMM_TEXT_MY_SCORE, false);
    s32 scoreTextX = (SCREEN_WIDTH / 2) - (omm_render_get_string_width(scoreText) + 11);
    omm_render_string(scoreTextX - 1, OMM_RENDER_STAR_SELECT_SCORE_Y,     0x40, 0x40, 0x00, sOmmStarSelectAlpha, scoreText, false);
    omm_render_string(scoreTextX + 1, OMM_RENDER_STAR_SELECT_SCORE_Y,     0x40, 0x40, 0x00, sOmmStarSelectAlpha, scoreText, false);
    omm_render_string(scoreTextX,     OMM_RENDER_STAR_SELECT_SCORE_Y - 1, 0x40, 0x40, 0x00, sOmmStarSelectAlpha, scoreText, false);
    omm_render_string(scoreTextX,     OMM_RENDER_STAR_SELECT_SCORE_Y + 1, 0x40, 0x40, 0x00, sOmmStarSelectAlpha, scoreText, false);
    omm_render_string(scoreTextX,     OMM_RENDER_STAR_SELECT_SCORE_Y,     0xFF, 0xFF, 0x00, sOmmStarSelectAlpha, scoreText, false);

    // Coin score
    s32 scoreCoins = omm_save_file_get_course_coin_score(gCurrSaveFileNum - 1, OMM_GAME_MODE, gCurrCourseNum - 1);
    s32 scoreCoinsX = (SCREEN_WIDTH / 2) - 7;
    omm_render_hud_coins(scoreCoinsX, OMM_RENDER_STAR_SELECT_SCORE_Y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmStarSelectAlpha, scoreCoins, false);

    // 100 coins star
    if (starFlags & 0x40) {
        s32 coinsStarX = OMM_RENDER_STAR_SELECT_100_COINS_STAR_X;
        s32 coinsStarY = OMM_RENDER_STAR_SELECT_SCORE_Y - ((OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE - 8) / 2) + 1;
        const void *coinsStarTex = omm_render_get_star_glyph(clamp_s(gCurrCourseNum, 0, 16), OMM_EXTRAS_COLORED_STARS);
        omm_render_glyph(coinsStarX + 1, coinsStarY,     OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, coinsStarTex, false);
        omm_render_glyph(coinsStarX - 1, coinsStarY,     OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, coinsStarTex, false);
        omm_render_glyph(coinsStarX,     coinsStarY + 1, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, coinsStarTex, false);
        omm_render_glyph(coinsStarX,     coinsStarY - 1, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmStarSelectAlpha, coinsStarTex, false);
        omm_render_glyph(coinsStarX,     coinsStarY,     OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0xFF, 0xFF, 0xFF, sOmmStarSelectAlpha, coinsStarTex, false);
    }

    // Time trials star time
    time_trials_render_star_select_time(selectedIndex);
}

//
// Level functions
//

static s32 omm_level_star_select_should_skip(UNUSED s32 arg, s32 levelNum) {
    if (OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE
#if OMM_GAME_IS_SMSR
        || (levelNum == LEVEL_CCM && sWarpDest.nodeId == 0x02) // Chuckya Harbor secret entrance
#endif
    ) {
        gCurrLevelNum = levelNum;
        gCurrCourseNum = omm_level_get_course(levelNum);
        gCurrActNum = 1;
#if !OMM_GAME_IS_SM74
        gCurrAreaIndex = 1;
#endif
        gDialogCourseActNum = gCurrActNum;

        // Reset the coin counter and star flags unless Mario enters a Bowser fight or returns to the Castle
        if (levelNum != LEVEL_BOWSER_1 &&
            levelNum != LEVEL_BOWSER_2 &&
            levelNum != LEVEL_BOWSER_3 &&
            levelNum != LEVEL_CASTLE   &&
            levelNum != LEVEL_GROUNDS  &&
            levelNum != LEVEL_COURT) {
            gMarioState->numCoins = 0;
            gHudDisplay.coins = 0;
            gCurrCourseStarFlags = omm_save_file_get_star_flags(gCurrSaveFileNum - 1, OMM_GAME_MODE, gCurrCourseNum - 1);
        }
        return 0;
    }
    return lvl_set_current_level(arg, levelNum);
}

static s32 omm_level_star_select_init(UNUSED s32 arg, UNUSED s32 unused) {
    gCurrAreaIndex = sWarpDest.areaIdx;
    u8 starFlags = omm_save_file_get_star_flags(gCurrSaveFileNum - 1, OMM_GAME_MODE, gCurrCourseNum - 1);
    sOmmStarSelectTimer = 0;
    sOmmStarSelectCourse = gCurrCourseNum - COURSE_MIN;
    sOmmStarSelectCurrent = 0;
    while ((sOmmStarSelectCurrent < OMM_NUM_ACTS_MAX_PER_COURSE) && (starFlags & (1 << ((s32) (sOmmStarSelectCurrent))))) sOmmStarSelectCurrent += 1;
    sOmmStarSelectCurrent = (((s32) sOmmStarSelectCurrent) % OMM_NUM_ACTS_MAX_PER_COURSE);
    sOmmStarSelectTarget = sOmmStarSelectCurrent;
    area_update_objects();
    return 0;
}

static s32 omm_level_star_select_init_objects(UNUSED s32 arg, UNUSED s32 unused) {
    s32 i = 0;
    for_each_object_with_behavior(obj, bhvOmmActSelectStar) {
        sOmmStarSelectStars[i++] = obj;
    }
#if OMM_GAME_IS_R96X
    r96_play_menu_jingle(R96_EVENT_STAR_SELECT, 1.0, 1.0, 1500);
#endif
    return 0;
}

static s32 omm_level_star_select_update(UNUSED s32 arg, UNUSED s32 unused) {

    // Update stars
    u8 starFlags = omm_save_file_get_star_flags(gCurrSaveFileNum - 1, OMM_GAME_MODE, gCurrCourseNum - 1);
    s32 selectedIndex = ((((s32) (sOmmStarSelectCurrent < 0.f ? (sOmmStarSelectCurrent - 0.5f) : (sOmmStarSelectCurrent + 0.5f))) + OMM_NUM_ACTS_MAX_PER_COURSE) % OMM_NUM_ACTS_MAX_PER_COURSE);
    for (s32 i = 0; i != OMM_NUM_ACTS_MAX_PER_COURSE; ++i) {
        f32 delta = invlerp_0_1_f(sOmmStarSelectTimer, 0, 15);
        f32 angle = ((f32) i - sOmmStarSelectCurrent) / OMM_NUM_ACTS_MAX_PER_COURSE;
        sOmmStarSelectStars[i]->oPosX = OMM_RENDER_STAR_SELECT_ACT_STAR_X + delta * OMM_RENDER_STAR_SELECT_ACT_STAR_DX * sins((angle - (1.f - delta) / 2.f) * 0x10000);
        sOmmStarSelectStars[i]->oPosY = OMM_RENDER_STAR_SELECT_ACT_STAR_Y;
        sOmmStarSelectStars[i]->oPosZ = OMM_RENDER_STAR_SELECT_ACT_STAR_Z + delta * OMM_RENDER_STAR_SELECT_ACT_STAR_DZ * coss((angle - (1.f - delta) / 2.f) * 0x10000);
        sOmmStarSelectStars[i]->oScaleX = delta;
        sOmmStarSelectStars[i]->oScaleY = delta;
        sOmmStarSelectStars[i]->oScaleZ = delta;
        sOmmStarSelectStars[i]->oGraphNode = gLoadedGraphNodes[((starFlags & (1 << i)) ? MODEL_STAR : MODEL_TRANSPARENT_STAR)];
        sOmmStarSelectStars[i]->oFaceAngleYaw = (sOmmStarSelectStars[i]->oFaceAngleYaw + 0x800) * (i == selectedIndex);
    }

    // Up-down scroll to change the course
    // A course is available only if at least 1 star has been collected
    // The course that showed up first is always available
    sOmmStarSelectScrollV.idx = gCurrCourseNum - COURSE_MIN;
    omm_render_update_scroll(&sOmmStarSelectScrollV, COURSE_STAGES_COUNT, -gPlayer1Controller->stickY);
    while (sOmmStarSelectScrollV.inc != 0) {
        s32 nextCourseIndex = (((gCurrCourseNum - COURSE_MIN) + sOmmStarSelectScrollV.inc + COURSE_STAGES_COUNT) % COURSE_STAGES_COUNT);
        if (nextCourseIndex == sOmmStarSelectCourse || (omm_save_file_get_course_star_count(gCurrSaveFileNum - 1, OMM_GAME_MODE, nextCourseIndex) != 0)) {
            gCurrLevelNum = omm_level_get_list()[nextCourseIndex];
            gCurrCourseNum = omm_level_get_course(gCurrLevelNum);
            gSavedCourseNum = gCurrCourseNum;
            sWarpDest.levelNum = gCurrLevelNum;
            sWarpDest.areaIdx = OMM_GAME_MODE + 1;
            sWarpDest.nodeId = OMM_LEVEL_ENTRY_WARP(gCurrLevelNum);
            starFlags = omm_save_file_get_star_flags(gCurrSaveFileNum - 1, OMM_GAME_MODE, gCurrCourseNum - 1);
            break;
        }
        gCurrCourseNum = nextCourseIndex + COURSE_MIN;
    }

    // Left-right scroll to change the act
    // An act is available only if its star or the star of the previous act has been collected
    // The first act is always available
    s32 targetIndex = ((sOmmStarSelectTarget + OMM_NUM_ACTS_MAX_PER_COURSE) % OMM_NUM_ACTS_MAX_PER_COURSE);
    if (targetIndex != 0 && !(starFlags & (1 << targetIndex)) && !(starFlags & (1 << (targetIndex - 1)))) {
        sOmmStarSelectScrollH.inc = -1;
    } else {
        sOmmStarSelectScrollH.idx = ((sOmmStarSelectTarget + OMM_NUM_ACTS_MAX_PER_COURSE) % OMM_NUM_ACTS_MAX_PER_COURSE);
        omm_render_update_scroll(&sOmmStarSelectScrollH, OMM_NUM_ACTS_MAX_PER_COURSE, +gPlayer1Controller->stickX);
    }
    while (sOmmStarSelectScrollH.inc != 0) {
        sOmmStarSelectTarget += sOmmStarSelectScrollH.inc;
        s32 nextTargetIndex = ((sOmmStarSelectTarget + OMM_NUM_ACTS_MAX_PER_COURSE) % OMM_NUM_ACTS_MAX_PER_COURSE);
        if (nextTargetIndex == 0 || (starFlags & (1 << nextTargetIndex)) || (starFlags & (1 << (nextTargetIndex - 1)))) {
            break;
        }
    }

    // Move smoothly the star selector
    f32 delta = clamp_f(sOmmStarSelectTarget - sOmmStarSelectCurrent, -1.f, +1.f) / 4.f;
    sOmmStarSelectCurrent += delta;
    if (abs_f(sOmmStarSelectTarget - sOmmStarSelectCurrent) < 0.01f) { sOmmStarSelectCurrent = sOmmStarSelectTarget; }
    while (sOmmStarSelectTarget <= -OMM_NUM_ACTS_MAX_PER_COURSE) { sOmmStarSelectTarget += OMM_NUM_ACTS_MAX_PER_COURSE; sOmmStarSelectCurrent += OMM_NUM_ACTS_MAX_PER_COURSE; }
    while (sOmmStarSelectTarget >= +OMM_NUM_ACTS_MAX_PER_COURSE) { sOmmStarSelectTarget -= OMM_NUM_ACTS_MAX_PER_COURSE; sOmmStarSelectCurrent -= OMM_NUM_ACTS_MAX_PER_COURSE; }
    while (abs_f(sOmmStarSelectTarget - sOmmStarSelectCurrent) >= OMM_NUM_ACTS_MAX_PER_COURSE) { sOmmStarSelectCurrent += OMM_NUM_ACTS_MAX_PER_COURSE * sign_f(sOmmStarSelectTarget - sOmmStarSelectCurrent); }

    // Press A, B or Start to select the act
    if ((sOmmStarSelectTimer++ >= 15) && (gPlayer1Controller->buttonPressed & (A_BUTTON | B_BUTTON | START_BUTTON))) {
        play_sound(SOUND_MENU_STAR_SOUND_LETS_A_GO, gGlobalSoundArgs);
        s32 selectedIndex = ((sOmmStarSelectTarget + OMM_NUM_ACTS_MAX_PER_COURSE) % OMM_NUM_ACTS_MAX_PER_COURSE);
        gDialogCourseActNum = selectedIndex + 1;
        return selectedIndex + 1;
    }

    // Update objects
    area_update_objects();
    return 0;
}

static Gfx *omm_level_star_select_render(s32 callContext, UNUSED struct GraphNode *node, UNUSED void *context) {
    if (callContext == GEO_CONTEXT_RENDER) {
        omm_star_select_render();
    }
    return NULL;
}

//
// Level geolayout
//

const GeoLayout omm_level_star_select_geo[] = {
    GEO_NODE_SCREEN_AREA(10, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, SCREEN_HEIGHT/2),
    GEO_OPEN_NODE(),
        GEO_ZBUFFER(0),
        GEO_OPEN_NODE(),
            GEO_NODE_ORTHO(100),
            GEO_OPEN_NODE(),
                GEO_BACKGROUND_COLOR(0xFFFF),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
        GEO_ZBUFFER(1),
        GEO_OPEN_NODE(),
            GEO_CAMERA_FRUSTUM(45, 100, 25000),
            GEO_OPEN_NODE(),
                GEO_CAMERA(0, 0, 0, 1000, 0, 0, 0, 0x00000000),
                GEO_OPEN_NODE(),
                    GEO_RENDER_OBJ(),
                GEO_CLOSE_NODE(),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
        GEO_ZBUFFER(0),
        GEO_OPEN_NODE(),
            GEO_ASM(0, omm_level_star_select_render),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};

//
// Level scripts
//

static const LevelScript omm_level_star_select_skip[] = {
    EXIT(),
};

const LevelScript level_script_star_select[] = {
    CALL(0, omm_level_star_select_should_skip),
    JUMP_IF(OP_EQ, 0, omm_level_star_select_skip),
    INIT_LEVEL(),
    ALLOC_LEVEL_POOL(),
    AREA(2, omm_level_star_select_geo),
        OBJECT(MODEL_STAR, 0, 0, 0, 0, 0, 0, 0, bhvOmmActSelectStar),
        OBJECT(MODEL_STAR, 0, 0, 0, 0, 0, 0, 0, bhvOmmActSelectStar),
        OBJECT(MODEL_STAR, 0, 0, 0, 0, 0, 0, 0, bhvOmmActSelectStar),
        OBJECT(MODEL_STAR, 0, 0, 0, 0, 0, 0, 0, bhvOmmActSelectStar),
        OBJECT(MODEL_STAR, 0, 0, 0, 0, 0, 0, 0, bhvOmmActSelectStar),
        OBJECT(MODEL_STAR, 0, 0, 0, 0, 0, 0, 0, bhvOmmActSelectStar),
        TERRAIN(main_menu_seg7_collision),
    END_AREA(),
    FREE_LEVEL_POOL(),
    LOAD_AREA(2),
    CALL(0, omm_level_star_select_init),
    TRANSITION(WARP_TRANSITION_FADE_FROM_COLOR, 15, 0xFF, 0xFF, 0xFF),
    SLEEP(15),
    SET_MENU_MUSIC(OMM_SEQ_STAR_SELECT),
    CALL(0, omm_level_star_select_init_objects),
    CALL_LOOP(0, omm_level_star_select_update),
    GET_OR_SET(OP_SET, VAR_CURR_ACT_NUM),
    STOP_MUSIC(190),
    TRANSITION(WARP_TRANSITION_FADE_INTO_COLOR, 15, 0xFF, 0xFF, 0xFF),
    SLEEP(15),
    CLEAR_LEVEL(),
    SLEEP_BEFORE_EXIT(1),
    EXIT(),
};
