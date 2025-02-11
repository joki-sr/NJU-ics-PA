#include <utils.h>

// ----------- mtrace -----------

extern MtraceEntry mring[MRING_SUM];

int mring_idx = 0;

void mtrace_add(paddr_t addr, int len, word_t data){
  mring_idx = (mring_idx + 1) % MRING_SUM;
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
