#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  // panic("Not implemented");
  size_t cnt = 0;
  while( s[cnt] != '\0')
    cnt ++;
  return cnt;
}

char *strcpy(char *dst, const char *src) {
  // panic("Not implemented");
  size_t dst_len = strlen(dst);
  size_t i = 0;

  for(;src[i]!='\0';i++){
    dst[i] = src[i];
  }

  for(;i<dst_len;i++){
    dst[i] = '\0';
  }

  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  // panic("Not implemented");
  size_t i = 0;

  for(;i<n;i++){
    dst[i] = src[i];
    if(src[i]=='\0')
      break;
  }

  return dst;
}

char *strcat(char *dst, const char *src) {
  // panic("Not implemented");
  size_t dst_len = strlen(dst);
  size_t i = 0;
  while( src[i]!='\0'){
    dst[dst_len + i] = src[i];
    i++;
  }

  dst[dst_len + i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;

  for(;s1[i] && s2[i];i++){
    if(s1[i] > s2[i])
      return 1;
    else if(s1[i] < s2[i])
      return -1;
  }
  return 0;
  // panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  for(;i<n;i++){
    if(s1[i]>s2[i])
      return 1;
    else if(s1[i]<s2[i])
      return -1;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  // panic("Not implemented");
  unsigned char *p = (unsigned char*)s;
  unsigned char tar = (unsigned char)c;
  size_t i = 0;
  for(;i<n;i++){
    p[i] = tar;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  // panic("Not implemented");
  if(n==0)
    return 0;
  unsigned char *p1 = (unsigned char *)s1;
  unsigned char *p2 = (unsigned char*)s2;
  size_t i = 0;

  for(;i<n;i++){
    if(p1[i] != p2[i])
      return (unsigned char)(p1[i] - p2[i]);
  }

  return 0;
}

#endif
