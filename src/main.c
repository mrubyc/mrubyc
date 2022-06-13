#include "mrubyc.h"
#include <stdint.h>
#include <genesis.h>

#define int8_t s8

typedef char s8;

static void c_myclass_func(mrb_vm *vm, mrb_value *v, int argc)
{
  char *ptr = mrbc_string_cstr(&v[1]);
  char x = mrbc_integer(v[2]);
  char y = mrbc_integer(v[3]);
  char buf[32];
  snprintf(buf, 31, "%s", ptr);
  VDP_drawText(buf, x, y);
}

void make_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "MegaMrbc", mrbc_class_object);
  mrbc_define_method(vm, cls, "draw_text", c_myclass_func);
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

  return 0;
}

void joypad_loop() {
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
}
