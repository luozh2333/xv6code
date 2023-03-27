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
  struct run *next;// this is a pointer to the next run
};

struct {
  struct spinlock lock;// this is a lock which can be used to lock the memory
  struct run *freelist;
} kmem;

void
kinit()
{
    // initializes the free list to hold every page between the end of the kernel and PHYSTOP
  // xv6 assumes that the machine has 128MB of RAM
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);//给end到PHYSTOP的空间分配以pagesize为单位大小的空间
}

void
freerange(void *pa_start, void *pa_end)//从pa_start到pa_end的空间分配以pagesize为单位大小的空间
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
void
kfree(void *pa)
{
  //给pa分配pagesize的空间，并挂载到kmem.freelist上
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)// checks if the address is a multiple of PGSIZE, 
  //if it is less than the end of the kernel, and if it is greater than the end of the physical memory
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  
  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;// adds the run to the free list head 
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  //从kmem.freelist上分配一个page
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;//allocate from the free list head which is the most recently freed page ,
  //when it is allocated it is removed from the free list
  //and it is allocated from big to small address
}
