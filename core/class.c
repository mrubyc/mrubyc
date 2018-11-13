/*! @file
  @brief
  mruby/c Object, Proc, Nil, False and True class and class specific functions.

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#include "vm_config.h"
#include <string.h>
#include <assert.h>

#include "value.h"
#include "alloc.h"
#include "class.h"
#include "vm.h"
#include "keyvalue.h"
#include "static.h"
#include "symbol.h"
#include "global.h"
#include "console.h"
#include "mruby/opcode.h"
#include "load.h"

#include "c_array.h"
#include "c_hash.h"
#include "c_string.h"
#include "c_range.h"



//================================================================
/*! Check the class is the class of object.

  @param  obj	target object
  @param  cls	class
  @return	result
*/
int mrbc_obj_is_kind_of( const mrbc_value *obj, const mrbc_class *cls )
{
  const mrbc_class *c = find_class_by_object( 0, obj );
  while( c != NULL ) {
    if( c == cls ) return 1;
    c = c->super;
  }

  return 0;
}


//================================================================
/*! mrbc rproc allocator

  @param  vm	Pointer to VM.
  @param  name	Proc name.
  @return	Pointer to allocated memory or NULL.
*/
mrbc_proc *mrbc_rproc_alloc(struct VM *vm, const char *name)
{
  mrbc_proc *ptr = (mrbc_proc *)mrbc_alloc(vm, sizeof(mrbc_proc));
  if( !ptr ) return ptr;

  ptr->ref_count = 1;
  ptr->sym_id = str_to_symid(name);
#ifdef MRBC_DEBUG
  ptr->names = name;	// for debug; delete soon.
#endif
  ptr->next = 0;

  return ptr;
}


//================================================================
/*! mrbc_instance constructor

  @param  vm    Pointer to VM.
  @param  cls	Pointer to Class (mrbc_class).
  @param  size	size of additional data.
  @return       mrbc_instance object.
*/
mrbc_value mrbc_instance_new(struct VM *vm, mrbc_class *cls, int size)
{
  mrbc_value v = {.tt = MRBC_TT_OBJECT};
  v.instance = (mrbc_instance *)mrbc_alloc(vm, sizeof(mrbc_instance) + size);
  if( v.instance == NULL ) return v;	// ENOMEM

  if( mrbc_kv_init_handle(vm, &v.instance->ivar, 0) != 0 ) {
    mrbc_raw_free(v.instance);
    v.instance = NULL;
    return v;
  }

  v.instance->ref_count = 1;
  v.instance->tt = MRBC_TT_OBJECT;	// for debug only.
  v.instance->cls = cls;

  return v;
}



//================================================================
/*! mrbc_instance destructor

  @param  v	pointer to target value
*/
void mrbc_instance_delete(mrbc_value *v)
{
  mrbc_kv_delete_data( &v->instance->ivar );
  mrbc_raw_free( v->instance );
}


//================================================================
/*! instance variable setter

  @param  obj		pointer to target.
  @param  sym_id	key symbol ID.
  @param  v		pointer to value.
*/
void mrbc_instance_setiv(mrbc_object *obj, mrbc_sym sym_id, mrbc_value *v)
{
  mrbc_dup(v);
  mrbc_kv_set( &obj->instance->ivar, sym_id, v );
}


//================================================================
/*! instance variable getter

  @param  obj		pointer to target.
  @param  sym_id	key symbol ID.
  @return		value.
*/
mrbc_value mrbc_instance_getiv(mrbc_object *obj, mrbc_sym sym_id)
{
  mrbc_value *v = mrbc_kv_get( &obj->instance->ivar, sym_id );
  if( !v ) return mrbc_nil_value();

  mrbc_dup(v);
  return *v;
}



