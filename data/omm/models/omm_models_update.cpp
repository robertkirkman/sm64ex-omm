#if !defined(DYNOS)
#include "omm_models.cpp.h"
extern "C" {
#include "data/omm/omm_defines.h"
#include "data/omm/system/omm_options.h"
#include "object_fields.h"
#include "engine/math_util.h"
#include "game/level_update.h"
#include "game/object_list_processor.h"
}

//
// Update animations
//

static s32 get_mario_current_anim_index() {
    MarioAnimDmaTableStruct *anim_dma_table = gMarioAnimDmaTable;
    for (s32 i = 0; i != (s32) anim_dma_table->count; ++i) {
        void *anim_addr = anim_dma_table->srcAddr + anim_dma_table->anim[i].offset;
        if (anim_addr == gMarioCurrAnimAddr) {
            return i;
        }
    }
    return -1;
}

// As we don't know the length of the table, let's hope that we'll always find the animation...
static s32 get_obj_current_anim_index(struct Object *obj) {
    if (!obj->oAnimations || !obj->oCurrAnim) {
        return -1;
    }
    for (s32 i = 0; obj->oAnimations[i] != NULL; ++i) {
        if (obj->oAnimations[i] == obj->oCurrAnim) {
            return i;
        }
    }
    return -1;
}

extern "C" {
void omm_models_swap_animations(void *ptr) {
    static Animation *default_anim = NULL;
    static Animation gfx_data_anim;

    // Does the object has a model?
    struct Object *obj = (struct Object *) ptr;
    if (!obj->oGraphNode) {
        return;
    }

    // Swap the current animation with the one from the Gfx data
    if (!default_anim) {
        default_anim = obj->oCurrAnim;

        // Actor index
        s32 actor_index = omm_models_get_actor_index(obj->oGraphNode->georef);
        if (actor_index == -1) {
            return;
        }

        // Gfx data
        GfxData *gfx_data = omm_models_get_actor_list()[actor_index].gfx_data;
        if (!gfx_data) {
            return;
        }

        // Animation table
        if (gfx_data->animation_table.is_empty()) {
            return;
        }

        // Animation index
        s32 anim_index = (obj == gMarioObject ? get_mario_current_anim_index() : get_obj_current_anim_index(obj));
        if (anim_index == -1) {
            return;
        }

        // Animation data
        const AnimData *anim_data = (const AnimData *) gfx_data->animation_table[anim_index].second;
        if (anim_data) {
            gfx_data_anim.flags = anim_data->flags;
            gfx_data_anim.mAnimYTransDivisor = anim_data->anim_y_trans_div;
            gfx_data_anim.mStartFrame = anim_data->start_frame;
            gfx_data_anim.mLoopStart = anim_data->loop_start;
            gfx_data_anim.mLoopEnd = anim_data->loop_end;
            gfx_data_anim.values = anim_data->values.second.begin();
            gfx_data_anim.index = anim_data->index.second.begin();
            gfx_data_anim.length = anim_data->length;
            obj->oCurrAnim = &gfx_data_anim;
        }

    // Restore the default animation
    } else {
        obj->oCurrAnim = default_anim;
        default_anim = NULL;
    }
}
}

//
// Update models
//

static bool omm_models_should_disable_billboard(const GfxData *gfx_data) {
    for (auto& vtx_node : gfx_data->vertices) {
        Vec3f pn = { 0, 0, 0 };
        for (u32 i = 2; i < vtx_node->size; ++i) {
            Vec3f p0; vec3f_copy(p0, vtx_node->data[i - 2].v.ob);
            Vec3f p1; vec3f_copy(p1, vtx_node->data[i - 1].v.ob);
            Vec3f p2; vec3f_copy(p2, vtx_node->data[i - 0].v.ob);
            Vec3f v0; vec3f_dif(v0, p0, p1);
            Vec3f v1; vec3f_dif(v1, p1, p2);
            Vec3f vn; vec3f_cross(vn, v0, v1);
            if (memcmp(vn, gVec3fZero, sizeof(Vec3f)) != 0) { // skip zero normals
                vec3f_normalize(vn);
                if (memcmp(pn, gVec3fZero, sizeof(Vec3f)) != 0) { // don't compare to zero normal
                    f32 dot = vec3f_dot(vn, pn);
                    if (dot < 0.9f) { // Points don't form a plane -> no billboard
                        return true;
                    }
                }
                vec3f_copy(pn, vn);
            }
        }
    }
    return false;
}

