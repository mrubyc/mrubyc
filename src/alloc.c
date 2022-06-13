/*! @file
  @brief
  mruby/c memory management.

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Memory management for objects in mruby/c.

  STRATEGY
   Using TLSF and FistFit algorithm.

  MEMORY POOL USAGE (see struct MEMORY_POOL)
     | Memory pool header | Memory blocks to provide to application     |
     +--------------------+---------------------------------------------+
     | size, bitmap, ...  | USED_BLOCK, FREE_BLOCK, ..., (SentinelBlock)|

  MEMORY BLOCK LINK
      with USED flag and PREV_IN_USE flag in size member's bit 0 and 1.

     |  USED_BLOCK     |  FREE_BLOCK                   |  USED_BLOCK     |...
     +-----------------+-------------------------------+-----------------+---
     |size| (contents) |size|*next|*prev| (empty) |*top|size| (contents) |
 USED|   1|            |   0|     |     |         |    |   1|            |
 PREV|   1|            |   1|     |     |         |    |   0|            |

                                           Sentinel block at the link tail.
                                      ...              |  USED_BLOCK     |
                                     ------------------+-----------------+
                                                       |size| (contents) |
                                                   USED|   1|            |
                                                   PREV|   ?|            |
    size : block size.
    *next: linked list, pointer to the next free block of same block size.
    *prev: linked list, pointer to the previous free block of same block size.
    *top : pointer to this block's top.

  </pre>
*/

#if !defined(MRBC_ALLOC_LIBC)

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "vm_config.h"
#include <stdint.h>
#include <types.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

/***** Local headers ********************************************************/
#include "alloc.h"
#include "hal_selector.h"

/***** Constant values ******************************************************/
/*
  Layer 1st(f) and 2nd(s) model
  last 4bit is ignored

 FLI range      SLI0  1     2     3     4     5     6     7         BlockSize
  0  0000-007f unused 0010- 0020- 0030- 0040- 0050- 0060- 0070-007f   16
  1  0080-00ff  0080- 0090- 00a0- 00b0- 00c0- 00d0- 00e0- 00f0-00ff   16
  2  0100-01ff  0100- 0120- 0140- 0160- 0180- 01a0- 01c0- 01e0-01ff   32
  3  0200-03ff  0200- 0240- 0280- 02c0- 0300- 0340- 0380- 03c0-03ff   64
  4  0400-07ff  0400- 0480- 0500- 0580- 0600- 0680- 0700- 0780-07ff  128
  5  0800-0fff  0800- 0900- 0a00- 0b00- 0c00- 0d00- 0e00- 0f00-0fff  256
  6  1000-1fff  1000- 1200- 1400- 1600- 1800- 1a00- 1c00- 1e00-1fff  512
  7  2000-3fff  2000- 2400- 2800- 2c00- 3000- 3400- 3800- 3c00-3fff 1024
  8  4000-7fff  4000- 4800- 5000- 5800- 6000- 6800- 7000- 7800-7fff 2048
  9  8000-ffff  8000- 9000- a000- b000- c000- d000- e000- f000-ffff 4096
*/

#ifndef MRBC_ALLOC_FLI_BIT_WIDTH	// 0000 0000 0000 0000
# define MRBC_ALLOC_FLI_BIT_WIDTH 9	// ~~~~~~~~~~~
#endif
#ifndef MRBC_ALLOC_SLI_BIT_WIDTH	// 0000 0000 0000 0000
# define MRBC_ALLOC_SLI_BIT_WIDTH 3	//            ~~~
#endif
#ifndef MRBC_ALLOC_IGNORE_LSBS		// 0000 0000 0000 0000
# define MRBC_ALLOC_IGNORE_LSBS	  4	//                ~~~~
#endif

#define SIZE_FREE_BLOCKS \
  ((MRBC_ALLOC_FLI_BIT_WIDTH + 1) * (1 << MRBC_ALLOC_SLI_BIT_WIDTH))
					// maybe 80 (0x50)
/*
   Minimum memory block size parameter.
   Choose large one from sizeof(FREE_BLOCK) or (1 << MRBC_ALLOC_IGNORE_LSBS)
*/
#if !defined(MRBC_MIN_MEMORY_BLOCK_SIZE)
// #define MRBC_MIN_MEMORY_BLOCK_SIZE sizeof(FREE_BLOCK)
#define MRBC_MIN_MEMORY_BLOCK_SIZE (1 << MRBC_ALLOC_IGNORE_LSBS)
#endif


