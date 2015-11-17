typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
typedef struct {
  uint locked;
} lock_t;


typedef struct  {
  uint value;
  uint active;
  lock_t mylock;
}semaphore_t;
