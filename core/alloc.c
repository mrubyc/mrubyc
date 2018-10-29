/*! @file
  @brief
  mrubyc memory management.

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Memory management for objects in mruby/c.

  </pre>
*/

#include "vm_config.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "vm.h"
#include "alloc.h"
#include "console.h"


// Layer 1st(f) and 2nd(s) model
// last 4bit is ignored
// f : size
// 0 : 0000-007f
// 1 : 0080-00ff
// 2 : 0100-01ff
// 3 : 0200-03ff
// 4 : 0400-07ff
// 5 : 0800-0fff
// 6 : 1000-1fff
// 7 : 2000-3fff
// 8 : 4000-7fff
// 9 : 8000-ffff

#ifndef MRBC_ALLOC_FLI_BIT_WIDTH	// 0000 0000 0000 0000
# define MRBC_ALLOC_FLI_BIT_WIDTH 9	// ~~~~~~~~~~~
#endif
#ifndef MRBC_ALLOC_SLI_BIT_WIDTH	// 0000 0000 0000 0000
# define MRBC_ALLOC_SLI_BIT_WIDTH 3	//            ~~~
#endif
#ifndef MRBC_ALLOC_IGNORE_LSBS		// 0000 0000 0000 0000
# define MRBC_ALLOC_IGNORE_LSBS	  4	//                ~~~~
#endif
#ifndef MRBC_ALLOC_MEMSIZE_T
# define MRBC_ALLOC_MEMSIZE_T     uint16_t
#endif

#define FLI(x) (((x) >> MRBC_ALLOC_SLI_BIT_WIDTH) & ((1 << MRBC_ALLOC_FLI_BIT_WIDTH) - 1))
#define SLI(x) ((x) & ((1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1))


// define flags
#define FLAG_TAIL_BLOCK     1
#define FLAG_NOT_TAIL_BLOCK 0
#define FLAG_FREE_BLOCK     1
#define FLAG_USED_BLOCK     0

// memory block header
typedef struct USED_BLOCK {
  unsigned int         t : 1;       //!< FLAG_TAIL_BLOCK or FLAG_NOT_TAIL_BLOCK
  unsigned int         f : 1;       //!< FLAG_FREE_BLOCK or BLOCK_IS_NOT_FREE
  uint8_t              vm_id;       //!< mruby/c VM ID

  MRBC_ALLOC_MEMSIZE_T size;        //!< block size, header included
  MRBC_ALLOC_MEMSIZE_T prev_offset; //!< offset of previous physical block
} USED_BLOCK;

typedef struct FREE_BLOCK {
  unsigned int         t : 1;       //!< FLAG_TAIL_BLOCK or FLAG_NOT_TAIL_BLOCK
  unsigned int         f : 1;       //!< FLAG_FREE_BLOCK or BLOCK_IS_NOT_FREE
  uint8_t              vm_id;       //!< dummy

  MRBC_ALLOC_MEMSIZE_T size;        //!< block size, header included
  MRBC_ALLOC_MEMSIZE_T prev_offset; //!< offset of previous physical block

  struct FREE_BLOCK *next_free;
  struct FREE_BLOCK *prev_free;
} FREE_BLOCK;

#define PHYS_NEXT(p) ((uint8_t *)(p) + (p)->size)
#define PHYS_PREV(p) ((uint8_t *)(p) - (p)->prev_offset)
#define SET_PHYS_PREV(p1,p2)				\
  ((p2)->prev_offset = (uint8_t *)(p2)-(uint8_t *)(p1))

#define SET_VM_ID(p,id)							\
  (((USED_BLOCK *)((uint8_t *)(p) - sizeof(USED_BLOCK)))->vm_id = (id))
#define GET_VM_ID(p)							\
  (((USED_BLOCK *)((uint8_t *)(p) - sizeof(USED_BLOCK)))->vm_id)


// memory pool
static unsigned int memory_pool_size;
static uint8_t     *memory_pool;

