#ifndef OMM_MODELS_C_H
#define OMM_MODELS_C_H
#if defined(DYNOS)
#define omm_models_init()
#define omm_models_update()
#define omm_models_swap_animations(ptr) dynos_gfx_swap_animations(ptr)
#define omm_models_get_mario_model_pack_index() dynos_gfx_get_mario_model_pack_index()
#define omm_models_find_texture(...) dynos_gfx_texture_find(__VA_ARGS__)
#else
const char **omm_models_init();
void omm_models_update();
void omm_models_swap_animations(void *ptr);
s32 omm_models_get_mario_model_pack_index();
#endif
#endif
