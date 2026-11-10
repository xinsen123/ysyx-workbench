#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {

  uint32_t size = inl(VGACTL_ADDR);
  int w = size >> 16;
  int h = size & 0xffff;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = w * h * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *src = (uint32_t *)ctl->pixels;
  int x = ctl->x, y = ctl->y, h = ctl->h, w = ctl->w;

  // pixels 为 NULL 表示"只刷新, 不绘图"(测试里用它做一次 sync)
  if (src != NULL && w > 0 && h > 0) {
    int screen_w = inl(VGACTL_ADDR) >> 16;   // 行跨度是屏幕宽度, 不是 w

    for (int row = 0; row < h; row++) {
      uint32_t *dst = fb + (y + row) * screen_w + x;
      uint32_t *s = src + row * w;
      for (int col = 0; col < w; col++) {
        dst[col] = s[col];
      }
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
