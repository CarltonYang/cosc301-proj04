#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

#include "memlayout.h"

#include <stddef.h>
/*
 * This is where you'll need to implement the user-level functions
 */

void
getcallerpcs(void *v, uint pcs[])
{
  uint *ebp;
  int i;
  
  ebp = (uint*)v - 2;
  for(i = 0; i < 10; i++){
    if(ebp == 0 || ebp < (uint*)KERNBASE || ebp == (uint*)0xffffffff)
      break;
    pcs[i] = ebp[1];     // saved %eip
    ebp = (uint*)ebp[0]; // saved %ebp
  }
  for(; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int
holding(lock_t *lock)
{
  return lock->locked && lock->cpu == cpu;
}

void
pushcli(void)
{
  int eflags;
  
  eflags = readeflags();
  cli();
  if(cpu->ncli++ == 0)
    cpu->intena = eflags & FL_IF;
}

void
popcli(void)
{
  if(readeflags()&FL_IF)
    //panic("popcli - interruptible");
  if(--cpu->ncli < 0)
    //panic("popcli");
  if(cpu->ncli == 0 && cpu->intena)
    sti();
}


void lock_init(lock_t *lock) {
  lock->locked = 0;
  lock->cpu = 0;
}

void lock_acquire(lock_t *lock) {
  pushcli(); // disable interrupts to avoid deadlock.
  if(holding(lock))
    //panic("acquire");
  while(xchg(&lock->locked, 1) != 0)
    ;

  // Record info about lock acquisition for debugging.
  lock->cpu = cpu;
  getcallerpcs(&lock, lock->pcs);

}

void lock_release(lock_t *lock) {
  if(!holding(lock))
    //panic("release");

  lock->pcs[0] = 0;
  lock->cpu = 0;
  xchg(&lock->locked, 0);

  popcli();
}

typedef struct {
    int pid;
    void* stack;
} tTuple;

tTuple List[1024];
int i =0;

int thread_join(int pid) {
	
	void* stack;
	int j;
        for (j=0;j<1024;j++){
	  if (List[j].pid==pid)
		stack=List[j].stack;
        }
        int tid =join(pid);
	free(stack);
	return tid;
}

int thread_create(void (*start_routine)(void *), void *arg) {
  // allocate 1-page for new thread's stack.
  // how to make this page-aligned if the original heap was not page-aligned? 
  //void* stack = malloc (sizeof(char)*PGSIZE);
  void *stack = malloc(PGSIZE*2);
  //assert(stack != NULL);
  if((uint)stack % PGSIZE)
    stack = stack + (PGSIZE - (uint)stack % PGSIZE);

  if (stack == NULL ) {
    printf (1, "malloc failure \n");
    return -1;
  }
  int tid = 0;
  tid = clone (start_routine, arg, stack);
  if (tid ==-1) {
    printf(1, "clone failure \n");
    return -1;
  }
  if (tid !=-1) { //child thread
    // call function passed to thread_create
    //fn (arg);
    (start_routine)(arg); 
    // fn has finished, deallocate stack
    //free(stack);
    //printf(1, "done freeing stack \n");
    //exit ();
  }
  // return tid to parent. 
  List[i].pid=tid;
  List[i].stack=stack;
  i++;
  return tid;
}



