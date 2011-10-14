#include "libsnes.hpp"
#include <nes/nes.hpp>
#include <iostream>
#include <nall/dsp.hpp>

using namespace NES;

unsigned snes_library_revision_major() { return 1; }
unsigned snes_library_revision_minor() { return 3; }
const char *snes_library_id() { return "bNES v083"; }

struct libSNES : Interface
{
   libSNES()
   {
      init_dsp();
      init_palette();
   }

   void init_palette()
   {
      static const unsigned palette_[] = {
         0x7c7c7c, 0x0000fc, 0x0000bc, 0x4428bc,
         0x940084, 0xa80020, 0xa81000, 0x881400,
         0x503000, 0x007800, 0x006800, 0x005800,
         0x004058, 0x000000, 0x000000, 0x000000,
         0xbcbcbc, 0x0078f8, 0x0058f8, 0x6844fc,
         0xd800cc, 0xe40058, 0xf83800, 0xe45c10,
         0xac7c00, 0x00b800, 0x00a800, 0x00a844,
         0x008888, 0x000000, 0x000000, 0x000000,
         0xf8f8f8, 0x3cbcfc, 0x6888fc, 0x9878f8,
         0xf878f8, 0xf85898, 0xf87858, 0xfca044,
         0xf8b800, 0xb8f818, 0x58d854, 0x58f898,
         0x00e8d8, 0x787878, 0x000000, 0x000000,
         0xfcfcfc, 0xa4e4fc, 0xb8b8b8, 0xd8d8f8,
         0xf8b8f8, 0xf8a4c0, 0xf0d0b0, 0xfce0a8,
         0xf8d878, 0xd8f878, 0xb8f8b8, 0xb8f8d8,
         0x00fcfc, 0xf8d8f8, 0x000000, 0x000000,
      };

      memcpy(palette, palette_, sizeof(palette_));

      for (unsigned e = 1; e < 8; e++) {
         static const double rfactor[8] = { 1.000, 1.239, 0.794, 1.019, 0.905, 1.023, 0.741, 0.750 };
         static const double gfactor[8] = { 1.000, 0.915, 1.086, 0.980, 1.026, 0.908, 0.987, 0.750 };
         static const double bfactor[8] = { 1.000, 0.743, 0.882, 0.653, 1.277, 0.979, 0.101, 0.750 };
         for (unsigned n = 0; n < 64; n++) {
            unsigned c = palette[n];
            uint8_t r = c >> 16, g = c >> 8, b = c >> 0;
            r = uclamp<8>((unsigned)(r * rfactor[e]));
            g = uclamp<8>((unsigned)(g * gfactor[e]));
            b = uclamp<8>((unsigned)(b * bfactor[e]));
            palette[e * 64 + n] = (r << 16) | (g << 8) | (b << 0);
         }
      }
   }

   void videoRefresh(const uint16_t *data)
   {
      for (unsigned y = 0; y < 240; y++)
      {
         const uint16_t *src = data + y * 256;
         uint16_t *dst = frame + y * 1024;

         for (unsigned x = 0; x < 256; x++)
         {
            unsigned color = palette[src[x]];
            dst[x] =
               ((color & 0xf80000) >> 9) |
               ((color & 0x00f800) >> 6) |
               ((color & 0x0000f8) >> 3);
         }
      }

      video_cb(frame, 256, 240);
      poll_cb();
   }

   void audioSample(int16_t sample)
   {
      signed samples[] = { sample };
      dspaudio.sample(samples);
      while (dspaudio.pending())
      {
         dspaudio.read(samples);
         audio_cb(samples[0], samples[0]);
      }
   }

   int16_t inputPoll(bool port, unsigned, unsigned id)
   {
      static const unsigned nes_to_snes_id[] = {
         SNES_DEVICE_ID_JOYPAD_A,
         SNES_DEVICE_ID_JOYPAD_B,
         SNES_DEVICE_ID_JOYPAD_SELECT,
         SNES_DEVICE_ID_JOYPAD_START,
         SNES_DEVICE_ID_JOYPAD_UP,
         SNES_DEVICE_ID_JOYPAD_DOWN,
         SNES_DEVICE_ID_JOYPAD_LEFT,
         SNES_DEVICE_ID_JOYPAD_RIGHT,
      };

      return state_cb(port, SNES_DEVICE_JOYPAD, 0, nes_to_snes_id[id]);
   }

