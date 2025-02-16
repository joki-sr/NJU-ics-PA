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
#include <memory/paddr.h>

// max number of registered map
#define NR_MAP 16

static IOMap maps[NR_MAP] = {};
static int nr_map = 0; //number of registered map

static IOMap* fetch_mmio_map(paddr_t addr) {
  int mapid = find_mapid_by_addr(maps, nr_map, addr);
  return (mapid == -1 ? NULL : &maps[mapid]);
}

static void report_mmio_overlap(const char *name1, paddr_t l1, paddr_t r1,
    const char *name2, paddr_t l2, paddr_t r2) {
  panic("MMIO region %s@[" FMT_PADDR ", " FMT_PADDR "] is overlapped "
               "with %s@[" FMT_PADDR ", " FMT_PADDR "]", name1, l1, r1, name2, l2, r2);
}

/* device interface */
// eg:add_mmio_map(name: "rtc", addr: CONFIG_RTC_MMIO, space: rtc_port_base, len: 8, callback: rtc_io_handler);
// 注册端口，记录逻辑低地址、逻辑高地址、物理起始地址、callback.
/**
 * @brief 注册一个内存映射的 I/O 端口，并设置相关的回调函数。
 * 
 * @param name 端口名称，用于标识和管理此 I/O 端口。
 * @param addr 该端口的物理地址。
 * @param space 该端口映射的空间指针，指向对应的内存区域或 I/O 空间。
 * @param len 该端口所占用的地址范围长度，通常是 8 或 16 字节。
 * @param callback 该端口的 I/O 操作回调函数，用于处理对该端口的访问。
 */
void add_mmio_map(const char *name, paddr_t addr, void *space, uint32_t len, io_callback_t callback) {
  assert(nr_map < NR_MAP);//注册数量过多
  paddr_t left = addr, right = addr + len - 1;
  //检查重叠
  if (in_pmem(left) || in_pmem(right)) {//如果要注册的设备地址在物理内存范围内
    report_mmio_overlap(name, left, right, "pmem", PMEM_LEFT, PMEM_RIGHT);// 与物理内存重叠
  }
  for (int i = 0; i < nr_map; i++) {
    if (left <= maps[i].high && right >= maps[i].low) { //什么玩意
      report_mmio_overlap(name, left, right, maps[i].name, maps[i].low, maps[i].high); //// 与其他设备内存重叠
    }
  }
  // 添加
  maps[nr_map] = (IOMap){ .name = name, .low = addr, .high = addr + len - 1,
    .space = space, .callback = callback };
  Log("Add mmio map '%s' at [" FMT_PADDR ", " FMT_PADDR "]",
      maps[nr_map].name, maps[nr_map].low, maps[nr_map].high);

  nr_map ++;
}

/* bus interface */
word_t mmio_read(paddr_t addr, int len) {
  return map_read(addr, len, fetch_mmio_map(addr));
}

void mmio_write(paddr_t addr, int len, word_t data) {
  map_write(addr, len, data, fetch_mmio_map(addr));
}