/***** Macros ***************************************************************/
#define FLI(x) ((x) >> MRBC_ALLOC_SLI_BIT_WIDTH)
#define SLI(x) ((x) & ((1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1))


/***** Typedefs *************************************************************/
/*
  define memory block header for 16 bit

  (note)
  Typical size of
    USED_BLOCK is 2 bytes
    FREE_BLOCK is 8 bytes
  on 16bit machine.
*/
#if defined(MRBC_ALLOC_16BIT)
#define MRBC_ALLOC_MEMSIZE_T	uint16_t

typedef struct USED_BLOCK {
  MRBC_ALLOC_MEMSIZE_T size;		//!< block size, header included
#if defined(MRBC_ALLOC_VMID)
  uint8_t	       vm_id;		//!< mruby/c VM ID
#endif
} USED_BLOCK;

typedef struct FREE_BLOCK {
  MRBC_ALLOC_MEMSIZE_T size;		//!< block size, header included
#if defined(MRBC_ALLOC_VMID)
  uint8_t	       vm_id;		//!< dummy
#endif

  struct FREE_BLOCK *next_free;
  struct FREE_BLOCK *prev_free;
  struct FREE_BLOCK *top_adrs;		//!< dummy for calculate sizeof(FREE_BLOCK)
} FREE_BLOCK;


/*
  define memory block header for 24/32 bit.

  (note)
  Typical size of
    USED_BLOCK is  4 bytes
    FREE_BLOCK is 16 bytes
  on 32bit machine.
*/
#elif defined(MRBC_ALLOC_24BIT)
#define MRBC_ALLOC_MEMSIZE_T	uint32_t

typedef struct USED_BLOCK {
#if defined(MRBC_ALLOC_VMID)
  MRBC_ALLOC_MEMSIZE_T size : 24;	//!< block size, header included
  uint8_t	       vm_id : 8;	//!< mruby/c VM ID
#else
  MRBC_ALLOC_MEMSIZE_T size;
#endif
} USED_BLOCK;

typedef struct FREE_BLOCK {
#if defined(MRBC_ALLOC_VMID)
  MRBC_ALLOC_MEMSIZE_T size : 24;	//!< block size, header included
  uint8_t	       vm_id : 8;	//!< dummy
#else
  MRBC_ALLOC_MEMSIZE_T size;
#endif

  struct FREE_BLOCK *next_free;
  struct FREE_BLOCK *prev_free;
  struct FREE_BLOCK *top_adrs;		//!< dummy for calculate sizeof(FREE_BLOCK)
} FREE_BLOCK;

#else
# error 'define MRBC_ALLOC_*' required.
#endif

/*
  and operation macro
*/
#define BLOCK_SIZE(p)		(((p)->size) & ~0x03)
#define PHYS_NEXT(p)		((void *)((uint8_t *)(p) + BLOCK_SIZE(p)))
#define SET_USED_BLOCK(p)	((p)->size |=  0x01)
#define SET_FREE_BLOCK(p)	((p)->size &= ~0x01)
#define IS_USED_BLOCK(p)	((p)->size &   0x01)
#define IS_FREE_BLOCK(p)	(!IS_USED_BLOCK(p))
#define SET_PREV_USED(p)	((p)->size |=  0x02)
#define SET_PREV_FREE(p)	((p)->size &= ~0x02)
#define IS_PREV_USED(p)		((p)->size &   0x02)
#define IS_PREV_FREE(p)		(!IS_PREV_USED(p))

#if defined(MRBC_ALLOC_VMID)
#define SET_VM_ID(p,id)	(((USED_BLOCK *)(p))->vm_id = (id))
#define GET_VM_ID(p)	(((USED_BLOCK *)(p))->vm_id)

#else
#define SET_VM_ID(p,id)	((void)0)
#define GET_VM_ID(p)	0
#endif


/*
  define memory pool header
*/
typedef struct MEMORY_POOL {
  MRBC_ALLOC_MEMSIZE_T size;

  // free memory bitmap
  uint16_t free_fli_bitmap;
  uint8_t  free_sli_bitmap[MRBC_ALLOC_FLI_BIT_WIDTH +1+1];
						// +1=bit_width, +1=sentinel
  uint8_t  pad[3]; // for alignment compatibility on 16bit and 32bit machines

 //  uint8_t  pad[5]; // hmm, for some reason, size is 342, so let's add 2...

  // free memory block index
  FREE_BLOCK *free_blocks[SIZE_FREE_BLOCKS +1];	// +1=sentinel
} MEMORY_POOL;

