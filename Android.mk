LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := main
### Build Options ###

# These options can either be changed by modifying the makefile, or
# by building with 'make SETTING=value'. 'make clean' may be required.

# Version of the game to build
VERSION ?= us

TOUCH_CONTROLS ?= 1

ifeq ($(VERSION),jp)
  VERSION_DEF := VERSION_JP
else
ifeq ($(VERSION),us)
  VERSION_DEF := VERSION_US
else
ifeq ($(VERSION),eu)
  VERSION_DEF := VERSION_EU
else
ifeq ($(VERSION),sh)
  $(warning Building SH is experimental and is prone to breaking. Try at your own risk.)
  VERSION_DEF := VERSION_SH
else
  $(error unknown version "$(VERSION)")
endif
endif
endif
endif

PC_BUILD_DIR := $(LOCAL_PATH)/build/$(VERSION)_pc

SRC_DIRS := src src/game src/audio src/menu src/buffers actors levels bin bin/$(VERSION) data assets src/pc src/pc/gfx src/pc/audio src/pc/controller
SRC_DIRS += \
data/omm \
data/omm/system \
data/omm/engine \
data/omm/object \
data/omm/models \
data/omm/mario \
data/omm/cappy \
data/omm/capture \
data/omm/peachy \
data/omm/level \
data/omm/dev
SRC_DIRS := $(addprefix $(LOCAL_PATH)/,$(SRC_DIRS))

GODDARD_SRC_DIRS := src/goddard src/goddard/dynlists
GODDARD_SRC_DIRS := $(addprefix $(LOCAL_PATH)/,$(GODDARD_SRC_DIRS))

# Source code files
LEVEL_C_FILES := $(wildcard $(LOCAL_PATH)/levels/*/leveldata.c) $(wildcard $(LOCAL_PATH)/levels/*/script.c) $(wildcard $(LOCAL_PATH)/levels/*/geo.c)
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

ULTRA_C_FILES := \
  alBnkfNew.c \
  guLookAtRef.c \
  guMtxF2L.c \
  guNormalize.c \
  guOrthoF.c \
  guPerspectiveF.c \
  guRotateF.c \
  guScaleF.c \
  guTranslateF.c

C_FILES := $(filter-out $(LOCAL_PATH)/src/game/main.c,$(C_FILES))
ULTRA_C_FILES := $(addprefix $(LOCAL_PATH)/lib/src/,$(ULTRA_C_FILES))

GODDARD_C_FILES := $(foreach dir,$(GODDARD_SRC_DIRS),$(wildcard $(dir)/*.c))

GENERATED_C_FILES := $(PC_BUILD_DIR)/assets/mario_anim_data.c $(PC_BUILD_DIR)/assets/demo_data.c \
  $(addprefix $(PC_BUILD_DIR)/bin/,$(addsuffix _skybox.c,$(notdir $(basename $(wildcard $(LOCAL_PATH)/textures/skyboxes/*.png)))))

LOCAL_SHORT_COMMANDS := true
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_LDLIBS := -lEGL -lGLESv2 -llog -lz
LOCAL_C_INCLUDES := $(LOCAL_PATH)/platform/android/include $(LOCAL_PATH)/include $(LOCAL_PATH)/src $(LOCAL_PATH)/sound $(PC_BUILD_DIR) $(PC_BUILD_DIR)/include
LOCAL_C_INCLUDES += \
$(LOCAL_PATH)/data/omm/engine/headers \
$(LOCAL_PATH)/data/omm/engine/headers/include \
$(LOCAL_PATH)/data/omm/engine/headers/include/PR \
$(LOCAL_PATH)/data/omm/engine/headers/src \
$(LOCAL_PATH)/data/omm/engine/headers/src/engine \
$(LOCAL_PATH)/data/omm/engine/headers/src/extras \
$(LOCAL_PATH)/data/omm/engine/headers/src/game \
$(LOCAL_PATH)/data/omm/engine/headers/src/menu \
$(LOCAL_PATH)/data/omm/engine/headers/src/pc
LOCAL_CFLAGS := -DNON_MATCHING -DAVOID_UB -DTARGET_LINUX -DENABLE_OPENGL -DWIDESCREEN -DF3DEX_GBI_2E -D_LANGUAGE_C -DNO_SEGMENTED_MEMORY -D$(VERSION_DEF) -DSTDC_HEADERS -DUSE_GLES -DTEXTURE_FIX -DBETTERCAMERA -DEXT_OPTIONS_MENU -DIMMEDIATELOAD -DRAPI_GL=1 -DWAPI_SDL2=1 -DAAPI_SDL2=1 -DCAPI_SDL2 -DHAVE_SDL2=1 -D_REENTRANT -DNIGHTLY -DOMM_VERSION="7.5.1" -DOMM_REVISION="5" -DOMM_DEVELOPER="PeachyPeach" -DSMEX -DCHEATS_ACTIONS -DNODRAWINGDISTANCE -DEXTERNAL_DATA -DFS_BASEDIR="\"res\"" -DOMM_MK_MARIO_COLORS=1 -DGIT_HASH="\"N/A\"" -O0 -w
ifeq ($(TOUCH_CONTROLS),1)
  LOCAL_CFLAGS += -DTOUCH_CONTROLS
endif
LOCAL_SRC_FILES :=  $(C_FILES) $(CXX_FILES) $(GENERATED_C_FILES) $(ULTRA_C_FILES) $(GODDARD_C_FILES) $(LOCAL_PATH)/sound/sound_data.c 
include $(BUILD_SHARED_LIBRARY)