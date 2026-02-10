/*! @file
  @brief
  mruby/c Hash class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


 Function summary

 (constructor)
    mrbc_hash_new()

 (destructor)
    mrbc_hash_delete()

 (setter)
  --[name]------------------[arg]--[ret]-[note]------------------------
    mrbc_hash_set()	    *K,*V   int

 (getter)
  --[name]------------------[arg]--[ret]-[note]------------------------
    mrbc_hash_get()	     *K      V	Data remains in the container
    mrbc_hash_get_p()	     *K     *V	Data remains in the container
    mrbc_hash_search()	     *K     *K	Data remains in the container
    mrbc_hash_search_by_id() SymID  *K	Data remains in the container
    mrbc_hash_remove()	     *K      V	Data does not remain in the container
    mrbc_hash_remove_by_id() SymID   V	Data does not remain in the container

 (iterator)
  --[name]------------------[arg]--[ret]-[note]------------------------
    mrbc_hash_iterator_new() *V     I
    mrbc_hash_i_has_next()   *I     bool
    mrbc_hash_i_next()	     *I     *V	Getter. Data remains in the container

 (others)
    mrbc_hash_size()
    mrbc_hash_resize()
    mrbc_hash_clear()
    mrbc_hash_compare()
    mrbc_hash_dup()

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <string.h>
#include <assert.h>
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! constructor

  @param  vm	pointer to VM.
  @param  size	initial size
  @return 	hash object
*/
mrbc_value mrbc_hash_new(struct VM *vm, int size)
{
  mrbc_value value = mrbc_immediate_value(MRBC_TT_HASH);

  /*
    Allocate handle and data buffer.
  */
  mrbc_hash *h = mrbc_alloc(vm, sizeof(mrbc_hash));
  if( !h ) return value;	// ENOMEM

  mrbc_value *data = mrbc_alloc(vm, sizeof(mrbc_value) * size * 2);
  if( !data ) {			// ENOMEM
    mrbc_raw_free( h );
    return value;
  }

  MRBC_INIT_OBJECT_HEADER( h, "HA" );
  h->data_size = size * 2;
  h->n_stored = 0;
  h->data = data;

  value.hash = h;
  return value;
}


//================================================================
/*! destructor

  @param  hash	pointer to target value
*/
void mrbc_hash_delete(mrbc_value *hash)
{
  // TODO: delete other members (for search).

  mrbc_array_delete(hash);
}


//================================================================
/*! search by key

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @return	pointer to found key or NULL(not found).
*/
mrbc_value * mrbc_hash_search(const mrbc_value *hash, const mrbc_value *key)
{
  mrbc_value *p1 = hash->hash->data;
  const mrbc_value *p2 = p1 + hash->hash->n_stored;

  while( p1 < p2 ) {
    if( mrbc_compare(p1, key) == 0 ) return p1;
    p1 += 2;
  }

  return NULL;
}


//================================================================
/*! search by symbol ID

  @param  hash		pointer to target hash
  @param  sym_id	symbol ID
  @return		pointer to found key or NULL(not found).
  @note			for use with OP_KEY_P.
*/
mrbc_value * mrbc_hash_search_by_id(const mrbc_value *hash, mrbc_sym sym_id)
{
  mrbc_value *p1 = hash->hash->data;
  const mrbc_value *p2 = p1 + hash->hash->n_stored;

  while( p1 < p2 ) {
    if( mrbc_type(*p1) == MRBC_TT_SYMBOL &&
        mrbc_symbol(*p1) == sym_id ) return p1;
    p1 += 2;
  }

  return NULL;
}


//================================================================
/*! setter

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @param  val	pointer to value
  @return	mrbc_error_code
*/
int mrbc_hash_set(mrbc_value *hash, mrbc_value *key, mrbc_value *val)
{
  mrbc_value *v = mrbc_hash_search(hash, key);
  int ret = 0;
  if( v == NULL ) {
    // set a new value
    if( (ret = mrbc_array_push(hash, key)) != 0 ) goto RETURN;
    ret = mrbc_array_push(hash, val);

  } else {
    // replace a value
    mrbc_decref(v);
    *v = *key;
    mrbc_decref(++v);
    *v = *val;
  }

 RETURN:
  return ret;
}