#define BLOCK_TOP(p) ((void *)((uint8_t *)(p) + sizeof(MEMORY_POOL)))
#define BLOCK_END(p) ((void *)((uint8_t *)(p) + ((MEMORY_POOL *)(p))->size))

#define MSB_BIT1_FLI 0x8000
#define MSB_BIT1_SLI 0x80
#define NLZ_FLI(x) nlz16(x)
#define NLZ_SLI(x) nlz8(x)


/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
// memory pool
// static MEMORY_POOL *memory_pool;


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
//================================================================
/*! Number of leading zeros. 16bit version.

  @param  x	target (16bit unsigned)
  @retval int	nlz value
*/
static inline int nlz16(uint16_t x)
{
  if( x == 0 ) return 16;

  int n = 1;
  if((x >>  8) == 0 ) { n += 8; x <<= 8; }
  if((x >> 12) == 0 ) { n += 4; x <<= 4; }
  if((x >> 14) == 0 ) { n += 2; x <<= 2; }
  return n - (x >> 15);
}


//================================================================
/*! Number of leading zeros. 8bit version.

  @param  x	target (8bit unsigned)
  @retval int	nlz value
*/
static inline int nlz8(uint8_t x)
{
  if( x == 0 ) return 8;

  int n = 1;
  if((x >> 4) == 0 ) { n += 4; x <<= 4; }
  if((x >> 6) == 0 ) { n += 2; x <<= 2; }
  return n - (x >> 7);
}


//================================================================
/*! calc f and s, and returns fli,sli of free_blocks

  @param  alloc_size	alloc size
  @retval unsigned int	index of free_blocks
*/
/*
static inline unsigned int calc_index(MRBC_ALLOC_MEMSIZE_T alloc_size)
{
  // check overflow
  if( (alloc_size >> (MRBC_ALLOC_FLI_BIT_WIDTH
                      + MRBC_ALLOC_SLI_BIT_WIDTH
                      + MRBC_ALLOC_IGNORE_LSBS)) != 0) {
    return SIZE_FREE_BLOCKS - 1;
  }

  // calculate First Level Index.
  unsigned int fli = 16 -
    nlz16( alloc_size >> (MRBC_ALLOC_SLI_BIT_WIDTH + MRBC_ALLOC_IGNORE_LSBS) );

  // calculate Second Level Index.
  unsigned int shift = (fli == 0) ? MRBC_ALLOC_IGNORE_LSBS :
                                   (MRBC_ALLOC_IGNORE_LSBS - 1 + fli);

  unsigned int sli = (alloc_size >> shift) & ((1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1);
  unsigned int index = (fli << MRBC_ALLOC_SLI_BIT_WIDTH) + sli;

  assert(fli <= MRBC_ALLOC_FLI_BIT_WIDTH);
  assert(sli <= (1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1);

  return index;
}
*/


//================================================================
/*! Mark that block free and register it in the free index table.

  @param  pool		Pointer to memory pool.
  @param  target	Pointer to target block.
*/
/*
static void add_free_block(MEMORY_POOL *pool, FREE_BLOCK *target)
{
  SET_FREE_BLOCK(target);

  FREE_BLOCK **top_adrs = (FREE_BLOCK **)((uint8_t*)target + BLOCK_SIZE(target) - sizeof(FREE_BLOCK *));
  *top_adrs = target;

  unsigned int index = calc_index(BLOCK_SIZE(target));
  unsigned int fli = FLI(index);
  unsigned int sli = SLI(index);
  assert( index < SIZE_FREE_BLOCKS );

  pool->free_fli_bitmap      |= (MSB_BIT1_FLI >> fli);
  pool->free_sli_bitmap[fli] |= (MSB_BIT1_SLI >> sli);

  target->prev_free = NULL;
  target->next_free = pool->free_blocks[index];
  if( target->next_free != NULL ) {
    target->next_free->prev_free = target;
  }
  pool->free_blocks[index] = target;

#if defined(MRBC_DEBUG)
  SET_VM_ID( target, 0xff );
  memset( (uint8_t *)target + sizeof(FREE_BLOCK) - sizeof(FREE_BLOCK *), 0xff,
          BLOCK_SIZE(target) - sizeof(FREE_BLOCK) );
#endif
}
*/


