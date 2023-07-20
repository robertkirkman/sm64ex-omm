//Feel free to use it in your port too, but please keep authorship!
//Touch Controls made by: VDavid003
#ifdef TOUCH_CONTROLS
#include <ultra64.h>
#include <PR/ultratypes.h>
#include <PR/gbi.h>

#include "config.h"
#include "sm64.h"
#include "game/game_init.h"
#include "game/memory.h"
#include "game/segment2.h"
#include "gfx_dimensions.h"
#include "pc/gfx/gfx_pc.h"

#include "controller_api.h"
#include "controller_touchscreen.h"

// Mouselook
s16 before_x = 0;
s16 before_y = 0;
s16 touch_x = 0;
s16 touch_y = 0;
s16 touch_cam_last_x = 0;
s16 touch_cam_last_y = 0;

// Config
// TOUCH_MOUSE used as None
enum ConfigControlElementIndex lastElementGrabbed = TOUCH_MOUSE;
u32 double_tap_timer = 0, double_tap_timer_last = 0;
bool gInTouchConfig = false,
     gGamepadActive = false,
     configAutohideTouch = false,
     configSlideTouch = true, 
     configElementSnap = false;

// these are the default screen positions and sizes of the touch controls 
// in the order of ConfigControlElementIndex
// right now only the first element of each array of each member of each
// struct in the array is used, I'm on the fence about whether to change
// that
ConfigControlElement configControlElementsDefault[CONTROL_ELEMENT_COUNT] = {
#include "controller_touchscreen_layouts.inc"
};

ConfigControlElement configControlElements[CONTROL_ELEMENT_COUNT] = {
#include "controller_touchscreen_layouts.inc"
};

ConfigControlElement configControlElementsLast[CONTROL_ELEMENT_COUNT] = {
#include "controller_touchscreen_layouts.inc"
};

ConfigControlElement configControlConfigElements[CONTROL_CONFIG_ELEMENT_COUNT] = {
#include "controller_touchscreen_config_layouts.inc"
};

// This order must match configControlElements and ConfigControlElementIndex
static struct ControlElement ControlElements[CONTROL_ELEMENT_COUNT] = {
[TOUCH_STICK] =      {.type = Joystick},
[TOUCH_MOUSE] =      {.type = Mouse},
[TOUCH_A] =          {.type = Button, .character = 'a',          .buttonID = A_BUTTON},
[TOUCH_B] =          {.type = Button, .character = 'b',          .buttonID = B_BUTTON},
[TOUCH_X] =          {.type = Button, .character = HUD_MULTIPLY, .buttonID = X_BUTTON},
[TOUCH_Y] =          {.type = Button, .character = 'y',          .buttonID = Y_BUTTON},
[TOUCH_START] =      {.type = Button, .character = 's',          .buttonID = START_BUTTON},
[TOUCH_L] =          {.type = Button, .character = 'l',          .buttonID = L_TRIG},
[TOUCH_R] =          {.type = Button, .character = 'r',          .buttonID = R_TRIG},
[TOUCH_Z] =          {.type = Button, .character = 'z',          .buttonID = Z_TRIG},
[TOUCH_CUP] =        {.type = Button, .character = HUD_CUP,      .buttonID = U_CBUTTONS},
[TOUCH_CDOWN] =      {.type = Button, .character = HUD_CDOWN,    .buttonID = D_CBUTTONS},
[TOUCH_CLEFT] =      {.type = Button, .character = HUD_CLEFT,    .buttonID = L_CBUTTONS},
[TOUCH_CRIGHT] =     {.type = Button, .character = HUD_CRIGHT,   .buttonID = R_CBUTTONS},
[TOUCH_DUP] =        {.type = Button, .character = HUD_UP,       .buttonID = U_JPAD},
[TOUCH_DDOWN] =      {.type = Button, .character = HUD_DOWN,     .buttonID = D_JPAD},
[TOUCH_DLEFT] =      {.type = Button, .character = HUD_LEFT,     .buttonID = L_JPAD},
[TOUCH_DRIGHT] =     {.type = Button, .character = HUD_RIGHT,    .buttonID = R_JPAD},
};

