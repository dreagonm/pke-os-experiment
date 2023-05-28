#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"

#define MAX_CMDLINE_ARGS 64

// elf header structure
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];
  uint16 type;      /* Object file type */
  uint16 machine;   /* Architecture */
  uint32 version;   /* Object file version */
  uint64 entry;     /* Entry point virtual address */
  uint64 phoff;     /* Program header table file offset */
  uint64 shoff;     /* Section header table file offset */
  uint32 flags;     /* Processor-specific flags */
  uint16 ehsize;    /* ELF header size in bytes */
  uint16 phentsize; /* Program header table entry size */
  uint16 phnum;     /* Program header table entry count */
  uint16 shentsize; /* Section header table entry size */
  uint16 shnum;     /* Section header table entry count */
  uint16 shstrndx;  /* Section header string table index */
} elf_header;

#define SHT_NULL 0 //表明section header无效，没有关联的section。  
#define SHT_PROGBITS 1 //section包含了程序需要的数据，格式和含义由程序解释。  
#define SHT_SYMTAB 2 // 包含了一个符号表。当前，一个ELF文件中只有一个符号表。SHT_SYMTAB提供了用于(link editor)链接编辑的符号，当然这些符号也可能用于动态链接。这是一个完全的符号表，它包含许多符号。  
#define SHT_STRTAB 3 //包含一个字符串表。一个对象文件包含多个字符串表，比如.strtab(包含符号的名字)和.shstrtab(包含section的名称)。  
#define SHT_RELA 4 //重定位节，包含relocation入口，参见Elf32_Rela。一个文件可能有多个Relocation Section。比如.rela.text，.rela.dyn。  
#define SHT_HASH 5 //这样的section包含一个符号hash表，参与动态连接的目标代码文件必须有一个hash表。目前一个ELF文件中只包含一个hash表。讲链接的时候再细讲。  
#define SHT_DYNAMIC 6 //包含动态链接的信息。目前一个ELF文件只有一个DYNAMIC section。  
#define SHT_NOTE 7 //note section, 以某种方式标记文件的信息，以后细讲。  
#define SHT_NOBITS 8 //这种section不含字节，也不占用文件空间，section header中的sh_offset字段只是概念上的偏移。  
#define SHT_REL 9 //重定位节，包含重定位条目。和SHT_RELA基本相同，两者的区别在后面讲重定位的时候再细讲。  
#define SHT_SHLIB 10 //保留，语义未指定，包含这种类型的section的elf文件不符合ABI。  
#define SHT_DYNSYM 11 // 用于动态连接的符号表，推测是symbol table的子集。


typedef struct elf_section_header_t{
    uint32   sh_name;  // 节区名称相对于字符串表的位置偏移
    uint32   sh_type;  // 节区类型
    uint64   sh_flags;  // 节区标志位集合
    uint64   sh_addr;  // 节区装入内存的地址
    uint64   sh_offset;  // 节区相对于文件的位置偏移
    uint64   sh_size;  // 节区内容大小
    uint32   sh_link;  // 指定链接的节索引，与具体的节有关
    uint32   sh_info;  // 指定附加信息
    uint64   sh_addralign;  // 节装入内存的地址对齐要求
    uint64   sh_entsize;  // 指定某些节的固定表大小，与具体的节有关
} elf_section_header;

// Program segment header.
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */
  uint32 flags;  /* Segment flags */
  uint64 off;    /* Segment file offset */
  uint64 vaddr;  /* Segment virtual address */
  uint64 paddr;  /* Segment physical address */
  uint64 filesz; /* Segment size in file */
  uint64 memsz;  /* Segment size in memory */
  uint64 align;  /* Segment alignment */
} elf_prog_header;

// elf section header
typedef struct elf_sect_header_t{
    uint32 name;
    uint32 type;
    uint64 flags;
    uint64 addr;
    uint64 offset;
    uint64 size;
    uint32 link;
    uint32 info;
    uint64 addralign;
    uint64 entsize;
} elf_sect_header;

// compilation units header (in debug line section)
typedef struct __attribute__((packed)) {
    uint32 length;
    uint16 version;
    uint32 header_length;
    uint8 min_instruction_length;
    uint8 default_is_stmt;
    int8 line_base;
    uint8 line_range;
    uint8 opcode_base;
    uint8 std_opcode_lengths[12];
} debug_header;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD 1

typedef enum elf_status_t {
  EL_OK = 0,

  EL_EIO,
  EL_ENOMEM,
  EL_NOTELF,
  EL_ERR,

} elf_status;

typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;
} elf_ctx;

elf_status elf_init(elf_ctx *ctx, void *info);
elf_status elf_load(elf_ctx *ctx);

void load_bincode_from_host_elf(process *p);

#endif
