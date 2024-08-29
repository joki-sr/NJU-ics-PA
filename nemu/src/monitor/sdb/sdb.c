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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

// str is not NULL
// NAN: return -1
// base: 2,8,10,16
int str_to_int(char * str){
  char *pos = str;
  int n = 0;
  char *end;

  //check
  if( pos[0] < '0' || pos[0] > '9'){
    printf("arg is NOT a num.\n");
    return -1;
  }

  n = strtol(str, &end, 10);
  return n;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  printf("Quiting nemu...\n");
  exit(0);
  //return -1;
}

static int cmd_si(char *args) {
  int n = 0;
  // char *arg = strtok(NULL, " ");//num of steps
  // char *pos = arg;
  // Get num
  // sscanf(args,%u,n);
  if(args == NULL) { //default
    n = 1;
  }
  // else if ( '0' < args[0] || args[0] > 9){ //not num
  //   printf("Error format. Example: (nemu)si 10\n");
  //   n = 0;
  // }else{
  //   // while(pos) {
  //   //   n = n * 10 + (int)*pos - (int)'0';
  //   //   pos++;
  //   // }
  //   n = str_to_int(args);
  //   // char *end;
  //   // n = strtol(args, &end, 10);
  // }
  else{
    n = str_to_int(args);
  }
  //printf("i'll run %d steps\n",n);
  if(n < 0) n = 0;
  cpu_exec(n);
  return 0;
}

static int cmd_info_r(char *args) {
  isa_reg_display();
  return 0;
}

static int cmd_info_w(char *args) {
  // todo
  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " ");
  if( arg[0] == 'r'){
    cmd_info_r(strtok(NULL, " "));
  }else if( arg[0] == 'w'){
    cmd_info_w(strtok(NULL, " "));
  }else{
    printf("Unknown cmd.");
  }
  return 0;
}

  
// x 10 0x80000000
// x N  guest_addr
static int cmd_x(char *args) {
  char *num = strtok(NULL, " ");
  char *addr = strtok(NULL," ");
  int n ;
  vaddr_t vaddr;

  //parse
  if( num == NULL || addr == NULL){
    printf("Error format of cmd_x\n");
    return 0;
  }
  n = str_to_int(num);
  sscanf(addr, "%x", &vaddr);

  int i;
  for(i=0;i<n;i++){
    printf("0x%x: 0x%-10x\n", vaddr, vaddr_read(vaddr, 4));
    vaddr++;
  }
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "x", "Scan memory and print the value", cmd_x},
  { "si", "Execute N instructions and then stop, default: 1 step", cmd_si},
  { "info", "r: regs / w: watching points", cmd_info}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
