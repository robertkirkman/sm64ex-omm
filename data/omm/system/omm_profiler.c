#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#define REFRESH_RATE 15
#define GFX_PROFILER 1 // Set to 1 if needed

static struct {
    struct {
        s64 curr;
        s64 sum;
        f64 disp;
    } pc[OMM_PRF_MAX];
#if OMM_CODE_DEBUG && GFX_PROFILER
    struct {
        s64 curr;
        s64 sum;
        s64 count;
        f64 disp[2];
    } op[0x40];
#endif
    s64 timer;
    s64 counter;
    s64 disp;
    Vtx vtx[0x10000];
    Gfx gfx[0x10000];
} sOmmProfiler[1];

void omm_profiler_start(s32 prf) {
#if OMM_CODE_DEBUG
    sOmmProfiler->pc[prf].curr = SDL_GetPerformanceCounter();
#endif
}

void omm_profiler_stop(s32 prf) {
#if OMM_CODE_DEBUG
    sOmmProfiler->pc[prf].sum += SDL_GetPerformanceCounter() - sOmmProfiler->pc[prf].curr;
#endif
}

void omm_profiler_start_gfx_op(u8 op) {
#if OMM_CODE_DEBUG && GFX_PROFILER
    u8 iop = (op == 0x06 ? 0x05 : (op == 0x25 ? 0x24 : op));
    sOmmProfiler->op[iop].curr = SDL_GetPerformanceCounter();
#endif
}

void omm_profiler_stop_gfx_op(u8 op) {
#if OMM_CODE_DEBUG && GFX_PROFILER
    u8 iop = (op == 0x06 ? 0x05 : (op == 0x25 ? 0x24 : op));
    sOmmProfiler->op[iop].sum += SDL_GetPerformanceCounter() - sOmmProfiler->op[iop].curr;
    sOmmProfiler->op[iop].count += (op == 0x06 ? 2 : 1);
#endif
}

void omm_profiler_frame_drawn() {
    sOmmProfiler->counter++;
}

OMM_ROUTINE_UPDATE(omm_profiler_update) {
    s64 curr = SDL_GetPerformanceCounter();
    sOmmProfiler->pc[OMM_PRF_FPS].sum += curr - sOmmProfiler->pc[OMM_PRF_FPS].curr;
    sOmmProfiler->pc[OMM_PRF_FPS].curr = curr;
    if (sOmmProfiler->timer++ % REFRESH_RATE == 0) {
        f64 freq = SDL_GetPerformanceFrequency();
        for (s32 i = 0; i != OMM_PRF_MAX; ++i) {
            sOmmProfiler->pc[i].disp = (f64) sOmmProfiler->pc[i].sum / (freq * REFRESH_RATE);
            sOmmProfiler->pc[i].sum = 0;
        }
        sOmmProfiler->disp = sOmmProfiler->counter;
        sOmmProfiler->counter = 0;
#if OMM_CODE_DEBUG && GFX_PROFILER
        for (s32 i = 0; i != 0x40; ++i) {
            sOmmProfiler->op[i].disp[0] = (f64) sOmmProfiler->op[i].sum / (freq * REFRESH_RATE);
            sOmmProfiler->op[i].disp[1] = (f64) sOmmProfiler->op[i].count / REFRESH_RATE; // calls per frame
            sOmmProfiler->op[i].sum = 0;
            sOmmProfiler->op[i].count = 0;
        }
#endif
    }
}

