#include "mrubyc.h"
#include <stdint.h>
#include <genesis.h>

#define int8_t s8

typedef char s8;

static void c_megamrbc_draw_text(mrb_vm *vm, mrb_value *v, int argc)
{
  char *ptr = mrbc_string_cstr(&v[1]);
  char x = mrbc_integer(v[2]);
  char y = mrbc_integer(v[3]);
  char buf[32];
  snprintf(buf, 31, "%s", ptr);
  VDP_drawText(buf, x, y);
}

static void c_megamrbc_read_joypad(mrb_vm *vm, mrb_value *v, int argc) {
  uint16_t pad_num = mrbc_integer(v[1]);
  uint16_t value = JOY_readJoypad(pad_num);
  SET_INT_RETURN(value);
}

static void c_megamrbc_wait_vblank(mrb_vm *vm, mrb_value *v, int argc) {
  SYS_doVBlankProcess();
}

void make_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "MegaMrbc", mrbc_class_object);
  mrbc_define_method(vm, cls, "draw_text", c_megamrbc_draw_text);
  mrbc_define_method(vm, cls, "read_joypad", c_megamrbc_read_joypad);
  mrbc_define_method(vm, cls, "wait_vblank", c_megamrbc_wait_vblank);
}

void mrubyc(uint8_t *mrbbuf)
{
  mrbc_init_global();
  mrbc_init_class();

  mrbc_vm *vm = mrbc_vm_open(NULL);
  if( vm == 0 ) {
    return;
  }

  if( mrbc_load_mrb(vm, mrbbuf) != 0 ) {
    return;
  }

  mrbc_vm_begin(vm);

  make_class(vm);

  mrbc_vm_run(vm);
  mrbc_vm_end(vm);
  mrbc_vm_close(vm);
}

extern const uint8_t mrbsrc[];

int main(void) {
  VDP_setScreenWidth320();
  VDP_setHInterrupt(0);
  VDP_setHilightShadow(0);
  PAL_setColor(15+16, 0x0222);
  VDP_setTextPalette(0);
  PAL_setColor(0, 0x0882);

  if( mrbsrc == 0 ) return 1;

  mrubyc( mrbsrc );

  uint16_t i, value;
  while(TRUE)
  {

    for(i=0; i < 2 ; i ++) {
      value = JOY_readJoypad(i);
      char buf[19];
      snprintf(buf, 18, "Joypad state: %03d", value);
      VDP_drawText(buf, 8, 10+i);
    }
    SYS_doVBlankProcess();
  }

  return 0;
}
