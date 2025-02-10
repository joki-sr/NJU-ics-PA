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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
}

#define IRING_SUM 16
#define IRING_LEN 64
char iring[IRING_SUM][IRING_LEN] = {};
char *p_iring = iring[0];
int iring_idx = 0;

// 让CPU执行当前PC指向的一条指令, 然后更新PC.
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf; // 用于存储日志的缓冲区指针
  // 0x80000000:
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
//~~~~~~~~~~~~
  iring_idx = (iring_idx + 1) % IRING_SUM; 
  p_iring = iring[iring_idx];
  p_iring += snprintf(p_iring, IRING_LEN, FMT_WORD ":", s->pc);
//~~~~~~~~~~~~
  int ilen = s->snpc - s->pc; // 指令长度 x86变长
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val; // 指令机器码
  // logbuf 0x80000000: ef be ad de
  for (i = ilen - 1; i >= 0; i --) {  // 机器码是 小端序存储（低字节在低地址），所以这里 逆序输出。
    p += snprintf(p, 4, " %02x", inst[i]); // 指令的机器码以十六进制格式存入 
    //~~~~~~~~~
    p_iring += snprintf(p_iring, 4, " %02x", inst[i]);
    //~~~~~~~~~
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4); // ？A：B // 如果是 x86，则最大指令长度 ilen_max = 8 
  int space_len = ilen_max - ilen;  // 计算 剩余空格，确保不同长度的指令在日志对齐：
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;
  //~~~~~~~~~
  memset(p_iring, ' ', space_len);
  p_iring += space_len;
  //~~~~~~~~~

#ifndef CONFIG_ISA_loongarch32r
  // 将机器码转换回汇编指令 并存入 logbuf
  // 0x80000000: ef be ad de   add r1, r2, r3
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      //x86 使用 s->snpc 作为 PC 地址（可能是下一条指令地址）；其他架构（如 RISC-V）使用 s->pc
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
  //~~~~~~~~~
  disassemble(p_iring, iring[iring_idx] + IRING_LEN - p_iring, 
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
  //~~~~~~~~~
  // printf("IRING[%d]:%s\n", iring_idx,iring[iring_idx]);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

// 模拟了CPU的工作方式: 不断执行指令
// n=-1 : 
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
