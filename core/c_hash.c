/*! @file
  @brief
  mruby/c Hash class

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
#include "vm.h"
#include "alloc.h"
#include "static.h"
#include "class.h"
#include "c_array.h"
#include "c_hash.h"
#include "c_string.h"

/*
  function summary

 (constructor)
    mrbc_hash_new

 (destructor)
    mrbc_hash_delete

 (setter)
  --[name]-------------[arg]---[ret]-------
    mrbc_hash_set	*K,*V	int

 (getter)
  --[name]-------------[arg]---[ret]---[note]------------------------
    mrbc_hash_get	*K	T	Data remains in the container
    mrbc_hash_remove	*K	T	Data does not remain in the container
    mrbc_hash_i_next		*T	Data remains in the container
*/



//================================================================
/*! constructor

  @param  vm	pointer to VM.
  @param  size	initial size
  @return 	hash object
*/
mrbc_value mrbc_hash_new(struct VM *vm, int size)
{
  mrbc_value value = {.tt = MRBC_TT_HASH};

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

  h->ref_count = 1;
  h->tt = MRBC_TT_HASH;
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
/*! search key

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @return	pointer to found key or NULL(not found).
*/
mrbc_value * mrbc_hash_search(const mrbc_value *hash, const mrbc_value *key)
{
#ifndef MRBC_HASH_SEARCH_LINER
#define MRBC_HASH_SEARCH_LINER
#endif

#ifdef MRBC_HASH_SEARCH_LINER
  mrbc_value *p1 = hash->hash->data;
  const mrbc_value *p2 = p1 + hash->hash->n_stored;

  while( p1 < p2 ) {
    if( mrbc_compare(p1, key) == 0 ) return p1;
    p1 += 2;
  }
  return NULL;
#endif

#ifdef MRBC_HASH_SEARCH_LINER_ITERATOR
  mrbc_hash_iterator ite = mrbc_hash_iterator_new(hash);
  while( mrbc_hash_i_has_next(&ite) ) {
    mrbc_value *v = mrbc_hash_i_next(&ite);
    if( mrbc_compare( v, key ) == 0 ) return v;
  }
  return NULL;
#endif
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
    mrbc_dec_ref_counter(v);
    *v = *key;
    mrbc_dec_ref_counter(++v);
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
mrbc_value mrbc_hash_get(mrbc_value *hash, mrbc_value *key)
{
  mrbc_value *v = mrbc_hash_search(hash, key);
  return v ? *++v : mrbc_nil_value();
}


//================================================================
/*! remove a data

  @param  hash	pointer to target hash
  @param  key	pointer to key value
  @return	removed data or Nil
*/
mrbc_value mrbc_hash_remove(mrbc_value *hash, mrbc_value *key)
{
  mrbc_value *v = mrbc_hash_search(hash, key);
  if( v == NULL ) return mrbc_nil_value();

  mrbc_dec_ref_counter(v);	// key
  mrbc_value val = v[1];		// value

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
  int i;
  for( i = 0; i < mrbc_hash_size(v1); i++, d1++ ) {
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
    mrbc_dup(p1++);
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
    return;	// raise ArgumentError.
  }

  mrbc_value val = mrbc_hash_get(&v[0], &v[1]);
  mrbc_dup(&val);
  SET_RETURN(val);
}


//================================================================
/*! (operator) []=
*/
static void c_hash_set(struct VM *vm, mrbc_value v[], int argc)
{
  if( argc != 2 ) {
    return;	// raise ArgumentError.
  }

  mrbc_value *v1 = &GET_ARG(1);
  mrbc_value *v2 = &GET_ARG(2);
  mrbc_hash_set(v, v1, v2);
  v1->tt = MRBC_TT_EMPTY;
  v2->tt = MRBC_TT_EMPTY;
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
      mrbc_dup( &kv[0] );
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
    mrbc_dup(key);
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
    mrbc_dup( &kv[0] );
    mrbc_dup( &kv[1] );
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
    mrbc_dup( &kv[0] );
    mrbc_dup( &kv[1] );
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
    mrbc_dup(val);
  }

  SET_RETURN(ret);
}


#if MRBC_USE_STRING
//================================================================
/*! (method) inspect
*/
static void c_hash_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value ret = mrbc_string_new_cstr(vm, "{");
  if( !ret.string ) goto RETURN_NIL;		// ENOMEM

  mrbc_hash_iterator ite = mrbc_hash_iterator_new(v);
  int flag_first = 1;

  while( mrbc_hash_i_has_next(&ite) ) {
    if( !flag_first ) mrbc_string_append_cstr( &ret, ", " );
    flag_first = 0;
    mrbc_value *kv = mrbc_hash_i_next(&ite);

    mrbc_value s1 = mrbc_send( vm, v, argc, &kv[0], "inspect", 0 );
    mrbc_string_append( &ret, &s1 );
    mrbc_string_delete( &s1 );

    mrbc_string_append_cstr( &ret, "=>" );

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




//================================================================
/*! initialize
*/
void mrbc_init_class_hash(struct VM *vm)
{
  mrbc_class_hash = mrbc_define_class(vm, "Hash", mrbc_class_object);

  mrbc_define_method(vm, mrbc_class_hash, "new",	c_hash_new);
  mrbc_define_method(vm, mrbc_class_hash, "[]",		c_hash_get);
  mrbc_define_method(vm, mrbc_class_hash, "[]=",	c_hash_set);
  mrbc_define_method(vm, mrbc_class_hash, "clear",	c_hash_clear);
  mrbc_define_method(vm, mrbc_class_hash, "dup",	c_hash_dup);
  mrbc_define_method(vm, mrbc_class_hash, "delete",	c_hash_delete);
  mrbc_define_method(vm, mrbc_class_hash, "empty?",	c_hash_empty);
  mrbc_define_method(vm, mrbc_class_hash, "has_key?",	c_hash_has_key);
  mrbc_define_method(vm, mrbc_class_hash, "has_value?",	c_hash_has_value);
  mrbc_define_method(vm, mrbc_class_hash, "key",	c_hash_key);
  mrbc_define_method(vm, mrbc_class_hash, "keys",	c_hash_keys);
  mrbc_define_method(vm, mrbc_class_hash, "size",	c_hash_size);
  mrbc_define_method(vm, mrbc_class_hash, "length",	c_hash_size);
  mrbc_define_method(vm, mrbc_class_hash, "count",	c_hash_size);
  mrbc_define_method(vm, mrbc_class_hash, "merge",	c_hash_merge);
  mrbc_define_method(vm, mrbc_class_hash, "merge!",	c_hash_merge_self);
  mrbc_define_method(vm, mrbc_class_hash, "to_h",	c_ineffect);
  mrbc_define_method(vm, mrbc_class_hash, "values",	c_hash_values);
#if MRBC_USE_STRING
  mrbc_define_method(vm, mrbc_class_hash, "inspect",	c_hash_inspect);
  mrbc_define_method(vm, mrbc_class_hash, "to_s",	c_hash_inspect);
#endif

}
