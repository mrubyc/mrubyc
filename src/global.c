/*! @file
  @brief
  Constant and global variables.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
static mrbc_kv_handle handle_const;	//!< for global(Object) constants.
static mrbc_kv_handle handle_global;	//!< for global variables.

/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! initialize const and global table with default value.
*/
void mrbc_init_global(void)
{
  mrbc_kv_init_handle( 0, &handle_const, 30 );
  mrbc_kv_init_handle( 0, &handle_global, 0 );
#if defined(MRBC_DEBUG)
  handle_const.data->obj_mark_[0] = 'C';	// "CV"
#endif
}


//================================================================
/*! setter constant

  @param  sym_id	symbol ID.
  @param  v		pointer to mrbc_value.
  @return		mrbc_error_code.
*/
int mrbc_set_const( mrbc_sym sym_id, mrbc_value *v )
{
  if( mrbc_kv_get( &handle_const, sym_id ) != NULL ) {
    mrbc_printf("warning: already initialized constant.\n");
  }

  return mrbc_kv_set( &handle_const, sym_id, v );
}


//================================================================
/*! setter class constant

  @param  cls		class.
  @param  sym_id	symbol ID.
  @param  v		pointer to mrbc_value.
  @return		mrbc_error_code.
*/
int mrbc_set_class_const( const struct RClass *cls, mrbc_sym sym_id, mrbc_value *v )
{
  char buf[sizeof(mrbc_sym)*4+1];

  make_nested_symbol_s( buf, cls->sym_id, sym_id );
  mrbc_sym id = mrbc_symbol( mrbc_symbol_new( 0, buf ));

  return mrbc_set_const( id, v );
}


//================================================================
/*! getter constant

  @param  sym_id	symbol ID.
  @return		pointer to mrbc_value or NULL.
*/
mrbc_value * mrbc_get_const( mrbc_sym sym_id )
{
  return mrbc_kv_get( &handle_const, sym_id );
}


//================================================================
/*! getter class constant

  @param  cls		class
  @param  sym_id	symbol ID.
  @return		pointer to mrbc_value or NULL.
*/
mrbc_value * mrbc_get_class_const( const struct RClass *cls, mrbc_sym sym_id )
{
  char buf[sizeof(mrbc_sym)*4+1];

  make_nested_symbol_s( buf, cls->sym_id, sym_id );
  mrbc_sym id = mrbc_search_symid(buf);
  if( id <= 0 ) return 0;

  return mrbc_kv_get( &handle_const, id );
}


//================================================================
/*! get all of class constant.

  @param  cls		class
  @param  ret		return value as array type.
*/
void mrbc_get_all_class_const( const struct RClass *cls, mrbc_value *ret )
{
  mrbc_kv_iterator ite = mrbc_kv_iterator_new( &handle_const );
  int flag_object_class = (cls == MRBC_CLASS(Object));

  while( mrbc_kv_i_has_next( &ite ) ) {
    const mrbc_kv *kv = mrbc_kv_i_next( &ite );

    if( mrbc_is_nested_symid(kv->sym_id) ) {
      mrbc_sym id1, id2;

      mrbc_separate_nested_symid( kv->sym_id, &id1, &id2 );
      if( id1 == cls->sym_id ) {
        mrbc_array_push(ret, &mrbc_symbol_value(id2));
      }

    } else if( flag_object_class ) {
      mrbc_array_push(ret, &mrbc_symbol_value(kv->sym_id));
    }
  }
}


//================================================================
/*! setter global variable.

  @param  sym_id	symbol ID.
  @param  v		pointer to mrbc_value.
  @return		mrbc_error_code.
*/
int mrbc_set_global( mrbc_sym sym_id, mrbc_value *v )
{
  return mrbc_kv_set( &handle_global, sym_id, v );
}


//================================================================
/*! getter global variable.

  @param  sym_id	symbol ID.
  @return		pointer to mrbc_value or NULL.
*/
mrbc_value * mrbc_get_global( mrbc_sym sym_id )
{
  return mrbc_kv_get( &handle_global, sym_id );
}


#if defined(MRBC_ALLOC_VMID)
//================================================================
/*! clear vm_id in global object for process terminated.
*/
void mrbc_global_clear_vm_id(void)
{
  mrbc_kv_clear_vm_id( &handle_const );
  mrbc_kv_clear_vm_id( &handle_global );
}
#endif


#ifdef MRBC_DEBUG
//================================================================
/*! debug dump all const table.

  (examples)
  mrbc_define_method(0, 0, "dump_const", (mrbc_func_t)mrbc_debug_dump_const);
*/
void mrbc_debug_dump_const( void )
{
  mrbc_print("<< Const table dump. >>\n(s_id:identifier = value)\n");
  mrbc_kv_iterator ite = mrbc_kv_iterator_new( &handle_const );

  while( mrbc_kv_i_has_next( &ite ) ) {
    const mrbc_kv *kv = mrbc_kv_i_next( &ite );
    const char *s = mrbc_symid_to_str(kv->sym_id);

    if( kv->sym_id < 0x100 ) continue;

    mrbc_printf(" %04x:\"%s\"", kv->sym_id, s );
    if( mrbc_is_nested_symid(kv->sym_id) ) {
      mrbc_printf("(");
      mrbc_print_symbol(kv->sym_id);
      mrbc_printf(")");
    }

    if( mrbc_type(kv->value) == MRBC_TT_CLASS ) {
      const mrbc_class *cls = kv->value.cls;
      mrbc_printf(" = Class(symid=$%x name=", cls->sym_id);
      mrbc_print_symbol(cls->sym_id);
      mrbc_printf(")\n");
      continue;
    }

    if( mrbc_type(kv->value) == MRBC_TT_MODULE ) {
      const mrbc_class *cls = kv->value.cls;
      mrbc_printf(" = Module(symid=$%x name=", cls->sym_id);
      mrbc_print_symbol(cls->sym_id);
      mrbc_printf(")\n");
      continue;
    }

    mrbc_printf(" = ");
    mrbc_p_sub( &kv->value );
    if( mrbc_type(kv->value) <= MRBC_TT_INC_DEC_THRESHOLD ) {
      mrbc_printf(".tt=%d\n", mrbc_type(kv->value));
    } else {
      mrbc_printf(".tt=%d.ref=%d\n", mrbc_type(kv->value), kv->value.obj->ref_count);
    }
  }
}


//================================================================
/*! debug dump all global table.

  (examples)
  mrbc_define_method(0, 0, "dump_global", (mrbc_func_t)mrbc_debug_dump_global);
*/
void mrbc_debug_dump_global( void )
{
  mrbc_print("<< Global table dump. >>\n(s_id:identifier = value)\n");

  mrbc_kv_iterator ite = mrbc_kv_iterator_new( &handle_global );
  while( mrbc_kv_i_has_next( &ite ) ) {
    mrbc_kv *kv = mrbc_kv_i_next( &ite );

    mrbc_printf(" %04x:%s = ", kv->sym_id, mrbc_symid_to_str(kv->sym_id));
    mrbc_p_sub( &kv->value );
    if( mrbc_type(kv->value) <= MRBC_TT_INC_DEC_THRESHOLD ) {
      mrbc_printf(" .tt=%d\n", mrbc_type(kv->value));
    } else {
      mrbc_printf(" .tt=%d refcnt=%d\n", mrbc_type(kv->value), kv->value.obj->ref_count);
    }
  }
}
#endif
