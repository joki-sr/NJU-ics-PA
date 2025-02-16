#include <am.h>
#include <nemu.h>

extern uint32_t *rtc_port_base;

void __am_timer_init() {
}

// 读出系统启动后的微秒数.
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // uptime->us = 0;
  //读reg
  uint32_t low , high;
  low = inl(RTC_ADDR);
  high = inl(RTC_ADDR + 4);
  //保存
  uptime->us = (uint64_t)low | ((uint64_t)high << 32);
}

// 暂时没有实现
// 总是输出1900年0月0日0时0分0秒
void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
