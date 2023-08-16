// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

//改为多个空闲链表的数组
struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void kinit(){
  for (int i = 0; i<NCPU; i++){
    initlock(&kmem[i].lock, "kmem");
  }
    
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa){
  struct run *r;
  push_off();
  int c = cpuid();
  pop_off();

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[c].lock);
  r->next = kmem[c].freelist;
  kmem[c].freelist = r;
  release(&kmem[c].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void){
  struct run *r;
  push_off();
  /*利用cpuid() 读取特殊寄存器以获得当前运行的 CPU 的编号，该编号对应着一个空闲链表，在获
  取该编号时需要关闭中断*/
  int c = cpuid();
  pop_off();
  /*成功获取 CPU 编号后，获取对应的空闲链表的锁，然后试图分配页面*/
  acquire(&kmem[c].lock);
  r = kmem[c].freelist;
  if(r){
    kmem[c].freelist = r->next;
    release(&kmem[c].lock);
  } 
  /*若该 CPU 的空闲链表中没有空闲页面，则到其它 CPU 中借用一个*/
  else {
    release(&kmem[c].lock);
    for (int i = 0; i<NCPU; i++){
      acquire(&kmem[i].lock);
      r = kmem[i].freelist;
      if(r){
        kmem[i].freelist = r->next;
        release(&kmem[i].lock);
        break;
      } 
      else {
        release(&kmem[i].lock);
      }
    }
  }
  /*最后若分配成功，则返回获取的页面*/
  if(r)
    memset((char*)r, 5, PGSIZE);
  return (void*)r;
}