//================================================================
/*! just remove the free_block *target from index

  @param  pool		Pointer to memory pool.
  @param  target	pointer to target block.
*/
/*
static void remove_free_block(MEMORY_POOL *pool, FREE_BLOCK *target)
{
  // top of linked list?
  if( target->prev_free == NULL ) {
    unsigned int index = calc_index(BLOCK_SIZE(target));

    pool->free_blocks[index] = target->next_free;
    if( target->next_free == NULL ) {
      unsigned int fli = FLI(index);
      unsigned int sli = SLI(index);
      pool->free_sli_bitmap[fli] &= ~(MSB_BIT1_SLI >> sli);
      if( pool->free_sli_bitmap[fli] == 0 ) pool->free_fli_bitmap &= ~(MSB_BIT1_FLI >> fli);
    }
  }
  else {
    target->prev_free->next_free = target->next_free;
  }

  if( target->next_free != NULL ) {
    target->next_free->prev_free = target->prev_free;
  }
}
*/


//================================================================
/*! Split block by size

  @param  target	pointer to target block
  @param  size		size
  @retval NULL		no split.
  @retval FREE_BLOCK *	pointer to splitted free block.
*/
/*
static inline FREE_BLOCK* split_block(FREE_BLOCK *target, MRBC_ALLOC_MEMSIZE_T size)
{
  assert( BLOCK_SIZE(target) >= size );
  if( (BLOCK_SIZE(target) - size) <= MRBC_MIN_MEMORY_BLOCK_SIZE ) return NULL;

  // split block, free
  FREE_BLOCK *split = (FREE_BLOCK *)((uint8_t *)target + size);

  split->size  = BLOCK_SIZE(target) - size;
  target->size = size | (target->size & 0x03);	// copy a size with flags.

  return split;
}
*/


//================================================================
/*! merge target and next block.
    next will disappear

  @param  target	pointer to free block 1
  @param  next	pointer to free block 2
*/
/*
static inline void merge_block(FREE_BLOCK *target, FREE_BLOCK *next)
{
  assert(target < next);

  // merge target and next
  target->size += BLOCK_SIZE(next);		// copy a size but save flags.
}
*/


/***** Global functions *****************************************************/
//================================================================
/*! initialize

  @param  ptr	pointer to free memory block.
  @param  size	size. (max 64KB. see MRBC_ALLOC_MEMSIZE_T)
*/
/*
void mrbc_init_alloc(void *ptr, unsigned int size)
{
  char buf[64];
  VDP_drawText("in init alloc0", 1, 1);
  assert( MRBC_MIN_MEMORY_BLOCK_SIZE >= sizeof(FREE_BLOCK) );
  assert( MRBC_MIN_MEMORY_BLOCK_SIZE >= (1 << MRBC_ALLOC_IGNORE_LSBS) );
  sprintf(buf, "size mem is %d", (sizeof(MEMORY_POOL)));
  VDP_drawText(buf, 1, 2);
  assert( (sizeof(MEMORY_POOL) & 0x03) == 0 );
  VDP_drawText("in init alloc3", 1, 4);
  assert( size != 0 );
  VDP_drawText("in init alloc4", 1, 5);
  assert( size <= (MRBC_ALLOC_MEMSIZE_T)(~0) );
  VDP_drawText("after asserts.", 1, 6);

  if( memory_pool != NULL ) return;
  VDP_drawText("after null check.", 1, 7);
  size &= ~(unsigned int)0x03;	// align 4 byte.
  memory_pool = ptr;
  memset( memory_pool, 0, sizeof(MEMORY_POOL) );
  sprintf(buf, "MRBC_ALLOC_MEMSIZE_T is: %d", sizeof(MRBC_ALLOC_MEMSIZE_T));
  VDP_drawText(buf, 1, 8);
  memory_pool->size = size; // this causes an issue??

  // initialize memory pool
  //  large free block + zero size used block (sentinel).
  MRBC_ALLOC_MEMSIZE_T sentinel_size = sizeof(USED_BLOCK);
  sentinel_size += (-sentinel_size & 0x03);
  MRBC_ALLOC_MEMSIZE_T free_size = size - sizeof(MEMORY_POOL) - sentinel_size;
  FREE_BLOCK *free_block = BLOCK_TOP(memory_pool);
  USED_BLOCK *used_block = (USED_BLOCK *)((uint8_t *)free_block + free_size);

  free_block->size = free_size | 0x02;		// flag prev=1, used=0
  used_block->size = sentinel_size | 0x01;	// flag prev=0, used=1
  SET_VM_ID( used_block, 0xff );

  add_free_block( memory_pool, free_block );
}
*/


