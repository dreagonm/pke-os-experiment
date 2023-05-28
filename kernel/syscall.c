/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"

#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

ssize_t sys_user_backtrace(uint64 depth){
  // uint64 fp = current->trapframe->regs.s0; // fp
  // uint64 sp = current->trapframe->regs.sp; // 用户态栈地址
  uint32 i = 0;
  // sprint("fp:%0x sp:%0x\n",fp,sp);
  // for(uint64 i = 64; i> 0;i-=8){
  //   sprint("%x : %x\n",fp-i, *((uint64 *)(fp-i)));
  // }
  // for(uint64 i = 0; i< 64;i+=8){
  //   sprint("%x : %x\n",fp+i, *((uint64 *)(fp+i)));
  // }
  // sprint("%0x %0x\n%0x %0x",fp,*(((uint64 *)fp)),
  //   *(((uint64 *)(fp+8))), *(((uint64 *)(fp+16)))
  // );
  // sprint("back trace the user app in the following:\n");
  uint64 s0_t = *((uint64*)(current->trapframe->regs.s0 - 8)); //70
  // sprint("!!!%0x\n",current->trapframe->regs.s0 - 8);
  uint64 sp_t = current->trapframe->regs.sp + 32; // 60
  for(i = 0;i<depth;i++){
    // sprint("sp = %0x s0 = %0x\n",sp_t, s0_t);
    uint64 ra = *((uint64*)(s0_t -8));
    // sprint("ra = %0x\n",ra);
    int64 maxt = -1;
    uint64 maxaddr = 0;
    for(int j = 0;j<current->Symbol_num;j++){
      // sprint("@%x %x\n",(uint64)current->Symbols[j].st_info,current->Symbols[j].st_value);
      if(ELF_ST_BIND(current->Symbols[j].st_info) == STB_GLOBAL &&
        ELF_ST_TYPE(current->Symbols[j].st_info) == STT_FUNC &&
        current->Symbols[j].st_value < ra &&
        current->Symbols[j].st_value > maxaddr
      ){
        maxt = j;
        maxaddr = current->Symbols[j].st_value;
      }
    }
    if(maxt != -1){
      // sprint("%d\n",current->Symbols[maxt].st_name);
      for(int k=current->Symbols[maxt].st_name;k<current->str_len;k++){
        sprint("%c",current->str[k]);
        if(!current->str[k]){
          sprint("\n");
          break;
        }
      }
    }
    sp_t = s0_t;
    s0_t = *((uint64*)(s0_t - 16));
  }
  return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    case SYS_user_backtrace:
      return sys_user_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