//================================================================
/*! getter

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @return	mrbc_value data at key position or Nil.
*/
mrbc_value mrbc_hash_get(const mrbc_value *hash, const mrbc_value *key)
{
  mrbc_value *v = mrbc_hash_search(hash, key);
  return v ? *++v : mrbc_nil_value();
}


//================================================================
/*! getter

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @return	pointer to mrbc_value or NULL
*/
mrbc_value * mrbc_hash_get_p(const mrbc_value *hash, const mrbc_value *key)
{
  mrbc_value *v = mrbc_hash_search(hash, key);
  return v ? ++v : v;
}


//================================================================
/*! remove a data

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @return	removed data or Nil
*/
mrbc_value mrbc_hash_remove(mrbc_value *hash, const mrbc_value *key)
{
  mrbc_value *v = mrbc_hash_search(hash, key);
  if( v == NULL ) return mrbc_nil_value();

  mrbc_decref(v);		// key
  mrbc_value val = v[1];	// value

  mrbc_hash *h = hash->hash;
  h->n_stored -= 2;

  memmove(v, v+2, (char*)(h->data + h->n_stored) - (char*)v);

  // TODO: re-index hash table if need.

  return val;
}


//================================================================
/*! remove a data by symbol ID.

  @param  hash		pointer to target hash
  @param  sym_id	symbol ID
  @return  		removed data.
  @return 		TT_EMPTY, if not found.
  @note			for use with OP_KARG.
*/
mrbc_value mrbc_hash_remove_by_id(mrbc_value *hash, mrbc_sym sym_id)
{
  mrbc_value *v = mrbc_hash_search_by_id(hash, sym_id);
  if( !v ) return mrbc_immediate_value(MRBC_TT_EMPTY);

  mrbc_value val = v[1];	// value

  mrbc_hash *h = hash->hash;
  h->n_stored -= 2;

  memmove(v, v+2, (char*)(h->data + h->n_stored) - (char*)v);

  // TODO: re-index hash table if need.

  return val;
}


//================================================================
/*! clear all

  @param  hash	pointer to target hash
*/
void mrbc_hash_clear(mrbc_value *hash)
{
  mrbc_array_clear(hash);

  // TODO: re-index hash table if need.
}


//================================================================
/*! compare

  @param  v1	Pointer to mrbc_value
  @param  v2	Pointer to another mrbc_value
  @retval 0	v1 == v2
  @retval 1	v1 != v2
*/
int mrbc_hash_compare(const mrbc_value *v1, const mrbc_value *v2)
{
  if( v1->hash->n_stored != v2->hash->n_stored ) return 1;

  mrbc_value *d1 = v1->hash->data;
  for( int i = 0; i < mrbc_hash_size(v1); i++, d1++ ) {
    mrbc_value *d2 = mrbc_hash_search(v2, d1);	// check key
    if( d2 == NULL ) return 1;
    if( mrbc_compare( ++d1, ++d2 ) ) return 1;	// check data
  }

  return 0;
}


//================================================================
/*! duplicate

  @param  vm	pointer to VM.
  @param  src	pointer to target hash.
*/
mrbc_value mrbc_hash_dup( struct VM *vm, mrbc_value *src )
{
  mrbc_value ret = mrbc_hash_new(vm, mrbc_hash_size(src));
  if( ret.hash == NULL ) return ret;		// ENOMEM

  mrbc_hash *h = src->hash;
  memcpy( ret.hash->data, h->data, sizeof(mrbc_value) * h->n_stored );
  ret.hash->n_stored = h->n_stored;

  mrbc_value *p1 = h->data;
  const mrbc_value *p2 = p1 + h->n_stored;
  while( p1 < p2 ) {
    mrbc_incref(p1++);
  }

  // TODO: dup other members.

  return ret;
}




//================================================================
/*! (method) new
*/
static void c_hash_new(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_hash_new(vm, 0);
  SET_RETURN(ret);
}


