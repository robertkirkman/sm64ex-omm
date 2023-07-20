#if !defined(DYNOS)
#include "omm_models.cpp.h"
extern "C" {
#include "data/omm/omm_defines.h"
#include "data/omm/omm_patches.h"
#include "data/omm/system/omm_system.h"
#include "data/omm/system/omm_memory.h"
#include "pc/gfx/gfx_rendering_api.h"
}

typedef struct { u32 hash, id, w, h, pal; u8 cms, cmt, lin, *data; } GfxTexture;

extern "C" {
void *omm_models_find_texture(OmmHMap *phmap, struct GfxRenderingAPI *rapi, const void *p) {
    Array<ActorGfx> &actor_list = omm_models_get_actor_list();
    for (auto &actor : actor_list) {
        if (actor.gfx_data) {
            for (auto &texture_node : actor.gfx_data->textures) {
                if ((const void *) texture_node == p) {
                    
                    // Node found, computing name hash
                    str_cat_paths_sa(name, SYS_MAX_PATH, "OMM_MODELS", texture_node->name.begin());
                    u32 hash = str_hash(name);
                    GfxTexture *tex = NULL;

                    // Look for texture in cache
                    s32 i = omm_hmap_find(*phmap, hash);
                    if (i != -1) {
                        tex = omm_hmap_get(*phmap, GfxTexture *, i);
                        if (!texture_node->data->uploaded) {
                            rapi->select_texture(0, tex->id);
                            rapi->upload_texture(texture_node->data->raw_data.begin(), texture_node->data->raw_width, texture_node->data->raw_height);
                            texture_node->data->uploaded = true;
                        }
                        return (void *) tex;
                    }

                    // Create new texture
                    tex = (GfxTexture *) mem_new(GfxTexture, 1);
                    tex->hash = hash;
                    tex->id = rapi->new_texture();
                    tex->w = texture_node->data->raw_width;
                    tex->h = texture_node->data->raw_height;
                    tex->pal = 0;
                    tex->cms = 0;
                    tex->cmt = 0;
                    tex->lin = 0;
                    tex->data = NULL;
                    rapi->select_texture(0, tex->id);
                    rapi->set_sampler_parameters(0, false, 0, 0);

                    // Cache texture
                    omm_hmap_insert(*phmap, tex->hash, tex);

                    // Upload texture
                    rapi->select_texture(0, tex->id);
                    rapi->upload_texture(texture_node->data->raw_data.begin(), texture_node->data->raw_width, texture_node->data->raw_height);
                    texture_node->data->uploaded = true;
                    return (void *) tex;
                }
            }
        }
    }
    return NULL;
}
}
#endif
