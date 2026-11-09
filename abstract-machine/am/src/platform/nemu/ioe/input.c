#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t kbd_data = inl(KBD_ADDR);

  kbd->keydown = (kbd_data & KEYDOWN_MASK) != 0;
  kbd->keycode = kbd_data & ~KEYDOWN_MASK;
}
