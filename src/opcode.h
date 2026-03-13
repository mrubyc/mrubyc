/*! @file
  @brief
  Define operation codes and associated macros.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#ifndef MRBC_SRC_OPCODE_H_
#define MRBC_SRC_OPCODE_H_

//@cond
#include <stdint.h>
//@endcond

#ifdef __cplusplus
extern "C" {
#endif


#define FETCH_Z() (void)0

#if defined(MRBC_SUPPORT_OP_EXT)
#define FETCH_B() \
  unsigned int a; \
  a = *vm->inst++; if( ext & 1 ) a = a << 8 | *vm->inst++; \
  (void)a

#define FETCH_BB() \
  unsigned int a, b; \
  a = *vm->inst++; if( ext & 1 ) a = a << 8 | *vm->inst++; \
  b = *vm->inst++; if( ext & 2 ) b = b << 8 | *vm->inst++; \
  (void)a, (void)b

#define FETCH_BBB() \
  unsigned int a, b, c; \
  a = *vm->inst++; if( ext & 1 ) a = a << 8 | *vm->inst++; \
  b = *vm->inst++; if( ext & 2 ) b = b << 8 | *vm->inst++; \
  c = *vm->inst++; \
  (void)a, (void)b, (void)c

#define FETCH_BS() \
  unsigned int a, b; \
  a = *vm->inst++; if( ext & 1 ) a = a << 8 | *vm->inst++; \
  b = *vm->inst++; b = b << 8 | *vm->inst++; \
  (void)a, (void)b

#define FETCH_BSS() \
  unsigned int a, b, c; \
  a = *vm->inst++; if( ext & 1 ) a = a << 8 | *vm->inst++; \
  b = *vm->inst++; b = b << 8 | *vm->inst++; \
  c = *vm->inst++; c = c << 8 | *vm->inst++; \
  (void)a, (void)b, (void)c

#else
#define FETCH_B() \
  unsigned int a; \
  a = *vm->inst++; \
  (void)a

#define FETCH_BB() \
  unsigned int a, b; \
  a = *vm->inst++; \
  b = *vm->inst++; \
  (void)a, (void)b

#define FETCH_BBB() \
  unsigned int a, b, c; \
  a = *vm->inst++; \
  b = *vm->inst++; \
  c = *vm->inst++; \
  (void)a, (void)b, (void)c

#define FETCH_BS() \
  unsigned int a, b; \
  a = *vm->inst++; \
  b = *vm->inst++; b = b << 8 | *vm->inst++; \
  (void)a, (void)b

#define FETCH_BSS() \
  unsigned int a, b, c; \
  a = *vm->inst++; \
  b = *vm->inst++; b = b << 8 | *vm->inst++; \
  c = *vm->inst++; c = c << 8 | *vm->inst++; \
  (void)a, (void)b, (void)c

#endif // defined(MRBC_SUPPORT_OP_EXT)

#define FETCH_S() \
  unsigned int a; \
  a = *vm->inst++; a = a << 8 | *vm->inst++; \
  (void)a

#define FETCH_W() \
  uint32_t a; \
  a = *vm->inst++; a = a << 8 | *vm->inst++; a = a << 8 | *vm->inst++; \
  (void)a


//================================================================
/*!@brief
  Operation codes.

 operand types:
   Z: no operand
   B: 8bit	   (a)
   BB: 8+8bit	   (a,b)
   BBB: 8+8+8bit   (a,b,c)
   BS: 8+16bit	   (a,b)
   BSS: 8+16+16bit (a,b,c)
   S: 16bit	   (a)
   W: 24bit	   (a)
*/
enum OPCODE {
/*-----------------------------------------------------------------------
  operation code        operands  semantics
------------------------------------------------------------------------*/
  OP_NOP        = 0x00, //!< Z    no operation
  OP_MOVE       = 0x01, //!< BB   R[a] = R[b]
  OP_LOADL      = 0x02, //!< BB   R[a] = Pool[b]
  OP_LOADI8     = 0x03, //!< BB   R[a] = mrb_int(b)
  OP_LOADINEG   = 0x04, //!< BB   R[a] = mrb_int(-b)
  OP_LOADI__1   = 0x05, //!< B    R[a] = mrb_int(-1)
  OP_LOADI_0    = 0x06, //!< B    R[a] = mrb_int(0)
  OP_LOADI_1    = 0x07, //!< B    R[a] = mrb_int(1)
  OP_LOADI_2    = 0x08, //!< B    R[a] = mrb_int(2)
  OP_LOADI_3    = 0x09, //!< B    R[a] = mrb_int(3)
  OP_LOADI_4    = 0x0A, //!< B    R[a] = mrb_int(4)
  OP_LOADI_5    = 0x0B, //!< B    R[a] = mrb_int(5)
  OP_LOADI_6    = 0x0C, //!< B    R[a] = mrb_int(6)
  OP_LOADI_7    = 0x0D, //!< B    R[a] = mrb_int(7)
  OP_LOADI16    = 0x0E, //!< BS   R[a] = mrb_int(b)
  OP_LOADI32    = 0x0F, //!< BSS  R[a] = mrb_int((b<<16)+c)
  OP_LOADSYM    = 0x10, //!< BB   R[a] = Syms[b]
  OP_LOADNIL    = 0x11, //!< B    R[a] = nil
  OP_LOADSELF   = 0x12, //!< B    R[a] = self
  OP_LOADTRUE   = 0x13, //!< B    R[a] = true
  OP_LOADFALSE  = 0x14, //!< B    R[a] = false
  OP_GETGV      = 0x15, //!< BB   R[a] = getglobal(Syms[b])
  OP_SETGV      = 0x16, //!< BB   setglobal(Syms[b], R[a])
  OP_GETSV      = 0x17, //!< BB   R[a] = Special[Syms[b]]
  OP_SETSV      = 0x18, //!< BB   Special[Syms[b]] = R[a]
  OP_GETIV      = 0x19, //!< BB   R[a] = ivget(Syms[b])
  OP_SETIV      = 0x1A, //!< BB   ivset(Syms[b],R[a])
  OP_GETCV      = 0x1B, //!< BB   R[a] = cvget(Syms[b])
  OP_SETCV      = 0x1C, //!< BB   cvset(Syms[b],R[a])
  OP_GETCONST   = 0x1D, //!< BB   R[a] = constget(Syms[b])
  OP_SETCONST   = 0x1E, //!< BB   constset(Syms[b],R[a])
  OP_GETMCNST   = 0x1F, //!< BB   R[a] = R[a]::Syms[b]
  OP_SETMCNST   = 0x20, //!< BB   R[a+1]::Syms[b] = R[a]
  OP_GETUPVAR   = 0x21, //!< BBB  R[a] = uvget(b,c)
  OP_SETUPVAR   = 0x22, //!< BBB  uvset(b,c,R[a])
  OP_GETIDX     = 0x23, //!< B    R[a] = R[a][R[a+1]]
  OP_GETIDX0    = 0x24, //!< BB   R[a] = R[b][0]; a+1 for method call
  OP_SETIDX     = 0x25, //!< B    R[a][R[a+1]] = R[a+2]
  OP_JMP        = 0x26, //!< S    pc+=a
  OP_JMPIF      = 0x27, //!< BS   if R[a] pc+=b
  OP_JMPNOT     = 0x28, //!< BS   if !R[a] pc+=b
  OP_JMPNIL     = 0x29, //!< BS   if R[a]==nil pc+=b
  OP_JMPUW      = 0x2A, //!< S    unwind_and_jump_to(a)
  OP_EXCEPT     = 0x2B, //!< B    R[a] = exc
  OP_RESCUE     = 0x2C, //!< BB   R[b] = R[a].isa?(R[b])
  OP_RAISEIF    = 0x2D, //!< B    raise(R[a]) if R[a]
  OP_MATCHERR   = 0x2E, //!< B    raise NoMatchingPatternError unless R[a]
  OP_SSEND      = 0x2F, //!< BBB  R[a] = self.send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..) (c=n|k<<4)
  OP_SSEND0     = 0x30, //!< BB   R[a] = self.send(Syms[b]) (no args)
  OP_SSENDB     = 0x31, //!< BBB  R[a] = self.send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..,&R[a+n+2k+1])
  OP_SEND       = 0x32, //!< BBB  R[a] = R[a].send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..) (c=n|k<<4)
  OP_SEND0      = 0x33, //!< BB   R[a] = R[a].send(Syms[b]) (no args)
  OP_SENDB      = 0x34, //!< BBB  R[a] = R[a].send(Syms[b],R[a+1]..,R[a+n+1]:R[a+n+2]..,&R[a+n+2k+1])
  OP_CALL       = 0x35, //!< Z    self.call(*, **, &) (But overlay the current call frame; tailcall)
  OP_BLKCALL    = 0x36, //!< BB   R[a] = R[a].call(R[a+1],... ,R[a+b]); direct block call
  OP_SUPER      = 0x37, //!< BB   R[a] = super(R[a+1],... ,R[a+b+1])
  OP_ARGARY     = 0x38, //!< BS   R[a] = argument array (16=m5:r1:m5:d1:lv4)
  OP_ENTER      = 0x39, //!< W    arg setup according to flags (24=n1:m5:o5:r1:m5:k5:d1:b1)
  OP_KEY_P      = 0x3A, //!< BB   R[a] = kdict.key?(Syms[b])
  OP_KEYEND     = 0x3B, //!< Z    raise unless kdict.empty?
  OP_KARG       = 0x3C, //!< BB   R[a] = kdict[Syms[b]]; kdict.delete(Syms[b])
  OP_RETURN     = 0x3D, //!< B    return R[a] (normal)
  OP_RETURN_BLK = 0x3E, //!< B    return R[a] (in-block return)
  OP_RETSELF    = 0x3F, //!< Z    return self
  OP_RETNIL     = 0x40, //!< Z    return nil
  OP_RETTRUE    = 0x41, //!< Z    return true
  OP_RETFALSE   = 0x42, //!< Z    return false
  OP_BREAK      = 0x43, //!< B    break R[a]
  OP_BLKPUSH    = 0x44, //!< BS   R[a] = block (16=m5:r1:m5:d1:lv4)
  OP_ADD        = 0x45, //!< B    R[a] = R[a]+R[a+1]
  OP_ADDI       = 0x46, //!< BB   R[a] = R[a]+mrb_int(b)
  OP_SUB        = 0x47, //!< B    R[a] = R[a]-R[a+1]
  OP_SUBI       = 0x48, //!< BB   R[a] = R[a]-mrb_int(b)
  OP_ADDILV     = 0x49, //!< BBB  R[a] = R[a]+mrb_int(c); R[b],R[b+1] for method call
  OP_SUBILV     = 0x4A, //!< BBB  R[a] = R[a]-mrb_int(c); R[b],R[b+1] for method call
  OP_MUL        = 0x4B, //!< B    R[a] = R[a]*R[a+1]
  OP_DIV        = 0x4C, //!< B    R[a] = R[a]/R[a+1]
  OP_EQ         = 0x4D, //!< B    R[a] = R[a]==R[a+1]
  OP_LT         = 0x4E, //!< B    R[a] = R[a]<R[a+1]
  OP_LE         = 0x4F, //!< B    R[a] = R[a]<=R[a+1]
  OP_GT         = 0x50, //!< B    R[a] = R[a]>R[a+1]
  OP_GE         = 0x51, //!< B    R[a] = R[a]>=R[a+1]
  OP_ARRAY      = 0x52, //!< BB   R[a] = ary_new(R[a],R[a+1]..R[a+b])
  OP_ARRAY2     = 0x53, //!< BBB  R[a] = ary_new(R[b],R[b+1]..R[b+c])
  OP_ARYCAT     = 0x54, //!< B    ary_cat(R[a],R[a+1])
  OP_ARYPUSH    = 0x55, //!< BB   ary_push(R[a],R[a+1]..R[a+b])
  OP_ARYSPLAT   = 0x56, //!< B    R[a] = ary_splat(R[a])
  OP_AREF       = 0x57, //!< BBB  R[a] = R[b][c]
  OP_ASET       = 0x58, //!< BBB  R[b][c] = R[a]
  OP_APOST      = 0x59, //!< BBB  *R[a],R[a+1]..R[a+c] = R[a][b..]
  OP_INTERN     = 0x5A, //!< B    R[a] = intern(R[a])
  OP_SYMBOL     = 0x5B, //!< BB   R[a] = intern(Pool[b])
  OP_STRING     = 0x5C, //!< BB   R[a] = str_dup(Pool[b])
  OP_STRCAT     = 0x5D, //!< B    str_cat(R[a],R[a+1])
  OP_HASH       = 0x5E, //!< BB   R[a] = hash_new(R[a],R[a+1]..R[a+b*2-1])
  OP_HASHADD    = 0x5F, //!< BB   hash_push(R[a],R[a+1]..R[a+b*2])
  OP_HASHCAT    = 0x60, //!< B    R[a] = hash_cat(R[a],R[a+1])
  OP_LAMBDA     = 0x61, //!< BB   R[a] = lambda(Irep[b],L_LAMBDA)
  OP_BLOCK      = 0x62, //!< BB   R[a] = lambda(Irep[b],L_BLOCK)
  OP_METHOD     = 0x63, //!< BB   R[a] = lambda(Irep[b],L_METHOD)
  OP_RANGE_INC  = 0x64, //!< B    R[a] = range_new(R[a],R[a+1],FALSE)
  OP_RANGE_EXC  = 0x65, //!< B    R[a] = range_new(R[a],R[a+1],TRUE)
  OP_OCLASS     = 0x66, //!< B    R[a] = ::Object
  OP_CLASS      = 0x67, //!< BB   R[a] = newclass(R[a],Syms[b],R[a+1])
  OP_MODULE     = 0x68, //!< BB   R[a] = newmodule(R[a],Syms[b])
  OP_EXEC       = 0x69, //!< BB   R[a] = blockexec(R[a],Irep[b])
  OP_DEF        = 0x6A, //!< BB   R[a].newmethod(Syms[b],R[a+1]); R[a] = Syms[b]
  OP_TDEF       = 0x6B, //!< BBB  target_class.newmethod(Syms[b],Irep[c]); R[a] = Syms[b]
  OP_SDEF       = 0x6C, //!< BBB  R[a].singleton_class.newmethod(Syms[b],Irep[c]); R[a] = Syms[b]
  OP_ALIAS      = 0x6D, //!< BB   alias_method(target_class,Syms[a],Syms[b])
  OP_UNDEF      = 0x6E, //!< B    undef_method(target_class,Syms[a])
  OP_SCLASS     = 0x6F, //!< B    R[a] = R[a].singleton_class
  OP_TCLASS     = 0x70, //!< B    R[a] = target_class
  OP_DEBUG      = 0x71, //!< BBB  print a,b,c
  OP_ERR        = 0x72, //!< B    raise(LocalJumpError, Pool[a])
  OP_EXT1       = 0x73, //!< Z    make 1st operand (a) 16bit
  OP_EXT2       = 0x74, //!< Z    make 2nd operand (b) 16bit
  OP_EXT3       = 0x75, //!< Z    make 1st and 2nd operands 16bit
  OP_STOP       = 0x76, //!< Z    stop VM
};


#ifdef __cplusplus
}
#endif
#endif
