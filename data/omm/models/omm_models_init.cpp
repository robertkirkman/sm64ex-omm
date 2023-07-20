#if !defined(DYNOS)
#include "omm_models.cpp.h"
extern "C" {
#include <dirent.h>
#include "pc/fs/fs.h"
#include "engine/geo_layout.h"
}

Array<ActorGfx> &omm_models_get_actor_list() {
    static Array<ActorGfx> actor_list;
    return actor_list;
}

Array<const char *> &omm_models_get_packs() {
    static Array<const char *> pack_list;
    return pack_list;
}

extern "C" {
const char **omm_models_init() {

    // Alloc and init the actors gfx list
    Array<ActorGfx> &actor_list = omm_models_get_actor_list();
    actor_list.resize(omm_models_get_actor_count());
    for (s32 i = 0; i != omm_models_get_actor_count(); ++i) {
        actor_list[i].pack_index = -1;
        actor_list[i].gfx_data   = NULL;
        actor_list[i].graph_node = (GraphNode *) geo_layout_to_graph_node(NULL, (const GeoLayout *) omm_models_get_actor_layout(i));
    }

    // Scan the packs folders (res, dynos)
    Array<const char *> &pack_list = omm_models_get_packs();
    for (const SysPath &basedir : { FS_BASEDIR, "dynos" }) {
        SysPath packs_folder = SysPath(sys_exe_path()) + "/" + basedir + "/packs";
        DIR *packs_dir = opendir(packs_folder.c_str());
        if (packs_dir) {
            struct dirent *packs_ent = NULL;
            while ((packs_ent = readdir(packs_dir)) != NULL) {

                // Skip . and ..
                if (SysPath(packs_ent->d_name) == ".") continue;
                if (SysPath(packs_ent->d_name) == "..") continue;

                // If pack folder exists, add it to the pack list
                SysPath pack_folder = packs_folder + "/" + packs_ent->d_name;
                if (fs_sys_dir_exists(pack_folder.c_str())) {
                    char *pack = (char *) calloc(sizeof(char), pack_folder.length() + 1);
                    memcpy(pack, pack_folder.c_str(), pack_folder.length());
                    pack_list.add((const char *) pack);
                }
            }
            closedir(packs_dir);
        }
    }

    // Return a list of pack names
    s32 pack_count = pack_list.get_count();
    if (pack_count) {
        char **pack_names = (char **) calloc(sizeof(char *), pack_count + 1);
        for (s32 i = 0; i != pack_count; ++i) {
            SysPath pack = SysPath(pack_list[i]);
            u64 dir_sep_w = pack.find_last_of('\\');
            u64 dir_sep_l = pack.find_last_of('/');
            if (dir_sep_w++ == SysPath::npos) dir_sep_w = 0;
            if (dir_sep_l++ == SysPath::npos) dir_sep_l = 0;
            SysPath dir_name = pack.substr(MAX(dir_sep_w, dir_sep_l));
            char *pack_name = (char *) calloc(sizeof(char), dir_name.length() + 1);
            memcpy(pack_name, dir_name.c_str(), dir_name.length());
            pack_names[i] = pack_name;
        }
        pack_names[pack_count] = NULL;
        return (const char **) pack_names;
    }
    return NULL;
}
}
#endif