//================================================================
/*! cleanup memory pool
*/
/*
void mrbc_cleanup_alloc(void)
{
#if defined(MRBC_DEBUG)
  if( memory_pool ) {
    memset( memory_pool, 0, memory_pool->size );
  }
#endif

  memory_pool = 0;
}
*/


//================================================================
/*! allocate memory

  @param  size	request size.
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/
void * mrbc_raw_alloc(unsigned int size)
{
  // can I just use platform's malloc?
  MRBC_ALLOC_MEMSIZE_T alloc_size = size + (-size & 3);	// align 4 byte
  return malloc(alloc_size);
/*
  MEMORY_POOL *pool = memory_pool;
  MRBC_ALLOC_MEMSIZE_T alloc_size = size + sizeof(USED_BLOCK);

  // align 4 byte
  alloc_size += (-alloc_size & 3);

  // check minimum alloc size.
  if( alloc_size < MRBC_MIN_MEMORY_BLOCK_SIZE ) alloc_size = MRBC_MIN_MEMORY_BLOCK_SIZE;

  FREE_BLOCK *target;
  unsigned int fli, sli;
  unsigned int index = calc_index(alloc_size);

  // At first, check only the beginning of the same size block.
  // because it immediately responds to the pattern in which
  // same size memory are allocated and released continuously.
  target = pool->free_blocks[index];
  if( target && BLOCK_SIZE(target) >= alloc_size ) {
    fli = FLI(index);
    sli = SLI(index);
    goto FOUND_TARGET_BLOCK;
  }

  // and then, check the next (larger) size block.
  target = pool->free_blocks[++index];
  fli = FLI(index);
  sli = SLI(index);
  if( target ) goto FOUND_TARGET_BLOCK;

  // check in SLI bitmap table.
  uint16_t masked = pool->free_sli_bitmap[fli] & ((MSB_BIT1_SLI >> sli) - 1);
  if( masked != 0 ) {
    sli = NLZ_SLI( masked );
    goto FOUND_FLI_SLI;
  }

  // check in FLI bitmap table.
  masked = pool->free_fli_bitmap & ((MSB_BIT1_FLI >> fli) - 1);
  if( masked != 0 ) {
    fli = NLZ_FLI( masked );
    sli = NLZ_SLI( pool->free_sli_bitmap[fli] );
    goto FOUND_FLI_SLI;
  }

  // Change strategy to First-fit.
  target = pool->free_blocks[--index];
  while( target ) {
    if( BLOCK_SIZE(target) >= alloc_size ) {
      remove_free_block( pool, target );
      goto SPLIT_BLOCK;
    }
    target = target->next_free;
  }

  // else out of memory
  static const char msg[] = "Fatal error: Out of memory.\n";
  hal_write(1, msg, sizeof(msg)-1);
#if defined(MRBC_OUT_OF_MEMORY)
  MRBC_OUT_OF_MEMORY();
#endif
  return NULL;  // ENOMEM


 FOUND_FLI_SLI:
  index = (fli << MRBC_ALLOC_SLI_BIT_WIDTH) + sli;
  assert( index < SIZE_FREE_BLOCKS );
  target = pool->free_blocks[index];
  assert( target != NULL );

 FOUND_TARGET_BLOCK:
  assert(BLOCK_SIZE(target) >= alloc_size);

  // remove free_blocks index
  pool->free_blocks[index] = target->next_free;
  if( target->next_free == NULL ) {
    pool->free_sli_bitmap[fli] &= ~(MSB_BIT1_SLI >> sli);
    if( pool->free_sli_bitmap[fli] == 0 ) pool->free_fli_bitmap &= ~(MSB_BIT1_FLI >> fli);
  }
  else {
    target->next_free->prev_free = NULL;
  }

 SPLIT_BLOCK: {
    FREE_BLOCK *release = split_block(target, alloc_size);
    if( release != NULL ) {
      SET_PREV_USED(release);
      add_free_block( pool, release );
    } else {
      FREE_BLOCK *next = PHYS_NEXT(target);
      SET_PREV_USED(next);
    }
  }

  SET_USED_BLOCK(target);
  SET_VM_ID( target, 0 );

#if defined(MRBC_DEBUG)
  memset( (uint8_t *)target + sizeof(USED_BLOCK), 0xaa,
          BLOCK_SIZE(target) - sizeof(USED_BLOCK) );
#endif

  return (uint8_t *)target + sizeof(USED_BLOCK);
*/
}


