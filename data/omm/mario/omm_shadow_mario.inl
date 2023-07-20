#ifdef OMM_MK_MARIO_COLORS
#ifdef GFX_PC_C
#ifdef VSCODE
#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#endif
#define OMM_ENABLE_SHADOW_MARIO_EFFECTS (OMM_CHEAT_SHADOW_MARIO && !OMM_EXTRAS_INVISIBLE_MODE && OMM_PLAYER_IS_MARIO)
#define OMM_SHADOW_MARIO_OPACITY 0xDD

// Change metal texture to Shadow Mario's body texture
static void omm_shadow_mario_set_texture() {
    if (OMM_ENABLE_SHADOW_MARIO_EFFECTS && !strstr(sGfxRdp->texToLoad, "metal_wing") && (strstr(sGfxRdp->texToLoad, "mario_metal") || strstr(sGfxRdp->texToLoad, "mario_cap_metal"))) {
        sGfxRdp->texToLoad = OMM_TEXTURE_MISC_SHADOW_MARIO_BODY;
    }
}

// Disable alpha noise for clear transparent effect
static void omm_shadow_mario_disable_alpha_noise() {
    if (OMM_ENABLE_SHADOW_MARIO_EFFECTS) {
        sGfxRdp->otherModeL &= ~G_AC_DITHER;
    }
}

// If enabled, make Mario always metal and slightly transparent
OMM_ROUTINE_PRE_RENDER(omm_shadow_mario_update) {
    if (gMarioObject) {
        struct MarioState *m = gMarioState;
        if (OMM_ENABLE_SHADOW_MARIO_EFFECTS) {

            // Mario object
            bool alpha = (m->marioBodyState->modelState & 0x100) != 0;
            u8 opacity = (alpha ? (m->marioBodyState->modelState & 0xFF) : 0xFF);
            m->marioBodyState->modelState &= ~0xFF;
            m->marioBodyState->modelState |= (0x400 * !alpha) | MODEL_STATE_METAL | 0x100 | min_s(opacity, OMM_SHADOW_MARIO_OPACITY);
            geo_preprocess_object_graph_node(gMarioObject);

            // Cap objects
            omm_array_for_each(omm_obj_get_cap_behaviors(), p) {
                const BehaviorScript *bhv = (const BehaviorScript *) p->as_ptr;
                for_each_object_with_behavior(obj, bhv) {
                    if (obj_check_model(obj, omm_player_graphics_get_normal_cap(OMM_PLAYER_MARIO))) {
                        obj->oGraphNode = gLoadedGraphNodes[omm_player_graphics_get_metal_cap(OMM_PLAYER_MARIO)];
                    } else if (obj_check_model(obj, omm_player_graphics_get_wing_cap(OMM_PLAYER_MARIO))) {
                        obj->oGraphNode = gLoadedGraphNodes[omm_player_graphics_get_winged_metal_cap(OMM_PLAYER_MARIO)];
                    }
                    obj->oOpacity = OMM_SHADOW_MARIO_OPACITY;
                    geo_preprocess_object_graph_node(obj);
                }
            }
        }
    }
}

#endif
#endif