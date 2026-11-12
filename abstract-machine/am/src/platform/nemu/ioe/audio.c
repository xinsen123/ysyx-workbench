#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);

  outl(AUDIO_INIT_ADDR, true);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

static uint32_t sbuf_wpos = 0;   // guest 侧写指针

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  uint8_t *src = (uint8_t *)ctl->buf.start;
  int len = (uint8_t *)ctl->buf.end - src;
  int bufsize = inl(AUDIO_SBUF_SIZE_ADDR);

  while (inl(AUDIO_COUNT_ADDR) + len > bufsize);   // 等 NEMU 把数据取走

  int remain = bufsize - sbuf_wpos;
  int first = len < remain ? len : remain;
  for (int i = 0; i < first; i++) outb(AUDIO_SBUF_ADDR + sbuf_wpos + i, src[i]);
  for (int i = 0; i < len - first; i++) outb(AUDIO_SBUF_ADDR + i, src[first + i]);
  sbuf_wpos = (sbuf_wpos + len) % bufsize;

  // 通知 NEMU
  outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len);
}
