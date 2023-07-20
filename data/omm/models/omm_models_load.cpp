#if !defined(DYNOS)
#include "omm_models.cpp.h"
extern "C" {
#include "pc/fs/fs.h"
#include "stb/stb_image.h"
}

#define FUNC_CODE (u32) 0x434E5546
#define PNTR_CODE (u32) 0x52544E50

//
// Read data
//

template <typename T>
static T* alloc(u64 count = 1llu) {
    T *ptr = (T *) calloc(count, sizeof(T));
    for (u64 i = 0; i != count; ++i) {
        new (ptr + i) T(); // Calls the constructor of type T at address (ptr + i)
    }
    return ptr;
}

template <typename T>
static T read_bytes(FILE* file) {
    T item = { 0 };
    fread(&item, sizeof(T), 1, file);
    return item;
}

static void *get_pointer(GfxData *gfx_data, const String &ptr_name, u32 ptr_data) {

    // Lights
    for (auto& light_node : gfx_data->lights) {
        if (light_node->name == ptr_name) {
            if (ptr_data == 1) {
                return (void *) &light_node->data->l[0];
            }
            if (ptr_data == 2) {
                return (void *) &light_node->data->a;
            }
            sys_fatal("Unknown Light type: %u", ptr_data);
        }
    }

    // Textures
    for (auto& texture_node : gfx_data->textures) {
        if (texture_node->name == ptr_name) {
            return (void *) texture_node;
        }
    }

    // Display lists
    for (auto &display_list_node : gfx_data->display_lists) {
        if (display_list_node->name == ptr_name) {
            return (void *) display_list_node->data;
        }
    }

    // Geo layouts
    for (auto &geo_layout_node : gfx_data->geo_layouts) {
        if (geo_layout_node->name == ptr_name) {
            return (void *) geo_layout_node->data;
        }
    }

    // Vertices
    for (auto &vtx_node : gfx_data->vertices) {
        if (vtx_node->name == ptr_name) {
            return (void *) (vtx_node->data + ptr_data);
        }
    }

    // Error
    sys_fatal("Pointer not found: %s", ptr_name.begin());
    return NULL;
}

static void *read_pointer(FILE *file, GfxData *gfx_data, u32 type) {

    // FUNC
    if (type == FUNC_CODE) {
        s32 index = read_bytes<s32>(file);
        return omm_models_get_func_pointer(index);
    }

    // PNTR
    if (type == PNTR_CODE) {
        String ptr_name; ptr_name.read(file);
        u32 ptr_data = read_bytes<u32>(file);
        return get_pointer(gfx_data, ptr_name, ptr_data);
    }

    // Not a pointer
    return NULL;
}

//
// Read binary
//

static void omm_models_load_light_data(FILE *file, GfxData *gfx_data) {
    DataNode<Lights1> *light_node = alloc<DataNode<Lights1>>();

    // Name
    light_node->name.read(file);

    // Data
    light_node->data = alloc<Lights1>();
    *light_node->data = read_bytes<Lights1>(file);

    // Append
    gfx_data->lights.add(light_node);
}

static void omm_models_load_texture_data(FILE *file, GfxData *gfx_data) {
    DataNode<TexData> *texture_node = alloc<DataNode<TexData>>();

    // Name
    texture_node->name.read(file);

    // Data
    texture_node->data = alloc<TexData>();
    texture_node->data->uploaded = false;
    texture_node->data->png_data.read(file);
    if (!texture_node->data->png_data.is_empty()) {
        u8 *raw_data = stbi_load_from_memory(
            texture_node->data->png_data.begin(),
            texture_node->data->png_data.get_count(),
            &texture_node->data->raw_width,
            &texture_node->data->raw_height,
            NULL, 4
        );
        texture_node->data->raw_data = Array<u8>(raw_data, raw_data + (texture_node->data->raw_width * texture_node->data->raw_height * 4));
        free(raw_data);
    } else { // Probably a palette
        texture_node->data->raw_data = Array<u8>();
        texture_node->data->raw_width = 0;
        texture_node->data->raw_height = 0;
    }

    // Append
    gfx_data->textures.add(texture_node);
}

static void omm_models_load_vertex_data(FILE *file, GfxData *gfx_data) {
    DataNode<Vtx> *vtx_node = alloc<DataNode<Vtx>>();

    // Name
    vtx_node->name.read(file);

    // Data
    vtx_node->size = read_bytes<u32>(file);
    vtx_node->data = alloc<Vtx>(vtx_node->size);
    for (u32 i = 0; i != vtx_node->size; ++i) {
        vtx_node->data[i].n.ob[0] = read_bytes<s16>(file);
        vtx_node->data[i].n.ob[1] = read_bytes<s16>(file);
        vtx_node->data[i].n.ob[2] = read_bytes<s16>(file);
        vtx_node->data[i].n.flag  = read_bytes<s16>(file);
        vtx_node->data[i].n.tc[0] = read_bytes<s16>(file);
        vtx_node->data[i].n.tc[1] = read_bytes<s16>(file);
        vtx_node->data[i].n.n[0]  = read_bytes<s8> (file);
        vtx_node->data[i].n.n[1]  = read_bytes<s8> (file);
        vtx_node->data[i].n.n[2]  = read_bytes<s8> (file);
        vtx_node->data[i].n.a     = read_bytes<u8> (file);
    }

    // Append
    gfx_data->vertices.add(vtx_node);
}

