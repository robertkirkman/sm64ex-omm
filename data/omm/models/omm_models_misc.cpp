#if !defined(DYNOS)
#include "omm_models.cpp.h"
extern "C" {
#include "object_fields.h"
#include "data/omm/object/omm_object_data.h"
#include "engine/math_util.h"
#include "engine/graph_node_o.h"
#include "game/object_helpers.h"
#include "game/segment2.h"
#include "game/level_geo.h"
#include "game/level_update.h"
#include "game/moving_texture.h"
#include "game/paintings.h"
#include "game/geo_misc.h"
#include "game/mario_misc.h"
#include "game/mario_actions_cutscene.h"
#include "game/screen_transition.h"
#include "game/object_list_processor.h"
#include "game/behavior_actions.h"
#include "game/rendering_graph_node.h"
#include "actors/common0.h"
#include "actors/common1.h"
#include "actors/group0.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group3.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group8.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group16.h"
#include "actors/group17.h"
}

//
// Geo
//

static void *geo_rotate_3d_coin(s32 callContext, void *node, UNUSED void *c) {
    if (callContext == GEO_CONTEXT_RENDER) {
        struct Object *obj = (struct Object *) gCurGraphNodeObject;
        obj->oFaceAnglePitch = obj->oFaceAngleRoll = 0;
        obj->oMoveAnglePitch = obj->oMoveAngleRoll = 0;
        struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) ((struct GraphNode *) node)->next;
        vec3s_set(gCurGraphNodeObject->angle, 0, 0, 0);
        vec3s_set(rotNode->rotation, 0, obj->oTimer * 0x800, 0);
    }
    return NULL;
}

//
// Actors
//

