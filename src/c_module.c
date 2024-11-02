/*! @file
  @brief
  mruby/c Module module

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
#include "value.h"
#include "error.h"
#include "class.h"
#include "c_array.h"
#include "global.h"
#include "vm.h"
#include "console.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/

//================================================================
/*! (class method) constants
 */
static void c_module_constants(mrb_vm *vm, mrb_value v[], int argc)
{
  int flag_inherit = 1;

  if( v[0].tt != MRBC_TT_CLASS && v[0].tt != MRBC_TT_MODULE ) {
    mrbc_raise(vm, MRBC_CLASS(NoMethodError), "");
    return;
  }
  if( argc >= 1 && v[1].tt == MRBC_TT_FALSE ) flag_inherit = 0;

  mrbc_class *cls = v[0].cls;
  mrbc_value ret = mrbc_array_new( vm, 0 );

  mrbc_get_all_class_const( cls, &ret );
  if( !flag_inherit ) goto RETURN;

  // support super class
  mrbc_class *mod_nest[3];
  int mod_nest_idx = 0;

  while( 1 ) {
    cls = cls->super;
    if( cls == 0 || cls == MRBC_CLASS(Object) ) {
      if( mod_nest_idx == 0 ) break;	// does not have super class.

      cls = mod_nest[--mod_nest_idx];	// rewind the module search nest.
    }

    // is the next module alias?
    if( cls->flag_alias ) {
      // save the super class pointer to mod_nest[]
      if( cls->super && cls->super != MRBC_CLASS(Object) ) {
        if( mod_nest_idx >= (sizeof(mod_nest) / sizeof(mrbc_class *)) ) {
          mrbc_printf("Warning: Module nest exceeds upper limit.\n");
        } else {
          mod_nest[mod_nest_idx++] = cls->super;
        }
      }
      cls = cls->aliased;
    }

    mrbc_get_all_class_const( cls, &ret );
  }

 RETURN:
  SET_RETURN(ret);
}

/* MRBC_AUTOGEN_METHOD_TABLE

  MODULE("Module")
  FILE("_autogen_module_module.h")
  SUPER(0)

  METHOD( "constants",	c_module_constants )
*/
#include "_autogen_module_module.h"
