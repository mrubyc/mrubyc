/*! @file
  @brief
  Constant and global variables.

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_GLOBAL_H_
#define MRBC_SRC_GLOBAL_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
/***** Local headers ********************************************************/
#include "value.h"
#include "symbol.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
void mrbc_init_global(void);
int mrbc_set_const(mrbc_sym sym_id, mrbc_value *v);
int mrbc_set_class_const(const struct RClass *cls, mrbc_sym sym_id, mrbc_value *v);
mrbc_value *mrbc_get_const(mrbc_sym sym_id);
mrbc_value *mrbc_get_class_const(const struct RClass *cls, mrbc_sym sym_id);
int mrbc_set_global(mrbc_sym sym_id, mrbc_value *v);
mrbc_value *mrbc_get_global(mrbc_sym sym_id);
void mrbc_global_clear_vm_id(void);
void mrbc_separate_nested_symid(mrbc_sym sym_id, mrbc_sym *id1, mrbc_sym *id2);
void mrbc_global_debug_dump(int mode_flag);


/***** Inline functions *****************************************************/
//================================================================
/*! is nested symbol ID ?

  @param  sym_id	target symbol ID.
  @return int		result.
*/
static inline int mrbc_is_nested_symid(mrbc_sym sym_id)
{
  const char *s = mrbc_symid_to_str(sym_id);

  return ('0' <= s[0] && s[0] <= ('9'+6));
}

#ifdef __cplusplus
}
#endif
#endif