//================================================================
/*!@brief
  find class by object

  @param  vm
  @param  obj
  @return pointer to mrbc_class
*/
mrbc_class *find_class_by_object(struct VM *vm, const mrbc_object *obj)
{
  mrbc_class *cls;

  switch( obj->tt ) {
  case MRBC_TT_TRUE:	cls = mrbc_class_true;		break;
  case MRBC_TT_FALSE:	cls = mrbc_class_false; 	break;
  case MRBC_TT_NIL:	cls = mrbc_class_nil;		break;
  case MRBC_TT_FIXNUM:	cls = mrbc_class_fixnum;	break;
  case MRBC_TT_FLOAT:	cls = mrbc_class_float; 	break;
  case MRBC_TT_SYMBOL:	cls = mrbc_class_symbol;	break;

  case MRBC_TT_OBJECT:	cls = obj->instance->cls;       break;
  case MRBC_TT_CLASS:   cls = obj->cls;                 break;
  case MRBC_TT_PROC:	cls = mrbc_class_proc;		break;
  case MRBC_TT_ARRAY:	cls = mrbc_class_array; 	break;
  case MRBC_TT_STRING:	cls = mrbc_class_string;	break;
  case MRBC_TT_RANGE:	cls = mrbc_class_range; 	break;
  case MRBC_TT_HASH:	cls = mrbc_class_hash;		break;

  default:		cls = mrbc_class_object;	break;
  }

  return cls;
}



//================================================================
/*!@brief
  find method from

  @param  vm
  @param  recv
  @param  sym_id
  @return
*/
mrbc_proc *find_method(struct VM *vm, const mrbc_object *recv, mrbc_sym sym_id)
{
  mrbc_class *cls = find_class_by_object(vm, recv);

  while( cls != 0 ) {
    mrbc_proc *proc = cls->procs;
    while( proc != 0 ) {
      if( proc->sym_id == sym_id ) {
        return proc;
      }
      proc = proc->next;
    }
    cls = cls->super;
  }
  return 0;
}



//================================================================
/*!@brief
  define class

  @param  vm		pointer to vm.
  @param  name		class name.
  @param  super		super class.
*/
mrbc_class * mrbc_define_class(struct VM *vm, const char *name, mrbc_class *super)
{
  if( super == NULL ) super = mrbc_class_object;  // set default to Object.

  mrbc_sym sym_id = str_to_symid(name);
  mrbc_object *obj = mrbc_get_const( sym_id );

  // create a new class?
  if( obj == NULL ) {
    mrbc_class *cls = mrbc_alloc( 0, sizeof(mrbc_class) );
    if( !cls ) return cls;	// ENOMEM

    cls->sym_id = sym_id;
#ifdef MRBC_DEBUG
    cls->names = name;	// for debug; delete soon.
#endif
    cls->super = super;
    cls->procs = NULL;

    // register to global constant.
    mrbc_set_const( sym_id, &(mrbc_value){.tt = MRBC_TT_CLASS, .cls = cls} );
    return cls;
  }

  // already?
  if( obj->tt == MRBC_TT_CLASS ) {
    return obj->cls;
  }

  // error.
  // raise TypeError.
  assert( !"TypeError" );
}



//================================================================
/*! get class by name

  @param  name		class name.
  @return		pointer to class object.
*/
mrbc_class * mrbc_get_class_by_name( const char *name )
{
  mrbc_sym sym_id = str_to_symid(name);
  mrbc_object *obj = mrbc_get_const( sym_id );

  if( obj == NULL ) return NULL;
  return (obj->tt == MRBC_TT_CLASS) ? obj->cls : NULL;
}


//================================================================
/*!@brief
  define class method or instance method.

  @param  vm		pointer to vm.
  @param  cls		pointer to class.
  @param  name		method name.
  @param  cfunc		pointer to function.
*/
void mrbc_define_method(struct VM *vm, mrbc_class *cls, const char *name, mrbc_func_t cfunc)
{
  if( cls == NULL ) cls = mrbc_class_object;	// set default to Object.

  mrbc_proc *rproc = mrbc_rproc_alloc(vm, name);
  rproc->c_func = 1;  // c-func
  rproc->next = cls->procs;
  cls->procs = rproc;
  rproc->func = cfunc;
}