extern "C" {
void omm_models_update() {
    if (gObjectLists && gOmmOptModelsEnabled) {

        // Check if there are packs
        const Array<const char *> &pack_list = omm_models_get_packs();
        if (!pack_list.is_empty()) {

            // Loop through all object lists
            for (s32 obj_list : { OBJ_LIST_PLAYER, OBJ_LIST_DESTRUCTIVE, OBJ_LIST_GENACTOR, OBJ_LIST_PUSHABLE, OBJ_LIST_LEVEL, OBJ_LIST_DEFAULT, OBJ_LIST_SURFACE, OBJ_LIST_POLELIKE, OBJ_LIST_UNIMPORTANT }) {
                struct Object *_Head = (struct Object *) &gObjectLists[obj_list];
                for (struct Object *obj = (struct Object *) _Head->header.next; obj != _Head; obj = (struct Object *) obj->header.next) {
                    if (obj->oGraphNode) {
                        
                        // Actor index
                        s32 actor_index = omm_models_get_actor_index(obj->oGraphNode->georef);
                        if (actor_index != -1) {
                            
                            // Replace the object's model and animations
                            ActorGfx *actor_gfx = &omm_models_get_actor_list()[actor_index];
                            for (s32 i = 0; i != pack_list.get_count(); ++i) {

                                // If enabled_packs and no pack is selected
                                // load the pack's model and replace the default actor's model
                                if (*(gOmmOptModelsEnabled[i]) && actor_gfx->pack_index == -1) {

                                    // Load Gfx data from binary
                                    SysPath pack_folder = SysPath(pack_list[i]);
                                    GfxData *gfx_data = omm_models_load_from_binary(pack_folder, omm_models_get_actor_name(actor_index));
                                    if (gfx_data) {
                                        actor_gfx->pack_index = i;
                                        actor_gfx->gfx_data = gfx_data;
                                        actor_gfx->graph_node = (GraphNode *) geo_layout_to_graph_node(NULL, (*(gfx_data->geo_layouts.end() - 1))->data);
                                        actor_gfx->graph_node->georef = omm_models_get_actor_layout(actor_index);
                                        actor_gfx->graph_node->noBillboard = omm_models_should_disable_billboard(gfx_data);
                                        // if (actor_gfx->graph_node->noBillboard) {
                                        //     printf("Billboard disabled for %s/%s\n", pack_list[i], omm_models_get_actor_name(actor_index));
                                        //     fflush(stdout);
                                        // }
                                        break;
                                    }
                                }

                                // If disabled and this pack is the one selected
                                // replace the actor's model by the default one
                                else if (!*(gOmmOptModelsEnabled[i]) && actor_gfx->pack_index == i) {
                                    actor_gfx->pack_index = -1;
                                    actor_gfx->gfx_data = NULL;
                                    actor_gfx->graph_node = (GraphNode *) geo_layout_to_graph_node(NULL, (const GeoLayout *) omm_models_get_actor_layout(actor_index));
                                    actor_gfx->graph_node->noBillboard = false;
                                }
                            }

                            // Update object
                            obj->oGraphNode = actor_gfx->graph_node;
                            obj->oNodeFlags &= ~((GRAPH_RENDER_BILLBOARD | GRAPH_RENDER_CYLBOARD) * actor_gfx->graph_node->noBillboard);
                        }
                    }
                }
            }
        }
    }
}

s32 omm_models_get_mario_model_pack_index() {
    extern const GeoLayout mario_geo[];
    return omm_models_get_actor_list()[omm_models_get_actor_index(mario_geo)].pack_index;
}
}
#endif
