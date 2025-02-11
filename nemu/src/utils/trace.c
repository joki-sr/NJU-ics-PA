#include <utils.h>

// ----------- mtrace -----------

MtraceEntry mring[MRING_SUM];
int mring_idx =0;

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


// 
char iring[IRING_SUM][IRING_LEN] = {};
int iring_idx =0;

void itrace_add(char *str){
  iring_idx = (iring_idx + 1) % IRING_SUM;
  // iring[iring_idx] = *str;
  memcpy(iring[iring_idx], str, IRING_LEN);
}

int itrace(void){
  printf("itrace:\n");
  for(int i=0;i<IRING_SUM;i++){
    if(i==iring_idx){
      printf("--> ");
    }else{
      printf("    ");
    }
    printf("%s\n", iring[i]);
  }
  return 0;
}