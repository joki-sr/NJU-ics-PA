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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

//expr.c
word_t expr(char *e, bool *success);

//watchpoint.c
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char *reg;//watch a reg
  char *addr;//watch an addr
  int enabled;  //启用/禁用

  //size_t size;//观察点监视的数据大小
  //char *expression;//条件表达式
  //    int type;            // 观察点的类型（读、写、读写）

} WP;
WP* new_wp(void);
void free_wp(WP *wp);

// void wp_newreg(char*);
// void wp_newaddr(char*);
void wp_add(char*);
void wp_clean(int);
void wp_del(int);
void wp_display(void);

#endif