//================================================================
/*! (operator) []
*/
static void c_hash_get(struct VM *vm, mrbc_value v[], int argc)
{
  if( argc != 1 ) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments");
    return;
  }

  mrbc_value val = mrbc_hash_get(&v[0], &v[1]);
  mrbc_incref(&val);
  SET_RETURN(val);
}


//================================================================
/*! (operator) []=
*/
static void c_hash_set(struct VM *vm, mrbc_value v[], int argc)
{
  if( argc != 2 ) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments");
    return;
  }

  mrbc_value *v1 = &v[1];
  mrbc_value *v2 = &v[2];
  mrbc_hash_set(v, v1, v2);
  mrbc_set_tt(v1, MRBC_TT_EMPTY);
  mrbc_set_tt(v2, MRBC_TT_EMPTY);
}


//================================================================
/*! (method) clear
*/
static void c_hash_clear(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_hash_clear(v);
}


//================================================================
/*! (method) dup
*/
static void c_hash_dup(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_hash_dup( vm, &v[0] );

  SET_RETURN(ret);
}


//================================================================
/*! (method) delete
*/
static void c_hash_delete(struct VM *vm, mrbc_value v[], int argc)
{
  // TODO : now, support only delete(key) -> object

  mrbc_value ret = mrbc_hash_remove(v, v+1);

  // TODO: re-index hash table if need.

  SET_RETURN(ret);
}


//================================================================
/*! (method) empty?
*/
static void c_hash_empty(struct VM *vm, mrbc_value v[], int argc)
{
  int n = mrbc_hash_size(v);

  if( n ) {
    SET_FALSE_RETURN();
  } else {
    SET_TRUE_RETURN();
  }
}


//================================================================
/*! (method) has_key?
*/
static void c_hash_has_key(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value *res = mrbc_hash_search(v, v+1);

  if( res ) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}


//================================================================
/*! (method) has_value?
*/
static void c_hash_has_value(struct VM *vm, mrbc_value v[], int argc)
{
  int ret = 0;
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(&v[0]);

  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *val = mrbc_hash_i_next(&ite) + 1;	// skip key, get value
    if( mrbc_compare(val, &v[1]) == 0 ) {
      ret = 1;
      break;
    }
  }

  if( ret ) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}


//================================================================
/*! (method) key
*/
static void c_hash_key(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value *ret = NULL;
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(&v[0]);

  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *kv = mrbc_hash_i_next(&ite);
    if( mrbc_compare( &kv[1], &v[1]) == 0 ) {
      mrbc_incref( &kv[0] );
      ret = &kv[0];
      break;
    }
  }

  if( ret ) {
    SET_RETURN(*ret);
  } else {
    SET_NIL_RETURN();
  }
}


//================================================================
/*! (method) keys
*/
static void c_hash_keys(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_array_new( vm, mrbc_hash_size(v) );
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(v);

  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *key = mrbc_hash_i_next(&ite);
    mrbc_array_push(&ret, key);
    mrbc_incref(key);
  }

  SET_RETURN(ret);
}


//================================================================
/*! (method) size,length,count
*/
static void c_hash_size(struct VM *vm, mrbc_value v[], int argc)
{
  int n = mrbc_hash_size(v);

  SET_INT_RETURN(n);
}


//================================================================
/*! (method) merge
*/
static void c_hash_merge(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_hash_dup( vm, &v[0] );
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(&v[1]);

  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *kv = mrbc_hash_i_next(&ite);
    mrbc_hash_set( &ret, &kv[0], &kv[1] );
    mrbc_incref( &kv[0] );
    mrbc_incref( &kv[1] );
  }

  SET_RETURN(ret);
}


//================================================================
/*! (method) merge!
*/
static void c_hash_merge_self(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(&v[1]);

  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *kv = mrbc_hash_i_next(&ite);
    mrbc_hash_set( v, &kv[0], &kv[1] );
    mrbc_incref( &kv[0] );
    mrbc_incref( &kv[1] );
  }
}


//================================================================
/*! (method) values
*/
static void c_hash_values(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_array_new( vm, mrbc_hash_size(v) );
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(v);

  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *val = mrbc_hash_i_next(&ite) + 1;
    mrbc_array_push(&ret, val);
    mrbc_incref(val);
  }

  SET_RETURN(ret);
}


