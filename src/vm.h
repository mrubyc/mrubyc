/*! @file
  @brief
  mruby bytecode executor.

  <pre>
  Copyright (C) 2015-2020 Kyushu Institute of Technology.
  Copyright (C) 2015-2020 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Fetch mruby VM bytecodes, decode and execute.

  </pre>
*/

#ifndef MRBC_SRC_VM_H_
#define MRBC_SRC_VM_H_

#include "vm_config.h"
#include "value.h"
#include "class.h"

#ifdef __cplusplus
extern "C" {
#endif


//================================================================
/*!@brief
  IREP Internal REPresentation
*/
typedef struct IREP {
#if defined(MRBC_DEBUG)
  uint8_t type[2];		//!< # for debug.
#endif

  uint16_t nlocals;		//!< # of local variables
  uint16_t nregs;		//!< # of register variables
  uint16_t rlen;		//!< # of child IREP blocks
  uint16_t ilen;		//!< # of irep
  uint16_t plen;		//!< # of pool

  uint8_t     *code;		//!< ISEQ (code) BLOCK
  mrbc_object **pools;		//!< array of POOL objects pointer.
  uint8_t     *ptr_to_sym;
  struct IREP **reps;		//!< array of child IREP's pointer.

} mrbc_irep;
typedef struct IREP mrb_irep;


//================================================================
/*!@brief
  Call information
*/
typedef struct CALLINFO {
  struct CALLINFO *prev;	//!< previous linked list.
  mrbc_irep *pc_irep;		//!< copy from mrbc_vm.
  uint8_t *inst;		//!< copy from mrbc_vm.
  mrbc_value *current_regs;	//!< copy from mrbc_vm.
  mrbc_class *target_class;	//!< copy from mrbc_vm.
  mrbc_class *own_class;	//!< class that owns method.
  mrbc_sym method_id;		//!< called method ID.
  uint8_t reg_offset;		//!< register offset after call.
  uint8_t n_args;		//!< # of arguments.
} mrbc_callinfo;
typedef struct CALLINFO mrb_callinfo;


//================================================================
/*!@brief
  Virtual Machine
*/
typedef struct VM {
  mrbc_irep *irep;

  uint8_t        vm_id; // vm_id : 1..n
  const uint8_t *mrb;   // bytecode

  mrbc_irep *pc_irep;   // PC
  uint8_t *inst;        // instruction

  mrbc_value    regs[MAX_REGS_SIZE];
  mrbc_value   *current_regs;
  mrbc_callinfo *callinfo_tail;

  mrbc_class *target_class;

#ifdef MRBC_DEBUG
  uint8_t flag_debug_mode;
#endif

  mrbc_class *exc, *exc_pending;
  mrbc_value exc_message;  // exception message
  mrbc_callinfo *exception_tail;

  int32_t error_code;

  volatile int8_t flag_preemption;
  int8_t flag_need_memfree;
} mrbc_vm;
typedef struct VM mrb_vm;



void mrbc_cleanup_vm(void);
const char *mrbc_get_callee_name(struct VM *vm);
mrbc_irep *mrbc_irep_alloc(struct VM *vm);
void mrbc_irep_free(mrbc_irep *irep);
mrbc_callinfo * mrbc_push_callinfo( struct VM *vm, mrbc_sym method_id, int reg_offset, int n_args );
void mrbc_pop_callinfo(struct VM *vm);
mrbc_vm *mrbc_vm_open(struct VM *vm_arg);
void mrbc_vm_close(struct VM *vm);
void mrbc_vm_begin(struct VM *vm);
void mrbc_vm_end(struct VM *vm);
int mrbc_vm_run(struct VM *vm);



//================================================================
/*! Get 32bit value from memory.

  @param  s	Pointer to memory.
  @return	32bit unsigned value.
*/
static inline uint32_t bin_to_uint32( const void *s )
{
  // Little endian, no alignment.
  //  e.g. ARM Coretex-M4, Intel x86
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  uint32_t x = *((uint32_t *)s);
  return (x << 24) | ((x & 0xff00) << 8) | ((x >> 8) & 0xff00) | (x >> 24);

  // Big endian, no alignment.
  //  e.g. IBM PPC405
#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  uint32_t x = *((uint32_t *)s);
  return x;

  // 32bit alignment required.
  // Little endian
  //  e.g. ARM Coretex-M0
  // Big endian
  //  e.g. OpenRISC
#elif defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  uint8_t *p = (uint8_t *)s;
  uint32_t x = *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p;
  return x;

#else
  #error "Specify MRBC_BIG_ENDIAN or MRBC_LITTLE_ENDIAN"
#endif
}


//================================================================
/*! Get 16bit value from memory.

  @param  s	Pointer to memory.
  @return	16bit unsigned value.
*/
static inline uint16_t bin_to_uint16( const void *s )
{
  // Little endian, no alignment.
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  uint16_t x = *((uint16_t *)s);
  return (x << 8) | (x >> 8);

  // Big endian, no alignment.
#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  uint16_t x = *((uint16_t *)s);
  return x;

  // 32bit alignment required.
#elif defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  uint8_t *p = (uint8_t *)s;
  uint16_t x = *p++;
  x <<= 8;
  x |= *p;
  return x;

#endif
}


//================================================================
/*! Set 32bit value to memory.

  @param  v	Source value.
  @param  d	Pointer to memory.
*/
static inline void uint32_to_bin( uint32_t v, void *d )
{
  uint8_t *p = (uint8_t *)d + 3;
  *p-- = 0xff & v;
  v >>= 8;
  *p-- = 0xff & v;
  v >>= 8;
  *p-- = 0xff & v;
  v >>= 8;
  *p = 0xff & v;
}


//================================================================
/*! Set 16bit value to memory.

  @param  v	Source value.
  @param  d	Pointer to memory.
*/
static inline void uint16_to_bin( uint16_t v, void *d )
{
  uint8_t *p = (uint8_t *)d;
  *p++ = (v >> 8);
  *p = 0xff & v;
}


#ifdef __cplusplus
}
#endif
#endif
