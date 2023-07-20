#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Data
//

#define STUB_LEVEL(_0, _1, courseenum, _3, _4, _5, _6, _7, _8) courseenum,
#define DEFINE_LEVEL(_0, _1, courseenum, _3, _4, _5, _6, _7, _8, _9, _10) courseenum,
s8 gLevelToCourseNumTable[] = { // index is levelIndex
#include "levels/level_defines.h"
};
#undef STUB_LEVEL
#undef DEFINE_LEVEL

s8 gCourseNumToLevelNumTable[] = { // index is courseNum
    [COURSE_NONE] = LEVEL_CASTLE,
    [COURSE_BOB] = LEVEL_BOB,
    [COURSE_WF] = LEVEL_WF,
    [COURSE_JRB] = LEVEL_JRB,
    [COURSE_CCM] = LEVEL_CCM,
    [COURSE_BBH] = LEVEL_BBH,
    [COURSE_HMC] = LEVEL_HMC,
    [COURSE_LLL] = LEVEL_LLL,
    [COURSE_SSL] = LEVEL_SSL,
    [COURSE_DDD] = LEVEL_DDD,
    [COURSE_SL] = LEVEL_SL,
    [COURSE_WDW] = LEVEL_WDW,
    [COURSE_TTM] = LEVEL_TTM,
    [COURSE_THI] = LEVEL_THI,
    [COURSE_TTC] = LEVEL_TTC,
    [COURSE_RR] = LEVEL_RR,
    [COURSE_BITDW] = LEVEL_BITDW,
    [COURSE_BITFS] = LEVEL_BITFS,
    [COURSE_BITS] = LEVEL_BITS,
    [COURSE_PSS] = LEVEL_PSS,
    [COURSE_COTMC] = LEVEL_COTMC,
    [COURSE_TOTWC] = LEVEL_TOTWC,
    [COURSE_VCUTM] = LEVEL_VCUTM,
    [COURSE_WMOTR] = LEVEL_WMOTR,
    [COURSE_SA] = LEVEL_SA,
};

typedef struct {
    const LevelScript *script;
    OmmArray warps;
    OmmArray reds[8];
    s32 areas;
} OmmLevelData;

static OmmLevelData sOmmLevelData[LEVEL_COUNT] = { 0 };
static s32 sOmmLevelList[LEVEL_COUNT] = { 0 }; // Ordered by Course Id, COURSE_NONE excluded
static s32 sOmmLevelCount = 0;
static s32 sCurrentLevelNum;

//
// Init
//

static Warp *omm_level_get_warp_data(s32 levelNum, s32 areaIndex, s32 id) {

    // Existing warp
    omm_array_for_each(sOmmLevelData[levelNum].warps, p) {
        Warp *warp = (Warp *) p->as_ptr;
        if (warp->srcLevelNum == levelNum && warp->srcAreaIndex == areaIndex && warp->srcId == id) {
            return warp;
        }
    }

    // New warp
    Warp *warp = mem_new(Warp, 1);
    warp->srcLevelNum = levelNum;
    warp->srcAreaIndex = areaIndex;
    warp->srcId = id;
    warp->srcType = -1;
    omm_array_add(sOmmLevelData[levelNum].warps, ptr, warp);
    return warp;
};

static s32 omm_level_preprocess_master_script(u8 type, void *cmd) {
    static bool sScriptExecLevelTable = false;
    static s32 sLevelNum = -1;
    if (sScriptExecLevelTable) {
        if (type == LEVEL_CMD_EXECUTE) {
            const LevelScript *script = level_cmd_get(cmd, const LevelScript *, 12);
            if (sLevelNum >= 0 && sLevelNum < LEVEL_COUNT && !sOmmLevelData[sLevelNum].script) {
                sOmmLevelData[sLevelNum].script = script;
            }
            sLevelNum = -1;
            return LEVEL_SCRIPT_RETURN;
        }
        if (type == LEVEL_CMD_EXIT || type == LEVEL_CMD_SLEEP) {
            return LEVEL_SCRIPT_STOP;
        }
        if (type == LEVEL_CMD_JUMP_IF) {
            sLevelNum = level_cmd_get(cmd, s32, 4);
        }
    } else if (type == LEVEL_CMD_JUMP_LINK) {
        sScriptExecLevelTable = true;
    }
    return LEVEL_SCRIPT_CONTINUE;
}

