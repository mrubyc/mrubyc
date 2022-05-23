/*! @file
  @brief
  Constant and global variables.

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#include <types.h>
#include "vm_config.h"
#include "value.h"
#include "global.h"
#include "keyvalue.h"
#include "class.h"
#include "symbol.h"
#include "console.h"


static mrbc_kv_handle handle_const;	//!< for global(Object) constants.
static mrbc_kv_handle handle_global;	//!< for global variables.


//================================================================
/*! initialize const and global table with default value.
*/
void mrbc_init_global(void)
{
  mrbc_kv_init_handle( 0, &handle_const, 30 );
  mrbc_kv_init_handle( 0, &handle_global, 0 );
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
  char buf[10];
  mrbc_sym id = cls->sym_id;
  int i;

  for( i = 3; i >= 0; i-- ) {
    buf[i] = '0' + (id & 0x0f);
    id >>= 4;
  }
  id = sym_id;
  for( i = 7; i >= 4; i-- ) {
    buf[i] = '0' + (id & 0x0f);
    id >>= 4;
  }
  buf[8] = 0;

  return mrbc_set_const( mrbc_symbol( mrbc_symbol_new( 0, buf )), v);
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
  char buf[10];
  mrbc_sym id = cls->sym_id;
  int i;
  for( i = 3; i >= 0; i-- ) {
    buf[i] = '0' + (id & 0x0f);
    id >>= 4;
  }
  id = sym_id;
  for( i = 7; i >= 4; i-- ) {
    buf[i] = '0' + (id & 0x0f);
    id >>= 4;
  }
  buf[8] = 0;

  id = mrbc_search_symid(buf);
  if( id < 0 ) return NULL;

  mrbc_value *v = mrbc_kv_get( &handle_const, id );

  return v;
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
/*! clear vm_id in global object for process terminated.
*/
void mrbc_global_debug_dump(void)
{
  mrbc_print("<< Const table dump. >>\n(s_id:identifier = value)\n");
  mrbc_kv_iterator ite = mrbc_kv_iterator_new( &handle_const );
  while( mrbc_kv_i_has_next( &ite ) ) {
    mrbc_kv *kv = mrbc_kv_i_next( &ite );
    const char *s = mrbc_symid_to_str(kv->sym_id);

    if( s && '0' <= s[0] && s[0] <= '9' ) {
      mrbc_sym id;
      const char *s1, *s2;

      id = (s[0]-'0') << 12 | (s[1]-'0') << 8 | (s[2]-'0') << 4 | (s[3]-'0');
      s1 = mrbc_symid_to_str(id);
      id = (s[4]-'0') << 12 | (s[5]-'0') << 8 | (s[6]-'0') << 4 | (s[7]-'0');
      s2 = mrbc_symid_to_str(id);
      mrbc_printf(" %04x:%s (%s::%s) = ", kv->sym_id, s, s1, s2 );
    } else {
      mrbc_printf(" %04x:%s = ", kv->sym_id, s );
    }
    mrbc_p_sub( &kv->value );
    if( mrbc_type(kv->value) <= MRBC_TT_INC_DEC_THRESHOLD ) {
      mrbc_printf(" .tt=%d\n", mrbc_type(kv->value));
    } else {
      mrbc_printf(" .tt=%d refcnt=%d\n", mrbc_type(kv->value), kv->value.obj->ref_count);
    }
  }

  mrbc_print("<< Global table dump. >>\n(s_id:identifier = value)\n");
  ite = mrbc_kv_iterator_new( &handle_global );
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
