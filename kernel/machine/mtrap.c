#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include "string.h"

static void print_errorline(){
  // sprint("@@\n");
  uint64 error_addr = read_csr(mepc);
  uint64 line = 0;
  static char path[256];
  // sprint("%0x\n",error_addr);
  for(int i=0;i<current->line_ind;i++){
    // sprint("?%0x\n",current->line[i].addr);
    if(current->line[i].addr == error_addr){
      strcpy(path,current->dir[current->line[i].file]);
      int pathlen = strlen(path);
      path[pathlen] = '/';
      strcpy(path+pathlen+1,current->file[current->line[i].file].file);
      sprint("Runtime error at ");
      sprint("%s",current->dir[current->line[i].file]);
      sprint("/%s",current->file[current->line[i].file].file);
      line = current->line[i].line;
      sprint(":%lld\n",line);
    }
  }
  char tmp;
  uint64 offset = 0, tmp_line = 0;
  // sprint("%s\n",path);
  spike_file_t *f = spike_file_open(path, O_RDONLY, 0);
  while(1){
    spike_file_pread(f, (void*)&tmp, 1, offset);
    // sprint("%c %d\n",tmp, tmp_line);
    offset++;
    if(tmp_line == line - 1){
      sprint("%c",tmp);
    }
    if(tmp == '\n'){
      tmp_line++;
    }
    if(tmp_line == line){
      break;
    }
  }
  spike_file_close(f);
}

static void handle_instruction_access_fault() { panic("Instruction access fault!"); }

static void handle_load_access_fault() { panic("Load access fault!"); }

static void handle_store_access_fault() { panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() { panic("Illegal instruction!"); }

static void handle_misaligned_load() { panic("Misaligned Load!"); }

static void handle_misaligned_store() { panic("Misaligned AMO!"); }

// added @lab1_3
static void handle_timer() {
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64*)CLINT_MTIMECMP(cpuid) = *(uint64*)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}

//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap() {
  uint64 mcause = read_csr(mcause);
  switch (mcause) {
    case CAUSE_MTIMER:
      print_errorline();
      handle_timer();
      break;
    case CAUSE_FETCH_ACCESS:
      print_errorline();
      handle_instruction_access_fault();
      break;
    case CAUSE_LOAD_ACCESS:
      print_errorline();
      handle_load_access_fault();
    case CAUSE_STORE_ACCESS:
      print_errorline();
      handle_store_access_fault();
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      // TODO (lab1_2): call handle_illegal_instruction to implement illegal instruction
      // interception, and finish lab1_2.
      // panic( "call handle_illegal_instruction to accomplish illegal instruction interception for lab1_2.\n" );
      print_errorline();
      handle_illegal_instruction();
      break;
    case CAUSE_MISALIGNED_LOAD:
      print_errorline();
      handle_misaligned_load();
      break;
    case CAUSE_MISALIGNED_STORE:
      print_errorline();
      handle_misaligned_store();
      break;

    default:
      print_errorline();
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      panic( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