//================================================================
/*! allocate memory that cannot free and realloc

  @param  size	request size.
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/

void * mrbc_raw_alloc_no_free(unsigned int size)
{
  // let's just try the regular malloc and see what happens...
  MRBC_ALLOC_MEMSIZE_T alloc_size = size + (-size & 3);	// align 4 byte
  return mrbc_raw_alloc(alloc_size);
  /*
  MEMORY_POOL *pool = memory_pool;
  MRBC_ALLOC_MEMSIZE_T alloc_size = size + (-size & 3);	// align 4 byte

  // find the tail block
  FREE_BLOCK *tail = BLOCK_TOP(pool);
  FREE_BLOCK *prev;
  do {
    prev = tail;
    tail = PHYS_NEXT(tail);
  } while( PHYS_NEXT(tail) < BLOCK_END(pool) );

  // can resize it block?
  if( IS_USED_BLOCK(prev) ) goto FALLBACK;
  if( (BLOCK_SIZE(prev) - sizeof(USED_BLOCK)) < alloc_size ) goto FALLBACK;

  remove_free_block( pool, prev );
  MRBC_ALLOC_MEMSIZE_T free_size = BLOCK_SIZE(prev) - alloc_size;

  if( free_size <= MRBC_MIN_MEMORY_BLOCK_SIZE ) {
    // no split, use all
    prev->size += BLOCK_SIZE(tail);
    SET_USED_BLOCK( prev );
    tail = prev;
  }
  else {
    // split block
    MRBC_ALLOC_MEMSIZE_T tail_size = tail->size + alloc_size;	// w/ flags.
    tail = (FREE_BLOCK*)((uint8_t *)tail - alloc_size);
    tail->size = tail_size;
    prev->size -= alloc_size;		// w/ flags.
    add_free_block( pool, prev );
  }
  SET_VM_ID( tail, 0xff );

  return (uint8_t *)tail + sizeof(USED_BLOCK);

 FALLBACK:
  return mrbc_raw_alloc(alloc_size);
  */
}



//================================================================
/*! release memory

  @param  ptr	Return value of mrbc_raw_alloc()
*/
void mrbc_raw_free(void *ptr)
{
  free(ptr);
  /*
  MEMORY_POOL *pool = memory_pool;

  // get target block
  FREE_BLOCK *target = (FREE_BLOCK *)((uint8_t *)ptr - sizeof(USED_BLOCK));

  // check next block, merge?
  FREE_BLOCK *next = PHYS_NEXT(target);

  if( IS_FREE_BLOCK(next) ) {
    remove_free_block( pool, next );
    merge_block(target, next);
  } else {
    SET_PREV_FREE(next);
  }

  // check prev block, merge?
  if( IS_PREV_FREE(target) ) {
    FREE_BLOCK *prev = *((FREE_BLOCK **)((uint8_t*)target - sizeof(FREE_BLOCK *)));

    assert( IS_FREE_BLOCK(prev) );
    remove_free_block( pool, prev );
    merge_block(prev, target);
    target = prev;
  }

  // target, add to index
  add_free_block( pool, target );
  */
}


