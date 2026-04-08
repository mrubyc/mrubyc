/*! @file
  @brief
  mruby/c memory management.

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.

  Memory management for objects in mruby/c.

  </pre>
*/

#ifndef MRBC_SRC_ALLOC_H_
#define MRBC_SRC_ALLOC_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#if defined(MRBC_ALLOC_LIBC)
#include <stdlib.h>
#endif
//@endcond

/***** Local headers ********************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/*!@brief
  Return value structure for mrbc_alloc_statistics function.
*/
struct MRBC_ALLOC_STATISTICS {
  unsigned int total;		//!< returns total memory.
  unsigned int used;		//!< returns used memory.
  unsigned int free;		//!< returns free memory.
  unsigned int fragmentation;	//!< returns memory fragmentation count.
};

/*!@brief
  for memory allocation profiling functions.
  if you use this, define MRBC_USE_ALLOC_PROF pre-processor macro.
*/
struct MRBC_ALLOC_PROF {
  unsigned long initial;
  unsigned long max;
  unsigned long min;
};


struct VM;

/***** Global variables *****************************************************/
/***** Function prototypes and inline functions *****************************/
//@cond
#if !defined(MRBC_ALLOC_LIBC)
/*
  Normally enabled
*/
void mrbc_init_alloc(void *ptr, unsigned int size);
void mrbc_cleanup_alloc(void);
void *mrbc_raw_alloc(unsigned int size);
void *mrbc_raw_alloc_no_free(unsigned int size);
void *mrbc_raw_calloc(unsigned int nmemb, unsigned int size);
void mrbc_raw_free(void *ptr);
void *mrbc_raw_realloc(void *ptr, unsigned int size);
#define mrbc_free(vm,ptr)		mrbc_raw_free(ptr)
#define mrbc_realloc(vm,ptr,size)	mrbc_raw_realloc(ptr, size)
unsigned int mrbc_alloc_usable_size(void *ptr);

#if defined(MRBC_ALLOC_VMID)
// Enables memory management by VMID.
void *mrbc_alloc(const struct VM *vm, unsigned int size);
void *mrbc_calloc(const struct VM *vm, unsigned int nmemb, unsigned int size);
void mrbc_free_all(const struct VM *vm);
void mrbc_set_vm_id(void *ptr, int vm_id);
int mrbc_get_vm_id(void *ptr);

# else
#define mrbc_alloc(vm,size)	mrbc_raw_alloc(size)
#define mrbc_free_all(vm)	((void)0)
#define mrbc_set_vm_id(ptr,id)	((void)0)
#define mrbc_get_vm_id(ptr)	0
#endif

void mrbc_alloc_statistics(struct MRBC_ALLOC_STATISTICS *ret);
void mrbc_alloc_start_profiling(void);
void mrbc_alloc_stop_profiling(void);
void mrbc_alloc_get_profiling(struct MRBC_ALLOC_PROF *prof);
void mrbc_alloc_print_statistics(void);
void mrbc_alloc_print_pool_header(void *pool_header);
void mrbc_alloc_print_memory_block(void *pool_header);
void mrbc_alloc_print_memory_pool(void);



#elif defined(MRBC_ALLOC_LIBC)
/*
  use the system (libc) memory allocator.
*/
#if defined(MRBC_ALLOC_VMID)
#error "Can't use MRBC_ALLOC_LIBC with MRBC_ALLOC_VMID"
#endif

static inline void mrbc_init_alloc(void *ptr, unsigned int size) {}
static inline void mrbc_cleanup_alloc(void) {}
static inline void *mrbc_raw_alloc(unsigned int size) {
  return malloc(size);
}
static inline void *mrbc_raw_alloc_no_free(unsigned int size) {
  return malloc(size);
}
static inline void *mrbc_raw_calloc(unsigned int nmemb, unsigned int size) {
  return calloc(nmemb, size);
}
static inline void mrbc_raw_free(void *ptr) {
  free(ptr);
}
static inline void *mrbc_raw_realloc(void *ptr, unsigned int size) {
  return realloc(ptr, size);
}
/*
 * When MRBC_ALLOC_LIBC is defined, you can not use mrbc_alloc_usable_size()
 * as malloc_usable_size() is not defined in C99.
 * If you want to use mrbc_alloc_usable_size(), you need to define like this:
 *
 * unsigned int mrbc_alloc_usable_size(void* ptr) {
 *   return malloc_usable_size(ptr);
 * }
*/
static inline void mrbc_free(const struct VM *vm, void *ptr) {
  free(ptr);
}
static inline void * mrbc_realloc(const struct VM *vm, void *ptr, unsigned int size) {
  return realloc(ptr, size);
}
static inline void *mrbc_alloc(const struct VM *vm, unsigned int size) {
  return malloc(size);
}
static inline void mrbc_free_all(const struct VM *vm) {
}
static inline void mrbc_set_vm_id(void *ptr, int vm_id) {
}
static inline int mrbc_get_vm_id(void *ptr) {
  return 0;
}
#endif	// MRBC_ALLOC_LIBC
//@endcond


#ifdef __cplusplus
}
#endif
#endif
