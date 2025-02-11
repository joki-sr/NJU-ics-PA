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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};// 0x8000000 B = 2^27 = 128MB
#endif

// 在guest中的offset等于在pmem中的offset
// guest: BASE----paddr
// host:  pmem----return_here
uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC) // 使用动态内存分配
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
  // 用随机数 (rand()) 填充 pmem 指向的内存区域，大小为 CONFIG_MSIZE
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
  // 记录物理内存的起始和结束地址
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

//~~~~~~mtrace
#define MRING_SUM 16
#define MRING_LEN 32
// #define CONFIG_MTRACE
// char mring[MRING_SUM][MRING_LEN] = {};
typedef struct {
  paddr_t addr;
  word_t value;
} MtraceEntry;
MtraceEntry mring[MRING_SUM];
// char *p_mring = mring[0];
int mring_idx = 0;
//~~~~~~mtrace

void mtrace_add(paddr_t addr, int len, word_t data){
  mring_idx = (mring_idx + 1) % MRING_SUM;
  // p_mring = mring[mring_idx];
  // p_mring += snprintf(p_mring, 10, FMT_WORD ": ", addr);
  // word_t value = 0;
  // switch (len)
  // {
  //   case 1: value = *(uint8_t  *)data; break;
  //   case 2: value = *(uint16_t *)data; break;
  //   case 4: value = *(uint32_t *)data; break;
  //   IFDEF(CONFIG_ISA64, case 8: value = *(uint64_t *)data; break);
  //   default: MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
  // }
  mring[mring_idx].addr = addr;
  mring[mring_idx].value = data;
}

int mtrace(void){
#ifndef CONFIG_MTRACE
  printf("macro CONFIG_MTRACE is not configured.\n");
  return 0;
#else
  printf("addr:data\n");
  for(int i=0;i<MRING_SUM;i++){
    if(mring_idx == i){
      printf("--> ");
    }else{
      printf("    ");
    }
    printf("0x%x: 0x%08x signed:%012d unsigned:%u\n",
            mring[i].addr, mring[i].value, mring[i].value,(unsigned int)mring[i].value);
  }
  return 0;
#endif
}

word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))){
    word_t out = pmem_read(addr, len);
#ifdef CONFIG_MTRACE
    mtrace_add(addr, len, out);
#endif
    return out;
  }
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) {
    pmem_write(addr, len, data);
#ifdef CONFIG_MTRACE
    mtrace_add(addr, len, data);
#endif
    return; 
  }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