//================================================================
/*! re-allocate memory

  @param  ptr	Return value of mrbc_raw_alloc()
  @param  size	request size
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/
void * mrbc_raw_realloc(void *ptr, unsigned int size)
{
  // platform realloc causes address error? Let's try a naive implementation for now...
  MRBC_ALLOC_MEMSIZE_T alloc_size = size + (-size & 3);	// align 4 byte
  void *newptr =  malloc(alloc_size);
  if( newptr == NULL ) return NULL;  // ENOMEM
  memcpy(newptr, ptr, size);
  char buf[48];
  free(ptr);
  return newptr;
  //return ptr;
  //return realloc(ptr, alloc_size);
  /*
  MEMORY_POOL *pool = memory_pool;
  USED_BLOCK *target = (USED_BLOCK *)((uint8_t *)ptr - sizeof(USED_BLOCK));
  MRBC_ALLOC_MEMSIZE_T alloc_size = size + sizeof(USED_BLOCK);
  FREE_BLOCK *next;

  // align 4 byte
  alloc_size += (-alloc_size & 3);

  // check minimum alloc size.
  if( alloc_size < MRBC_MIN_MEMORY_BLOCK_SIZE ) alloc_size = MRBC_MIN_MEMORY_BLOCK_SIZE;

  // expand? part1.
  // next phys block is free and enough size?
  if( alloc_size > BLOCK_SIZE(target) ) {
    next = PHYS_NEXT(target);
    if( IS_USED_BLOCK(next) ) goto ALLOC_AND_COPY;
    if( (BLOCK_SIZE(target) + BLOCK_SIZE(next)) < alloc_size ) goto ALLOC_AND_COPY;

    remove_free_block( pool, next );
    merge_block((FREE_BLOCK *)target, next);
  }
  next = PHYS_NEXT(target);

  // try shrink.
  FREE_BLOCK *release = split_block((FREE_BLOCK *)target, alloc_size);
  if( release != NULL ) {
    SET_PREV_USED(release);
  } else {
    SET_PREV_USED(next);
    return ptr;
  }

  // check next block, merge?
  if( IS_FREE_BLOCK(next) ) {
    remove_free_block( pool, next );
    merge_block(release, next);
  } else {
    SET_PREV_FREE(next);
  }
  add_free_block( pool, release );
  return ptr;


  // expand part2.
  // new alloc and copy
 ALLOC_AND_COPY: {
    void *new_ptr = mrbc_raw_alloc(size);
    if( new_ptr == NULL ) return NULL;  // ENOMEM

    memcpy(new_ptr, ptr, BLOCK_SIZE(target) - sizeof(USED_BLOCK));
    mrbc_set_vm_id(new_ptr, target->vm_id);

    mrbc_raw_free(ptr);

    return new_ptr;
  }
  */
}


#if defined(MRBC_ALLOC_VMID)
//================================================================
/*! allocate memory

  @param  vm	pointer to VM.
  @param  size	request size.
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/
void * mrbc_alloc(const struct VM *vm, unsigned int size)
{
  void *ptr = mrbc_raw_alloc(size);
  if( ptr == NULL ) return NULL;	// ENOMEM

  if( vm ) mrbc_set_vm_id(ptr, vm->vm_id);

  return ptr;
}


//================================================================
/*! release memory, vm used.

  @param  vm	pointer to VM.
*/
/*
void mrbc_free_all(const struct VM *vm)
{
  MEMORY_POOL *pool = memory_pool;
  USED_BLOCK *target = BLOCK_TOP(pool);
  USED_BLOCK *next;
  int vm_id = vm->vm_id;

  while( target < (USED_BLOCK *)BLOCK_END(pool) ) {
    next = PHYS_NEXT(target);
    if( IS_FREE_BLOCK(next) ) next = PHYS_NEXT(next);

    if( IS_USED_BLOCK(target) && (target->vm_id == vm_id) ) {
      mrbc_raw_free( (uint8_t *)target + sizeof(USED_BLOCK) );
    }
    target = next;
  }
}
*/


//================================================================
/*! set vm id

  @param  ptr	Return value of mrbc_alloc()
  @param  vm_id	vm id
*/
void mrbc_set_vm_id(void *ptr, int vm_id)
{
  SET_VM_ID( (uint8_t *)ptr - sizeof(USED_BLOCK), vm_id );
}


//================================================================
/*! get vm id

  @param  ptr	Return value of mrbc_alloc()
  @return int	vm id
*/
int mrbc_get_vm_id(void *ptr)
{
  return GET_VM_ID( (uint8_t *)ptr - sizeof(USED_BLOCK) );
}
#endif	// defined(MRBC_ALLOC_VMID)


//================================================================
/*! statistics

  @param  ret		pointer to return value.
*/

/*
void mrbc_alloc_statistics( struct MRBC_ALLOC_STATISTICS *ret )
{
  MEMORY_POOL *pool = memory_pool;
  USED_BLOCK *block = BLOCK_TOP(pool);
  int flag_used_free = IS_USED_BLOCK(block);

  ret->total = pool->size;
  ret->used = 0;
  ret->free = 0;
  ret->fragmentation = -1;

  while( block < (USED_BLOCK *)BLOCK_END(pool) ) {
    if( IS_FREE_BLOCK(block) ) {
      ret->free += BLOCK_SIZE(block);
    } else {
      ret->used += BLOCK_SIZE(block);
    }
    if( flag_used_free != IS_USED_BLOCK(block) ) {
      ret->fragmentation++;
      flag_used_free = IS_USED_BLOCK(block);
    }
    block = PHYS_NEXT(block);
  }
}
*/



