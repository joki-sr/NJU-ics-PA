#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void itoa(int num, char *str, int base) {
  int is_neg = (num < 0);
  int str_idx = 0;
  char tmp[12];  // 足够存放 int (-2147483648)
  int tmp_idx = 0;

  if (is_neg) {
    num = -num;
  }

  do {
    tmp[tmp_idx++] = (num % base) + '0';  // **转换成字符**
    num /= base;
  } while (num > 0);

  if (is_neg) {
    tmp[tmp_idx++] = '-';  // 负数补上 `-`
  }

  // 反转字符串
  while (tmp_idx > 0) {
    str[str_idx++] = tmp[--tmp_idx];
  }

  str[str_idx] = '\0';  // **确保字符串结尾**
}

char buf[256];
int printf(const char *fmt, ...) {
  // panic("Not implemented");
  memset(buf, 0, 256);
  
  va_list args;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);  // 结束变参处理

  int len =strlen(buf);
  for(int i=0;i<len;i++)
    putch(buf[i]);
    // putch('*');
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  // panic("Not implemented");
  size_t i=0,out_idx = 0;

  for(i=0;fmt[i]!='\0';i++){
    if(fmt[i]=='%'){
      i++;
      if(fmt[i]=='s'){
        char *s = va_arg(ap, char*);
        size_t len = strlen(s);
        memcpy(out + out_idx, s, len);
        out_idx+=len;
      }else if(fmt[i]=='d'){
        // %d
        int n = va_arg(ap, int);
        char num_str[12];
        itoa(n, num_str, 10);
        size_t len = strlen(num_str);
        memcpy(out + out_idx, num_str, len);
        out_idx += len;
      }else{
        putch('%');
        putch(fmt[i]);
        assert(0);
      }
    }else{
      // strcat(out, fmt + i); don't use this
      out[out_idx++] = fmt[i];
    }
  }
  out[out_idx] = '\0'; // 确保字符串正确终止
  return out_idx;
}

// return out lenth
// sprintf(buf, "%d + %d", 1,2);
int sprintf(char *out, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  size_t i = 0, out_idx = 0;

  for(i=0;fmt[i] != '\0';i++){
    if(fmt[i]=='%'){
      i++;
      if(fmt[i]=='s'){
        // %s
        // strcat(out, va_arg(va,char*)); don't use this
        char *s = va_arg(va, char*);
        size_t len = strlen(s);
        memcpy(out + out_idx, s, len);// 使用 memcpy() 避免 strcat() 可能的越界访问。
        out_idx+=len;
      }else if(fmt[i]=='d'){
        // %d
        int n = va_arg(va, int);
        char num_str[12];
        itoa(n, num_str, 10);
        size_t len = strlen(num_str);
        memcpy(out + out_idx, num_str, len);
        out_idx += len;
      }else assert(0);
    }else{
      // strcat(out, fmt + i); don't use this
      out[out_idx++] = fmt[i];
    }
  }

  out[out_idx] = '\0'; // 确保字符串正确终止
  va_end(va);
  return out_idx;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
