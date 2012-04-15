include nall/Makefile

ifeq ($(platform),osx)
   fpic := -fPIC
   TARGET := libretro.dylib
else ifeq ($(platform),win)
   fpic :=
   TARGET := retro.dll
   CC = gcc
   CXX = g++
else
   fpic := -fPIC
   TARGET := libretro.so
endif

retro := retro

all: $(TARGET)

retro_objects := retro-interface retro-system retro-scheduler retro-input
retro_objects += retro-cartridge retro-memory
retro_objects += retro-cpu retro-apu retro-ppu
retro_objects += retro-cheat
retro_objects += libretro
objects += $(retro_objects) libco

obj/retro-interface.o: $(retro)/interface/interface.cpp $(call rwildcard,$(retro)/interface/)
obj/retro-system.o: $(retro)/system/system.cpp $(call rwildcard,$(retro)/system/)
obj/retro-scheduler.o: $(retro)/scheduler/scheduler.cpp $(call rwildcard,$(retro)/scheduler/)
obj/retro-input.o: $(retro)/input/input.cpp $(call rwildcard,$(retro)/input/)
obj/retro-mapper.o: $(retro)/mapper/mapper.cpp $(call rwildcard,$(retro)/mapper/)
obj/retro-cartridge.o: $(retro)/cartridge/cartridge.cpp $(call rwildcard,$(retro)/cartridge/)
obj/retro-memory.o: $(retro)/memory/memory.cpp $(call rwildcard,$(retro)/memory/)
obj/retro-cpu.o: $(retro)/cpu/cpu.cpp $(call rwildcard,$(retro)/cpu/)
obj/retro-apu.o: $(retro)/apu/apu.cpp $(call rwildcard,$(retro)/apu/)
obj/retro-ppu.o: $(retro)/ppu/ppu.cpp $(call rwildcard,$(retro)/ppu/)
obj/retro-cheat.o: $(retro)/cheat/cheat.cpp $(call rwildcard,$(retro)/cheat/)
obj/libretro.o: libretro/libretro.cpp $(call rwildcard,libretro/)
obj/libco.o: libco/libco.c

c := $(CC) -std=gnu99
cpp := $(CXX) -std=gnu++0x
flags := -O3 -fomit-frame-pointer -fno-tree-vectorize -I. $(fpic)

compile = \
  $(strip \
    $(if $(filter %.c,$<), \
      $(c) $(flags) $1 -c $< -o $@, \
      $(if $(filter %.cpp,$<), \
        $(cpp) $(flags) $1 -c $< -o $@ \
      ) \
    ) \
  )


%.o: $<; $(call compile)

libretro_objects := $(patsubst %,obj/%.o,$(objects))


$(TARGET): $(libretro_objects)
ifeq ($(platform),x)
	$(cpp) -o $@ -shared $(libretro_objects) -Wl,--no-undefined -Wl,--version-script=link.T
else ifeq ($(platform),win)
	$(cpp) -o $@ -shared $(libretro_objects) -Wl,--no-undefined -static-libgcc -static-libstdc++ -Wl,--version-script=link.T
else
	$(cpp) -o $@ -dynamiclib $(libretro_objects) -Wl,--no-undefined
endif

clean:
	rm -f $(libretro_objects) $(TARGET)

.PHONY: clean all