// Call a method
// v[0]: receiver
// v[1..]: params
//================================================================
/*!@brief
  call a method with params

  @param  vm		pointer to vm
  @param  name		method name
  @param  v		receiver and params
  @param  argc		num of params
*/
void mrbc_funcall(struct VM *vm, const char *name, mrbc_value *v, int argc)
{
  mrbc_sym sym_id = str_to_symid(name);
  mrbc_proc *m = find_method(vm, &v[0], sym_id);

  if( m==0 ) return;   // no method

  mrbc_callinfo *callinfo = mrbc_alloc(vm, sizeof(mrbc_callinfo));
  callinfo->current_regs = vm->current_regs;
  callinfo->pc_irep = vm->pc_irep;
  callinfo->pc = vm->pc;
  callinfo->n_args = 0;
  callinfo->target_class = vm->target_class;
  callinfo->prev = vm->callinfo_tail;
  vm->callinfo_tail = callinfo;

  // target irep
  vm->pc_irep = m->irep;
  vm->pc = vm->pc_irep->code;

  // new regs
  vm->current_regs += 2;   // recv and symbol

}



//================================================================
/*! (BETA) Call any method of the object, but written by C.

  @param  vm		pointer to vm.
  @param  v		see bellow example.
  @param  reg_ofs	see bellow example.
  @param  recv		pointer to receiver.
  @param  name		method name.
  @param  argc		num of params.

  @example
  // (Fixnum).to_s(16)
  static void c_fixnum_to_s(struct VM *vm, mrbc_value v[], int argc)
  {
    mrbc_value *recv = &v[1];
    mrbc_value arg1 = mrbc_fixnum_value(16);
    mrbc_value ret = mrbc_send( vm, v, argc, recv, "to_s", 1, &arg1 );
    SET_RETURN(ret);
  }
 */
mrbc_value mrbc_send( struct VM *vm, mrbc_value *v, int reg_ofs,
		     mrbc_value *recv, const char *method, int argc, ... )
{
  mrbc_sym sym_id = str_to_symid(method);
  mrbc_proc *m = find_method(vm, recv, sym_id);

  if( m == 0 ) {
    console_printf("No method. vtype=%d method='%s'\n", recv->tt, method );
    goto ERROR;
  }
  if( !m->c_func ) {
    console_printf("Method %s is not C function\n", method );
    goto ERROR;
  }

  // create call stack.
  mrbc_value *regs = v + reg_ofs + 2;
  mrbc_release( &regs[0] );
  regs[0] = *recv;
  mrbc_dup(recv);

  va_list ap;
  va_start(ap, argc);
  int i;
  for( i = 1; i <= argc; i++ ) {
    mrbc_release( &regs[i] );
    regs[i] = *va_arg(ap, mrbc_value *);
  }
  mrbc_release( &regs[i] );
  regs[i] = mrbc_nil_value();
  va_end(ap);

  // call method.
  m->func(vm, regs, argc);
  mrbc_value ret = regs[0];

  for(; i >= 0; i-- ) {
    regs[i].tt = MRBC_TT_EMPTY;
  }

  return ret;

 ERROR:
  return mrbc_nil_value();
}



//================================================================
/*! p - sub function
 */
int mrbc_p_sub(mrbc_value *v)
{
  switch( v->tt ){
  case MRBC_TT_NIL:
    console_print("nil");
    break;

  case MRBC_TT_SYMBOL:{
    const char *s = mrbc_symbol_cstr( v );
    char const *fmt = strchr(s, ':') ? "\":%s\"" : ":%s";
    console_printf(fmt, s);
  } break;

#if MRBC_USE_STRING
  case MRBC_TT_STRING:{
    console_putchar('"');
    const unsigned char *s = (const unsigned char *)mrbc_string_cstr(v);
    int i;
    for( i = 0; i < mrbc_string_size(v); i++ ) {
      if( s[i] < ' ' || 0x7f <= s[i] ) {	// tiny isprint()
	console_printf("\\x%02X", s[i]);
      } else {
	console_putchar(s[i]);
      }
    }
    console_putchar('"');
  } break;
#endif

  case MRBC_TT_RANGE:{
    mrbc_value v1 = mrbc_range_first(v);
    mrbc_p_sub(&v1);
    console_print( mrbc_range_exclude_end(v) ? "..." : ".." );
    v1 = mrbc_range_last(v);
    mrbc_p_sub(&v1);
  } break;

  default:
    mrbc_print_sub(v);
    break;
  }

  return 0;
}


