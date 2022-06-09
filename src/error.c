/*! @file
  @brief
  exception classes

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "vm_config.h"
#include <types.h>
#include <string.h>
#include <memory.h>

/***** Local headers ********************************************************/
#include "alloc.h"
#include "value.h"
#include "symbol.h"
#include "error.h"
#include "class.h"
#include "c_string.h"
#include "vm.h"


/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Functions ************************************************************/
//================================================================
/*! constructor

  @param  vm		pointer to VM.
  @param  exc_cls	pointer to Exception class.
  @param  message	message.
  @param  len		message length.
  @return		exception object.
*/
mrbc_value mrbc_exception_new(struct VM *vm, struct RClass *exc_cls, const void *message, int len )
{
  // allocate memory.
  mrbc_exception *ex = mrbc_alloc( vm, sizeof(mrbc_exception) );
  if( !ex ) {		// ENOMEM
    return mrbc_nil_value();
  }

  MRBC_INIT_OBJECT_HEADER( ex, "EX" );
  ex->cls = exc_cls;
  if( !message ) goto NO_MESSAGE;

  ex->message = mrbc_alloc( vm, len+1 );
  if( !ex->message ) goto NO_MESSAGE;

  // copy source string.
  memcpy( ex->message, message, len );
  ex->message[len] = '\0';
  ex->message_size = len;
  return (mrbc_value){.tt = MRBC_TT_EXCEPTION, .exception = ex};


 NO_MESSAGE:
    ex->message = 0;
    ex->message_size = 0;
    return (mrbc_value){.tt = MRBC_TT_EXCEPTION, .exception = ex};
}


//================================================================
/*! destructor

  @param  value		target.
*/
void mrbc_exception_delete(mrbc_value *value)
{
  if( value->exception->message ) mrbc_raw_free( value->exception->message );
  mrbc_raw_free( value->exception );
}


//================================================================
/*! message setter.

  @param  vm		pointer to VM.
  @param  value		target.
  @param  message	message.
  @param  len		message length.
*/
void mrbc_exception_set_message(struct VM *vm, mrbc_value *value, const void *message, int len)
{
  mrbc_exception *ex = value->exception;

  if( ex->message ) {
    mrbc_raw_free( ex->message );
    ex->message = NULL;
    ex->message_size = 0;
  }

  if( message ) {
    ex->message = mrbc_alloc( vm, len+1 );
    if( !ex->message ) return;	// ENOMEM

    // copy source string.
    memcpy( ex->message, message, len );
    ex->message[len] = '\0';

    ex->message_size = len;
  }
}


//================================================================
/*! raise exception

  @param  vm		pointer to VM.
  @param  exc_cls	pointer to Exception class.
  @param  msg		message.
  @note	(usage) mrbc_raise(vm, MRBC_CLASS(TypeError), "message here.");
*/
void mrbc_raise( struct VM *vm, struct RClass *exc_cls, const char *msg )
{
  vm->exception = mrbc_exception_new( vm,
			exc_cls ? exc_cls : MRBC_CLASS(RuntimeError),
			msg,
			msg ? strlen(msg) : 0 );
  vm->flag_preemption = 2;
}


//----------------------------------------------------------------
// Exception class
//----------------------------------------------------------------
//================================================================
/*! (method) new
 */
static void c_exception_new(struct VM *vm, mrbc_value v[], int argc)
{
  assert( mrbc_type(v[0]) == MRBC_TT_CLASS );

  mrbc_value value;
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_STRING ) {
    value = mrbc_exception_new(vm, v[0].cls, mrbc_string_cstr(&v[1]), mrbc_string_size(&v[1]));
  } else {
    value = mrbc_exception_new(vm, v[0].cls, NULL, 0);
  }

  SET_RETURN(value);
}


//================================================================
/*! (method) message
 */
static void c_exception_message(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value value;

  if( v[0].exception->message ) {
    value = mrbc_string_new( vm, v[0].exception->message, v[0].exception->message_size );
  } else {
    value = mrbc_string_new_cstr(vm, mrbc_symid_to_str(v->exception->cls->sym_id));
  }

  mrbc_decref( &v[0] );
  v[0] = value;
}


/* mruby/c Exception class hierarchy.

    Exception
      NoMemoryError
      StandardError
        ArgumentError
        IndexError
        NameError
          NoMethodError
        RangeError
        RuntimeError
        TypeError
        ZeroDivisionError
*/

/* MRBC_AUTOGEN_METHOD_TABLE
  FILE("_autogen_class_exception.h")

  CLASS("Exception")
  METHOD("new", c_exception_new )
  METHOD("message", c_exception_message )

  CLASS("NoMemoryError")
  SUPER("Exception")

  CLASS("StandardError")
  SUPER("Exception")

  CLASS("ArgumentError")
  SUPER("StandardError")

  CLASS("IndexError")
  SUPER("StandardError")

  CLASS("NameError")
  SUPER("StandardError")

  CLASS("NoMethodError")
  SUPER("NameError")

  CLASS("RangeError")
  SUPER("StandardError")

  CLASS("RuntimeError")
  SUPER("StandardError")

  CLASS("TypeError")
  SUPER("StandardError")

  CLASS("ZeroDivisionError")
  SUPER("StandardError")
*/
#include "_autogen_class_exception.h"
