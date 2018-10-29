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

#ifndef MRBC_SRC_ALLOC_H_
#define MRBC_SRC_ALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif

struct VM;

void mrbc_init_alloc(void *ptr, unsigned int size);
void *mrbc_raw_alloc(unsigned int size);
void mrbc_raw_free(void *ptr);
void *mrbc_raw_realloc(void *ptr, unsigned int size);
void *mrbc_alloc(const struct VM *vm, unsigned int size);
void *mrbc_realloc(const struct VM *vm, void *ptr, unsigned int size);
void mrbc_free(const struct VM *vm, void *ptr);
void mrbc_free_all(const struct VM *vm);
void mrbc_set_vm_id(void *ptr, int vm_id);
int mrbc_get_vm_id(void *ptr);

// for statistics or debug. (need #define MRBC_DEBUG)
void mrbc_alloc_statistics(int *total, int *used, int *free, int *fragmentation);
int mrbc_alloc_vm_used(int vm_id);


#ifdef __cplusplus
}
#endif
#endif