#if defined(MRBC_DEBUG)
#include "console.h"
//================================================================
/*! print memory block for debug.

*/
/*
void mrbc_alloc_print_memory_pool( void )
{
  int i;
  MEMORY_POOL *pool = memory_pool;
  const int DUMP_BYTES = 32;

  mrbc_printf("== MEMORY POOL HEADER DUMP ==\n");
  mrbc_printf(" Address: %p - %p - %p  ", pool,
		BLOCK_TOP(pool), BLOCK_END(pool));
  mrbc_printf(" Size Total: %d User: %d\n",
		pool->size, pool->size - sizeof(MEMORY_POOL));
  mrbc_printf(" sizeof MEMORY_POOL: %d(%04x), USED_BLOCK: %d(%02x), FREE_BLOCK: %d(%02x)\n",
		sizeof(MEMORY_POOL), sizeof(MEMORY_POOL),
		sizeof(USED_BLOCK), sizeof(USED_BLOCK),
		sizeof(FREE_BLOCK), sizeof(FREE_BLOCK) );

  mrbc_printf(" FLI/SLI bitmap and free_blocks table.\n");
  mrbc_printf("    FLI :S[0123 4567] -- free_blocks ");
  for( i = 0; i < 64; i++ ) { mrbc_printf("-"); }
  mrbc_printf("\n");
  for( i = 0; i < sizeof(pool->free_sli_bitmap); i++ ) {
    mrbc_printf(" [%2d] %d :  ", i, !!((pool->free_fli_bitmap << i) & MSB_BIT1_FLI));
    int j;
    for( j = 0; j < 8; j++ ) {
      mrbc_printf("%d", !!((pool->free_sli_bitmap[i] << j) & MSB_BIT1_SLI));
      if( (j % 4) == 3 ) mrbc_printf(" ");
    }

    for( j = 0; j < 8; j++ ) {
      int idx = i * 8 + j;
      if( idx >= sizeof(pool->free_blocks) / sizeof(FREE_BLOCK *) ) break;
      mrbc_printf(" %p", pool->free_blocks[idx] );
    }
    mrbc_printf( "\n" );
  }

  mrbc_printf("== MEMORY BLOCK DUMP ==\n");
  FREE_BLOCK *block = BLOCK_TOP(pool);

  while( block < (FREE_BLOCK *)BLOCK_END(pool) ) {
    mrbc_printf("%p", block );
#if defined(MRBC_ALLOC_VMID)
    mrbc_printf(" id:%02x", block->vm_id );
#endif
    mrbc_printf(" size:%5d(%04x) use:%d prv:%d ",
		block->size & ~0x03, block->size & ~0x03,
		!!(block->size & 0x01), !!(block->size & 0x02) );

    if( IS_USED_BLOCK(block) ) {
      int n = DUMP_BYTES;
      if( n > (BLOCK_SIZE(block) - sizeof(USED_BLOCK)) ) {
	n = BLOCK_SIZE(block) - sizeof(USED_BLOCK);
      }
      uint8_t *p = (uint8_t *)block + sizeof(USED_BLOCK);
      for( i = 0; i < n; i++) mrbc_printf(" %02x", *p++ );
      for( ; i < DUMP_BYTES; i++ ) mrbc_printf("   ");

      mrbc_printf("  ");
      p = (uint8_t *)block + sizeof(USED_BLOCK);
      for( i = 0; i < n; i++) {
	int ch = *p++;
	mrbc_printf("%c", (' ' <= ch && ch < 0x7f)? ch : '.');
      }
    }

    if( IS_FREE_BLOCK(block) ) {
      unsigned int index = calc_index(BLOCK_SIZE(block));
      mrbc_printf(" fli:%d sli:%d pf:%p nf:%p",
		FLI(index), SLI(index), block->prev_free, block->next_free);
    }

    mrbc_printf("\n");
    block = PHYS_NEXT(block);
  }
}
*/

#endif // defined(MRBC_DEBUG)
#endif // !defined(MRBC_ALLOC_LIBC)