// config-only elements
static struct ControlElement ControlConfigElements[CONTROL_CONFIG_ELEMENT_COUNT] = {
[TOUCH_CONFIRM] =    {.type = Button, .character = HUD_CHECK,    .buttonID = CONFIRM_BUTTON},
[TOUCH_CANCEL] =     {.type = Button, .character = HUD_CROSS,    .buttonID = CANCEL_BUTTON},
[TOUCH_RESET] =      {.type = Button, .character = HUD_RESET,    .buttonID = RESET_BUTTON},
[TOUCH_SNAP] =       {.type = Button, .character = HUD_SNAP,     .buttonID = SNAP_BUTTON},
};

static u32 ControlElementsLength = sizeof(ControlElements)/sizeof(struct ControlElement);
static u32 ControlConfigElementsLength = sizeof(ControlConfigElements)/sizeof(struct ControlElement);

void exit_control_config(void) {
    gInTouchConfig = false;
}

struct Position get_pos(ConfigControlElement *config, u32 idx) {
    struct Position ret;
    switch (config->anchor[idx]) {
        case CONTROL_ELEMENT_LEFT:
            ret.x = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(config->x[idx]) << 2;
            ret.y = config->y[idx];
            break;
        case CONTROL_ELEMENT_RIGHT:
            ret.x = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(config->x[idx]) << 2;
            ret.y = config->y[idx];
            break;
        case CONTROL_ELEMENT_CENTER:
            ret.x = SCREEN_WIDTH_API / 2;
            ret.y = config->y[idx];
            break;
        case CONTROL_ELEMENT_HIDDEN:
        default:
            if (gInTouchConfig) {
                ret.x = SCREEN_WIDTH_API / 2;
                ret.y = SCREEN_HEIGHT_API / 2;
            }
            else {
                ret.x = HIDE_POS;
                ret.y = HIDE_POS;
            }
            break;
    }
    if (configElementSnap) {
        ret.x = 50 * ((ret.x + 49) / 50) - 25; 
        ret.y = 50 * ((ret.y + 49) / 50) - 25; 
    }
    return ret;
}

void move_touch_element(struct TouchEvent * event, enum ConfigControlElementIndex i) {
    s32 x_raw, x, y;
    enum ConfigControlElementAnchor anchor;
    x_raw = CORRECT_TOUCH_X(event->x);
    y = CORRECT_TOUCH_Y(event->y);
    if (x_raw < SCREEN_WIDTH_API / 2 - 30) {
        // algebraic inversion
        x = -GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(-(x_raw >> 2));
        anchor = CONTROL_ELEMENT_LEFT;
    }
    else if (x_raw > SCREEN_WIDTH_API / 2 + 30) {
        x = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x_raw >> 2);
        anchor = CONTROL_ELEMENT_RIGHT;
    }
    else {
        x = SCREEN_WIDTH_API / 2;
        anchor = CONTROL_ELEMENT_CENTER;
    }
    if (anchor == CONTROL_ELEMENT_CENTER &&
        y > 400 && y < 560) {
        anchor = CONTROL_ELEMENT_HIDDEN;
    }
    configControlElements[i].x[0] = x;
    configControlElements[i].y[0] = y;
    configControlElements[i].anchor[0] = anchor;
}

void touch_down(struct TouchEvent* event) {
    gGamepadActive = false;
    struct Position pos;
    s32 size;
    // config-only elements
    if (gInTouchConfig) {
        for(u32 i = 0; i < ControlConfigElementsLength; i++) {
            pos = get_pos(&configControlConfigElements[i], 0);
            if (pos.y == HIDE_POS) continue;
            size = configControlConfigElements[i].size[0];
            if (TRIGGER_DETECT(size)) {
                ControlConfigElements[i].touchID = event->touchID;
                if (ControlConfigElements[i].buttonID == SNAP_BUTTON) 
                    configElementSnap = !configElementSnap;
            }
        }
    }
    // Everything else
    for(u32 i = 0; i < ControlElementsLength; i++) {
        if (ControlElements[i].touchID == 0) {
            pos = get_pos(&configControlElements[i], 0);
            if (pos.y == HIDE_POS) continue;
            size = configControlElements[i].size[0];
            switch (ControlElements[i].type) {
                case Joystick:
                    if (TRIGGER_DETECT(size)) {
                        ControlElements[i].touchID = event->touchID;
                        lastElementGrabbed = i;
                        if (!gInTouchConfig) {
                            ControlElements[i].joyX = CORRECT_TOUCH_X(event->x) - pos.x;
                            ControlElements[i].joyY = CORRECT_TOUCH_Y(event->y) - pos.y;
                        }
                    }
                    break;
                case Mouse:
                    if (TRIGGER_DETECT(size)) {
                        ControlElements[i].touchID = event->touchID;
                    }
                    break;
                case Button:
                    if (TRIGGER_DETECT(size)) {
                        ControlElements[i].touchID = event->touchID;
                        lastElementGrabbed = i;
                    }
                    break;
            }
        }
    }
}

