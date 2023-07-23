#ifndef CONTROLLER_TOUCHSCREEN_H
#define CONTROLLER_TOUCHSCREEN_H
#ifdef TOUCH_CONTROLS
#include <SDL2/SDL.h>
#include "pc/configfile.h"

#define HIDE_POS -1000

#define CONFIRM_BUTTON 0x0001
#define CANCEL_BUTTON 0x0002
#define RESET_BUTTON 0x0003
#define SNAP_BUTTON 0x0004

#define HUD_CUP 123
#define HUD_CDOWN 124
#define HUD_CLEFT 125
#define HUD_CRIGHT 126
#define HUD_SNAP 127
#define HUD_CHECK 128
#define HUD_CROSS 129
#define HUD_RESET 130
#define HUD_TRASH 131
#define HUD_UP 132
#define HUD_DOWN 133
#define HUD_LEFT 134
#define HUD_RIGHT 135
#define HUD_MULTIPLY 137

#define CONTROL_ELEMENT_COUNT 18
#define CONTROL_CONFIG_ELEMENT_COUNT 4

#define SCREEN_WIDTH_API 1280
#define SCREEN_HEIGHT_API 960

#define LEFT_EDGE ((int)floorf(SCREEN_WIDTH_API / 2 - SCREEN_HEIGHT_API / 2 * gfx_current_dimensions.aspect_ratio))
#define RIGHT_EDGE ((int)ceilf(SCREEN_WIDTH_API / 2 + SCREEN_HEIGHT_API / 2 * gfx_current_dimensions.aspect_ratio))

#define CORRECT_TOUCH_X(x) ((x * (RIGHT_EDGE - LEFT_EDGE)) + LEFT_EDGE)
#define CORRECT_TOUCH_Y(y) (y * SCREEN_HEIGHT_API)

#define TRIGGER_DETECT(size) (((pos.x + size / 2 > CORRECT_TOUCH_X(event->x)) &&\
                               (pos.x - size / 2 < CORRECT_TOUCH_X(event->x))) &&\
                              ((pos.y + size / 2 > CORRECT_TOUCH_Y(event->y)) &&\
                               (pos.y - size / 2 < CORRECT_TOUCH_Y(event->y))))

enum ConfigControlElementAnchor {
    CONTROL_ELEMENT_LEFT,
    CONTROL_ELEMENT_RIGHT,
    CONTROL_ELEMENT_CENTER,
    CONTROL_ELEMENT_HIDDEN,
};

enum ConfigControlElementIndex {
    TOUCH_STICK,
    TOUCH_MOUSE,
    TOUCH_A,
    TOUCH_B,
    TOUCH_X,
    TOUCH_Y,
    TOUCH_START,
    TOUCH_L,
    TOUCH_R,
    TOUCH_Z,
    TOUCH_CUP,
    TOUCH_CDOWN,
    TOUCH_CLEFT,
    TOUCH_CRIGHT,
    TOUCH_DUP,
    TOUCH_DDOWN,
    TOUCH_DLEFT,
    TOUCH_DRIGHT,
};

enum ConfigControlConfigElementIndex {
    TOUCH_CONFIRM,
    TOUCH_CANCEL,
    TOUCH_RESET,
    TOUCH_SNAP,
};

typedef struct {
    u32 x[MAX_BINDS], y[MAX_BINDS], size[MAX_BINDS];
    enum ConfigControlElementAnchor anchor[MAX_BINDS];
} ConfigControlElement;

extern ConfigControlElement configControlElements[];
extern ConfigControlElement configControlElementsLast[];

extern struct ControllerAPI controller_touchscreen;
extern s16 touch_x;
extern s16 touch_y;

extern bool gInTouchConfig, gGamepadActive,
            configAutohideTouch, configSlideTouch, configElementSnap, configRenderCappy;

struct TouchEvent {
    // Note to VDavid003: In Xorg, touchID became large!
    // SurfaceFlinger SDL2 only populated this with 1-255. 
    // But X11 SDL2 populated this with 699!
    // For X11 compatibility, I matched the types.
    SDL_TouchID touchID; //Should not be 0
    float x, y; //Should be from 0 to 1
};

struct Position {
    s32 x, y;
};

enum ControlElementType {
    Joystick,
    Mouse,
    Button
};

struct ControlElement {
    enum ControlElementType type;
    SDL_TouchID touchID; //0 = not being touched, 1+ = Finger being used
    //Joystick
    int joyX, joyY;
    //Button
    int buttonID;
    u8 character;
    int slideTouch;
};

void touch_down(struct TouchEvent* event);
void touch_motion(struct TouchEvent* event);
void touch_up(struct TouchEvent* event);

void render_touch_controls(void);

#endif
#endif