   void init_dsp()
   {
      dspaudio.setResampler(DSP::ResampleEngine::Sinc);
      dspaudio.setChannels(1);
      dspaudio.setResamplerFrequency(32000.0);
      dspaudio.setFrequency(1789772.0);
      dspaudio.clear();
   }

   void message(const string &text) { std::cerr << "[libNES]: " << text << std::endl; }

   snes_video_refresh_t video_cb;
   snes_audio_sample_t audio_cb;
   snes_input_poll_t poll_cb;
   snes_input_state_t state_cb;

   nall::DSP dspaudio;
   uint16_t frame[1024 * 240];
   unsigned palette[512];
};

static libSNES libsnes;

void snes_init()
{
   libsnes.initialize(&libsnes);
   libsnes.connect(0, Input::Device::Joypad);
   libsnes.connect(1, Input::Device::Joypad);
}

void snes_term()
{}

void snes_set_cartridge_basename(const char *basename) {}

// Set callbacks.
void snes_set_video_refresh(snes_video_refresh_t callback)
{
   libsnes.video_cb = callback;
}
void snes_set_audio_sample(snes_audio_sample_t callback)
{
   libsnes.audio_cb = callback;
}
void snes_set_input_poll(snes_input_poll_t callback)
{
   libsnes.poll_cb = callback;
}
void snes_set_input_state(snes_input_state_t callback)
{
   libsnes.state_cb = callback;
}

// Save states.
unsigned snes_serialize_size() { return NES::system.serialize_size; }
bool snes_serialize(uint8_t *data, unsigned size)
{
   serializer s = libsnes.serialize();
   if (s.size() > size) return false;
   memcpy(data, s.data(), s.size());
   return true;
}

bool snes_unserialize(const uint8_t *data, unsigned size)
{
   serializer s(data, size);
   return libsnes.unserialize(s);
}

// Cheats.
void snes_cheat_reset() {}
void snes_cheat_set(unsigned index, bool enabled, const char *code) {}

// Cartridge load.
bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
)
{
   libsnes.loadCartridge(rom_xml ? nall::string(rom_xml) : nall::string(), (const uint8_t*)rom_data, rom_size);
   return libsnes.cartridgeLoaded();
}

bool snes_load_cartridge_bsx_slotted(
  const char *, const uint8_t *, unsigned,
  const char *, const uint8_t *, unsigned
) { return false; }

bool snes_load_cartridge_bsx(
  const char *, const uint8_t *, unsigned,
  const char *, const uint8_t *, unsigned
) { return false; }

bool snes_load_cartridge_sufami_turbo(
  const char *, const uint8_t *, unsigned,
  const char *, const uint8_t *, unsigned,
  const char *, const uint8_t *, unsigned
) { return false; }

bool snes_load_cartridge_super_game_boy(
  const char *, const uint8_t *, unsigned,
  const char *, const uint8_t *, unsigned
) { return false; }


// Unload cartridge.
void snes_unload_cartridge(void) { libsnes.unloadCartridge(); }

// Hard reset.
void snes_power() { libsnes.power(); }

// Soft reset.
void snes_reset() { libsnes.reset(); }

// Run frame.
void snes_run() { libsnes.run(); }

// Region query.
bool snes_get_region() { return true; }

// Cartridge RAM.
uint8_t *snes_get_memory_data(unsigned id)
{
   if (id == SNES_MEMORY_CARTRIDGE_RAM)
      return libsnes.memoryData(Interface::Memory::RAM);
   else
      return nullptr;
}

unsigned snes_get_memory_size(unsigned id)
{
   if (id == SNES_MEMORY_CARTRIDGE_RAM)
      return libsnes.memorySize(Interface::Memory::RAM);
   else
      return 0;
}

// Controllers.
void snes_set_controller_port_device(bool, unsigned) 
{}