//================================================================
/*! print - sub function
  @param  v	pointer to target value.
  @retval 0	normal return.
  @retval 1	already output LF.
*/
int mrbc_print_sub(mrbc_value *v)
{
  int ret = 0;

  switch( v->tt ){
  case MRBC_TT_EMPTY:	console_print("(empty)");	break;
  case MRBC_TT_NIL:					break;
  case MRBC_TT_FALSE:	console_print("false");		break;
  case MRBC_TT_TRUE:	console_print("true");		break;
  case MRBC_TT_FIXNUM:	console_printf("%d", v->i);	break;
#if MRBC_USE_FLOAT
  case MRBC_TT_FLOAT:	console_printf("%g", v->d);	break;
#endif
  case MRBC_TT_SYMBOL:
    console_print(mrbc_symbol_cstr(v));
    break;

  case MRBC_TT_CLASS:
    console_print(symid_to_str(v->cls->sym_id));
    break;

  case MRBC_TT_OBJECT:
    console_printf( "#<%s:%08x>",
	symid_to_str( find_class_by_object(0,v)->sym_id ), v->instance );
    break;

  case MRBC_TT_PROC:
    console_printf( "#<Proc:%08x>", v->proc );
    break;

  case MRBC_TT_ARRAY:{
    console_putchar('[');
    int i;
    for( i = 0; i < mrbc_array_size(v); i++ ) {
      if( i != 0 ) console_print(", ");
      mrbc_value v1 = mrbc_array_get(v, i);
      mrbc_p_sub(&v1);
    }
    console_putchar(']');
  } break;

#if MRBC_USE_STRING
  case MRBC_TT_STRING:
    console_nprint( mrbc_string_cstr(v), mrbc_string_size(v) );
    if( mrbc_string_size(v) != 0 &&
	mrbc_string_cstr(v)[ mrbc_string_size(v) - 1 ] == '\n' ) ret = 1;
    break;
#endif

  case MRBC_TT_RANGE:{
    mrbc_value v1 = mrbc_range_first(v);
    mrbc_print_sub(&v1);
    console_print( mrbc_range_exclude_end(v) ? "..." : ".." );
    v1 = mrbc_range_last(v);
    mrbc_print_sub(&v1);
  } break;

  case MRBC_TT_HASH:{
    console_putchar('{');
    mrbc_hash_iterator ite = mrbc_hash_iterator_new(v);
    while( mrbc_hash_i_has_next(&ite) ) {
      mrbc_value *vk = mrbc_hash_i_next(&ite);
      mrbc_p_sub(vk);
      console_print("=>");
      mrbc_p_sub(vk+1);
      if( mrbc_hash_i_has_next(&ite) ) console_print(", ");
    }
    console_putchar('}');
  } break;

  default:
    console_printf("Not support MRBC_TT_XX(%d)", v->tt);
    break;
  }

  return ret;
}


//================================================================
/*! puts - sub function

  @param  v	pointer to target value.
  @retval 0	normal return.
  @retval 1	already output LF.
*/
int mrbc_puts_sub(mrbc_value *v)
{
  if( v->tt == MRBC_TT_ARRAY ) {
    int i;
    for( i = 0; i < mrbc_array_size(v); i++ ) {
      if( i != 0 ) console_putchar('\n');
      mrbc_value v1 = mrbc_array_get(v, i);
      mrbc_puts_sub(&v1);
    }
    return 0;
  }

  return mrbc_print_sub(v);
}