void touch_motion(struct TouchEvent* event) {
    struct Position pos;
    s32 size;
    for(u32 i = 0; i < ControlElementsLength; i++) {
        pos = get_pos(&configControlElements[i], 0);
        if (pos.y == HIDE_POS) continue;
        size = configControlElements[i].size[0];
        // config mode
        if (gInTouchConfig) {
            if (ControlElements[i].touchID == event->touchID &&
                ControlElements[i].type != Mouse &&
                i == lastElementGrabbed) {
                    move_touch_element(event, i);
            }
        }
        // normal use
        else {
            if (ControlElements[i].touchID == event->touchID) {
                s32 x, y;
                switch (ControlElements[i].type) {
                    case Joystick:
                        x = CORRECT_TOUCH_X(event->x) - pos.x;
                        y = CORRECT_TOUCH_Y(event->y) - pos.y;
                        if (pos.x + size / 2 < CORRECT_TOUCH_X(event->x))
                            x = size / 2;
                        if (pos.x - size / 2 > CORRECT_TOUCH_X(event->x))
                            x = - size / 2;
                        if (pos.y + size / 2 < CORRECT_TOUCH_Y(event->y))
                            y = size / 2;
                        if (pos.y - size / 2 > CORRECT_TOUCH_Y(event->y))
                            y = - size / 2;
                        ControlElements[i].joyX = x;
                        ControlElements[i].joyY = y;
                        break;
                    case Mouse:
                        if (before_x > 0)
                            touch_x = CORRECT_TOUCH_X(event->x) - before_x;
                        if (before_y > 0)
                            touch_y = CORRECT_TOUCH_Y(event->y) - before_y;
                        before_x = CORRECT_TOUCH_X(event->x);
                        before_y = CORRECT_TOUCH_Y(event->y);
                        if ((u16)touch_x < configStickDeadzone / 4)
                            touch_x = 0;
                        if ((u16)touch_y < configStickDeadzone / 4)
                            touch_y = 0;
                        break;
                    case Button:
                        if (ControlElements[i].slideTouch && !TRIGGER_DETECT(size)) {
                            ControlElements[i].slideTouch = 0;
                            ControlElements[i].touchID = 0;
                        }
                        break;
                }
            }
            // slide touch
            else if (TRIGGER_DETECT(size) &&
                     (ControlElements[TOUCH_MOUSE].touchID != event->touchID ||
                      !configCameraMouse) &&
                     configSlideTouch) {
                switch (ControlElements[i].type) {
                    case Joystick:
                        break;
                    case Mouse:
                        break;
                    case Button:
                        ControlElements[i].slideTouch = 1;
                        ControlElements[i].touchID = event->touchID;
                        break;
                }
            }
        }
    }
}

static void handle_touch_up(u32 i) { // separated for when the layout changes
    ControlElements[i].touchID = 0;
    struct Position pos = get_pos(&configControlElements[i], 0);
    if (pos.y == HIDE_POS) return;
    switch (ControlElements[i].type) {
        case Joystick:
            ControlElements[i].joyX = 0;
            ControlElements[i].joyY = 0;
            break;
        case Mouse:
            touch_x = before_x = 0;
            touch_y = before_y = 0;
            break;
        case Button:
            if (gInTouchConfig) {
                // toggle size of buttons on double-tap
                if (double_tap_timer - double_tap_timer_last < 10) {
                    double_tap_timer_last = 0;
                    u32 *size = &configControlElements[i].size[0];
                    *size = *size > 120 ? 120 : 220;
                }
            }
            break;
    }
}

