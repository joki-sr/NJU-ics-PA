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

// 生成一个小于 n 的随机数
uint32_t choose(uint32_t n) {
  return rand() % n; // 随机选择 0 到 n-1 之间的一个数
}

unsigned int count_bits(int val){
  unsigned int count = 0;
  while(val){
    count ++;
    val >>= 1;
  }
  return count;
}

void gen(char ch) {
    size_t len = strlen(buf);
    // size_t buf_size = sizeof(buf);
    if (len < BUF_LEN - 1) {
        buf[len] = ch;
        buf[len + 1] = '\0';
    }
}


void gen_num(void) {
    uint32_t num = rand(); // 使用随机数生成器生成随机数
    num = num % 137;
    char num_str[4]; // 用于存储转换后的数字字符串（10位数字 + '\0'）
    snprintf(num_str, sizeof(num_str), "%u", num); // 转换为字符串
    strcat(buf, num_str); // 追加到 buf 的末尾
}

void gen_rand_op(void) {
    char ops[] = "+-*/";
    char op = ops[choose(4)]; // 从运算符中选择一个
    gen(op); // 将运算符追加到 buf 的末尾
}

// void gen_rand_expr() {
//   switch (choose(3)) {
//     case 0: gen_num(); break;
//     case 1: gen('('); gen_rand_expr(); gen(')'); break;
//     default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
//   }
// }

void gen_rand_expr(void) {
  switch (choose(4)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    case 2: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
    case 3: // 处理插入空格
      if (strlen(buf) > 0) {
          gen(' '); // 在表达式中随机插入空格
      }
      gen_rand_expr();
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
    buf[0] = '\0';
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