static s32 omm_level_fill_warp_data(u8 type, void *cmd) {
    static s32 sCurrentAreaIndex = 0;
    switch (type) {
        case LEVEL_CMD_SLEEP:
        case LEVEL_CMD_SLEEP_BEFORE_EXIT: {
        } return LEVEL_SCRIPT_STOP;

        case LEVEL_CMD_AREA: {
            sCurrentAreaIndex = level_cmd_get(cmd, u8, 2);
            sOmmLevelData[sCurrentLevelNum].areas |= (1 << sCurrentAreaIndex);
        } break;

        case LEVEL_CMD_OBJECT_WITH_ACTS: {
            const BehaviorScript *bhv = level_cmd_get(cmd, const BehaviorScript *, 20);

            // Red coin
            if (bhv == bhvRedCoin) {
                if (omm_array_find(sOmmLevelData[sCurrentLevelNum].reds[sCurrentAreaIndex], ptr, cmd) == -1) {
                    omm_array_add(sOmmLevelData[sCurrentLevelNum].reds[sCurrentAreaIndex], ptr, cmd);
                }
            }

            // Warps
            for (s32 i = 0; i != 20; ++i) {
                if (sWarpBhvSpawnTable[i] == bhv) {
                    s32 warpId = ((level_cmd_get(cmd, u32, 16) >> 16) & 0xFF);
                    Warp *warp = omm_level_get_warp_data(sCurrentLevelNum, sCurrentAreaIndex, warpId);
                    if (warp->srcType == -1) {
                        warp->srcType = i;
                        warp->x = (f32) level_cmd_get(cmd, s16, 4);
                        warp->y = (f32) level_cmd_get(cmd, s16, 6);
                        warp->z = (f32) level_cmd_get(cmd, s16, 8);
                        warp->yaw = (level_cmd_get(cmd, s16, 12) * 0x8000) / 180;
                    }
                    break;
                }
            }
        } break;

        case LEVEL_CMD_WARP_NODE:
        case LEVEL_CMD_PAINTING_WARP_NODE: {
            Warp *warp = omm_level_get_warp_data(sCurrentLevelNum, sCurrentAreaIndex, level_cmd_get(cmd, u8, 2));
            if (warp->dstLevelNum == 0) {
                warp->dstLevelNum = (s32) level_cmd_get(cmd, u8, 3);
                warp->dstAreaIndex = (s32) level_cmd_get(cmd, u8, 4);
                warp->dstId = (s32) level_cmd_get(cmd, u8, 5);
            }
        } break;

        case LEVEL_CMD_MACRO_OBJECTS: {
            MacroObject *data = level_cmd_get(cmd, MacroObject *, 4);
            for (; *data != MACRO_OBJECT_END(); data += 5) {
                s32 presetId = (s32) ((data[0] & 0x1FF) - 0x1F);

                // Red coin
                if (presetId == macro_red_coin) {
                    if (omm_array_find(sOmmLevelData[sCurrentLevelNum].reds[sCurrentAreaIndex], ptr, data) == -1) {
                        omm_array_add(sOmmLevelData[sCurrentLevelNum].reds[sCurrentAreaIndex], ptr, data);
                    }
                }
            }
        } break;
    }
    return LEVEL_SCRIPT_CONTINUE;
}

static void omm_level_init() {
    static bool sInited = false;
    if (OMM_UNLIKELY(!sInited)) {

        // Level scripts
        level_script_preprocess(level_main_scripts_entry, omm_level_preprocess_master_script);

        // Level warps
        for (sCurrentLevelNum = 0; sCurrentLevelNum != LEVEL_COUNT; ++sCurrentLevelNum) {
            if (sOmmLevelData[sCurrentLevelNum].script) {
                level_script_preprocess(sOmmLevelData[sCurrentLevelNum].script, omm_level_fill_warp_data);
            }
        }

        // Level list ordered by course id
        for (s32 courseNum = COURSE_MIN; courseNum <= COURSE_MAX; ++courseNum) {
            if (courseNum == COURSE_CAKE_END) continue;
            for (s32 levelNum = 1; levelNum != LEVEL_COUNT; ++levelNum) {
                if (gLevelToCourseNumTable[levelNum - 1] == courseNum) {
                    sOmmLevelList[sOmmLevelCount++] = levelNum;
                }
            }
        }

        // Done
        sInited = true;
    }
}

//
// Common data
//

OMM_INLINE void convert_text_and_set_buffer(u8 *buffer, const char *str) {
    u8 *str64 = omm_text_convert(str, false);
    mem_cpy(buffer, str64, omm_text_length(str64) + 1);
}

s32 omm_level_get_count() {
    omm_level_init();
    return sOmmLevelCount;
}

s32 *omm_level_get_list() {
    omm_level_init();
    return sOmmLevelList;
}

s32 omm_level_get_course(s32 levelNum) {
    omm_level_init();
    return (s32) gLevelToCourseNumTable[levelNum - 1];
}

s32 omm_level_from_course(s32 courseNum) {
    omm_level_init();
    return (s32) gCourseNumToLevelNumTable[courseNum];
}

const LevelScript *omm_level_get_script(s32 levelNum) {
    omm_level_init();
    return sOmmLevelData[levelNum].script;
}

s32 omm_level_get_areas(s32 levelNum) {
    omm_level_init();
    return sOmmLevelData[levelNum].areas;
}

s32 omm_level_get_num_red_coins(s32 levelNum, s32 areaIndex) {
    omm_level_init();
    return omm_array_count(sOmmLevelData[levelNum].reds[areaIndex]);
}