//================================================================
// Object class
//================================================================
/*! (method) alias_method

  note: using the 'alias' keyword, this method will be called.
*/
mrbc_static_method(Object, alias_method){
  // find method only in this class.
  mrbc_proc *proc = v[0].cls->procs;
  while( proc != NULL ) {
    if( proc->sym_id == v[2].i ) break;
    proc = proc->next;
  }
  if( !proc ) {
    console_printf("NameError: undefined_method '%s'\n", symid_to_str(v[2].i));
    return;
  }

  // copy the Proc object
  mrbc_proc *proc_alias = mrbc_alloc(0, sizeof(mrbc_proc));
  if( !proc_alias ) return;		// ENOMEM
  memcpy( proc_alias, proc, sizeof(mrbc_proc) );

  // regist procs link.
  proc_alias->sym_id = v[1].i;
#if defined(MRBC_DEBUG)
  proc_alias->names = symid_to_str(v[1].i);
#endif
  proc_alias->next = v[0].cls->procs;
  v[0].cls->procs = proc_alias;
}


//================================================================
/*! (method) p
 */
mrbc_static_method(Object, p){
  int i;
  for( i = 1; i <= argc; i++ ) {
    mrbc_p_sub( &v[i] );
    console_putchar('\n');
  }
}


//================================================================
/*! (method) print
 */
mrbc_static_method(Object, print){
  int i;
  for( i = 1; i <= argc; i++ ) {
    mrbc_print_sub( &v[i] );
  }
}


//================================================================
/*! (method) puts
 */
mrbc_static_method(Object, puts){
  int i;
  if( argc ){
    for( i = 1; i <= argc; i++ ) {
      if( mrbc_puts_sub( &v[i] ) == 0 ) console_putchar('\n');
    }
  } else {
    console_putchar('\n');
  }
}


//================================================================
/*! (operator) !
 */
mrbc_static_method(Object, not){
  SET_FALSE_RETURN();
}


//================================================================
/*! (operator) !=
 */
mrbc_static_method(Object, neq){
  int result = mrbc_compare( &v[0], &v[1] );
  SET_BOOL_RETURN( result != 0 );
}


//================================================================
/*! (operator) <=>
 */
mrbc_static_method(Object, compare){
  int result = mrbc_compare( &v[0], &v[1] );
  SET_INT_RETURN( result );
}


//================================================================
/*! (operator) ===
 */
mrbc_static_method(Object, equal3){
  int result;

  if( v[0].tt == MRBC_TT_CLASS ) {
    result = mrbc_obj_is_kind_of( &v[1], v[0].cls );
  } else {
    result = (mrbc_compare( &v[0], &v[1] ) == 0);
  }

  SET_BOOL_RETURN( result );
}


//================================================================
/*! (method) class
 */
mrbc_static_method(Object, class){
  mrbc_value value = {.tt = MRBC_TT_CLASS};
  value.cls = find_class_by_object( vm, v );
  SET_RETURN( value );
}



// Object.new
mrbc_static_method(Object, new){
  mrbc_value new_obj = mrbc_instance_new(vm, v->cls, 0);

  char syms[]="______initialize";
  uint32_to_bin( 1,(uint8_t*)&syms[0]);
  uint16_to_bin(10,(uint8_t*)&syms[4]);

  /*
  uint32_t code[2] = {
    MKOPCODE(OP_SEND) | MKARG_A(0) | MKARG_B(0) | MKARG_C(argc),
    MKOPCODE(OP_ABORT)
    };
  */
  uint8_t code[] = {
    OP_SEND, 0, 0, argc,
    OP_ABORT,
  };
  mrbc_irep irep = {
    0,     // nlocals
    0,     // nregs
    0,     // rlen
    2,     // ilen
    0,     // plen
    code,   // iseq
    NULL,  // pools
    (uint8_t *)syms,  // ptr_to_sym
    NULL,  // reps
  };

  mrbc_release(&v[0]);
  v[0] = new_obj;
  mrbc_dup(&new_obj);

  mrbc_irep *org_pc_irep = vm->pc_irep;
  uint8_t const *org_pc = vm->pc;
  mrbc_value* org_regs = vm->current_regs;
  vm->pc_irep = &irep;
  vm->pc = vm->pc_irep->code;
  vm->current_regs = v;

  while( mrbc_vm_run(vm) == 0 )
    ;

  vm->pc = org_pc;
  vm->pc_irep = org_pc_irep;
  vm->current_regs = org_regs;

  SET_RETURN(new_obj);
}

//================================================================
/*! (method) instance variable getter
 */
