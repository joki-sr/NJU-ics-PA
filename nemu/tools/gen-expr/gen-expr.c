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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#ifndef BUF_LEN
#define BUF_LEN 65536
#endif

// this should be enough
static char buf[BUF_LEN] = {};
static char code_buf[BUF_LEN + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "  //%s将用来插入 buf 中的表达式
"  printf(\"%%u\", result); "
"  return 0; "
"}";

void gen_num(void){
  //static int seed = time(0);
  uint32_t num = srandom( (unsigned int)time(NULL));
  snprintf(buf, sizeof(buf),"%u",num);
}

void gen_rand_op(void){
  switch(choose(4)){
    case 0:gen('+');break;
    case 1:gen('-');break;
    case 2:gen('*');break;
    default:gen('/');break;
  }
}

void gen(char ch){
  if( sizeof(buf) > BUF_LEN)return;
  strcat(buf, ch);
}

// todo
static void gen_rand_expr() {
  buf[0] = '\0';
  switch (choose(3)) {
    case 0: 
      gen_num();   if( sizeof(buf) > BUF_LEN)return;
      break;
    case 1: 
      gen('(');    if( sizeof(buf) > BUF_LEN)return;
      gen_rand_expr();    if( sizeof(buf) > BUF_LEN)return;
      gen(')');    if( sizeof(buf) > BUF_LEN)return;
      break;
    default: 
      gen_rand_expr();    if( sizeof(buf) > BUF_LEN)return;
      gen_rand_op();    if( sizeof(buf) > BUF_LEN)return;
      gen_rand_expr();    if( sizeof(buf) > BUF_LEN)return;
      break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);  //初始化随机数生成器
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop); // 从命令行参数获取循环次数 loop（默认值为 1）
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf); // code_buf<-code_format<-buf

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);    // 将生成的 C 代码写入到 /tmp/.code.c 文件
    fclose(fp);
    // 调用 gcc 编译 /tmp/.code.c 成可执行文件 /tmp/.expr
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr"); 
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r"); // popen 运行 /tmp/.expr 并读取其输出
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
