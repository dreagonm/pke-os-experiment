#ifndef _PROC_H_
#define _PROC_H_

#include "riscv.h"

#define ELF_ST_BIND(val)      (((unsigned char) (val)) >> 4)
#define ELF_ST_TYPE(val)      ((val) & 0xf)
#define ELF_ST_INFO(bind, type)   (((bind) << 4) + ((type) & 0xf))

#define STB_LOCAL   0       /* Local symbol */
#define STB_GLOBAL  1       /* Global symbol */
#define STB_WEAK    2       /* Weak symbol */
#define STB_NUM     3       /* Number of defined types.  */
#define STB_LOOS    10      /* Start of OS-specific */
#define STB_GNU_UNIQUE  10      /* Unique symbol.  */
#define STB_HIOS    12      /* End of OS-specific */
#define STB_LOPROC  13      /* Start of processor-specific */
#define STB_HIPROC  15      /* End of processor-specific */

#define STT_NOTYPE  0       /* Symbol type is unspecified */
#define STT_OBJECT  1       /* Symbol is a data object */
#define STT_FUNC    2       /* Symbol is a code object */
#define STT_SECTION 3       /* Symbol associated with a section */
#define STT_FILE    4       /* Symbol's name is file name */
#define STT_COMMON  5       /* Symbol is a common data object */
#define STT_TLS     6       /* Symbol is thread-local data object*/
#define STT_NUM     7       /* Number of defined types.  */
#define STT_LOOS    10      /* Start of OS-specific */
#define STT_GNU_IFUNC   10      /* Symbol is indirect code object */
#define STT_HIOS    12      /* End of OS-specific */
#define STT_LOPROC  13      /* Start of processor-specific */
#define STT_HIPROC  15      /* End of processor-specific */

typedef struct elf_Sym_t{
  uint32 st_name; // 4 B (B for bytes)
  uint8 st_info; // 1 B
  uint8 st_other; // 1 B
  uint16 st_shndx; // 2 B
  uint64 st_value; // 8 B
  uint64 st_size; // 8 B
} elf_Sym; // total size = 24 B


typedef struct trapframe_t {
  // space to store context (all common registers)
  /* offset:0   */ riscv_regs regs;

  // process's "user kernel" stack
  /* offset:248 */ uint64 kernel_sp;
  // pointer to smode_trap_handler
  /* offset:256 */ uint64 kernel_trap;
  // saved user process counter
  /* offset:264 */ uint64 epc;
}trapframe;

// the extremely simple definition of process, used for begining labs of PKE
typedef struct process_t {
  // pointing to the stack used in trap handling.
  uint64 kstack;
  // trapframe storing the context of a (User mode) process.
  trapframe* trapframe;
  uint8 *str;
  elf_Sym *Symbols;
  uint64 Symbol_num, str_len;
}process;

void switch_to(process*);

extern process* current;

#endif
