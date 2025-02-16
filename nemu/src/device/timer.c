/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

#include <device/map.h>
#include <device/alarm.h>
#include <utils.h>

// 模拟了i8253计时器的功能
// 自定义的时钟

static uint32_t *rtc_port_base = NULL; //指向 RTC（实时时钟）寄存器的指针

// 映射到两个32位的RTC寄存器. CPU可以访问这两个寄存器来获得用64位表示的当前时间.
static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
  assert(offset == 0 || offset == 4); // 确保偏移量只能是 0 或 4（这两个地址对应 RTC 的两个 32 位寄存器）。
  if (!is_write && offset == 4) { //读操作，并且偏移量是 4（即读取高 32 位的时间戳）
    uint64_t us = get_time();
    rtc_port_base[0] = (uint32_t)us; // 低 32 位
    rtc_port_base[1] = us >> 32; // 高 32 位
  }
}

// 发起时钟中断
#ifndef CONFIG_TARGET_AM
static void timer_intr() {
  if (nemu_state.state == NEMU_RUNNING) {
    extern void dev_raise_intr();
    dev_raise_intr(); // 触发中断请求
  }
}
#endif

void init_timer() {
  rtc_port_base = (uint32_t *)new_space(8);//为 RTC 寄存器分配了 8 字节空间（两个 32 位寄存器）
#ifdef CONFIG_HAS_PORT_IO // 端口映射IO
  add_pio_map ("rtc", CONFIG_RTC_PORT, rtc_port_base, 8, rtc_io_handler);
#else // 内存映射IO
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
#endif
  IFNDEF(CONFIG_TARGET_AM, add_alarm_handle(timer_intr));
}
