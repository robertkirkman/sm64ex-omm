#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Init
//

bool omm_cappy_crazy_box_init(UNUSED struct Object *o) {
    gOmmObject->state.actionState = 0;
    gOmmObject->state.actionTimer = 0;
    gOmmMario->capture.lockTimer = 1;
    return true;
}

void omm_cappy_crazy_box_end(UNUSED struct Object *o) {
}

f32 omm_cappy_crazy_box_get_top(struct Object *o) {
    return 200.f * o->oScaleY;
}

//
// Update
//

s32 omm_cappy_crazy_box_update(struct Object *o) {

    // Hitbox
    o->hitboxRadius = omm_capture_get_hitbox_radius(o);
    o->hitboxHeight = omm_capture_get_hitbox_height(o);
    o->hitboxDownOffset = omm_capture_get_hitbox_down_offset(o);
    o->oWallHitboxRadius = omm_capture_get_wall_hitbox_radius(o);

    // Properties
    POBJ_SET_ABOVE_WATER;
    POBJ_SET_ABLE_TO_MOVE_ON_SLOPES;

    // Inputs
    if (!omm_mario_is_locked(gMarioState)) {
        if (obj_is_on_ground(o) && (gOmmObject->state.actionState == 3)) {
            omm_mario_unpossess_object(gMarioState, OMM_MARIO_UNPOSSESS_ACT_JUMP_OUT, false, 0);
            obj_destroy(o);
            pobj_return_unpossess;
        }
        pobj_move(o, false, false, false);
        switch (pobj_jump(o, 1.6f, 1)) {
            case POBJ_RESULT_HOP_SMALL: {
                obj_play_sound(o, SOUND_GENERAL_BOING1);
            } break;
            case POBJ_RESULT_HOP_LARGE: {
                if (POBJ_B_BUTTON_DOWN) {
                    o->oVelY = omm_capture_get_jump_velocity(o) * POBJ_PHYSICS_JUMP * (1.50f + 0.25f * (gOmmObject->state.actionState++)); // 1.50, 1.75, 2.00
                    obj_spawn_white_puff(o, SOUND_GENERAL_BOING2);
                } else {
                    obj_play_sound(o, SOUND_GENERAL_BOING1);
                }
            } break;
        }
    }
    pobj_stop_if_unpossessed();

    // Movement
    perform_object_step(o, POBJ_STEP_FLAGS);
    pobj_decelerate(o, 0.80f, 0.95f);
    pobj_apply_gravity(o, 1.f);

    // Lava boost
    if (o->oVelY > 0 && gOmmObject->state.actionTimer == 1) {
        spawn_object(o, MODEL_BURN_SMOKE, bhvBlackSmokeMario);
        obj_play_sound(o, SOUND_MOVING_LAVA_BURN);
    } else if (o->oFloor) {
        if (o->oFloor->type == SURFACE_BURNING) {
            if (gOmmObject->state.actionTimer == 0 && o->oDistToFloor <= 10.f && POBJ_A_BUTTON_DOWN) {
                o->oVelY = max_f(o->oVelY, omm_capture_get_jump_velocity(o) * POBJ_PHYSICS_JUMP);
                o->oPosY = max_f(o->oPosY, o->oFloorHeight + 10.f);
                for (s32 i = 0; i != 8; ++i) spawn_object(o, MODEL_RED_FLAME, bhvKoopaShellFlame);
                obj_play_sound(o, SOUND_MOVING_RIDING_SHELL_LAVA);
                obj_play_sound(o, SOUND_GENERAL_BOING1);
                gOmmObject->state.actionTimer = 1;
            }
        } else {
            gOmmObject->state.actionTimer = 0;
        }
    }

    // Special floors
    pobj_handle_special_floors(o);
    pobj_stop_if_unpossessed();

    // Interactions
    pobj_process_interactions();
    pobj_stop_if_unpossessed();

    // Gfx
    obj_update_gfx(o);

    // Cappy values
    gOmmObject->cappy.offset[1] = 200.f;
    gOmmObject->cappy.scale     = 2.f;

    // OK
    pobj_return_ok;
}
