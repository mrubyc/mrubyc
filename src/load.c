/*! @file
  @brief
  mruby bytecode loader.

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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
//@endcond

/***** Local headers ********************************************************/
#include "mrubyc.h"

/***** Constat values *******************************************************/
// for mrb file structure.
static const char RITE[4] = "RITE";
static const char RITE_VERSION[4] = "0300";
static const int SIZE_RITE_BINARY_HEADER = 20;
static const int SIZE_RITE_SECTION_HEADER = 12;
static const int SIZE_RITE_CATCH_HANDLER = 13;
static const char IREP[4] = "IREP";
static const char END[4] = "END\0";


/*! IREP TT */
enum irep_pool_type {
  IREP_TT_STR   = 0,	// string (need free)
  IREP_TT_SSTR  = 2,	// string (static)
  IREP_TT_INT32 = 1,	// 32bit integer
  IREP_TT_INT64 = 3,	// 64bit integer
  IREP_TT_FLOAT = 5,	// float (double/float)
};


/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/

//================================================================
/*! Parse header section.

  @param  vm    A pointer to VM.
  @param  bin	A pointer to RITE header.
  @return int	zero if no error.

  <pre>
  Structure
   "RITE"     identifier
   "03"       major version
   "00"       minor version
   0000_0000  total size
   "MATZ"     compiler name
   "0000"     compiler version
  </pre>
*/
static int load_header(struct VM *vm, const uint8_t *bin)
{
  if( memcmp(bin, RITE, sizeof(RITE)) != 0 ) {
    mrbc_raise( vm, MRBC_CLASS(Exception), "Illegal bytecode");
    return -1;
  }
  bin += sizeof(RITE);

  if( memcmp(bin, RITE_VERSION, sizeof(RITE_VERSION)) != 0 ) {
    mrbc_raise( vm, MRBC_CLASS(Exception), "Bytecode version mismatch");
    return -1;
  }

  /* Ignore others. */

  return 0;
}