// free memory block index
#define SIZE_FREE_BLOCKS \
  ((MRBC_ALLOC_FLI_BIT_WIDTH + 1) * (1 << MRBC_ALLOC_SLI_BIT_WIDTH))
static FREE_BLOCK *free_blocks[SIZE_FREE_BLOCKS + 1];

// free memory bitmap
#define MSB_BIT1 0x8000
static uint16_t free_fli_bitmap;
static uint16_t free_sli_bitmap[MRBC_ALLOC_FLI_BIT_WIDTH + 2]; // + sentinel


//================================================================
/*! Number of leading zeros.

  @param  x	target (16bit unsined)
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
/*! calc f and s, and returns fli,sli of free_blocks

  @param  alloc_size	alloc size
  @retval int		index of free_blocks
*/
static int calc_index(unsigned int alloc_size)
{
  // check overflow
  if( (alloc_size >> (MRBC_ALLOC_FLI_BIT_WIDTH
                      + MRBC_ALLOC_SLI_BIT_WIDTH
                      + MRBC_ALLOC_IGNORE_LSBS)) != 0) {
    return SIZE_FREE_BLOCKS;
  }

  // calculate First Level Index.
  int fli = 16 -
    nlz16( alloc_size >> (MRBC_ALLOC_SLI_BIT_WIDTH + MRBC_ALLOC_IGNORE_LSBS) );

  // calculate Second Level Index.
  int shift = (fli == 0) ? (fli + MRBC_ALLOC_IGNORE_LSBS) :
			   (fli + MRBC_ALLOC_IGNORE_LSBS - 1);

  int sli   = (alloc_size >> shift) & ((1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1);
  int index = (fli << MRBC_ALLOC_SLI_BIT_WIDTH) + sli;

  assert(fli >= 0);
  assert(fli <= MRBC_ALLOC_FLI_BIT_WIDTH);
  assert(sli >= 0);
  assert(sli <= (1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1);

  return index;
}


//================================================================
/*! Mark that block free and register it in the free index table.

  @param  target	Pointer to target block.
*/
static void add_free_block(FREE_BLOCK *target)
{
  target->f = FLAG_FREE_BLOCK;

  int index = calc_index(target->size) - 1;
  int fli   = FLI(index);
  int sli   = SLI(index);

  free_fli_bitmap      |= (MSB_BIT1 >> fli);
  free_sli_bitmap[fli] |= (MSB_BIT1 >> sli);

  target->prev_free = NULL;
  target->next_free = free_blocks[index];
  if( target->next_free != NULL ) {
    target->next_free->prev_free = target;
  }
  free_blocks[index] = target;

#ifdef MRBC_DEBUG
  target->vm_id = UINT8_MAX;
  memset( (uint8_t *)target + sizeof(FREE_BLOCK), 0xff,
          target->size - sizeof(FREE_BLOCK) );
#endif

}


//================================================================
/*! just remove the free_block *target from index

  @param  target	pointer to target block.
*/
static void remove_index(FREE_BLOCK *target)
{
  // top of linked list?
  if( target->prev_free == NULL ) {
    int index = calc_index(target->size) - 1;
    free_blocks[index] = target->next_free;

    if( free_blocks[index] == NULL ) {
      int fli = FLI(index);
      int sli = SLI(index);
      free_sli_bitmap[fli] &= ~(MSB_BIT1 >> sli);
      if( free_sli_bitmap[fli] == 0 ) free_fli_bitmap &= ~(MSB_BIT1 >> fli);
    }
  }
  else {
    target->prev_free->next_free = target->next_free;
  }

  if( target->next_free != NULL ) {
    target->next_free->prev_free = target->prev_free;
  }
}


//================================================================
/*! Split block by size

  @param  target	pointer to target block
  @param  size	size
  @retval NULL	no split.
  @retval FREE_BLOCK *	pointer to splitted free block.
*/
static inline FREE_BLOCK* split_block(FREE_BLOCK *target, unsigned int size)
{
  if( target->size < (size + sizeof(FREE_BLOCK)
                      + (1 << MRBC_ALLOC_IGNORE_LSBS)) ) return NULL;

  // split block, free
  FREE_BLOCK *split = (FREE_BLOCK *)((uint8_t *)target + size);
  FREE_BLOCK *next  = (FREE_BLOCK *)PHYS_NEXT(target);

  split->size  = target->size - size;
  SET_PHYS_PREV(target, split);
  split->t     = target->t;
  target->size = size;
  target->t    = FLAG_NOT_TAIL_BLOCK;
  if( split->t == FLAG_NOT_TAIL_BLOCK ) {
    SET_PHYS_PREV(split, next);
  }

  return split;
}


//================================================================
/*! merge ptr1 and ptr2 block.
    ptr2 will disappear

  @param  ptr1	pointer to free block 1
  @param  ptr2	pointer to free block 2
*/
static void merge_block(FREE_BLOCK *ptr1, FREE_BLOCK *ptr2)
{
  assert(ptr1 < ptr2);

  // merge ptr1 and ptr2
  ptr1->t     = ptr2->t;
  ptr1->size += ptr2->size;

  // update block info
  if( ptr1->t == FLAG_NOT_TAIL_BLOCK ) {
    FREE_BLOCK *next = (FREE_BLOCK *)PHYS_NEXT(ptr1);
    SET_PHYS_PREV(ptr1, next);
  }
}


//================================================================
/*! initialize

  @param  ptr	pointer to free memory block.
  @param  size	size. (max 64KB. see MRBC_ALLOC_MEMSIZE_T)
*/
void mrbc_init_alloc(void *ptr, unsigned int size)
{
  assert( size != 0 );
  assert( size <= (MRBC_ALLOC_MEMSIZE_T)(~0) );

  memory_pool      = ptr;
  memory_pool_size = size;

  // initialize memory pool
  FREE_BLOCK *block = (FREE_BLOCK *)memory_pool;
  block->t           = FLAG_TAIL_BLOCK;
  block->f           = FLAG_FREE_BLOCK;
  block->size        = memory_pool_size;
  block->prev_offset = 0;

  add_free_block(block);
}


//================================================================
/*! allocate memory

  @param  size	request size.
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/
void * mrbc_raw_alloc(unsigned int size)
{
  // TODO: maximum alloc size
  //  (1 << (FLI_BIT_WIDTH + SLI_BIT_WIDTH + IGNORE_LSBS)) - alpha

  unsigned int alloc_size = size + sizeof(FREE_BLOCK);

  // align 4 byte
  alloc_size += ((4 - alloc_size) & 3);

  // check minimum alloc size. if need.
#if 0
  if( alloc_size < (1 << MRBC_ALLOC_IGNORE_LSBS) ) {
    alloc_size = (1 << MRBC_ALLOC_IGNORE_LSBS);
  }
#else
  assert( alloc_size >= (1 << MRBC_ALLOC_IGNORE_LSBS) );
#endif

  // find free memory block.
  int index = calc_index(alloc_size);
  int fli   = FLI(index);
  int sli   = SLI(index);

  FREE_BLOCK *target = free_blocks[index];

  if( target == NULL ) {
    // uses free_fli/sli_bitmap table.
    uint16_t masked = free_sli_bitmap[fli] & ((MSB_BIT1 >> sli) - 1);
    if( masked != 0 ) {
      sli = nlz16(masked);
    }
    else {
      masked = free_fli_bitmap & ((MSB_BIT1 >> fli) - 1);
      if( masked != 0 ) {
	fli = nlz16(masked);
	sli = nlz16(free_sli_bitmap[fli]);
      }
      else {
	// out of memory
	console_print("Fatal error: Out of memory.\n");
	return NULL;  // ENOMEM
      }
    }
    assert(fli >= 0);
    assert(fli <= MRBC_ALLOC_FLI_BIT_WIDTH);
    assert(sli >= 0);
    assert(sli <= (1 << MRBC_ALLOC_SLI_BIT_WIDTH) - 1);

    index = (fli << MRBC_ALLOC_SLI_BIT_WIDTH) + sli;
    target = free_blocks[index];
    assert( target != NULL );
  }
  assert(target->size >= alloc_size);

  // remove free_blocks index
  target->f          = FLAG_USED_BLOCK;
  free_blocks[index] = target->next_free;

  if( target->next_free == NULL ) {
    free_sli_bitmap[fli] &= ~(MSB_BIT1 >> sli);
    if( free_sli_bitmap[fli] == 0 ) free_fli_bitmap &= ~(MSB_BIT1 >> fli);
  }
  else {
    target->next_free->prev_free = NULL;
  }

  // split a block
  FREE_BLOCK *release = split_block(target, alloc_size);
  if( release != NULL ) {
    add_free_block(release);
  }

#ifdef MRBC_DEBUG
  memset( (uint8_t *)target + sizeof(USED_BLOCK), 0xaa,
          target->size - sizeof(USED_BLOCK) );
#endif
  target->vm_id = 0;

  return (uint8_t *)target + sizeof(USED_BLOCK);
}


//================================================================
/*! release memory

  @param  ptr	Return value of mrbc_raw_alloc()
*/
void mrbc_raw_free(void *ptr)
{
  // get target block
  FREE_BLOCK *target = (FREE_BLOCK *)((uint8_t *)ptr - sizeof(USED_BLOCK));

  // check next block, merge?
  FREE_BLOCK *next = (FREE_BLOCK *)PHYS_NEXT(target);

  if((target->t == FLAG_NOT_TAIL_BLOCK) && (next->f == FLAG_FREE_BLOCK)) {
    remove_index(next);
    merge_block(target, next);
  }

  // check previous block, merge?
  FREE_BLOCK *prev = (FREE_BLOCK *)PHYS_PREV(target);

  if((prev != NULL) && (prev->f == FLAG_FREE_BLOCK)) {
    remove_index(prev);
    merge_block(prev, target);
    target = prev;
  }

  // target, add to index
  add_free_block(target);
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
  USED_BLOCK  *target     = (USED_BLOCK *)((uint8_t *)ptr - sizeof(USED_BLOCK));
  unsigned int alloc_size = size + sizeof(FREE_BLOCK);

  // align 4 byte
  alloc_size += ((4 - alloc_size) & 3);

  // expand? part1.
  // next phys block is free and enough size?
  if( alloc_size > target->size ) {
    FREE_BLOCK *next = (FREE_BLOCK *)PHYS_NEXT(target);
    if((target->t == FLAG_NOT_TAIL_BLOCK) &&
       (next->f == FLAG_FREE_BLOCK) &&
       ((target->size + next->size) >= alloc_size)) {
      remove_index(next);
      merge_block((FREE_BLOCK *)target, next);

      // and fall through.
    }
  }

  // same size?
  if( alloc_size == target->size ) {
    return (uint8_t *)ptr;
  }

  // shrink?
  if( alloc_size < target->size ) {
    FREE_BLOCK *release = split_block((FREE_BLOCK *)target, alloc_size);
    if( release != NULL ) {
      // check next block, merge?
      FREE_BLOCK *next = (FREE_BLOCK *)PHYS_NEXT(release);
      if((release->t == FLAG_NOT_TAIL_BLOCK) && (next->f == FLAG_FREE_BLOCK)) {
        remove_index(next);
        merge_block(release, next);
      }
      add_free_block(release);
    }

    return (uint8_t *)ptr;
  }

  // expand part2.
  // new alloc and copy
  uint8_t *new_ptr = mrbc_raw_alloc(size);
  if( new_ptr == NULL ) return NULL;  // ENOMEM

  memcpy(new_ptr, ptr, target->size - sizeof(USED_BLOCK));
  SET_VM_ID(new_ptr, target->vm_id);

  mrbc_raw_free(ptr);

  return new_ptr;
}



//// for mruby/c

//================================================================
/*! allocate memory

  @param  vm	pointer to VM.
  @param  size	request size.
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/
void * mrbc_alloc(const struct VM *vm, unsigned int size)
{
  uint8_t *ptr = mrbc_raw_alloc(size);
  if( ptr == NULL ) return NULL;	// ENOMEM
  if( vm ) SET_VM_ID(ptr, vm->vm_id);

  return ptr;
}


//================================================================
/*! re-allocate memory

  @param  vm	pointer to VM.
  @param  ptr	Return value of mrbc_alloc()
  @param  size	request size.
  @return void * pointer to allocated memory.
  @retval NULL	error.
*/
void * mrbc_realloc(const struct VM *vm, void *ptr, unsigned int size)
{
  return mrbc_raw_realloc(ptr, size);
}


//================================================================
/*! release memory

  @param  vm	pointer to VM.
  @param  ptr	Return value of mrbc_alloc()
*/
void mrbc_free(const struct VM *vm, void *ptr)
{
  mrbc_raw_free(ptr);
}


//================================================================
/*! release memory, vm used.

  @param  vm	pointer to VM.
*/
void mrbc_free_all(const struct VM *vm)
{
  USED_BLOCK *ptr = (USED_BLOCK *)memory_pool;
  void *free_target = NULL;
  int flag_loop = 1;
  int vm_id = vm->vm_id;

  while( flag_loop ) {
    if( ptr->t == FLAG_TAIL_BLOCK ) flag_loop = 0;
    if( ptr->f == FLAG_USED_BLOCK && ptr->vm_id == vm_id ) {
      if( free_target ) {
	mrbc_raw_free(free_target);
      }
      free_target = (char *)ptr + sizeof(USED_BLOCK);
    }
    ptr = (USED_BLOCK *)PHYS_NEXT(ptr);
  }
  if( free_target ) {
    mrbc_raw_free(free_target);
  }
}


//================================================================
/*! set vm id

  @param  ptr	Return value of mrbc_alloc()
  @param  vm_id	vm id
*/
void mrbc_set_vm_id(void *ptr, int vm_id)
{
  SET_VM_ID(ptr, vm_id);
}


//================================================================
/*! get vm id

  @param  ptr	Return value of mrbc_alloc()
  @return int	vm id
*/
int mrbc_get_vm_id(void *ptr)
{
  return GET_VM_ID(ptr);
}



#ifdef MRBC_DEBUG
//================================================================
/*! statistics

  @param  *total	returns total memory.
  @param  *used		returns used memory.
  @param  *free		returns free memory.
  @param  *fragment	returns memory fragmentation
*/
void mrbc_alloc_statistics(int *total, int *used, int *free, int *fragmentation)
{
  *total = memory_pool_size;
  *used = 0;
  *free = 0;
  *fragmentation = 0;

  USED_BLOCK *ptr = (USED_BLOCK *)memory_pool;
  int flag_used_free = ptr->f;
  while( 1 ) {
    if( ptr->f ) {
      *free += ptr->size;
    } else {
      *used += ptr->size;
    }
    if( flag_used_free != ptr->f ) {
      (*fragmentation)++;
      flag_used_free = ptr->f;
    }

    if( ptr->t == FLAG_TAIL_BLOCK ) break;

    ptr = (USED_BLOCK *)PHYS_NEXT(ptr);
  }
}



//================================================================
/*! statistics

  @param  vm_id		vm_id
  @return int		total used memory size
*/
int mrbc_alloc_vm_used( int vm_id )
{
  USED_BLOCK *ptr = (USED_BLOCK *)memory_pool;
  int total = 0;

  while( 1 ) {
    if( ptr->vm_id == vm_id && !ptr->f ) {
      total += ptr->size;
    }
    if( ptr->t == FLAG_TAIL_BLOCK ) break;

    ptr = (USED_BLOCK *)PHYS_NEXT(ptr);
  }

  return total;
}

#endif