void touch_up(struct TouchEvent* event) {
    // Config-only elements
    if (gInTouchConfig) {
        for(u32 i = 0; i < ControlConfigElementsLength; i++) {
            ControlConfigElements[i].touchID = 0;
        }
        double_tap_timer_last = double_tap_timer;
        double_tap_timer = gGlobalTimer;
    }
    // Everything else
    for(u32 i = 0; i < ControlElementsLength; i++) {
        if (ControlElements[i].touchID == event->touchID) {
            handle_touch_up(i);
        }
    }
}

// TODO: move all touchscreen textures into their own array
ALIGNED8 const u8 texture_button[] = {
#include "textures/touchcontrols/touch_button.rgba16.inc.c"
};

ALIGNED8 const u8 texture_button_dark[] = {
#include "textures/touchcontrols/touch_button_dark.rgba16.inc.c"
};

// Sprite drawing code stolen from src/game/print.c

static void select_button_texture(int dark) {
    gDPPipeSync(gDisplayListHead++);

    if (!dark) {
        gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_button);
    } else {
        gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_button_dark);
    }

    gSPDisplayList(gDisplayListHead++, dl_hud_img_load_tex_block);
}

static void select_char_texture(u8 num) {
    const u8 *const *glyphs = segmented_to_virtual(main_hud_lut);

    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, glyphs[num - 87]);
    gSPDisplayList(gDisplayListHead++, dl_hud_img_load_tex_block);
}

static void DrawSprite(s32 x, s32 y, int scaling) {
    gSPTextureRectangle(gDisplayListHead++, x - (15 << scaling), y - (15 << scaling), x + (15 << scaling),
                        y + (15 << scaling), G_TX_RENDERTILE, 0, 0, 4 << (11 - scaling), 1 << (11 - scaling));
}

void render_touch_controls(void) {
    if (gGamepadActive && configAutohideTouch) return;
    Mtx *mtx;

    mtx = alloc_display_list(sizeof(*mtx));

    if (mtx == NULL) {
        return;
    }

    guOrtho(mtx, 0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -10.0f, 10.0f, 1.0f);
    gSPPerspNormalize((Gfx *) (gDisplayListHead++), 0xFFFF);
    gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(mtx), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPDisplayList(gDisplayListHead++, dl_hud_img_begin);

    struct Position pos;
    s32 size;
    // All normal elements
    for (u32 i = 0; i < ControlElementsLength; i++) {
        pos = get_pos(&configControlElements[i], 0);
        if (pos.y == HIDE_POS) continue;
        size = configControlElements[i].size[0];
        select_button_texture(0);
        switch (ControlElements[i].type) {
            case Joystick:
                DrawSprite(pos.x, pos.y, 3);
                DrawSprite(pos.x + 4 + ControlElements[i].joyX, pos.y + 4 + ControlElements[i].joyY, 2);
                break;
            case Mouse:
                if ((before_x > 0 || before_y > 0) &&
                    ControlElements[i].touchID &&
                    configCameraMouse &&
                    !gInTouchConfig) {
                    touch_cam_last_x = before_x > 0 ? before_x : touch_cam_last_x;
                    touch_cam_last_y = before_y > 0 ? before_y : touch_cam_last_y;
                    DrawSprite(touch_cam_last_x, touch_cam_last_y, 2);
                }
                break;
            case Button:
                if (ControlElements[i].touchID)
                    select_button_texture(1);
                DrawSprite(pos.x - 8, pos.y, 1 + size / 100);
                select_char_texture(ControlElements[i].character);
                DrawSprite(pos.x, pos.y, size / 100);
                break;
        }
    }
    // Config-only elements
    if (gInTouchConfig) {
        for (u32 i = 0; i < ControlConfigElementsLength; i++) {
            pos = get_pos(&configControlConfigElements[i], 0);
            if (pos.y == HIDE_POS) continue;
            size = configControlConfigElements[i].size[0];
            select_button_texture(0);
            if (ControlConfigElements[i].touchID || 
                (i == TOUCH_SNAP && configElementSnap))
                select_button_texture(1);
            DrawSprite(pos.x - 8, pos.y, 1 + size / 100);
            select_char_texture(ControlConfigElements[i].character);
            DrawSprite(pos.x, pos.y, size / 100);
        }
        // trash icon
        select_char_texture(HUD_TRASH);
        DrawSprite(SCREEN_WIDTH_API / 2, SCREEN_HEIGHT_API / 2, 2);
    }

    gSPDisplayList(gDisplayListHead++, dl_hud_img_end);
}

