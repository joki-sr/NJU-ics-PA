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
int nr_wp = 0;//已经添加的watchpoint数

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

// WP* wp_new(void){
//   if(free_ == NULL){
//     printf("Too much watchpoints.\n");
//     return NULL;
//   }
//   WP *wp = free_;
//   free_ = wp->next;
//   wp->next = head;
//   head = wp;
//   nr_wp++;
//   return wp;
// }

void wp_add(char *args){
  //parse
  char *arg = strtok(NULL, " ");
  //取free_
  WP *p = NULL;
  if(free_ == NULL){
    printf("Cannot add. Too much watchpoints.\n");
  }else{
    p=free_;
    free_ = free_->next;
  }

  //添加到head
  if(head == NULL){
    head = p;
    p->next = NULL;
  }else{
    p->next = head;
    head = p;
  }
  //补充wp信息
  p->enabled = 1;
  if(arg[0] == '$'){
    p->addr = NULL;
    if ( (p->reg = malloc(strlen(arg) + 1)) != NULL){
      strcpy(p->reg, arg);
    }else{
      printf("wp_add() cannot malloc.(1)\n");
    }
  }else{
    p->reg = NULL;
    if( (p->addr = malloc(strlen(arg) + 1)) == NULL ){
      printf("wp_add() cannot malloc.(2)\n");
    }else{
      strcpy(p->addr, arg);
    }
  }
  nr_wp ++;
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
//wp_del, wp_init调用
void wp_clean(int n){
  if(n < 0 || n > 31)return;
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

  //找
  //pp为找到的，p是pp前驱
  WP *p=head, *pp = head;
  while(pp){
    if(pp->NO == n )break;
    else{
      p = pp;
      pp = pp->next;
      //continue
    }
  }
  if(pp==NULL){//找不到
    printf("Error: wp[%d] not in headList.\n",n);
    WP *tmp = head;
    while(tmp){
      printf("%d\n",tmp->NO);
      tmp = tmp->next;
    }
    return;
  }

  //删除
  if(p!=NULL){//pp前有节点
    p->next = pp->next;
  }else{
    head = pp->next;
  }

  //添加到free
  wp_clean(n);
  if(free_==NULL){
    free_ = pp;
    pp->next = NULL;
  }else{
    pp->next = free_;
    free_ = pp;
  }
  nr_wp --;
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