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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

//表达式中token的类型
enum token_types{
  TK_NOTYPE = 256, //SPACE 
  TK_EQ, TK_INTEGER, TK_PLUS, TK_MINUS, TK_MULTIPLY, TK_DIVIDE,// =+-*/ 
  TK_PAREN_OPEN, TK_PAREN_CLOSE, //()
  /* TODO: Add more token types */

};

// 正则表达式+对应的token类型
static struct rule {
  const char *regex; //regular expression of token
  int token_type;   // 
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  //{" +", TK_NOTYPE},    // spaces
  //{"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"[0-9]+",TK_INTEGER},     // 十进制整数
  {"\\+",TK_PLUS},        // 加号
  {"-",TK_MINUS},          // 减号
  {"\\*",TK_MULTIPLY},        // 乘号
  {"/",TK_DIVIDE},          // 除号
  {"\\(",TK_PAREN_OPEN},        // 左括号
  {"\\)",TK_PAREN_CLOSE},        // 右括号
  {"[ \t\n]+",TK_NOTYPE}    // 空格串（包含空格、制表符和换行符）
};

// number of rules[]
// token的类型数量
#define NR_REGEX ARRLEN(rules)

// compiled rules to use
static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};//不被优化；初始化默认值
static int nr_token __attribute__((used))  = 0;//分析后得到的token数量

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      // regexec():进行匹配
      // re:指向已编译正则表达式的 regex_t 结构体的指针
      // e+pos:指向待匹配的字符串
      // 1:指定 pmatch 数组的大小，即可以存储多少个匹配结果.成员：
      // pmatch：pmatch[0].rm_so 和 pmatch[0].rm_eo 分别表示匹配子串的起始和结束位置。
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;


        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;//不记录space
          default:
            // 记录匹配的token：str，type
            memcpy(tokens[i].str, substr_start, substr_len);
            tokens[i].type = rules[i].token_type;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
