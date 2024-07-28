/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2012-2015 - Michael Lelli
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libretro.h"
#include "internal_cores.h"
#include <SDL2/SDL_image.h>
#include <math.h>

#define FRAME_BUF_WIDTH 320
#define FRAME_BUF_HEIGHT 240

static uint16_t *frame_buf;

void retro_init(void) { lr_retro_init(); }
void retro_deinit(void) { lr_retro_deinit(); }
unsigned retro_api_version(void) { return lr_retro_api_version(); }
void retro_set_controller_port_device(unsigned port, unsigned device) { lr_retro_set_controller_port_device(port, device); }
void retro_get_system_info(struct retro_system_info *info) { lr_retro_get_system_info(info); }
void retro_get_system_av_info(struct retro_system_av_info *info) { lr_retro_get_system_av_info(info); }
void retro_set_environment(retro_environment_t cb) { lr_retro_set_environment(cb); }
void retro_set_audio_sample(retro_audio_sample_t cb) { lr_retro_set_audio_sample(cb); }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { lr_retro_set_audio_sample_batch(cb); }
void retro_set_input_poll(retro_input_poll_t cb) { lr_retro_set_input_poll(cb); }
void retro_set_input_state(retro_input_state_t cb) { lr_retro_set_input_state(cb); }
void retro_set_video_refresh(retro_video_refresh_t cb) { lr_retro_set_video_refresh(cb); }
void retro_reset(void) { lr_retro_reset(); }
void retro_run(void) { lr_retro_run(); }
bool retro_load_game(const struct retro_game_info *info) { return lr_retro_load_game(info); }
void retro_unload_game(void) { lr_retro_unload_game(); }
unsigned retro_get_region(void) { return lr_retro_get_region(); }
bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num) { return lr_retro_load_game_special(type, info, num); }
size_t retro_serialize_size(void) { return lr_retro_serialize_size(); }
bool retro_serialize(void *data, size_t size) { return lr_retro_serialize(data, size); }
bool retro_unserialize(const void *data, size_t size) { return lr_retro_unserialize(data, size); }
void *retro_get_memory_data(unsigned id) { return lr_retro_get_memory_data(id); }
size_t retro_get_memory_size(unsigned id) { return lr_retro_get_memory_size(id); }
void retro_cheat_reset(void) { lr_retro_cheat_reset(); }
void retro_cheat_set(unsigned idx, bool enabled, const char *code) { lr_retro_cheat_set(idx, enabled, code); }

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_log_printf_t log_cb;

static struct retro_variable variables[] = {
    {NULL, NULL}, // This indicates the end of variables
};

static void convert_to_rgb565(SDL_Surface *surface, uint16_t *buffer)
{
   int x, y;
   uint8_t *pixel;
   Uint8 r, g, b;

   for (y = 0; y < surface->h; y++)
   {
      for (x = 0; x < surface->w; x++)
      {
         pixel = (uint8_t *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
         SDL_GetRGB(*((uint32_t *)pixel), surface->format, &r, &g, &b);

         /* Now we need to convert to RGB565 format */
         r = r >> 3;
         g = g >> 2;
         b = b >> 3;

         buffer[y * surface->w + x] = (r << 11) | (g << 5) | b;
      }
   }
}

static void load_bg()
{
   SDL_Surface *bg_img = IMG_Load("./images/grid.png");

   frame_buf = (uint16_t *)calloc(FRAME_BUF_WIDTH * FRAME_BUF_HEIGHT, sizeof(uint16_t));
   for (unsigned i = 0; i < (unsigned)(FRAME_BUF_WIDTH * FRAME_BUF_HEIGHT); i++)
      frame_buf[i] = 4 << 5;

   convert_to_rgb565(bg_img, frame_buf);
   SDL_FreeSurface(bg_img);
}

void lr_retro_init(void)
{
   load_bg();
}

void lr_retro_deinit(void)
{
   if (frame_buf)
      free(frame_buf);
   frame_buf = NULL;
}

unsigned lr_retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void lr_retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

void lr_retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name = "RePlay Screen Test";
   info->library_version = "v1.0";
   info->need_fullpath = false;
   info->valid_extensions = "n/a"; /* Nothing. */
}

/* Doesn't really matter, but need something sane. */
void lr_retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->timing.fps = 60;
   info->timing.sample_rate = 48000.0;
   info->geometry.base_width = FRAME_BUF_WIDTH;
   info->geometry.base_height = FRAME_BUF_HEIGHT;
   info->geometry.max_width = FRAME_BUF_WIDTH;
   info->geometry.max_height = FRAME_BUF_HEIGHT;
   info->geometry.aspect_ratio = (float)FRAME_BUF_WIDTH / (float)FRAME_BUF_HEIGHT;
}

void lr_retro_set_environment(retro_environment_t cb)
{
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;

   environ_cb = cb;

   struct retro_log_callback logging;
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
   {
      log_cb = logging.log;
   }

   environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

   /* We know it's supported, it's internal to RetroArch. */
   environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

const char *lr_retro_get_environment(const char *key)
{
   struct retro_variable var = {0};

   var.key = key;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      return var.value;
   }

   return NULL; // Return NULL if the key wasn't found or there was another error
}

void lr_retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void lr_retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void lr_retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void lr_retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void lr_retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void lr_retro_reset(void)
{
}

void lr_retro_run(void)
{
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   {
      log_cb(RETRO_LOG_INFO, "Variable updated\n");

      free(frame_buf);
      load_bg();

      struct retro_system_av_info av_info;
      lr_retro_get_system_av_info(&av_info);
      environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &av_info);
   }

   input_poll_cb();
   video_cb(frame_buf, FRAME_BUF_WIDTH, FRAME_BUF_HEIGHT, 2 * FRAME_BUF_WIDTH);
}

/* This should never be called, it's only used as a placeholder. */
bool lr_retro_load_game(const struct retro_game_info *info)
{
   (void)info;
   return false;
}

void lr_retro_unload_game(void)
{
}

unsigned lr_retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool lr_retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t lr_retro_serialize_size(void)
{
   return 0;
}

bool lr_retro_serialize(void *data, size_t size)
{
   (void)data;
   (void)size;
   return false;
}

bool lr_retro_unserialize(const void *data, size_t size)
{
   (void)data;
   (void)size;
   return false;
}

void *lr_retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t lr_retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void lr_retro_cheat_reset(void)
{
}

void lr_retro_cheat_set(unsigned idx, bool enabled, const char *code)
{
   (void)idx;
   (void)enabled;
   (void)code;
}