//================================================================
/*! (method) deconstruct_keys
*/
static void c_hash_deconstruct_keys(struct VM *vm, mrbc_value v[], int argc)
{
  if (argc != 1) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments");
    return;
  }

  mrbc_value keys = v[1];

  /* If keys is nil, return self */
  if (mrbc_type(keys) == MRBC_TT_NIL) {
    return;
  }

  /* Keys must be an array */
  if (mrbc_type(keys) != MRBC_TT_ARRAY) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "wrong argument type (expected Array or nil)");
    return;
  }

  /* Create result hash */
  mrbc_value result = mrbc_hash_new(vm, 0);

  /* Extract specified keys that exist in the hash */
  int len = mrbc_array_size(&keys);
  for (int i = 0; i < len; i++) {
    mrbc_value key = mrbc_array_get(&keys, i);
    mrbc_value *found_key = mrbc_hash_search(&v[0], &key);

    /* Only add key if it exists in original hash */
    if (found_key != NULL) {
      mrbc_value *val = found_key + 1;  /* Value is next to key */
      mrbc_hash_set(&result, &key, val);
      mrbc_incref(&key);
      mrbc_incref(val);
    }
  }

  SET_RETURN(result);
}


#if MRBC_USE_STRING
//================================================================
/*! (method) inspect, to_s
*/
static void c_hash_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    mrbc_object_inspect(vm, v, argc);
    return;
  }

  mrbc_value ret = mrbc_string_new_cstr(vm, "{");
  if( !ret.string ) goto RETURN_NIL;		// ENOMEM

  mrbc_hash_iterator ite = mrbc_hash_iterator_new(v);
  int flag_first = 1;

  while( mrbc_hash_i_has_next(&ite) ) {
    if( !flag_first ) mrbc_string_append_cstr( &ret, ", " );
    flag_first = 0;
    mrbc_value *kv = mrbc_hash_i_next(&ite);
    mrbc_value s1;

    if (mrbc_type(*kv) == MRBC_TT_SYMBOL) {
      const char *s = mrbc_symid_to_str(kv->sym_id);
      mrbc_string_append_cstr( &ret, s );
      mrbc_string_append_cstr( &ret, ": " );
    } else {
      s1 = mrbc_send( vm, v, argc, &kv[0], "inspect", 0 );
      mrbc_string_append( &ret, &s1 );
      mrbc_string_delete( &s1 );
      mrbc_string_append_cstr( &ret, " => " );
    }

    s1 = mrbc_send( vm, v, argc, &kv[1], "inspect", 0 );
    mrbc_string_append( &ret, &s1 );
    mrbc_string_delete( &s1 );
  }

  mrbc_string_append_cstr( &ret, "}" );

  SET_RETURN(ret);
  return;

 RETURN_NIL:
  SET_NIL_RETURN();
}
#endif


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Hash")
  FILE("_autogen_class_hash.h")

  METHOD( "new",	c_hash_new )
  METHOD( "[]",		c_hash_get )
  METHOD( "[]=",	c_hash_set )
  METHOD( "clear",	c_hash_clear )
  METHOD( "deconstruct_keys", c_hash_deconstruct_keys )
  METHOD( "dup",	c_hash_dup )
  METHOD( "delete",	c_hash_delete )
  METHOD( "empty?",	c_hash_empty )
  METHOD( "has_key?",	c_hash_has_key )
  METHOD( "has_value?",	c_hash_has_value )
  METHOD( "key",	c_hash_key )
  METHOD( "keys",	c_hash_keys )
  METHOD( "size",	c_hash_size )
  METHOD( "length",	c_hash_size )
  METHOD( "count",	c_hash_size )
  METHOD( "merge",	c_hash_merge )
  METHOD( "merge!",	c_hash_merge_self )
  METHOD( "to_h",	c_ineffect )
  METHOD( "values",	c_hash_values )
#if MRBC_USE_STRING
  METHOD( "inspect",	c_hash_inspect )
  METHOD( "to_s",	c_hash_inspect )
#endif
*/
#include "_autogen_class_hash.h"