static void omm_profiler_print_text(Vtx **vtx, Gfx **gfx, const char *str, s32 x, s32 y, s32 w, s32 h) {
    *gfx = gfx_create_ortho_matrix(*gfx);
    OMM_RENDER_ENABLE_ALPHA((*gfx)++);
    gDPSetEnvColor((*gfx)++, 0xFF, 0xFF, 0xFF, 0xE0);
    gSPClearGeometryMode((*gfx)++, G_LIGHTING);
    gDPSetCombineLERP((*gfx)++, TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT);
    gSPTexture((*gfx)++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock((*gfx)++, OMM_TEXTURE_MENU_FONT, G_IM_FMT_RGBA, G_IM_SIZ_32b, 1024, 1024, 0, G_TX_WRAP, G_TX_WRAP, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
    for (char *c = (char *) str, d = 0, e = 0; *c; ++c, d = (d + (*c >= '1' && *c <= '9')) * (*c != ' '), e = ((*c >= '0' && *c <= '9') || *c == '.')) {
        if (*c >= ' ' && *c <= '~') {
            s32 u = ((((*c - 0x20) % 20) * 48 + 0) * 32);
            s32 v = ((((*c - 0x20) / 20) * 80 - 8) * 32);
            s32 i = (*c == '0' && !d ? 0x10 : (e ? 0xFF : 0xAA));
            s32 j = (*c == '$' ? 2 : 1);
            gSPVertex((*gfx)++, *vtx, 4, 0);
            gSP2Triangles((*gfx)++, 0, 1, 2, 0, 0, 2, 3, 0);
            *((*vtx)++) = (Vtx) { { { x ,          y,     0 }, 0, { u,        v + 2560 }, { i, i, i, 0xFF } } };
            *((*vtx)++) = (Vtx) { { { x + (w / j), y,     0 }, 0, { u + 1536, v + 2560 }, { i, i, i, 0xFF } } };
            *((*vtx)++) = (Vtx) { { { x + (w / j), y + h, 0 }, 0, { u + 1536, v        }, { i, i, i, 0xFF } } };
            *((*vtx)++) = (Vtx) { { { x,           y + h, 0 }, 0, { u,        v        }, { i, i, i, 0xFF } } };
            x += (w / j);
        }
    }
    gSPTexture((*gfx)++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetCombineLERP((*gfx)++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    gSPSetGeometryMode((*gfx)++, G_LIGHTING);
}

void omm_profiler_display() {
    if (gOmmShowFPS && !omm_is_main_menu()) {
        Vtx *vtx = sOmmProfiler->vtx;
        Gfx *gfx = sOmmProfiler->gfx;
        gSPDisplayList(gDisplayListHead++, gfx);
#if OMM_CODE_DEBUG && GFX_PROFILER
        static const char *_ops[] = {
            [0x01] = "sp_vertex",
            [0x05] = "sp_tri",
            [0x17] = "sp_texture",
            [0x18] = "sp_pop_matrix",
            [0x19] = "sp_geometry_mode",
            [0x1A] = "sp_matrix",
            [0x1B] = "sp_moveword",
            [0x1C] = "sp_movemem",
            [0x22] = "sp_set_other_mode_l",
            [0x23] = "sp_set_other_mode_h",
            [0x24] = "dp_texture_rectangle",
            [0x2D] = "dp_set_scissor",
            [0x31] = "xp_swap_cmd",
            [0x32] = "dp_set_tile_size",
            [0x33] = "dp_load_block",
            [0x34] = "dp_load_tile",
            [0x35] = "dp_set_tile",
            [0x36] = "dp_fill_rectangle",
            [0x37] = "dp_set_fill_color",
            [0x38] = "dp_set_fog_color",
            [0x3A] = "dp_set_prim_color",
            [0x3B] = "dp_set_env_color",
            [0x3C] = "dp_set_combine_mode",
            [0x3D] = "dp_set_texture_image",
            [0x3E] = "dp_set_z_image",
            [0x3F] = "dp_set_color_image",
        };
        for (s32 i = array_length(_ops) - 1, j = 0; i != 0; --i) {
            if (!_ops[i]) continue;
            char buffer[64];
            str_fmt(buffer, 64, "$%-20s %06d us %08d %06d ns$",
                _ops[i],
                (s32) (sOmmProfiler->op[i].disp[0] * 1000000.0),
                (s32) (sOmmProfiler->op[i].disp[1]),
                (s32) (sOmmProfiler->op[i].disp[0] * 1000000000.0 / max(1, sOmmProfiler->op[i].disp[1]))
            );
            omm_profiler_print_text(&vtx, &gfx, buffer, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(50 * 3 + 2), 2 + 5 * j++, 3, 5);
        }
#endif
#if OMM_CODE_DEBUG
        static const char *_prf[] = { "OMM", "LVL", "PRE", "GEO", "GFX", "RDR", "FRM" };
        for (s32 i = OMM_PRF_OMM; i <= OMM_PRF_FRM; ++i) {
            char buffer[32];
            str_fmt(buffer, 32, "$%s %06d$", _prf[i], (s32) (1000000.0 * sOmmProfiler->pc[i].disp));
            omm_profiler_print_text(&vtx, &gfx, buffer, GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(2), 2 + 8 * (OMM_PRF_MAX - (i + 1)), 5, 8);
        }
#endif
        {
            char buffer[32];
            str_fmt(buffer, 32, "$FPS %06.2f$", (f32) sOmmProfiler->disp / (sOmmProfiler->pc[OMM_PRF_FPS].disp * REFRESH_RATE));
            omm_profiler_print_text(&vtx, &gfx, buffer, GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(2), 2, 5, 8);
        }
        gSPEndDisplayList(gfx);
    }
}