#define define_actor(geo) (const void *) #geo, (const void *) geo
static const void *actor_layout_refs[] = {
define_actor(amp_geo),
define_actor(birds_geo),
define_actor(blargg_geo),
define_actor(blue_coin_switch_geo),
define_actor(black_bobomb_geo),
define_actor(bobomb_buddy_geo),
define_actor(boo_geo),
define_actor(boo_castle_geo),
define_actor(bookend_geo),
define_actor(bookend_part_geo),
define_actor(bowling_ball_geo),
define_actor(bowling_ball_track_geo),
define_actor(bowser_geo),
define_actor(bowser2_geo),
define_actor(bowser_bomb_geo),
define_actor(bowser_flames_geo),
define_actor(bowser_impact_smoke_geo),
define_actor(bowser_1_yellow_sphere_geo),
define_actor(invisible_bowser_accessory_geo),
define_actor(bowser_key_geo),
define_actor(bowser_key_cutscene_geo),
define_actor(breakable_box_geo),
define_actor(breakable_box_small_geo),
define_actor(bub_geo),
define_actor(bubba_geo),
define_actor(bubble_geo),
define_actor(bullet_bill_geo),
define_actor(bully_geo),
define_actor(bully_boss_geo),
define_actor(burn_smoke_geo),
define_actor(butterfly_geo),
define_actor(cannon_barrel_geo),
define_actor(cannon_base_geo),
define_actor(cap_switch_geo),
define_actor(cartoon_star_geo),
define_actor(chain_chomp_geo),
define_actor(checkerboard_platform_geo),
define_actor(chilly_chief_geo),
define_actor(chilly_chief_big_geo),
define_actor(chuckya_geo),
define_actor(clam_shell_geo),
define_actor(yellow_coin_geo),
define_actor(yellow_coin_no_shadow_geo),
define_actor(blue_coin_geo),
define_actor(blue_coin_no_shadow_geo),
define_actor(red_coin_geo),
define_actor(red_coin_no_shadow_geo),
define_actor(dirt_animation_geo),
define_actor(dorrie_geo),
define_actor(cabin_door_geo),
define_actor(castle_door_geo),
define_actor(castle_door_0_star_geo),
define_actor(castle_door_1_star_geo),
define_actor(castle_door_3_stars_geo),
define_actor(haunted_door_geo),
define_actor(hazy_maze_door_geo),
define_actor(metal_door_geo),
define_actor(key_door_geo),
define_actor(wooden_door_geo),
define_actor(enemy_lakitu_geo),
define_actor(exclamation_box_geo),
define_actor(exclamation_box_outline_geo),
define_actor(explosion_geo),
define_actor(eyerok_left_hand_geo),
define_actor(eyerok_right_hand_geo),
define_actor(fish_geo),
define_actor(cyan_fish_geo),
define_actor(flyguy_geo),
define_actor(red_flame_geo),
define_actor(red_flame_shadow_geo),
define_actor(blue_flame_geo),
define_actor(fwoosh_geo),
define_actor(goomba_geo),
define_actor(haunted_cage_geo),
define_actor(haunted_chair_geo),
define_actor(heart_geo),
define_actor(heave_ho_geo),
define_actor(hoot_geo),
define_actor(king_bobomb_geo),
define_actor(klepto_geo),
define_actor(koopa_with_shell_geo),
define_actor(koopa_without_shell_geo),
define_actor(koopa_flag_geo),
define_actor(koopa_shell_geo),
define_actor(lakitu_geo),
define_actor(mad_piano_geo),
define_actor(manta_seg5_geo_05008D14),
define_actor(mario_geo),
define_actor(marios_cap_geo),
define_actor(marios_metal_cap_geo),
define_actor(marios_wing_cap_geo),
define_actor(marios_winged_metal_cap_geo),
define_actor(metal_box_geo),
define_actor(metallic_ball_geo),
define_actor(mips_geo),
define_actor(mist_geo),
define_actor(moneybag_geo),
define_actor(monty_mole_geo),
define_actor(mr_blizzard_geo),
define_actor(mr_blizzard_hidden_geo),
define_actor(mr_i_geo),
define_actor(mr_i_iris_geo),
define_actor(mushroom_1up_geo),
define_actor(number_geo),
define_actor(peach_geo),
define_actor(penguin_geo),
define_actor(piranha_plant_geo),
define_actor(pokey_head_geo),
define_actor(pokey_body_part_geo),
define_actor(purple_marble_geo),
define_actor(purple_switch_geo),
define_actor(scuttlebug_geo),
define_actor(seaweed_geo),
define_actor(skeeter_geo),
define_actor(small_key_geo),
define_actor(small_water_splash_geo),
define_actor(smoke_geo),
define_actor(snufit_geo),
define_actor(sparkles_geo),
define_actor(sparkles_animation_geo),
define_actor(spindrift_geo),
define_actor(spiny_geo),
define_actor(spiny_ball_geo),
define_actor(star_geo),
define_actor(transparent_star_geo),
define_actor(sushi_geo),
define_actor(swoop_geo),
define_actor(thwomp_geo),
define_actor(toad_geo),
define_actor(treasure_chest_base_geo),
define_actor(treasure_chest_lid_geo),
define_actor(bubbly_tree_geo),
define_actor(spiky_tree_geo),
define_actor(snow_tree_geo),
define_actor(palm_tree_geo),
define_actor(leaves_geo),
define_actor(tweester_geo),
define_actor(ukiki_geo),
define_actor(unagi_geo),
define_actor(warp_pipe_geo),
define_actor(water_bomb_geo),
define_actor(water_bomb_shadow_geo),
define_actor(water_ring_geo),
define_actor(water_splash_geo),
define_actor(idle_water_wave_geo),
define_actor(wave_trail_geo),
define_actor(white_particle_geo),
define_actor(white_puff_geo),
define_actor(whomp_geo),
define_actor(wiggler_head_geo),
define_actor(wiggler_body_geo),
define_actor(wooden_post_geo),
define_actor(wooden_signpost_geo),
define_actor(yellow_sphere_geo),
define_actor(yoshi_geo),
define_actor(yoshi_egg_geo),
#define OMM_GEO_(geo) define_actor(geo),
#include "data/omm/object/omm_object_data_geo.inl"
#undef OMM_GEO
};

