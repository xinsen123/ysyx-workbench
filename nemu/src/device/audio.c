/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static uint32_t sbuf_rpos = 0;   // 宿主读指针

static void sdl_audio_callback(void *userdata, uint8_t *stream, int len) {
  uint32_t count = audio_base[reg_count];
  uint32_t n = count < (uint32_t)len ? count : (uint32_t)len;

  if (n > 0) {
    uint32_t remain = CONFIG_SB_SIZE - sbuf_rpos;
    uint32_t first = n < remain ? n : remain;
    memcpy(stream, sbuf + sbuf_rpos, first);
    if (n > first) memcpy(stream + first, sbuf, n - first);   // 回绕
    sbuf_rpos = (sbuf_rpos + n) % CONFIG_SB_SIZE;
    audio_base[reg_count] -= n;
  }
  if (n < (uint32_t)len) memset(stream + n, 0, len - n);       // 不够补静音
}

static void init_sdl_audio(void) {
  SDL_AudioSpec s = {
    .freq     = audio_base[reg_freq],
    .format   = AUDIO_S16SYS,
    .channels = audio_base[reg_channels],
    .samples  = audio_base[reg_samples],
    .callback = sdl_audio_callback,
    .userdata = NULL
  };
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  if (SDL_OpenAudio(&s, NULL) < 0) panic("SDL_OpenAudio: %s", SDL_GetError());
  SDL_PauseAudio(0);          // 0 = 开始播放
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  switch (offset / 4) {
    case reg_sbuf_size:                 // 读: 填大小
      if (!is_write) audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
      break;
    case reg_init:                      // 写 1: 启动 SDL
      if (is_write && audio_base[reg_init]) init_sdl_audio();
      break;
    case reg_count:
      // 读: count 已经是 audio_base[reg_count], host_read 直接取
      // 若采用"guest 写 count 通知"的方案, 这里处理 is_write 的累加
      break;
    default: break;
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
