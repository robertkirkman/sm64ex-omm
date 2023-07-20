#ifndef OMM_OBJECT_FIELDS_H
#define OMM_OBJECT_FIELDS_H

#include "types.h"
#include "object_fields.h"

struct OmmData {
    void (*reset)(void);
    void (*resetMario)(void);
    void (*resetObject)(void);
    void (*update)(void);
    void (*updateMario)(void);
    void (*updateObject)(void);
    bool (*readStats)(const char **tokens);
    void (*writeStats)(char **buffer);

    // Mario data
    struct {

        // State data
        struct {
            s32 ticks;
            s32 coins;
            s32 o2;
            s32 airCombo;
            f32 peakHeight;
            void *poleObject;
            struct {
                s32 dmg;
                s32 gfx;
                s32 sfx;
            } freeze;
            struct {
                Vec3f pos;
                Vec4f vel;
                Vec3s angle;
            } previous;
        } state;

        // Grab
        struct {
            struct Object *obj;
        } grab;

        // Peach-only data
        struct {
            bool floated;
            s32 floatTimer;
            s32 vibeType;
            s32 vibeGauge;
            s32 vibeTimer;
            s32 joySpinYaw;
            s32 perryCharge;
            bool perryBlast;
            struct Object *perry;
        } peach;

        // Wall-slide moves data
        struct {
            bool jumped;
            s16 angle;
            f32 height;
        } wallSlide;

        // Spin moves data
        struct {
            bool pressed;
            s16 yaw;
            s32 timer;
            s32 decel;
        } spin;

        // Midair spin move data
        struct {
            s32 timer;
            s32 counter;
        } midairSpin;

        // Metal-water moves data
        struct {
            s32 punchType;
            u32 jumpNext;
            s32 jumpTimer;
        } metalWater;

        // Cappy data
        struct {
            bool bounced;
            struct Object *cappy;
        } cappy;
    
        // Capture data
        struct {
            const void *data;
            struct Object *obj;
            struct Object *prev;
            const BehaviorScript *bhv;
            Vec3f animPos[3];
            s32 model;
            s32 timer;
            s32 lockTimer;
            f32 stickX;   // [-1, +1], positive is right
            f32 stickY;   // [-1, +1], positive is up
            f32 stickMag; // [0, 1]
            s16 stickYaw; // intended yaw
            u16 buttonPressed;
            u16 buttonDown;
#if OMM_CODE_DEBUG
            f32 hitboxRadius;
            f32 hitboxHeight;
            f32 hitboxOffset;
            f32 hitboxWall;
#endif
        } capture;

        // Sparkly stars data
        struct {
            s32 mode;
            s32 ending;
            bool starBlock;
            bool grandStar;
            bool gamePaused;
            bool timeStop;
            bool transition;
            bool marioUpdated;
            bool cheatDetected;
            struct {
                s32 currMsg;
                s32 introId;
                s32 messageId;
                s32 endingId;
                s32 counter;
            } cheats[1];
        } sparkly;
    } mario[1];

    // Object data
    struct {

        // State data
        struct {
            u32 properties;
            bool actionFlag;
            s32 actionState;
            s32 actionTimer;
            s32 squishTimer;
            s32 bullyTimer;
            s32 invincTimer;
            f32 walkDistance;
            Vec3f initialPos;
            bool camBehindMario;
        } state;

        // Cappy data
        struct {
            bool copyGfx;
            Vec3f offset;
            Vec3s angle;
            f32 scale;
        } cappy;

        // Per object data
        union {

            // Goomba
            struct {
                struct Object *stackObj[16];
                s32 stackCount;
            } goomba;

            // Koopa
            struct {
                f32 scale;
            } koopa;

            // Chain chomp
            struct {
                bool isFreed;
                bool isBiting;
            } chain_chomp;

            // Bullet bill
            struct {
                f32 scale;
            } bullet_bill;

            // Snowman's body
            struct {
                f32 peakHeight;
                bool headFound;
            } snowmans_body;

            // Boo
            struct {
                f32 gfxOffsetY;
            } boo;

            // Toad
            struct {
                s16 dialogId;
                s32 spawnStar;
            } toad;

            // Flaming bob-omb
            struct {
                s32 interactedTimer;
                bool interactedFire;
                bool captureDuringAscent;
            } flaming_bobomb;

            // Swoop
            struct {
                s32 ceilingType;
                union {
                    struct {
                        struct Surface *s;
                        f32 height;
                    } surface;
                    struct {
                        struct Object *o;
                        Vec3f pos;
                        Vec3f angle;
                        Vec3f scale;
                    } object;
                } ceiling;
            } swoop;

            // Monty mole
            struct {
                struct Object *holes[32];
                s32 count;
                s32 current;
            } monty_mole;

            // Motos
            struct {
                struct Object *heldObj;
            } motos;
        };
    } object[1];

    // Stats data
    struct {

        // Objects
        u64 starsCollected;
        u64 sparklyStarsCollected;
        u64 coinsCollected;
        u64 capsCollected;
        u64 mushrooms1upCollected;
        u64 secretsCollected;
        u64 exclamationBoxesBroken;
        u64 enemiesDefeated;
        u64 bowsersDefeated;

        // Actions
        u64 aPresses;
        u64 jumps;
        u64 attacks;
        u64 cappyThrows;
        u64 cappyBounces;
        u64 captures;
        u64 hitsTaken;
        u64 restarts;
        u64 deaths;

        // Distance (Mario/Capture)
        u64 distanceTotal[2];
        u64 distanceOnGround[2];
        u64 distanceAirborne[2];
        u64 distanceUnderwater[2];
        u64 distanceWingCap[2];
        u64 distanceMetalCap[2];
        u64 distanceVanishCap[2];

        // Time (Mario/Capture)
        u64 timeTotal[2];
        u64 timeOnGround[2];
        u64 timeAirborne[2];
        u64 timeUnderwater[2];
        u64 timeWingCap[2];
        u64 timeMetalCap[2];
        u64 timeVanishCap[2];
    } stats[1];
};
extern struct OmmData *gOmmData;
#define gOmmMario      gOmmData->mario
#define gOmmPeach    (&gOmmData->mario->peach)
#define gOmmCappy      gOmmData->mario->cappy.cappy
#define gOmmPerry      gOmmData->mario->peach.perry
#define gOmmCapture    gOmmData->mario->capture.obj
#define gOmmObject     gOmmData->object
#define gOmmStats      gOmmData->stats

#define oFields             header.gfx._oFields
#define oGfxInited          oFields._oGfxInited
#define oTransparency       oFields._oTransparency
#define oGeoData            oFields._oGeoData
#define oBhvPointer         oFields._oBhvPointer
#define oBhvCommand         oFields._oBhvCommand
#define oBhvStackIndex      oFields._oBhvStackIndex
#define oBhvTypes           oFields._oBhvTypes
#define oSafeStepInited     oFields._oSafeStepInited
#define oSafeStepIgnore     oFields._oSafeStepIgnore
#define oSafeStepIndex      oFields._oSafeStepIndex
#define oSafeStepHeight     oFields._oSafeStepHeight
#define oSafeStepCoords     oFields._oSafeStepCoords

#endif // OMM_OBJECT_FIELDS_H