//================================================================
/*! read one irep section.

  @param  vm	A pointer to VM.
  @param  bin	A pointer to RITE ISEQ.
  @param  len	Returns the parsed length.
  @return	Pointer to allocated mrbc_irep or NULL

  <pre>
   (loop n of child irep below)
   0000_0000	record size
   0000		n of local variable
   0000		n of register
   0000		n of child irep
   0000		n of catch handler
   0000_0000	n of byte code  (ISEQ BLOCK)
   ...		byte codes

   (CATCH HANDLER 13bytes)
   00		type
   0000_0000	begin
   0000_0000	end
   0000_0000	target

   (POOL BLOCK)
   0000		n of pool
   (loop n of pool)
     00		type
     ...	pool data

   (SYMS BLOCK)
   0000		n of symbol
   (loop n of symbol)
     0000	length
     ...	symbol data
  </pre>
*/
static mrbc_irep * load_irep_1(struct VM *vm, const uint8_t *bin, int *len)
{
  mrbc_irep irep;
  const uint8_t *p = bin + 4;	// 4 = skip record size.

#if defined(MRBC_DEBUG)
  irep.obj_mark_[0] = 'I';	// set "IR"
  irep.obj_mark_[1] = 'R';
#endif

  irep.ref_count = 0;
#if defined(MRBC_DEBUG)
  irep.nlocals = bin_to_uint16(p);	p += 2;
#else
  p += 2;	// skip nlocals
#endif
  irep.nregs = bin_to_uint16(p);	p += 2;
  irep.rlen = bin_to_uint16(p);		p += 2;
  irep.clen = bin_to_uint16(p);		p += 2;
  uint32_t ilen = bin_to_uint32(p);	p += 4;
  if( ilen > 0xffff ) goto ERROR_TOO_LARGE;
  irep.ilen = ilen;
  irep.inst = p;

  // POOL block
  p += irep.ilen + SIZE_RITE_CATCH_HANDLER * irep.clen;
  irep.pool = p;
  uint16_t plen = bin_to_uint16(p);	p += 2;

  // skip pool
  for( int i = 0; i < plen; i++ ) {
    int siz = 0;
    int tt = *p++;

    switch( tt ) {
    case IREP_TT_STR:
    case IREP_TT_SSTR:	siz = bin_to_uint16(p) + 3;	break;
    case IREP_TT_INT32:	siz = 4;	break;
    case IREP_TT_INT64:
#if !defined(MRBC_INT64)
      mrbc_raise(vm, MRBC_CLASS(NotImplementedError), "Unsupported int64 (set MRBC_INT64 in vm_config)");
#endif
    case IREP_TT_FLOAT:	siz = 8;	break;
    default:
      mrbc_raisef(vm, MRBC_CLASS(Exception), "Not support such type (IREP_TT=%d)", tt);
      return NULL;
    }
    p += siz;
  }

  // num of symbols, offset of tbl_ireps.
  uint16_t slen = bin_to_uint16(p);	p += 2;
  uint32_t siz = sizeof(mrbc_sym) * slen;
  if( siz > 0xffff ) goto ERROR_TOO_LARGE;
  irep.ofs_pools = siz;

  siz += sizeof(uint16_t) * plen;
  if( siz > 0xffff ) goto ERROR_TOO_LARGE;
  siz += (-siz & 0x03);	// padding. 32bit align.
  irep.ofs_ireps = siz;

#if defined(MRBC_DEBUG)
  irep.plen = plen;
  irep.slen = slen;
#endif

  // allocate new irep
  siz = sizeof(mrbc_irep) + siz + sizeof(mrbc_irep*) * irep.rlen;
  mrbc_irep *p_irep = mrbc_raw_alloc( siz );
  if( !p_irep ) {	// ENOMEM
    mrbc_raise(vm, MRBC_CLASS(NoMemoryError),0);
    return NULL;
  }
  *p_irep = irep;

  // make a symbol ID table. (tbl_syms[slen])
  mrbc_sym *tbl_syms = mrbc_irep_tbl_syms(p_irep);
  for( int i = 0; i < slen; i++ ) {
    int siz = bin_to_uint16(p) + 1;	p += 2;
    char *sym_str;
    if (vm->flag_permanence == 1) {
      sym_str = mrbc_raw_alloc_no_free(siz);
      if( !sym_str ) return NULL;  // ENOMEM
      memcpy(sym_str, p, siz);
    } else {
      sym_str = (char *)p;
    }
    mrbc_sym sym = mrbc_str_to_symid( sym_str );
    if( sym < 0 ) {
      mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow MAX_SYMBOLS_COUNT");
      return NULL;
    }
    *tbl_syms++ = sym;
    p += (siz);
  }

  // make a pool data's offset table. (tbl_pools[plen])
  uint16_t *ofs_pools = mrbc_irep_tbl_pools(p_irep);
  p = p_irep->pool + 2;
  for( int i = 0; i < plen; i++ ) {
    int siz = 0;
    if( (p - irep.pool) > UINT16_MAX ) {
      mrbc_raise(vm, MRBC_CLASS(Exception), "Overflow IREP data offset table");
      return NULL;
    }
    *ofs_pools++ = (uint16_t)(p - irep.pool);
    switch( *p++ ) {
    case IREP_TT_STR:
    case IREP_TT_SSTR:	siz = bin_to_uint16(p) + 3;  break;
    case IREP_TT_INT32:	siz = 4;  break;
    case IREP_TT_INT64:
    case IREP_TT_FLOAT:	siz = 8;  break;
    }
    p += siz;
  }

  // return length
  *len = bin_to_uint32(bin);
  return p_irep;


 ERROR_TOO_LARGE:
  mrbc_raise(vm, MRBC_CLASS(Exception), "Too large IREP size");
  return NULL;
}


