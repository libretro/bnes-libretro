LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/../..
NES_DIR  := $(CORE_DIR)/nes

SOURCES_C   := $(CORE_DIR)/libco/libco.c
SOURCES_CXX := $(CORE_DIR)/libretro/libretro.cpp \
               $(NES_DIR)/interface/interface.cpp \
               $(NES_DIR)/system/system.cpp \
               $(NES_DIR)/scheduler/scheduler.cpp \
               $(NES_DIR)/input/input.cpp \
               $(NES_DIR)/memory/memory.cpp \
               $(NES_DIR)/cartridge/cartridge.cpp \
               $(NES_DIR)/cpu/cpu.cpp \
               $(NES_DIR)/apu/apu.cpp \
               $(NES_DIR)/ppu/ppu.cpp \
               $(NES_DIR)/cheat/cheat.cpp

INCFLAGS  := -I$(CORE_DIR)
COREFLAGS := -DANDROID $(INCFLAGS) -Wno-return-type -Wno-tautological-compare -Wno-delete-non-virtual-dtor

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SOURCES_CXX) $(SOURCES_C)
LOCAL_CXXFLAGS     := $(COREFLAGS) -std=gnu++0x
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=$(CORE_DIR)/link.T
LOCAL_CPP_FEATURES := exceptions

# armv5 clang workarounds
ifeq ($(TARGET_ARCH_ABI),armeabi)
  LOCAL_LDLIBS := -latomic
endif

include $(BUILD_SHARED_LIBRARY)
