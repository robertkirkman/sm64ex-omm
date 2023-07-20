#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// LiveSplit auto-splitter support
// The file 'omm.asl' is needed to make it work
//
// Splits formatting:
// - For star splits, you must indicate the amount of stars needed in brackets:
//   * Use square brackets to split upon star collection: "[20]".
//   * Use parentheses to delay the split until the level exit: "(20)".
// - If a split has "Bowser" in its name but no star amount, it is interpreted as a Bowser
//   key split or a Grand Star split and the split is triggered on key/star collection.
// - If the final split has no star amount, it is interpreted as a Grand Star split
//   (i.e. after defeating the last Bowser) and immediately stops the timer.
//

#pragma GCC push_options
#pragma GCC optimize ("O0")
static volatile u8 sOmmSplitFlags[16] = { 'O', 'M', 'M', 'A', 'U', 'T', 'O', 'S', 'P', 'L', 'I', 'T', 0, 0, 0, 0 };
static volatile s32 *sOmmSplitIndex = (s32 *) (&sOmmSplitFlags[12]);
static volatile s32 sOmmSplitFrames = 0;
static OmmArray sOmmSplits = omm_array_zero;
#pragma GCC pop_options

typedef struct {
    s32 type;
    s32 value;
} OmmSplit;

static s32 omm_speedrun_get_split_frames(OmmSplit *split) {
    switch (split->type) {
        case OMM_SPLIT_STAR:   return  1 * (omm_save_file_get_total_star_count(gCurrSaveFileNum - 1, OMM_GAME_MODE, COURSE_MIN - 1, COURSE_MAX - 1) >= split->value);
        case OMM_SPLIT_EXIT:   return 30 * (omm_save_file_get_total_star_count(gCurrSaveFileNum - 1, OMM_GAME_MODE, COURSE_MIN - 1, COURSE_MAX - 1) >= split->value);
        case OMM_SPLIT_BOWSER: return  1;
    }
    return false;
}

void omm_speedrun_split(s32 type) {
    if (*sOmmSplitIndex >= 0 && *sOmmSplitIndex < omm_array_count(sOmmSplits)) {
        OmmSplit *split = (OmmSplit *) omm_array_get(sOmmSplits, ptr, *sOmmSplitIndex);
        if (split->type == type && sOmmSplitFrames <= 0) {
            sOmmSplitFrames = omm_speedrun_get_split_frames(split);
        }
    }
}

// Main menu/File select screen -> 'reset' (-1)
// 'start' (-2) or 'reset' (-1) but empty save and Mario not loaded -> 'start'
// As soon as Mario loads or if an existing save file is selected, set index to 0
OMM_ROUTINE_UPDATE(omm_speedrun_update) {
    if (omm_is_main_menu()) {
        *sOmmSplitIndex = -1;
    } else if (*sOmmSplitIndex < 0) {
        if (!gMarioObject && !omm_save_file_exists(gCurrSaveFileNum - 1, OMM_GAME_MODE)) {
            *sOmmSplitIndex = -2;
        } else {
            *sOmmSplitIndex = 0;
        }
    } else if (sOmmSplitFrames-- == 1) {
        (*sOmmSplitIndex)++;
    }
}

//
// Init
//

static const char *omm_speedrun_lss_get_data(const char *s, const char *beginsWith, const char *endsWith) {
    static char data[1024];
    const char *a = strstr(s, beginsWith);
    const char *b = strstr(s, endsWith);
    if (a && b) {
        a += strlen(beginsWith);
        s32 length = (s32) (b - a);
        mem_clr(data, sizeof(data));
        mem_cpy(data, a, max_s(0, length));
        return data;
    }
    return NULL;
}

static OmmSplit *omm_speedrun_split_data_create_split(const char *s) {

    // Star or level exit split
    for_each_until_null(const char *, brackets, array_of(const char *) { "[]", "()", NULL }) {
        const char *a = strrchr(s, (*brackets)[0]);
        const char *b = strrchr(s, (*brackets)[1]);
        if (a && b && a < b) {
            s32 stars;
            if (sscanf(a + 1, "%d", &stars)) {
                OmmSplit *split = mem_new(OmmSplit, 1);
                split->type = (*a == '(' ? OMM_SPLIT_EXIT : OMM_SPLIT_STAR);
                split->value = stars;
                return split;
            }
        }
    }

    // Bowser split
    str_lwr_sa(lower, 1024, s);
    if (strstr(lower, "bowser")) {
        OmmSplit *split = mem_new(OmmSplit, 1);
        split->type = OMM_SPLIT_BOWSER;
        return split;
    }

    // Invalid split
    return NULL;
}

void omm_speedrun_init() {
    str_cat_paths_sa(filename, SYS_MAX_PATH, sys_exe_path(), "splits.lss");
    FILE *f = fopen(filename, "r");
    if (f) {
        omm_log("Extracting split data from file: %s\n",, filename);

        // Looking for game info and splits
        OmmArray splits = omm_array_zero;
        bool isSegment = false;
        char buffer[1024];
        while (fgets(buffer, 1024, f)) {

            // Game name
            const char *gameName = omm_speedrun_lss_get_data(buffer, "<GameName>", "</GameName>");
            if (gameName) {
                omm_printf("Game: %s\n",, gameName);
                continue;
            }
            
            // Category name
            const char *categoryName = omm_speedrun_lss_get_data(buffer, "<CategoryName>", "</CategoryName>");
            if (categoryName) {
                omm_printf("Category: %s\n",, categoryName);
                continue;
            }

            // Segment start
            if (omm_speedrun_lss_get_data(buffer, "<Segment>", "")) {
                isSegment = true;
                continue;
            }
            
            // Segment end
            if (omm_speedrun_lss_get_data(buffer, "</Segment>", "")) {
                isSegment = false;
                continue;
            }
            
            // Splits
            if (isSegment) {
                const char *splitName = omm_speedrun_lss_get_data(buffer, "<Name>", "</Name>");
                if (splitName) {
                    omm_array_add(splits, ptr, mem_dup(splitName, strlen(splitName) + 1));
                }
            }
        }

        // Generating splits
        omm_printf("Splits:\n");
        omm_array_for_each(splits, p) {
            const char *s = (const char *) p->as_ptr;
            OmmSplit *split = omm_speedrun_split_data_create_split(s);
            if (split) {
                omm_array_add(sOmmSplits, ptr, split);
                switch (split->type) {
                    case OMM_SPLIT_STAR: {
                        omm_printf("- %d Star split on collect: %s\n",, split->value, s);
                    } break;

                    case OMM_SPLIT_EXIT: {
                        omm_printf("- %d Star split on level exit: %s\n",, split->value, s);
                    } break;

                    case OMM_SPLIT_BOWSER: {
                        if (i_p == omm_array_count(splits) - 1) {
                            omm_printf("- Grand Star split: %s\n",, s);
                        } else {
                            omm_printf("- Bowser Key split: %s\n",, s);
                        }
                    } break;
                }
            } else {
                omm_printf("[!] Invalid split format: %s\n",, s);
            }
        }

        // Done
        omm_log("Data successfully extracted. Closing file.\n\n");
        fflush(stdout);
        fclose(f);
    }
}