static void c_object_getiv(struct VM *vm, mrbc_value v[], int argc)
{
  const char *name = mrbc_get_callee_name(vm);
  mrbc_sym sym_id = str_to_symid( name );
  mrbc_value ret = mrbc_instance_getiv(&v[0], sym_id);

  SET_RETURN(ret);
}


//================================================================
/*! (method) instance variable setter
 */
static void c_object_setiv(struct VM *vm, mrbc_value v[], int argc)
{
  const char *name = mrbc_get_callee_name(vm);

  char *namebuf = mrbc_alloc(vm, strlen(name));
  if( !namebuf ) return;
  strcpy(namebuf, name);
  namebuf[strlen(name)-1] = '\0';	// delete '='
  mrbc_sym sym_id = str_to_symid(namebuf);

  mrbc_instance_setiv(&v[0], sym_id, &v[1]);
  mrbc_raw_free(namebuf);
}



//================================================================
/*! (class method) access method 'attr_reader'
 */
mrbc_static_method(Object, attr_reader){
  int i;
  for( i = 1; i <= argc; i++ ) {
    if( v[i].tt != MRBC_TT_SYMBOL ) continue;	// TypeError raise?

    // define reader method
    const char *name = mrbc_symbol_cstr(&v[i]);
    mrbc_define_method(vm, v[0].cls, name, c_object_getiv);
  }
}


//================================================================
/*! (class method) access method 'attr_accessor'
 */
mrbc_static_method(Object, attr_accessor){
  int i;
  for( i = 1; i <= argc; i++ ) {
    if( v[i].tt != MRBC_TT_SYMBOL ) continue;	// TypeError raise?

    // define reader method
    const char *name = mrbc_symbol_cstr(&v[i]);
    mrbc_define_method(vm, v[0].cls, name, c_object_getiv);

    // make string "....=" and define writer method.
    char *namebuf = mrbc_alloc(vm, strlen(name)+2);
    if( !namebuf ) return;
    strcpy(namebuf, name);
    strcat(namebuf, "=");
    mrbc_symbol_new(vm, namebuf);
    mrbc_define_method(vm, v[0].cls, namebuf, c_object_setiv);
    mrbc_raw_free(namebuf);
  }
}


//================================================================
/*! (method) is_a, kind_of
 */
mrbc_static_method(Object, kind_of){
  int result = 0;
  if( v[1].tt != MRBC_TT_CLASS ) goto DONE;

  result = mrbc_obj_is_kind_of( &v[0], v[1].cls );

 DONE:
  SET_BOOL_RETURN( result );
}


#if MRBC_USE_STRING
//================================================================
/*! (method) to_s
 */
mrbc_static_method(Object, to_s){
  char buf[32];
  const char *s = buf;

  switch( v->tt ) {
  case MRBC_TT_CLASS:
    s = symid_to_str( v->cls->sym_id );
    break;

  case MRBC_TT_OBJECT:{
    // (NOTE) address part assumes 32bit. but enough for this.
    mrbc_printf pf;

    mrbc_printf_init( &pf, buf, sizeof(buf), "#<%s:%08x>" );
    while( mrbc_printf_main( &pf ) > 0 ) {
      switch(pf.fmt.type) {
      case 's':
	mrbc_printf_str( &pf, symid_to_str(v->instance->cls->sym_id), ' ' );
	break;
      case 'x':
	mrbc_printf_int( &pf, (uintptr_t)v->instance, 16 );
	break;
      }
    }
    mrbc_printf_end( &pf );
  } break;

  default:
    s = "";
    break;
  }

  SET_RETURN( mrbc_string_new_cstr( vm, s ) );
}
#endif


#ifdef MRBC_DEBUG
mrbc_static_method(Object, instance_methods)
{
  // TODO: check argument.

  // temporary code for operation check.
  console_printf( "[" );
  int flag_first = 1;

  mrbc_class *cls = find_class_by_object( vm, v );
  mrbc_proc *proc = cls->procs;
  while( proc ) {
    console_printf( "%s:%s", (flag_first ? "" : ", "),
		    symid_to_str(proc->sym_id) );
    flag_first = 0;
    proc = proc->next;
  }

  console_printf( "]" );

  SET_NIL_RETURN();
}
#endif