u8 *omm_level_get_course_name(s32 levelNum, s32 modeIndex, bool decaps, bool num) {
    omm_level_init();
    static u8 sBuffer[256];
    mem_set(sBuffer, 0xFF, 256);
    s32 courseNum = omm_level_get_course(levelNum);

    // Level name
    if (levelNum == LEVEL_BOWSER_1) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_BOWSER_1);
    } else if (levelNum == LEVEL_BOWSER_2) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_BOWSER_2);
    } else if (levelNum == LEVEL_BOWSER_3) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_BOWSER_3);
    } else if (courseNum < COURSE_BOB) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_CASTLE);
    } else if (courseNum >= COURSE_CAKE_END) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_CASTLE);
    } else {
        const u8 *courseName = gCourseNameTable(modeIndex)[courseNum - COURSE_BOB] + 3;
        mem_cpy(sBuffer, courseName, omm_text_length(courseName));
    }

    // Decaps
    if (decaps) {
        omm_text_decapitalize(sBuffer);
    }

    // Course number
    if (num && (courseNum >= COURSE_BOB) && (courseNum <= COURSE_STAGES_MAX)) {
        mem_mov(sBuffer + 5, sBuffer, omm_text_length(sBuffer));
        sBuffer[0] = ((courseNum / 10) == 0 ? 158 : (courseNum / 10));
        sBuffer[1] = (courseNum % 10);
        sBuffer[2] = 158;
        sBuffer[3] = 159;
        sBuffer[4] = 158;
    }

    return sBuffer;
}

u8 *omm_level_get_act_name(s32 levelNum, s32 actNum, s32 modeIndex, bool decaps, bool num) {
    omm_level_init();
    static u8 sBuffer[256];
    mem_set(sBuffer, 0xFF, 256);
    s32 courseNum = omm_level_get_course(levelNum);

    // Star name
    if (courseNum < COURSE_BOB) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_ONE_SECRET_STAR);
    } else if (actNum < 0 || actNum > OMM_NUM_STARS_MAX_PER_COURSE || levelNum == LEVEL_BOWSER_1 || levelNum == LEVEL_BOWSER_2 || levelNum == LEVEL_BOWSER_3) { // Fake stars, invalid stars, Bowser fights
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_EMPTY);
#if OMM_GAME_IS_SM64
    } else if (levelNum == LEVEL_PSS) {
        convert_text_and_set_buffer(sBuffer, actNum == 1 ? OMM_TEXT_LEVEL_PSS_STAR_1 : OMM_TEXT_LEVEL_PSS_STAR_2);
    } else if (courseNum > COURSE_STAGES_MAX) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_RED_COINS_STAR);
#else
#if OMM_GAME_IS_SMSR
    } else if (actNum == 0) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_STAR_REPLICA);
#endif
    } else if (courseNum > COURSE_STAGES_MAX) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_STAR__);
        sBuffer[5] = actNum;
#endif
    } else if (actNum == OMM_NUM_STARS_MAX_PER_COURSE) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_100_COINS_STAR);
    } else {
        const u8 *actName = gActNameTable(modeIndex)[(courseNum - COURSE_BOB) * OMM_NUM_ACTS_MAX_PER_COURSE + (actNum - 1)];
        mem_cpy(sBuffer, actName, omm_text_length(actName));
    }

    // Decaps
    if (decaps) {
        omm_text_decapitalize(sBuffer);
    }

    // Star number
    if (num && (courseNum >= COURSE_BOB) && (courseNum <= COURSE_STAGES_MAX)) {
        mem_mov(sBuffer + 5, sBuffer, omm_text_length(sBuffer));
        sBuffer[0] = ((actNum / 10) == 0 ? 158 : (actNum / 10));
        sBuffer[1] = (actNum % 10);
        sBuffer[2] = 158;
        sBuffer[3] = 159;
        sBuffer[4] = 158;
    }

    return sBuffer;
}

//
// Warps
//

Warp *omm_level_get_warp(s32 levelNum, s32 areaIndex, s32 id) {
    omm_level_init();
    omm_array_for_each(sOmmLevelData[levelNum].warps, p) {
        Warp *warp = (Warp *) p->as_ptr;
        if (warp->srcLevelNum == levelNum && warp->srcAreaIndex == areaIndex && warp->srcId == id) {
            return warp;
        }
    }
    return NULL;
}

Warp *omm_level_get_entry_warp(s32 levelNum, s32 areaIndex) {
    omm_level_init();
    return omm_level_get_warp(levelNum, areaIndex, OMM_LEVEL_ENTRY_WARP(levelNum));
}

Warp *omm_level_get_exit_warp(s32 levelNum, s32 areaIndex) {
    omm_level_init();
    return omm_level_get_warp(levelNum, areaIndex, 0xF0);
}

Warp *omm_level_get_death_warp(s32 levelNum, s32 areaIndex) {
    omm_level_init();
    Warp *warp = omm_level_get_warp(levelNum, areaIndex, 0xF1);
    if (!warp) warp = omm_level_get_warp(levelNum, areaIndex, 0xF3);
    return warp;
}

bool omm_level_can_warp(s32 levelNum) {
    omm_level_init();
    return omm_level_get_course(levelNum) != COURSE_NONE;
}