s32 omm_models_get_actor_count() {
    return (s32) (sizeof(actor_layout_refs) / (2 * sizeof(actor_layout_refs[0])));
}

const char *omm_models_get_actor_name(s32 index) {
    return (const char *) actor_layout_refs[2 * index];
}

const void *omm_models_get_actor_layout(s32 index) {
    return (const void *) actor_layout_refs[2 * index + 1];
}

s32 omm_models_get_actor_index(const void *geo_layout) {
    for (s32 i = 0; i != omm_models_get_actor_count(); ++i) {
        if (actor_layout_refs[2 * i + 1] == geo_layout) {
            return i;
        }
    }
    return -1;
}

//
// Geo Functions
//

static const Array<Pair<const char *, void *>> &__geo_functions() {
#define define_geo_function(name) { #name, (void *) name }
static const Array<Pair<const char *, void *>> geo_functions = {
    define_geo_function(geo_mirror_mario_set_alpha),
    define_geo_function(geo_switch_mario_stand_run),
    define_geo_function(geo_switch_mario_eyes),
    define_geo_function(geo_mario_tilt_torso),
    define_geo_function(geo_mario_head_rotation),
    define_geo_function(geo_switch_mario_hand),
    define_geo_function(geo_mario_hand_foot_scaler),
    define_geo_function(geo_switch_mario_cap_effect),
    define_geo_function(geo_switch_mario_cap_on_off),
    define_geo_function(geo_mario_rotate_wing_cap_wings),
    define_geo_function(geo_switch_mario_hand_grab_pos),
    define_geo_function(geo_render_mirror_mario),
    define_geo_function(geo_mirror_mario_backface_culling),
    define_geo_function(geo_update_projectile_pos_from_parent),
    define_geo_function(geo_update_layer_transparency),
    define_geo_function(geo_switch_anim_state),
    define_geo_function(geo_switch_area),
    define_geo_function(geo_camera_main),
    define_geo_function(geo_camera_fov),
    define_geo_function(geo_envfx_main),
    define_geo_function(geo_skybox_main),
    define_geo_function(geo_wdw_set_initial_water_level),
    define_geo_function(geo_movtex_pause_control),
    define_geo_function(geo_movtex_draw_water_regions),
    define_geo_function(geo_movtex_draw_nocolor),
    define_geo_function(geo_movtex_draw_colored),
    define_geo_function(geo_movtex_draw_colored_no_update),
    define_geo_function(geo_movtex_draw_colored_2_no_update),
    define_geo_function(geo_movtex_update_horizontal),
    define_geo_function(geo_movtex_draw_colored_no_update),
    define_geo_function(geo_painting_draw),
    define_geo_function(geo_painting_update),
    define_geo_function(geo_exec_inside_castle_light),
    define_geo_function(geo_exec_flying_carpet_timer_update),
    define_geo_function(geo_exec_flying_carpet_create),
    define_geo_function(geo_exec_cake_end_screen),
    define_geo_function(geo_cannon_circle_base),
    define_geo_function(geo_move_mario_part_from_parent),
    define_geo_function(geo_bits_bowser_coloring),
    define_geo_function(geo_update_body_rot_from_parent),
    define_geo_function(geo_switch_bowser_eyes),
    define_geo_function(geo_switch_tuxie_mother_eyes),
    define_geo_function(geo_update_held_mario_pos),
    define_geo_function(geo_snufit_move_mask),
    define_geo_function(geo_snufit_scale_body),
    define_geo_function(geo_scale_bowser_key),
    { "geo_rotate_coin", (void *) geo_rotate_3d_coin },
    define_geo_function(geo_offset_klepto_held_object),
    define_geo_function(geo_switch_peach_eyes),
};
#undef define_geo_function
return geo_functions;
}
#define geo_functions __geo_functions()

void *omm_models_get_func_pointer(s32 index) {
    return geo_functions[index].second;
}
#endif