// =============== ProcClass

mrbc_static_method(Proc, call)
{
  // self in block call
  mrbc_value *self = vm->callinfo_tail->current_regs;
  
  // push callinfo, but not release regs
  mrbc_push_callinfo(vm, 0, argc);  // TODO: mid==0 is right?

  // target irep
  vm->pc_irep = v[0].proc->irep;
  vm->pc = vm->pc_irep->code;

  // copy regs for object
  // original v[] : [proc][argc][nil]
  //                |current_regs
  // copied   v[] : [proc][argc][obj][argc][nil]
  //                            |current_regs  
  int offset = 1+argc;
  vm->current_regs = v+offset;
  // [obj]
  mrbc_dup(self);
  v[offset] = *self;
  // [argc]
  int i;
  for( i = 1 ; i<=argc ; i++ ){
    mrbc_dup(&v[i]);
    v[offset+i] = v[i];
  }
  // [nil]
  v[offset+argc+1].tt = MRBC_TT_NIL;
}


#if MRBC_USE_STRING
mrbc_static_method(Proc, to_s)
{
  // (NOTE) address part assumes 32bit. but enough for this.
  char buf[32];
  mrbc_printf pf;

  mrbc_printf_init( &pf, buf, sizeof(buf), "<#Proc:%08x>" );
  while( mrbc_printf_main( &pf ) > 0 ) {
    mrbc_printf_int( &pf, (uintptr_t)v->proc, 16 );
  }
  mrbc_printf_end( &pf );

  SET_RETURN( mrbc_string_new_cstr( vm, buf ) );
}
#endif


//================================================================
// Nil class

//================================================================
/*! (method) !
*/
mrbc_static_method(NilClass, not)
{
  v[0].tt = MRBC_TT_TRUE;
}
mrbc_static_method(FalseClass, not)
{
  v[0].tt = MRBC_TT_TRUE;
}


#if MRBC_USE_STRING
//================================================================
/*! (method) inspect
*/
mrbc_static_method(NilClass, inspect)
{
  v[0] = mrbc_string_new_cstr(vm, "nil");
}

mrbc_static_method(TrueClass, inspect)
{
  return mrbc_static_method_sym(Object, to_s)(vm, v, argc);
}

mrbc_static_method(FalseClass, inspect)
{
  return mrbc_static_method_sym(Object, to_s)(vm, v, argc);
}


//================================================================
/*! (method) to_s
*/
mrbc_static_method(NilClass, to_s)
{
  v[0] = mrbc_string_new(vm, NULL, 0);
}
#endif



//================================================================
// False class

#if MRBC_USE_STRING
//================================================================
/*! (method) to_s
*/
mrbc_static_method(FalseClass, to_s)
{
  v[0] = mrbc_string_new_cstr(vm, "false");
}
#endif



//================================================================
// True class

#if MRBC_USE_STRING
//================================================================
/*! (method) to_s
*/
mrbc_static_method(TrueClass, to_s)
{
  v[0] = mrbc_string_new_cstr(vm, "true");
}
#endif



//================================================================
/*! Ineffect operator / method
*/
void c_ineffect(struct VM *vm, mrbc_value v[], int argc)
{
  // nothing to do.
}


#include <stdio.h>

//================================================================
/*! Run mrblib, which is mruby bytecode
*/
static void mrbc_run_mrblib(void)
{
  extern const uint8_t mrblib_bytecode[];

  mrbc_vm vm;
  mrbc_vm_open(&vm);
  int ret = mrbc_load_mrb(&vm, mrblib_bytecode);
  assert(ret == 0);
  mrbc_vm_begin(&vm);
  mrbc_vm_run(&vm);
  mrbc_vm_end(&vm);
  //mrbc_vm_close(&vm);
}


extern void mrbc_init_method_table(mrbc_vm* vm);

//================================================================
// initialize

void mrbc_init_class(void)
{
  mrbc_init_method_table(0);

  mrbc_run_mrblib();
}
