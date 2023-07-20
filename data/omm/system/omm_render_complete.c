#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Data
//

s32 sOmmCompleteState = 0;
s32 sOmmCompleteCoins = 0;
s32 sOmmCompleteTimer = 0;
s32 sOmmCompleteAlpha = 0;

//
// Course complete
//

static void omm_render_course_complete_init() {
    if (sOmmCompleteState == 0) {
        sOmmCompleteCoins = -1;
        sOmmCompleteTimer = -1;
        sOmmCompleteAlpha = 0;
        sOmmCompleteState = 1;
    }
}

static void omm_render_course_complete_update() {
    if (sOmmCompleteCoins < gHudDisplay.coins) {
        sOmmCompleteCoins = min_s(sOmmCompleteCoins + 1, gHudDisplay.coins);
        play_sound(SOUND_MENU_YOSHI_GAIN_LIVES, gGlobalSoundArgs);
        if (sOmmCompleteCoins == gHudDisplay.coins) {
            sOmmCompleteTimer = 0;
            audio_play_star_fanfare();
            if (gGotFileCoinHiScore) {
                play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gGlobalSoundArgs);
            }
        }
    }
}

static s16 omm_render_course_complete_coins(s16 y) {
    u8 *textScore = omm_text_convert(OMM_TEXT_MY_SCORE, false);
    omm_render_string_right_align(OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmCompleteAlpha, textScore, true);
    omm_render_hud_coins(OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmCompleteAlpha, sOmmCompleteCoins, false);
    if (sOmmCompleteCoins == gHudDisplay.coins && gGotFileCoinHiScore) {
        u8 intensity = (0xFF * ((1.f + sins(gGlobalTimer * 0x1000)) / 2.f));
        u8 *textHiScore = omm_text_convert(OMM_TEXT_COURSE_COMPLETE_HI_SCORE, false);
        omm_render_string_left_align(OMM_RENDER_COURSE_COMPLETE_HI_SCORE_X, y, intensity, intensity, intensity, sOmmCompleteAlpha, textHiScore, true);
    }
    y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;
    return y;
}

static s16 omm_render_course_complete_stars(s16 y) {
    s32 levelNum = OMM_BOWSER_IN_THE_LEVEL(gLastCompletedLevelNum);
    u8 *textStars = omm_text_convert(OMM_TEXT_MY_STARS, false);
    omm_render_string_right_align(OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmCompleteAlpha, textStars, true);
    omm_render_hud_stars(OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmCompleteAlpha, levelNum, levelNum != OMM_LEVEL_END, false);
    y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;
    return y;
}

static s16 omm_render_course_complete_act_name(s16 y) {
    s32 levelNum = OMM_BOWSER_IN_THE_LEVEL(gLastCompletedLevelNum);
    if (COURSE_IS_MAIN_COURSE(omm_level_get_course(levelNum))) {
        u8 *textAct = omm_text_convert(OMM_TEXT_ACT__, false);
        s32 length = omm_text_length(textAct);
        textAct[length - 2] = DIALOG_CHAR_SPACE;
        textAct[length - 1] = gLastCompletedStarNum;
        textAct[length - 0] = 0xFF;
        omm_render_string_right_align(OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmCompleteAlpha, textAct, true);
        omm_render_string_left_align(OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmCompleteAlpha, omm_level_get_act_name(levelNum, gLastCompletedStarNum, OMM_GAME_MODE, false, false), false);
        y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;
    }
    return y;
}

static s16 omm_render_course_complete_course_name(s16 y) {
    s32 levelNum = OMM_BOWSER_IN_THE_LEVEL(gLastCompletedLevelNum);
    s32 courseNum = omm_level_get_course(levelNum);
    u8 *textCourse = omm_text_convert(OMM_TEXT_COURSE___, false);
    s32 textCourseLength = omm_text_length(textCourse);
    if (COURSE_IS_MAIN_COURSE(courseNum)) {
        textCourse[textCourseLength - 3] = DIALOG_CHAR_SPACE;
        if (courseNum >= 10) {
            textCourse[textCourseLength - 2] = courseNum / 10;
            textCourse[textCourseLength - 1] = courseNum % 10;
            textCourse[textCourseLength - 0] = 0xFF;
        } else {
            textCourse[textCourseLength - 2] = courseNum;
            textCourse[textCourseLength - 1] = 0xFF;
        }
    } else {
        textCourse[textCourseLength - 3] = 0xFF;
    }
    omm_render_string_right_align(OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmCompleteAlpha, textCourse, true);
    omm_render_string_left_align(OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmCompleteAlpha, omm_level_get_course_name(levelNum, OMM_GAME_MODE, false, false), false);
    y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;
    return y;
}

static s16 omm_render_course_complete_message(s16 y) {
    y = (((SCREEN_HEIGHT + (y - 8)) / 2) - 8);
    u8 intensity = 0x80 + (0x7F * ((1.f + sins(gGlobalTimer * 0x1000)) / 2.f));
    u8 *textMessage = omm_text_convert(((gLastCompletedLevelNum == LEVEL_BITDW || gLastCompletedLevelNum == LEVEL_BITFS) ? OMM_TEXT_COURSE_COMPLETE_CONGRATULATIONS : OMM_TEXT_COURSE_COMPLETE_GOT_A_STAR), false);
    omm_render_string_hud_centered(y, intensity, intensity, intensity, sOmmCompleteAlpha, textMessage, false);
    return y;
}

static void omm_render_course_complete_end() {
    if (sOmmCompleteTimer != -1 && sOmmCompleteTimer++ >= 90 && obj_anim_is_at_end(gMarioObject)) {
        sOmmCompleteAlpha = max_s(sOmmCompleteAlpha - 0x10, 0x00);
        if (sOmmCompleteAlpha == 0) {
            level_set_transition(0, 0);
            gSaveOptSelectIndex = 1; // Save and continue
            sOmmCompleteState = 0;
            gMenuMode = -1;
        }
    } else {
        sOmmCompleteAlpha = min_s(sOmmCompleteAlpha + 0x40, 0xFF);
    }
}

s32 omm_render_course_complete() {
    s16 y = ((SCREEN_HEIGHT - ((16 * (3 + (COURSE_IS_MAIN_COURSE(omm_level_get_course(OMM_BOWSER_IN_THE_LEVEL(gLastCompletedLevelNum))) ? 1 : 0))) - 8)) / 2);
    omm_render_course_complete_init();
    omm_render_course_complete_update();
    omm_render_shade_screen(sOmmCompleteAlpha / 2);
    y = omm_render_course_complete_coins(y);
    y = omm_render_course_complete_stars(y);
    y = omm_render_course_complete_act_name(y);
    y = omm_render_course_complete_course_name(y);
    y = omm_render_course_complete_message(y);
    omm_render_course_complete_end();
    return 0;
}
