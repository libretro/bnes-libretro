#include "libretro.h"
#include <nes/nes.hpp>
#include <iostream>
#include <nall/dsp.hpp>

using namespace NES;

unsigned retro_api_version() { return RETRO_API_VERSION; }

struct libRETRO : Interface
{
   libRETRO()
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
      for (unsigned x = 0; x < 240 * 256; x++)
      {
         unsigned color = palette[data[x]];
         frame[x] =
            ((color & 0xf80000) >> 9) |
            ((color & 0x00f800) >> 6) |
            ((color & 0x0000f8) >> 3);
      }

      video_cb(frame, 256, 240, 512);
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
      static const unsigned nes_to_retro_id[] = {
         RETRO_DEVICE_ID_JOYPAD_A,
         RETRO_DEVICE_ID_JOYPAD_B,
         RETRO_DEVICE_ID_JOYPAD_SELECT,
         RETRO_DEVICE_ID_JOYPAD_START,
         RETRO_DEVICE_ID_JOYPAD_UP,
         RETRO_DEVICE_ID_JOYPAD_DOWN,
         RETRO_DEVICE_ID_JOYPAD_LEFT,
         RETRO_DEVICE_ID_JOYPAD_RIGHT,
      };

      struct retro_input_descriptor desc[] = {
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 0 },
      };

      environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

      return state_cb(port, RETRO_DEVICE_JOYPAD, 0, nes_to_retro_id[id]);
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

   retro_video_refresh_t video_cb;
   retro_audio_sample_t audio_cb;
   retro_input_poll_t poll_cb;
   retro_input_state_t state_cb;
   retro_environment_t environ_cb;

   nall::DSP dspaudio;
   uint16_t frame[256 * 240];
   unsigned palette[512];
};

static libRETRO libretro;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "bnes";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   info->library_version = "v083" GIT_VERSION;
   info->need_fullpath = false;
   info->block_extract = false;
   info->valid_extensions = "nes";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   struct retro_game_geometry geom = { 256, 240, 256, 240 };
   struct retro_system_timing timing = { 1008307711.0 / 16777215.0, 32000.0 };

   info->geometry = geom;
   info->timing   = timing;
}

void retro_init()
{
   libretro.initialize(&libretro);
   libretro.connect(0, Input::Device::Joypad);
   libretro.connect(1, Input::Device::Joypad);
}

void retro_deinit() {}

// Set callbacks.
void retro_set_video_refresh(retro_video_refresh_t callback) { libretro.video_cb = callback; }
void retro_set_audio_sample(retro_audio_sample_t callback) { libretro.audio_cb = callback; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t) {}
void retro_set_input_poll(retro_input_poll_t callback) { libretro.poll_cb = callback; }
void retro_set_input_state(retro_input_state_t callback) { libretro.state_cb = callback; }
void retro_set_environment(retro_environment_t callback) { libretro.environ_cb = callback; }

// Save states.
size_t retro_serialize_size() { return NES::system.serialize_size; }
bool retro_serialize(void *data, size_t size)
{
   serializer s = libretro.serialize();
   if (s.size() > size) return false;
   memcpy(data, s.data(), s.size());
   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   serializer s((const uint8_t*)data, size);
   return libretro.unserialize(s);
}

// Cheats.
void retro_cheat_reset() {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}

// Cartridge load.
bool retro_load_game(const struct retro_game_info *info)
{
   libretro.loadCartridge(info->meta ? nall::string(info->meta) : nall::string(),
         (const uint8_t*)info->data, info->size);

   struct retro_input_descriptor desc[] = {
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
         { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

         { 0 },
      };

   libretro.environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   return libretro.cartridgeLoaded();
}

bool retro_load_game_special(unsigned, const struct retro_game_info*, size_t) { return false; }

// Unload cartridge.
void retro_unload_game(void) { libretro.unloadCartridge(); }

// Soft reset.
void retro_reset() { libretro.reset(); }

// Run frame.
void retro_run() { libretro.run(); }

// Region query.
unsigned retro_get_region() { return RETRO_REGION_NTSC; }

// Cartridge RAM.
void *retro_get_memory_data(unsigned id)
{
   if (id == RETRO_MEMORY_SAVE_RAM)
      return libretro.memoryData(Interface::Memory::RAM);
   else
      return nullptr;
}

size_t retro_get_memory_size(unsigned id)
{
   if (id == RETRO_MEMORY_SAVE_RAM)
      return libretro.memorySize(Interface::Memory::RAM);
   else
      return 0;
}

// Controllers.
void retro_set_controller_port_device(unsigned, unsigned) {}

