/*! @file
  @brief
  mruby/c Object, Proc, Nil, False and True class and class specific functions.

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#ifndef MRBC_SRC_CLASS_H_
#define MRBC_SRC_CLASS_H_

#include "value.h"
#include "keyvalue.h"

#ifdef __cplusplus
extern "C" {
#endif


//================================================================
/*! mruby/c class object.
*/
typedef struct RClass {
  mrbc_sym sym_id;	// class name
#ifdef MRBC_DEBUG
  const char *names;	// for debug. delete soon.
#endif
  struct RClass *super;	// mrbc_class[super]
  struct RProc *procs;	// mrbc_proc[rprocs], linked list

} mrbc_class;
typedef struct RClass mrb_class;


//================================================================
/*! mruby/c instance object.
*/
typedef struct RInstance {
  MRBC_OBJECT_HEADER;

  struct RClass *cls;
  struct RKeyValueHandle ivar;
  uint8_t data[];

} mrbc_instance;
typedef struct RInstance mrb_instance;


//================================================================
/*! mruby/c proc object.
*/
typedef struct RProc {
  MRBC_OBJECT_HEADER;

  unsigned int c_func : 1;	// 0:IREP, 1:C Func
  mrbc_sym sym_id;
#ifdef MRBC_DEBUG
  const char *names;		// for debug; delete soon
#endif
  struct RProc *next;
  union {
    struct IREP *irep;
    mrbc_func_t func;
  };

} mrbc_proc;
typedef struct RProc mrb_proc;



int mrbc_obj_is_kind_of(const mrbc_value *obj, const mrb_class *cls);
mrbc_proc *mrbc_rproc_alloc(struct VM *vm, const char *name);
mrbc_value mrbc_instance_new(struct VM *vm, mrbc_class *cls, int size);
void mrbc_instance_delete(mrbc_value *v);
void mrbc_instance_setiv(mrbc_object *obj, mrbc_sym sym_id, mrbc_value *v);
mrbc_value mrbc_instance_getiv(mrbc_object *obj, mrbc_sym sym_id);
mrbc_class *find_class_by_object(struct VM *vm, const mrbc_object *obj);
mrbc_proc *find_method(struct VM *vm, const mrbc_object *recv, mrbc_sym sym_id);
mrbc_class *mrbc_define_class(struct VM *vm, const char *name, mrbc_class *super);
mrbc_class *mrbc_get_class_by_name(const char *name);
void mrbc_define_method(struct VM *vm, mrbc_class *cls, const char *name, mrbc_func_t cfunc);
void mrbc_funcall(struct VM *vm, const char *name, mrbc_value *v, int argc);
mrbc_value mrbc_send(struct VM *vm, mrbc_value *v, int reg_ofs, mrbc_value *recv, const char *method, int argc, ...);
int mrbc_p_sub(mrbc_value *v);
int mrbc_print_sub(mrbc_value *v);
int mrbc_puts_sub(mrbc_value *v);
void c_proc_call(struct VM *vm, mrbc_value v[], int argc);
void c_ineffect(struct VM *vm, mrbc_value v[], int argc);
void mrbc_init_class(void);


#ifdef __cplusplus
}
#endif
#endif
