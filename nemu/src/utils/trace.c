#include <utils.h>

//~~~~~~mtrace
#define MRING_SUM 16
#define MRING_LEN 32

typedef struct {
  paddr_t addr;
  word_t value;
} MtraceEntry;
MtraceEntry mring[MRING_SUM];

int mring_idx = 0;
//~~~~~~mtrace

void mtrace_add(paddr_t addr, int len, word_t data){
  mring_idx = (mring_idx + 1) % MRING_SUM;
  // p_mring = mring[mring_idx];
  // p_mring += snprintf(p_mring, 10, FMT_WORD ": ", addr);
  // word_t value = 0;
  // switch (len)
  // {
  //   case 1: value = *(uint8_t  *)data; break;
  //   case 2: value = *(uint16_t *)data; break;
  //   case 4: value = *(uint32_t *)data; break;
  //   IFDEF(CONFIG_ISA64, case 8: value = *(uint64_t *)data; break);
  //   default: MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
  // }
  mring[mring_idx].addr = addr;
  mring[mring_idx].value = data;
}

int mtrace(void){
#ifndef CONFIG_MTRACE
  printf("macro CONFIG_MTRACE is not configured.\n");
  return 0;
#else
  for(int i=0;i<MRING_SUM;i++){
    if(mring_idx == i){
      printf("--> ");
    }else{
      printf("    ");
    }
    printf("0x%08x: 0x%08x signed:%-12d unsigned:%u\n",
            mring[i].addr, mring[i].value, mring[i].value,(unsigned int)mring[i].value);
  }
  return 0;
#endif
}
