#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Geo layout
//

const GeoLayout omm_geo_monty_mole_rock[] = {
    GEO_NODE_START(),
    GEO_OPEN_NODE(),
        GEO_BILLBOARD(),
        GEO_OPEN_NODE(),
            GEO_DISPLAY_LIST(LAYER_ALPHA, pebble_seg3_dl_0301CB00),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END()
};

//
// Behavior
//

static void bhv_omm_monty_mole_rock_delete(struct Object *o) {
    obj_spawn_white_puff(o, -1);
    obj_mark_for_deletion(o);
}

static void bhv_omm_monty_mole_rock_update() {
    struct Object *o = gCurrentObject;
    perform_object_step(o, OBJ_STEP_UPDATE_HOME);
    o->oVelY -= 2.f;

    // Out of bounds, or collided with a wall/ceiling/floor
    if (!o->oFloor || o->oWall || (o->oCeil && o->oCeil->normal.y > -0.9f) || o->oDistToFloor <= 5) {
        bhv_omm_monty_mole_rock_delete(o);
        return;
    }

    // Update
    obj_update_gfx(o);
    obj_set_params(o, 0, 0, 0, 0, true);
    obj_reset_hitbox(o, 30, 50, 0, 0, 15, 25);
    struct Object *interacted = omm_obj_process_interactions(o, (o->oMontyMoleRockPower >= 2.f ? OBJ_INT_PRESET_ROCK_LARGE : OBJ_INT_PRESET_ROCK_SMALL));
    if (interacted && !omm_obj_is_collectible(interacted)) {
        bhv_omm_monty_mole_rock_delete(o);
    }
}

const BehaviorScript bhvOmmMontyMoleRock[] = {
    OBJ_TYPE_SPECIAL,
    0x08000000,
    0x0C000000, (uintptr_t) bhv_omm_monty_mole_rock_update,
    0x09000000,
};

//
// Spawner
//

struct Object *omm_spawn_monty_mole_rock(struct Object *o, f32 power) {
    struct Object *rock = obj_spawn_from_geo(o, omm_geo_monty_mole_rock, bhvOmmMontyMoleRock);
    s16 da = o->oFaceAngleYaw + 0x4000;
    f32 dx = -40.f * o->oScaleX * sins(da);
    f32 dy = +25.f * o->oScaleY;
    f32 dz = -40.f * o->oScaleX * coss(da);
    rock->oPosX = o->oPosX + dx;
    rock->oPosY = o->oPosY + dy;
    rock->oPosZ = o->oPosZ + dz;
    rock->oFaceAngleYaw = o->oFaceAngleYaw;
    rock->oVelX = 36.f * (0.5f + power / 2.f) * sins(rock->oFaceAngleYaw);
    rock->oVelY = 20.f;
    rock->oVelZ = 36.f * (0.5f + power / 2.f) * coss(rock->oFaceAngleYaw);
    rock->oScaleX = (0.5f + power);
    rock->oScaleY = (0.5f + power);
    rock->oScaleZ = (0.5f + power);
    rock->oMontyMoleRockPower = power;
    obj_play_sound(o, SOUND_OBJ_MONTY_MOLE_ATTACK);
    return rock;
}
