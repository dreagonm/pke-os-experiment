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
#include "pmm.h"
#include "vmm.h"
#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert( current );
  char* pa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)buf);
  sprint(pa);
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

uint64 allocate_exist(size_t n){
  mcb* cur = (mcb*)current->mcb_start;
  while(cur != NULL){
    if(cur->is_used == 0 && cur->size >= n+sizeof(mcb)){
      cur->is_used = 1;
      return cur -> vaddr;
    }
    cur = cur->next;
  }
  return (uint64)NULL;
}

uint64 allocate_new(size_t n){
  g_ufree_page = ((g_ufree_page + 15) >> 4)<<4; // 16位对齐
  if(ROUNDDOWN(g_ufree_page, PGSIZE) != ROUNDDOWN(g_ufree_page + sizeof(mcb) -1, PGSIZE))
    g_ufree_page = ROUNDDOWN(g_ufree_page + sizeof(mcb) -1, PGSIZE);
  uint64 start_vaddr = g_ufree_page;
  uint64 start = ROUNDDOWN(g_ufree_page, PGSIZE), end = ROUNDDOWN(g_ufree_page + n +sizeof(mcb) -1 , PGSIZE);
  g_ufree_page += n+sizeof(mcb);
  while(start <= end){
    pte_t *pte = page_walk(current->pagetable,start,1);
    if(!((*pte)&PTE_V)){
      void* pa = alloc_page();
      user_vm_map((pagetable_t)current->pagetable, start, PGSIZE, (uint64)pa,
         prot_to_type(PROT_WRITE | PROT_READ, 1));
    }
    start += PGSIZE;
  }
  return start_vaddr;
}

uint64 my_better_allocate(size_t n){
  uint64 vaddr = allocate_exist(n);
  if((void *)vaddr != NULL){ // existing block
    // sprint("@");
    mcb *block = (mcb*) user_va_to_pa(current->pagetable, (void *)vaddr);
    // // split block
    block -> size = n+sizeof(mcb);
    uint64 next_addr = block->vaddr+block->size;
    next_addr = ((next_addr + 15) >>4) << 4;
    if(block->next != NULL){
      mcb *nxt_block = block->next;
      if(next_addr + sizeof(mcb) < nxt_block->vaddr){
        mcb *new_mcb = (mcb*) user_va_to_pa(current->pagetable, (void *)next_addr);
        block ->next = new_mcb;
        // sprint("#%x %x %x",next_addr,nxt_block,new_mcb);
        new_mcb ->next = nxt_block;
        new_mcb ->is_used = 0;
        new_mcb ->size = nxt_block->vaddr - next_addr + 1;
        new_mcb ->vaddr = next_addr;
      }
    } 
    return block ->vaddr+sizeof(mcb);
  }
  else{ // new
    // sprint("!");
    vaddr = allocate_new(n);
    mcb *block = (mcb*) user_va_to_pa(current->pagetable, (void *)vaddr);
    block->is_used = 1;
    block->next = NULL;
    block->size = n+sizeof(mcb);
    block->vaddr = vaddr;
    if((mcb*)(current->mcb_start) == NULL){
      // sprint("aa\n");
      current->mcb_start = (uint64)block;
      current->mcb_tail = (uint64)block;
    }
    else{
      // sprint("%x %x %x\n",(current->mcb_tail),block,((mcb*)(current->mcb_tail))->next);
      ((mcb*)(current->mcb_tail))->next = block;
      current->mcb_tail = (uint64)block;
    }
    return vaddr + sizeof(mcb);
  }
}

uint64 my_better_free(uint64 va){
  mcb *now = (mcb *) user_va_to_pa(current->pagetable, (void *)(va - sizeof(mcb)));
  //sprint("now:%p\n",now);
  now->is_used = 0;
  mcb *cur = now;
  uint64 tot = now->size;
  while(cur->next != NULL && cur->next->is_used == 0) {
    sprint("$");
    cur = cur ->next;
    tot += cur->size;
  }
  mcb *start = now;
  start -> is_used = 0;
  start -> next = cur -> next;
  start -> size = tot;
  return 0;
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page(size_t n) {
  // void* pa = alloc_page();
  // uint64 va = g_ufree_page;
  // g_ufree_page += PGSIZE;
  // user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
  //        prot_to_type(PROT_WRITE | PROT_READ, 1));
  uint64 va = my_better_allocate(n);
  return va;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  // user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  my_better_free(va);
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
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page(a1);
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
