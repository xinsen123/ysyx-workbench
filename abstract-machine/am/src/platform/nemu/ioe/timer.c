#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // NEMU 的 RTC 由两个 32 位寄存器拼成 64 位微秒数,
  // 高 32 位在 RTC_ADDR + 4, 低 32 位在 RTC_ADDR.
  // 注意: NEMU 的 rtc_io_handler 只在读 RTC_ADDR + 4 时才刷新这两个寄存器,
  // 所以必须"先读高 32 位, 再读低 32 位", 否则读到的是两次刷新的混合值.
  uint32_t hi = inl(RTC_ADDR + 4);
  uint32_t lo = inl(RTC_ADDR);
  uptime->us = ((uint64_t)hi << 32) | lo;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