//================================================================
/*! Load IREP section.

  @param  vm	A pointer to VM.
  @param  bin	A pointer to RITE ISEQ.
  @param  len	Returns the parsed length.
  @return	Pointer to allocated mrbc_irep or NULL
*/
static mrbc_irep *load_irep(struct VM *vm, const uint8_t *bin, int *len)
{
  int len1;
  mrbc_irep *irep = load_irep_1(vm, bin, &len1);
  if( !irep ) return NULL;
  int total_len = len1;

  mrbc_irep **tbl_ireps = mrbc_irep_tbl_ireps(irep);

  for( int i = 0; i < irep->rlen; i++ ) {
    tbl_ireps[i] = load_irep(vm, bin + total_len, &len1);
    if( ! tbl_ireps[i] ) return NULL;
    total_len += len1;
  }

  if( len ) *len = total_len;
  return irep;
}


/***** Global functions *****************************************************/

//================================================================
/*! Load the VM bytecode. (full .mrb file)

  @param  vm		Pointer to VM.
  @param  bytecode	Pointer to bytecode.
  @return int		zero if no error.
*/
int mrbc_load_mrb(struct VM *vm, const void *bytecode)
{
  const uint8_t *bin = bytecode;

  vm->exception = mrbc_nil_value();
  if( load_header(vm, bin) != 0 ) return -1;

  bin += SIZE_RITE_BINARY_HEADER;

  while( 1 ) {
    if( memcmp(bin, IREP, sizeof(IREP)) == 0 ) {
      if( mrbc_load_irep( vm, bin ) != 0 ) break;

    } else if( memcmp(bin, END, sizeof(END)) == 0 ) {
      break;
    }
    // ignore other section.

    bin += bin_to_uint32(bin+4);	// add section size, to next section.
  }

  return mrbc_israised(vm);
}


//================================================================
/*! Load the IREP section.

  @param  vm		Pointer to VM.
  @param  bytecode	Pointer to IREP section.
  @return int		zero if no error.
*/
int mrbc_load_irep(struct VM *vm, const void *bytecode)
{
  const uint8_t *bin = bytecode;

  vm->top_irep = load_irep( vm, bin + SIZE_RITE_SECTION_HEADER, 0 );
  if( vm->top_irep == NULL ) return -1;

  return mrbc_israised(vm);
}


//================================================================
/*! release mrbc_irep holds memory

  @param  irep	Pointer to allocated mrbc_irep.
*/
void mrbc_irep_free(struct IREP *irep)
{
  // release child ireps.
  for( int i = 0; i < irep->rlen; i++ ) {
    mrbc_irep_free( mrbc_irep_child_irep(irep, i) );
  }

  if( irep->ref_count == 0 ) {
    mrbc_raw_free( irep );
  }
}


//================================================================
/*! get a mrbc_value in irep pool.

  @param  vm		Pointer to VM.
  @param  n		n'th
  @return mrbc_value	value
*/
mrbc_value mrbc_irep_pool_value(struct VM *vm, int n)
{
  const uint8_t *p = mrbc_irep_pool_ptr(vm->cur_irep, n);
  mrbc_value obj;

  int tt = *p++;
  switch( tt ) {
#if MRBC_USE_STRING
  case IREP_TT_STR:
  case IREP_TT_SSTR: {
    int len = bin_to_uint16(p);
    obj = mrbc_string_new( vm, p+2, len );
    break;
  }
#endif

  case IREP_TT_INT32:
    mrbc_set_integer(&obj, bin_to_uint32(p));
    break;

#if MRBC_USE_FLOAT
  case IREP_TT_FLOAT:
    mrbc_set_float(&obj, bin_to_double64(p));
    break;
#endif

#if defined(MRBC_INT64)
  case IREP_TT_INT64:
    mrbc_set_integer(&obj, bin_to_int64(p));
    break;
#endif

  default:
    mrbc_raisef(vm, MRBC_CLASS(Exception), "Not support such type (IREP_TT=%d)", tt);
    mrbc_set_nil(&obj);
  }

  return obj;
}