static void touchscreen_init(void) {
    for (u32 i = 0; i < ControlConfigElementsLength; i++) {
        ControlConfigElements[i].touchID = 0;
        ControlConfigElements[i].joyX = 0;
        ControlConfigElements[i].joyY = 0;
        ControlConfigElements[i].slideTouch = 0;
    }
    for (u32 i = 0; i < ControlElementsLength; i++) {
        ControlElements[i].touchID = 0;
        ControlElements[i].joyX = 0;
        ControlElements[i].joyY = 0;
        ControlElements[i].slideTouch = 0;
    }
}

static void touchscreen_read(OSContPad *pad) {
    struct Position pos;
    s32 size;
    // config mode
    if (gInTouchConfig) {
        for(u32 i = 0; i < ControlConfigElementsLength; i++) {
            if (ControlConfigElements[i].touchID) {
                switch (ControlConfigElements[i].buttonID) {
                    case CONFIRM_BUTTON:
                        ControlConfigElements[i].touchID = 0;
                        configfile_save(configfile_name());
                        exit_control_config();
                        break;
                    case CANCEL_BUTTON:
                        ControlConfigElements[i].touchID = 0;
                        for (u32 i = 0; i < CONTROL_ELEMENT_COUNT; i++) {
                            configControlElements[i].x[0] = configControlElementsLast[i].x[0];
                            configControlElements[i].y[0] = configControlElementsLast[i].y[0];
                            configControlElements[i].size[0] = configControlElementsLast[i].size[0];
                            configControlElements[i].anchor[0] = configControlElementsLast[i].anchor[0];
                        }
                        exit_control_config();
                        break;
                    case RESET_BUTTON:
                        for (u32 i = 0; i < CONTROL_ELEMENT_COUNT; i++) {
                            configControlElements[i].x[0] = configControlElementsDefault[i].x[0];
                            configControlElements[i].y[0] = configControlElementsDefault[i].y[0];
                            configControlElements[i].size[0] = configControlElementsDefault[i].size[0];
                            configControlElements[i].anchor[0] = configControlElementsDefault[i].anchor[0];
                        }
                    case SNAP_BUTTON:
                    default:
                        break;
                }
            }
        }
    }
    // normal use
    else {
        for(u32 i = 0; i < ControlElementsLength; i++) {
            pos = get_pos(&configControlElements[i], 0);
            size = configControlElements[i].size[0];
            if (pos.y == HIDE_POS) continue;
            switch (ControlElements[i].type) {
                case Joystick:
                    if (ControlElements[i].joyX || ControlElements[i].joyY) {
                        pad->stick_x = (ControlElements[i].joyX + size / 2) * 255 / size - 128;
                        pad->stick_y = (-ControlElements[i].joyY + size / 2) * 255 / size - 128; //inverted for some reason
                    }
                    break;
                case Mouse:
                    break;
                case Button:
                    if (ControlElements[i].touchID) {
                        pad->button |= ControlElements[i].buttonID;
                    }
                    break;
            }
        }
    }
}

// Used by other controller types for setting keybinds
// Doesn't make a huge amount of sense for a touchscreen,
// So instead I allow customizing all button positions in
// an entirely separate construction, which is fine for now
// until someone wants multiple copies of the same button,
// at which point I will have to decide how to do that
static u32 touchscreen_rawkey(void) { 
    return VK_INVALID;
}

struct ControllerAPI controller_touchscreen = {
    0,
    touchscreen_init,
    touchscreen_read,
    touchscreen_rawkey,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif