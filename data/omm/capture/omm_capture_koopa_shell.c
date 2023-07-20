#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Init
//

bool omm_cappy_koopa_shell_init(struct Object *o) {
    gOmmMario->capture.bhv = bhvOmmPossessedKoopaShell;
    gOmmObject->state.actionFlag = false;
    gOmmObject->state.actionState = (o->behavior == bhvKoopaShellUnderwater);
    o->behavior = bhvOmmPossessedKoopaShell;
    return true;
}

void omm_cappy_koopa_shell_end(struct Object *o) {
    o->behavior = (gOmmObject->state.actionState ? bhvKoopaShellUnderwater : bhvKoopaShell);
    audio_stop_shell_music();
}

f32 omm_cappy_koopa_shell_get_top(struct Object *o) {
    return 100.f * o->oScaleY;
}

//
// Update
//

s32 omm_cappy_koopa_shell_update(struct Object *o) {
    bool isUnderwater = (gOmmObject->state.actionState && obj_is_underwater(o, find_water_level(o->oPosX, o->oPosZ)));

    // Init
    if (!gOmmObject->state.actionFlag) {
        audio_play_shell_music();
        gOmmObject->state.actionFlag = true;
    }

    // Inputs
    if (!omm_mario_is_locked(gMarioState)) {
        pobj_move(o, false, POBJ_B_BUTTON_DOWN, false);
        if (pobj_jump(o, 0, 1) == POBJ_RESULT_JUMP_START) {
            obj_play_sound(o, SOUND_OBJ_GOOMBA_ALERT);
        }
    }

    // Hitbox
    o->hitboxRadius = omm_capture_get_hitbox_radius(o);
    o->hitboxHeight = omm_capture_get_hitbox_height(o);
    o->hitboxDownOffset = omm_capture_get_hitbox_down_offset(o);
    o->oWallHitboxRadius = omm_capture_get_wall_hitbox_radius(o);

    // Properties
    POBJ_SET_ABOVE_WATER;
    POBJ_SET_IMMUNE_TO_LAVA;
    POBJ_SET_IMMUNE_TO_SAND;
    POBJ_SET_ABLE_TO_MOVE_ON_SLOPES;
    POBJ_SET_ATTACKING;
    if (gOmmObject->state.actionState) {
        POBJ_SET_UNDER_WATER;
    } else {
        POBJ_SET_ABLE_TO_MOVE_ON_WATER;
    }

    // Movement
    f32 gravityFactor = 1.f;
    f32 velocityFactor = 1.f;
    if (isUnderwater) {
        gravityFactor = 0.5f;
        velocityFactor = 0.8f;
        o->oVelY = clamp_f(o->oVelY,
            gravityFactor * omm_capture_get_terminal_velocity(o) * POBJ_PHYSICS_GRAVITY,
            gravityFactor * omm_capture_get_jump_velocity(o) * POBJ_PHYSICS_JUMP * 1.6f
        );
    }
    o->oVelX *= velocityFactor;
    o->oVelZ *= velocityFactor;
    o->oForwardVel *= velocityFactor;
    perform_object_step(o, POBJ_STEP_FLAGS);
    o->oVelX /= velocityFactor;
    o->oVelZ /= velocityFactor;
    o->oForwardVel /= velocityFactor;
    pobj_decelerate(o, 0.80f, 0.95f);
    pobj_apply_gravity(o, gravityFactor);
    pobj_handle_special_floors(o);
    pobj_stop_if_unpossessed();

    // Interactions
    pobj_process_interactions();
    pobj_stop_if_unpossessed();

    // Gfx
    obj_update_gfx(o);
    o->oGfxAngle[0] = 0;
    o->oGfxAngle[1] = o->oTimer * 0x2000;
    o->oGfxAngle[2] = 0;
    spawn_object(o, MODEL_NONE, bhvSparkleSpawn);

    // Particles and sound effect
    if (isUnderwater && (gGlobalTimer % 6) == 0) obj_spawn_particle_preset(o, PARTICLE_BUBBLE, false);
    switch (o->oFloorType) {
        case OBJ_FLOOR_TYPE_GROUND: {
            obj_make_step_sound_and_particle(o,
                &gOmmObject->state.walkDistance, 0.f, 0.f,
                SOUND_MOVING_TERRAIN_RIDING_SHELL + gMarioState->terrainSoundAddend,
                OBJ_PARTICLE_MIST
            );
        } break;

        case OBJ_FLOOR_TYPE_WATER: {
            obj_make_step_sound_and_particle(o,
                &gOmmObject->state.walkDistance, 0.f, 0.f,
                SOUND_MOVING_TERRAIN_RIDING_SHELL + SOUND_TERRAIN_WATER,
                OBJ_PARTICLE_WATER_TRAIL | OBJ_PARTICLE_WATER_DROPLET
            );
        } break;

        case OBJ_FLOOR_TYPE_LAVA: {
            obj_make_step_sound_and_particle(o,
                &gOmmObject->state.walkDistance, 0.f, 0.f,
                SOUND_MOVING_RIDING_SHELL_LAVA,
                OBJ_PARTICLE_FLAME
            );
        } break;
    }

    // Cappy values
    gOmmObject->cappy.scale = 0.f;

    // OK
    pobj_return_ok;
}
