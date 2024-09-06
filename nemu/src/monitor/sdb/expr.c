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

//return number of tokens ,max: 32.
static int make_token(char *e) {
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
      //if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
      int regexec_ret = regexec(&re[i], e + position, 1, &pmatch, 0);
      if(regexec_ret == 0 && pmatch.rm_so == 0){
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
          case 0: break;
          case TK_NOTYPE: break;//不记录space
          default:
            // 记录匹配的token：str，type
            // nr_token++;
            printf("i'll store this token\n");
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
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
  printf("get %d tokens.\n",nr_token);
  return nr_token;
}

// 判断表达式是否被一对匹配的括号包围着, 
// 同时检查表达式的左右括号是否匹配
bool check_parentheses(int p, int q){
  if( ! (tokens[p].type == TK_PAREN_OPEN && tokens[q].type == TK_PAREN_CLOSE) )
    return false;
  p++, q--;
  if(p>q)assert(0);
  //删去外边的括号，如果还能配对
  int sum = 0;
  for(int i=p;i<=q;i++){
    switch (tokens[i].type)
    {
    case TK_PAREN_OPEN:
      sum++;
      break;
    case TK_PAREN_CLOSE:
      if(sum<1)return false;
      sum--;
    default:
      break;
    }
  }
  if(sum)return false;
  return true;
}

//-1: a < b
// 0: a=b
// 1: a>b
static int op_bigger_than(int a, int b){
  if(a == TK_MINUS || a == TK_PLUS) 
    a = 1;
  else if(a == TK_DIVIDE || a == TK_MULTIPLY) 
    a = 2;
  else
    a = 3;

  if((b == TK_MINUS || b == TK_PLUS)) b = 1;
  else if(b == TK_DIVIDE || b == TK_MULTIPLY) b = 2;
  else b = 3;

  if( a < b)return -1;
  else if(a == b)return 0;
  else return 1;

  // if( (a == TK_MINUS || a == TK_PLUS) && (b == TK_MINUS || b == TK_PLUS))
  //   return 0;
  // if( (a == TK_DIVIDE || a == TK_MULTIPLY) && (b == TK_DIVIDE || b == TK_MULTIPLY))
  //   return 0;
  // if
}

// tokens[p] ~ tokens[q]
// error result: UINT32_MAX
static uint32_t eval(int p, int q){
  uint32_t ret = 0;
  if (p > q) {
    /* Bad expression */
    printf("error: eval() p > q\n");
    ret = UINT32_MAX;
  }else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    char *end;
    unsigned long ul = strtoul(tokens[p].str, &end, 10);
    if(*end != '\0'){
      printf("error: eval() p == q, conversion error, wrong format of tokens[%d]:str=%s, type = %d.\n", p, tokens[p].str,tokens[p].type);
      ret = UINT32_MAX;
    }else{
      ret = (uint32_t)ul;
    }
  // p < q && ...
  }else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }else {
    /* We should do more things here. */
    //scan and find the main operator
    int i;
    int flag = 1;
    int mtype = 666, mpos = p, type;
    for(i=p;i<=q;i++){
      type = tokens[i].type;
      switch ( type )
      {
      case TK_PLUS:
        //op = '+';
        //break;
      case TK_MINUS:
        //op = '-';
        //break;
      case TK_MULTIPLY:
        //op = '*';
        //break;
      case TK_DIVIDE:
        //op = '/';
        //if(flag) mop=(mop<op)?op:mop; //not in (),choose bigger.
        // not in () && type < mtype
        if(flag == 1 && op_bigger_than(type, mtype) == -1){
          mtype = type;
          mpos = i;
        }
        break;
      case TK_PAREN_OPEN:
        flag = 0;
        break;
      case TK_PAREN_CLOSE:
        flag = 1;
      case TK_INTEGER:
        break;
      default:
        assert(0);
        break;
      }
    }
    //after find main op
    int val1 = eval(p, mpos - 1);
    int val2 = eval(mpos + 1, q);

    switch (mtype) {
      case TK_PLUS: return val1 + val2;
      case TK_MINUS: return val1 - val2;
      case TK_MULTIPLY: return val1 * val2;
      case TK_DIVIDE: 
        if(val2 == 0){
          printf("invalid expr, divide 0.\n");
          return UINT32_MAX;
        }else{
          return val1 / val2;
        }
      default: assert(0);
  }
  }
  return ret;
}

word_t expr(char *e, bool *success) {
  int nr_token = make_token(e);
  if ( nr_token == 0) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // tokens[0:nrtoken]
  uint32_t val = 0;
  val = eval(0,nr_token-1);
  return val;
}
