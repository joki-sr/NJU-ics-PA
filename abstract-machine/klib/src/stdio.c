#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void itoa(int num, char *str, int base){
  int is_neg = (num < 0);
  int str_idx = 0;
  char tmp[256];
  int tmp_idx = 0;

  if(is_neg){
    str[str_idx++] = '-';
    num = -num;
  }

  while(num > 0){
    tmp[tmp_idx++] = num % base;
    num /= base;
  }
  //reverse
  while(tmp_idx>0){
    tmp_idx --;
    str[str_idx++] = tmp[tmp_idx];
  }
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  size_t i = 0;

  for(i=0;fmt[i] != '\0';i++){
    if(fmt[i]=='%'){
      i++;
      if(fmt[i]=='s'){
        // %s
        strcat(out, va_arg(va,char*));
      }else if(fmt[i]=='d'){
        // %d
        int n = va_arg(va, int);
        // printf("sprintf:n=%d\n",n);
        assert(n==-115);
        itoa(n, out+strlen(out), 10);
        // printf("sprintf:out=%s", *out);
      }else assert(0);
    }else{
      strcat(out, fmt + i);
    }
  }
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
