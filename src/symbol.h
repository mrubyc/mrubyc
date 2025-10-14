/*! @file
  @brief
  mruby/c Symbol class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_SYMBOL_H_
#define MRBC_SRC_SYMBOL_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
/***** Local headers ********************************************************/
#include "value.h"
#include "_autogen_builtin_symbol.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/*! Macros to specify built-in symbol IDs.
 */
#define MRBC_SYM(sym) MRBC_SYMID_##sym
//@cond
#define MRB_SYM(sym)  MRBC_SYMID_##sym
//@endcond


/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
//@cond
void mrbc_cleanup_symbol(void);
mrbc_sym mrbc_str_to_symid(const char *str);
const char *mrbc_symid_to_str(mrbc_sym sym_id);
mrbc_sym mrbc_search_symid(const char *str);
void make_nested_symbol_s(char *buf, mrbc_sym id1, mrbc_sym id2);
void mrbc_separate_nested_symid(mrbc_sym sym_id, mrbc_sym *id1, mrbc_sym *id2);
mrbc_value mrbc_symbol_new(struct VM *vm, const char *str);
void mrbc_debug_dump_symbol(void);
void mrbc_symbol_statistics(int *total_used);
//@endcond


/***** Inline functions *****************************************************/

//================================================================
/*! get c-language string (char *)
*/
static inline const char * mrbc_symbol_cstr(const mrbc_value *v)
{
  return mrbc_symid_to_str(v->sym_id);
}


//================================================================
/*! is nested symbol ID ?

  @param  sym_id	target symbol ID.
  @return int		result.
  @see	make_nested_symbol_s
*/
static inline int mrbc_is_nested_symid(mrbc_sym sym_id)
{
  const char *s = mrbc_symid_to_str(sym_id);

  return ('0' <= s[0] && s[0] <= ('9'+6));
}


// for legacy compatibility.
static inline mrbc_sym str_to_symid(const char *str) {
  return mrbc_str_to_symid(str);
}

static inline const char *symid_to_str(mrbc_sym sym_id) {
  return mrbc_symid_to_str(sym_id);
}


#ifdef __cplusplus
}
#endif
#endif
