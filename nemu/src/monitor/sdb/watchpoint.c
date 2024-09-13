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

#include "sdb.h"

#define NR_WP 32
int nr_wp = 0;

// typedef struct watchpoint {
//   int NO;
//   struct watchpoint *next;

//   char *reg;//watch a reg
//   char *addr;//watch an addr
//   int enabled;  //启用/禁用

//   //size_t size;//观察点监视的数据大小
//   //char *expression;//条件表达式
//   //    int type;            // 观察点的类型（读、写、读写）

// } WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_clean(i);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* wp_new(void){
  if(free_ == NULL){
    printf("Too much watchpoints.\n");
    return NULL;
  }
  WP *wp = free_;
  free_ = wp->next;
  wp->next = head;
  head = wp;
  nr_wp++;
  return wp;
}

void wp_add(char *args){
  WP *p = wp_new();
  char *arg = strtok(NULL, " ");
  if(arg[0] == '$'){
    p->addr = NULL;
    p->reg = malloc(strlen(arg) +1);
    if(p->reg != NULL){
      strcpy(p->reg, arg);
    }else{
      printf("wp_add: cannot malloc.\n");
      return;
    }
  }else{
    //p->addr = *arg;
    p->addr = malloc(strlen(arg) + 1);
    if(p->addr!=NULL){
      strcpy(p->addr, arg);
    }else{
      printf("wp_add: cannot malloc.\n");
      return ;
    }
    p->reg = NULL;
    printf("todo,wp_add()\n");
    return;
  }
  
  p->enabled = 1;
  return;

}

// void wp_newreg(char *args){
//   WP *p = wp_new();
//   p->addr = NULL;
//   p->reg = args;

//   //add info
// }

// void wp_newaddr(char *args){
//   WP *p = wp_new();
//   p->addr = args;
//   p->reg = NULL;
//   //add info
// }

//清空wp_pool[n]的NO之外的内容
//wp_clean, wp_init调用
void wp_clean(int n){
  WP *p = &wp_pool[n];
  p->reg = NULL;
  p->addr = NULL;
  p->enabled = 0;

}

void wp_del(int n){
  if(n >  NR_WP){
    printf("Error: n >  NR_WP\n ");
    return ;
  }
  // wp_clean(n);

  //找到head队列中的wp n
  //p->pp(3)
  //WP *p = head,*pp = p->next;
  WP *p=NULL, *pp = head;
  while(pp){
    if(pp->NO ==3)break;
    else{
      p = pp;
      pp = pp->next;
      //continue
    }
  }
  if(pp==NULL){//找不到
    printf("Error: wp[%d] not in headList.\n",n);
    return;
  }

  p->next = pp->next;
  pp->next = free_;
  free_ = pp;
  wp_clean(n);
  nr_wp--;
  return;
}

void wp_display(void){
  printf("Num   Type         Enb     What\n");
  for(int i=0;i<NR_WP;i++){
    WP *p = &wp_pool[i];
    if(p->enabled == 1){
      printf("%5d watchpoint   true   %s%s\n",i,p->reg,p->addr);
    }else{
      printf("%5d watchpoint   false \n",i);
    }
  }
}