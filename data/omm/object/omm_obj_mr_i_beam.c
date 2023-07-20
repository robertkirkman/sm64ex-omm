#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Geo layout
//

const GeoLayout omm_geo_mr_i_beam[] = {
    OMM_GEO_BILLBOARD_AND_BRANCH(purple_marble_geo),
};

//
// Behavior
//

static void bhv_omm_mr_i_beam_delete(struct Object *o) {
    for (s32 i = 0; i != 10; ++i) {
        struct Object *p = spawn_object(o, MODEL_PURPLE_MARBLE, bhvPurpleParticle);
        obj_scale(p, 0.5f);
    }
    obj_play_sound(o, SOUND_OBJ_DEFAULT_DEATH);
    obj_mark_for_deletion(o);
}

static void bhv_omm_mr_i_beam_loop() {
    struct Object *o = gCurrentObject;
    perform_object_step(o, OBJ_STEP_UPDATE_HOME);
    if (o->oTimer > 60 || !o->oFloor || o->oWall || (o->oCeil && o->oCeil->normal.y > -0.9f) || (o->oDistToFloor <= 5 && o->oFloor->normal.y < 0.9f)) {
        bhv_omm_mr_i_beam_delete(o);
        return;
    }
    obj_reset_hitbox(o, 20, 30, 0, 0, 15, 0);
    obj_set_params(o, 0, 0, 0, 0, true);
    struct Object *interacted = omm_obj_process_interactions(o, (o->oMrIBeamPower >= 2.f ? OBJ_INT_PRESET_BEAM_LARGE : OBJ_INT_PRESET_BEAM_SMALL));
    if (interacted && !omm_obj_is_collectible(interacted)) {
        bhv_omm_mr_i_beam_delete(o);
    }
}

const BehaviorScript bhvOmmMrIBeam[] = {
    OBJ_TYPE_GENACTOR,
    0x11010001,
    0x08000000,
    0x0C000000, (uintptr_t) bhv_omm_mr_i_beam_loop,
    0x09000000,
};

//
// Spawner
//

struct Object *omm_spawn_mr_i_beam(struct Object *o, f32 power) {
    struct Object *beam = obj_spawn_from_geo(o, omm_geo_mr_i_beam, bhvOmmMrIBeam);
    beam->oPosX += o->hitboxRadius * 1.1f * sins(o->oMoveAngleYaw);
    beam->oPosY += o->hitboxHeight * 0.5f - 30.f * power;
    beam->oPosZ += o->hitboxRadius * 1.1f * coss(o->oMoveAngleYaw);
    beam->oVelX = 30.f * (0.5f + power) * o->oScaleX * sins(o->oMoveAngleYaw);
    beam->oVelY = 0.f;
    beam->oVelZ = 30.f * (0.5f + power) * o->oScaleZ * coss(o->oMoveAngleYaw);
    beam->oScaleX = 2.f * power * o->oScaleX;
    beam->oScaleY = 2.f * power * o->oScaleY;
    beam->oScaleZ = 2.f * power * o->oScaleZ;
    beam->oMrIBeamPower = power;
    obj_play_sound(o, SOUND_OBJ_MRI_SHOOT);
    return beam;
}
