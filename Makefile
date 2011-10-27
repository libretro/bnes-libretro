include nall/Makefile

ifeq ($(platform),osx)
   fpic := -fPIC
   TARGET := libnes.dylib
else ifeq ($(platform),win)
   fpic :=
   TARGET := nes.dll
   CC = gcc
   CXX = g++
else
   fpic := -fPIC
   TARGET := libnes.so
endif

nes := nes

all: $(TARGET)

nes_objects := nes-interface nes-system nes-scheduler nes-input
nes_objects += nes-cartridge nes-memory
nes_objects += nes-cpu nes-apu nes-ppu
nes_objects += nes-cheat
nes_objects += libnes
objects += $(nes_objects) libco

obj/nes-interface.o: $(nes)/interface/interface.cpp $(call rwildcard,$(nes)/interface/)
obj/nes-system.o: $(nes)/system/system.cpp $(call rwildcard,$(nes)/system/)
obj/nes-scheduler.o: $(nes)/scheduler/scheduler.cpp $(call rwildcard,$(nes)/scheduler/)
obj/nes-input.o: $(nes)/input/input.cpp $(call rwildcard,$(nes)/input/)
obj/nes-mapper.o: $(nes)/mapper/mapper.cpp $(call rwildcard,$(nes)/mapper/)
obj/nes-cartridge.o: $(nes)/cartridge/cartridge.cpp $(call rwildcard,$(nes)/cartridge/)
obj/nes-memory.o: $(nes)/memory/memory.cpp $(call rwildcard,$(nes)/memory/)
obj/nes-cpu.o: $(nes)/cpu/cpu.cpp $(call rwildcard,$(nes)/cpu/)
obj/nes-apu.o: $(nes)/apu/apu.cpp $(call rwildcard,$(nes)/apu/)
obj/nes-ppu.o: $(nes)/ppu/ppu.cpp $(call rwildcard,$(nes)/ppu/)
obj/nes-cheat.o: $(nes)/cheat/cheat.cpp $(call rwildcard,$(nes)/cheat/)
obj/libnes.o: libnes/libnes.cpp $(call rwildcard,libnes/)
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

libnes_objects := $(patsubst %,obj/%.o,$(objects))


$(TARGET): $(libnes_objects)
ifeq ($(platform),x)
	$(cpp) -o $@ -shared $(libnes_objects) -Wl,--no-undefined
else ifeq ($(platform),win)
	$(cpp) -o $@ -shared $(libnes_objects) -Wl,--no-undefined -static-libgcc -static-libstdc++
else
	$(cpp) -o $@ -dynamiclib $(libnes_objects) -Wl,--no-undefined
endif

clean:
	rm -f $(libnes_objects) $(TARGET)

.PHONY: clean all
