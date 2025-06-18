/*! @file
  @brief
  mruby bytecode executor.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Fetch mruby VM bytecodes, decode and execute.

  </pre>
*/

#ifndef MRBC_SRC_VM_H_
#define MRBC_SRC_VM_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdint.h>
//@endcond


/***** Local headers ********************************************************/
#include "value.h"
#include "class.h"
#include "c_proc.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  IREP Internal REPresentation
*/
typedef struct IREP {
#if defined(MRBC_DEBUG)
  uint8_t obj_mark_[2];		//!< set "RP" for debug.
#endif

  uint16_t ref_count;		//!< reference counter
#if defined(MRBC_DEBUG)
  uint16_t nlocals;		//!< num of local variables
#endif
  uint16_t nregs;		//!< num of register variables
  uint16_t rlen;		//!< num of child IREP blocks
  uint16_t clen;		//!< num of catch handlers
  uint16_t ilen;		//!< num of bytes in OpCode
#if defined(MRBC_DEBUG)
  uint16_t plen;		//!< num of pools
  uint16_t slen;		//!< num of symbols
#endif
  uint16_t ofs_pools;		//!< offset of data->tbl_pools.
  uint16_t ofs_ireps;		//!< offset of data->tbl_ireps. (32bit aligned)

  const uint8_t *inst;		//!< pointer to instruction in RITE binary
  const uint8_t *pool;		//!< pointer to pool in RITE binary

  uint8_t data[];		//!< variable data. (see load.c)
				//!<  mrbc_sym   tbl_syms[slen]
				//!<  uint16_t   tbl_pools[plen]
				//!<  mrbc_irep *tbl_ireps[rlen]
} mrbc_irep;
typedef struct IREP mrb_irep;

// mrbc_irep manipulate macro.
//! get a symbol id table pointer.
#define mrbc_irep_tbl_syms(irep)	((mrbc_sym *)(irep)->data)

//! get a n'th symbol id in irep
#define mrbc_irep_symbol_id(irep, n)	mrbc_irep_tbl_syms(irep)[(n)]

//! get a n'th symbol string in irep
#define mrbc_irep_symbol_cstr(irep, n)	mrbc_symid_to_str( mrbc_irep_symbol_id(irep, n) )


//! get a pool data offset table pointer.
#define mrbc_irep_tbl_pools(irep) \
  ( (uint16_t *)((irep)->data + (irep)->ofs_pools) )

//! get a pointer to n'th pool data.
#define mrbc_irep_pool_ptr(irep, n) \
  ( (irep)->pool + mrbc_irep_tbl_pools(irep)[(n)] )


//! get a child irep table pointer.
#define mrbc_irep_tbl_ireps(irep) \
  ( (mrbc_irep **)((irep)->data + (irep)->ofs_ireps) )

//! get a n'th child irep
#define mrbc_irep_child_irep(irep, n) \
  ( mrbc_irep_tbl_ireps(irep)[(n)] )



//================================================================
/*!@brief
  IREP Catch Handler
*/
typedef struct IREP_CATCH_HANDLER {
  uint8_t type;		//!< enum mrb_catch_type, 1 byte. 0=rescue, 1=ensure
  uint8_t begin[4];	//!< The starting address to match the handler. Includes this.
  uint8_t end[4];	//!< The endpoint address that matches the handler. Not Includes this.
  uint8_t target[4];	//!< The address to jump to if a match is made.
} mrbc_irep_catch_handler;


//================================================================
/*!@brief
  Call information
*/
typedef struct CALLINFO {
  struct CALLINFO *prev;	//!< previous linked list.
  const mrbc_irep *cur_irep;	//!< copy from mrbc_vm.
  const uint8_t *inst;		//!< copy from mrbc_vm.
  mrbc_value *cur_regs;		//!< copy from mrbc_vm.
  mrbc_class *target_class;	//!< copy from mrbc_vm.

  mrbc_class *own_class;	//!< class that owns method.
  struct RHash *karg_keep;	//!< keyword argument backup for OP_ARGARY.
  mrbc_sym method_id;		//!< called method ID.
  uint8_t reg_offset;		//!< register offset after call.
  uint8_t n_args;		//!< num of arguments.
  uint8_t is_called_super;	//!< this is called by op_super.

} mrbc_callinfo;
typedef struct CALLINFO mrb_callinfo;


//================================================================
/*!@brief
  Virtual Machine
*/
typedef struct VM {
#if defined(MRBC_DEBUG)
  uint8_t obj_mark_[2];			// set "VM" for debug
#endif
  uint8_t vm_id;			//!< vm_id : 1..MAX_VM_COUNT
  volatile int8_t flag_preemption;
  unsigned int flag_need_memfree : 1;
  unsigned int flag_stop : 1;
  unsigned int flag_permanence : 1;

  uint16_t	  regs_size;		//!< size of regs[]

  mrbc_irep       *top_irep;		//!< IREP tree top.
  const mrbc_irep *cur_irep;		//!< IREP currently running.
  const uint8_t   *inst;		//!< Instruction pointer.
  mrbc_value	  *cur_regs;		//!< Current register top.
  mrbc_class      *target_class;	//!< Target class.
  mrbc_callinfo	  *callinfo_tail;	//!< Last point of CALLINFO link.
  mrbc_proc	  *ret_blk;		//!< Return block.

  mrbc_value	  exception;		//!< Raised exception or nil.
  mrbc_value      regs[];
} mrbc_vm;
typedef struct VM mrb_vm;


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
void mrbc_cleanup_vm(void);
mrbc_sym mrbc_get_callee_symid(struct VM *vm);
const char *mrbc_get_callee_name(struct VM *vm);
mrbc_callinfo *mrbc_push_callinfo(struct VM *vm, mrbc_sym method_id, int reg_offset, int n_args);
void mrbc_pop_callinfo(struct VM *vm);
mrbc_vm *mrbc_vm_new(int regs_size);
mrbc_vm *mrbc_vm_open(struct VM *vm);
void mrbc_vm_close(struct VM *vm);
void mrbc_vm_begin(struct VM *vm);
void mrbc_vm_end(struct VM *vm);
int mrbc_vm_run(struct VM *vm);
//@endcond


/***** Inline functions *****************************************************/
//================================================================
/*! get the self object

  @param  vm	A pointer to VM.
  @param  regs	registor
  @return	pointer to self object
*/
static inline mrbc_value * mrbc_get_self( struct VM *vm, mrbc_value *regs )
{
  return regs[0].tt == MRBC_TT_PROC ? &(regs[0].proc->self) : &regs[0];
}


//================================================================
/*! (BETA) check if a block is passed to a method.

  @param  vm	A pointer to VM.
  @param  v	register top.
  @param  argc	n of arguments.
  @return	0 or 1
*/
static inline int mrbc_c_block_given( struct VM *vm, mrbc_value v[], int argc )
{
  int ofs = 1 + (v[argc+1].tt == MRBC_TT_HASH);

  return v[argc + ofs].tt == MRBC_TT_PROC;
}

#ifdef __cplusplus
}
#endif
#endif