static void omm_models_load_display_list_data(FILE *file, GfxData *gfx_data) {
    DataNode<Gfx> *display_list_node = alloc<DataNode<Gfx>>();

    // Name
    display_list_node->name.read(file);

    // Data
    display_list_node->size = read_bytes<u32>(file);
    display_list_node->data = alloc<Gfx>(display_list_node->size);
    for (u32 i = 0; i != display_list_node->size; ++i) {
        u32 words_w0 = read_bytes<u32>(file);
        u32 words_w1 = read_bytes<u32>(file);
        void *ptr = read_pointer(file, gfx_data, words_w1);
        if (ptr) {
            display_list_node->data[i].words.w0 = (uintptr_t) words_w0;
            display_list_node->data[i].words.w1 = (uintptr_t) ptr;
        } else {
            display_list_node->data[i].words.w0 = (uintptr_t) words_w0;
            display_list_node->data[i].words.w1 = (uintptr_t) words_w1;
        }
    }

    // Append
    gfx_data->display_lists.add(display_list_node);
}

static void omm_models_load_geo_layout_data(FILE *file, GfxData *gfx_data) {
    DataNode<GeoLayout> *geo_layout_node = alloc<DataNode<GeoLayout>>();

    // Name
    geo_layout_node->name.read(file);

    // Data
    geo_layout_node->size = read_bytes<u32>(file);
    geo_layout_node->data = alloc<GeoLayout>(geo_layout_node->size);
    for (u32 i = 0; i != geo_layout_node->size; ++i) {
        u32 value = read_bytes<u32>(file);
        void *ptr = read_pointer(file, gfx_data, value);
        if (ptr) {
            geo_layout_node->data[i] = (uintptr_t) ptr;
        } else {
            geo_layout_node->data[i] = (uintptr_t) value;
        }
    }

    // Append
    gfx_data->geo_layouts.add(geo_layout_node);
}

static void omm_models_load_animation_data(FILE *file, GfxData *gfx_data) {
    DataNode<AnimData> *anim_node = alloc<DataNode<AnimData>>();

    // Name
    anim_node->name.read(file);

    // Data
    anim_node->data = alloc<AnimData>();
    anim_node->data->flags = read_bytes<s16>(file);
    anim_node->data->anim_y_trans_div = read_bytes<s16>(file);
    anim_node->data->start_frame = read_bytes<s16>(file);
    anim_node->data->loop_start = read_bytes<s16>(file);
    anim_node->data->loop_end = read_bytes<s16>(file);
    anim_node->data->bone_count = read_bytes<s16>(file);
    anim_node->data->length = read_bytes<u32>(file);
    anim_node->data->values.second.read(file);
    anim_node->data->index.second.read(file);

    // Append
    gfx_data->animations.add(anim_node);
}

static void omm_models_load_animation_table(FILE *file, GfxData *gfx_data) {
    void *anim_ptr = NULL;

    // Data
    String anim_name; anim_name.read(file);
    if (anim_name != "NULL") {
        for (auto &anim_node : gfx_data->animations) {
            if (anim_node->name == anim_name) {
                anim_ptr = (void *) anim_node->data;
                break;
            }
        }
        if (!anim_ptr) { // Not a fatal error; the model can still use the vanilla anims
            return;
        }
    }

    // Append
    gfx_data->animation_table.add({ "", anim_ptr });
}

//
// Load from binary
//

GfxData *omm_models_load_from_binary(const SysPath &pack_folder, const char *actor_name) {
    struct GfxDataCache { SysPath pack_folder; Array<Pair<const char *, GfxData *>> gfx_data; };
    static Array<GfxDataCache *> gfx_data_cache;

    // Look for pack in cache
    GfxDataCache *cached_pack = NULL;
    for (s32 i = 0; i != gfx_data_cache.get_count(); ++i) {
        if (gfx_data_cache[i]->pack_folder == pack_folder) {
            cached_pack = gfx_data_cache[i];
            break;
        }
    }

    // Look for actor in pack
    if (cached_pack) {
        for (s32 i = 0; i != cached_pack->gfx_data.get_count(); ++i) {
            if (cached_pack->gfx_data[i].first == actor_name) { // Perfectly valid, actor_name comes from static RO data, so its address never changes during execution
                return cached_pack->gfx_data[i].second;
            }
        }
    }

    // Load data from binary file
    GfxData *gfx_data = NULL;
    SysPath filename = pack_folder + "/" + actor_name + ".bin";
    FILE *file = fopen(filename.c_str(), "rb");
    if (file) {
        gfx_data = alloc<GfxData>();
        for (bool done = false; !done;) {
            switch (read_bytes<u8>(file)) {
                case OMM_MODELS_DATA_TYPE_LIGHT:           omm_models_load_light_data       (file, gfx_data); break;
                case OMM_MODELS_DATA_TYPE_TEXTURE:         omm_models_load_texture_data     (file, gfx_data); break;
                case OMM_MODELS_DATA_TYPE_VERTEX:          omm_models_load_vertex_data      (file, gfx_data); break;
                case OMM_MODELS_DATA_TYPE_DISPLAY_LIST:    omm_models_load_display_list_data(file, gfx_data); break;
                case OMM_MODELS_DATA_TYPE_GEO_LAYOUT:      omm_models_load_geo_layout_data  (file, gfx_data); break;
                case OMM_MODELS_DATA_TYPE_ANIMATION:       omm_models_load_animation_data   (file, gfx_data); break;
                case OMM_MODELS_DATA_TYPE_ANIMATION_TABLE: omm_models_load_animation_table  (file, gfx_data); break;
                default:                                   done = true;                                       break;
            }
        }
        fclose(file);
    }

    // Add data to cache, even if not loaded
    if (cached_pack) {
        cached_pack->gfx_data.add({ actor_name, gfx_data });
    } else {
        cached_pack = alloc<GfxDataCache>();
        cached_pack->pack_folder = pack_folder;
        cached_pack->gfx_data.add({ actor_name, gfx_data });
        gfx_data_cache.add(cached_pack);
    }
    return gfx_data;
}
#endif
