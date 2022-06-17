#include "mrubyc.h"
#include <stdint.h>
#include <genesis.h>
#include <bmp.h> // drawline

#define int8_t s8

typedef char s8;

// Sprites in code

enum rect_parts { tl, horiz, vert };

const u32 top_left[8] =
  {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00011111,
    0x00011111,
    0x00011000,
    0x00011000,
    0x00011000
  };

const u32 horizontal[8] =
  {
    0x00000000,
    0x00000000,
    0x00000000,
    0x11111111,
    0x11111111,
    0x00000000,
    0x00000000,
    0x00000000
  };

const u32 vertical[8] =
  {
    0x00011000,
    0x00011000,
    0x00011000,
    0x00011000,
    0x00011000,
    0x00011000,
    0x00011000,
    0x00011000
  };

const u32 cursor[8] =
  {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000050,
    0x00000555,
    0x00000050
  };

const u32 tick[8] =
  {
    0x00000000,
    0x00000011,
    0x00000110,
    0x00000110,
    0x01101110,
    0x00111100,
    0x00011000,
    0x00000000
  };

const u32 green[8] =
  {
    0x22222222,
    0x22222222,
    0x22222222,
    0x22222222,
    0x22222222,
    0x22222222,
    0x22222222,
    0x22222222
  };

const u32 yellow[8] =
  {
    0x33333333,
    0x33333333,
    0x33333333,
    0x33333333,
    0x33333333,
    0x33333333,
    0x33333333,
    0x33333333
  };

const u32 grey[8] =
  {
    0x44444444,
    0x44444444,
    0x44444444,
    0x44444444,
    0x44444444,
    0x44444444,
    0x44444444,
    0x44444444
  };

const u32 blank[8] =
  {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
  };

enum other_sprites_enum { csr, tck, grn, ylw, gry, blnk };

void load_tiles() {
  VDP_loadTileData(top_left, TILE_USERINDEX + tl, 1, 0);
  VDP_loadTileData(horizontal, TILE_USERINDEX + horiz, 1, 0);
  VDP_loadTileData(vertical, TILE_USERINDEX + vert, 1, 0);

  VDP_loadTileData( cursor, TILE_USERINDEX + vert + 1 + csr, 1, 0);
  VDP_loadTileData( tick, TILE_USERINDEX + vert + 1 + tck, 1, 0);
  VDP_loadTileData( green, TILE_USERINDEX + vert + 1 + grn, 1, 0);
  VDP_loadTileData( yellow, TILE_USERINDEX + vert + 1 + ylw, 1, 0);
  VDP_loadTileData( grey, TILE_USERINDEX + vert + 1 + gry, 1, 0);
  VDP_loadTileData( blank, TILE_USERINDEX + vert + 1 + blnk, 1, 0);
}

static void c_megamrbc_draw_text(mrb_vm *vm, mrb_value *v, int argc)
{
  char *ptr = mrbc_string_cstr(&v[1]);
  char x = mrbc_integer(v[2]);
  char y = mrbc_integer(v[3]);
  char buf[32];
  snprintf(buf, 31, "%s", ptr);
  VDP_drawText(buf, x, y);
}

static void c_megamrbc_draw_top_left(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+tl, x, y);
}

static void c_megamrbc_draw_horizontal(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+horiz, x, y);
}

static void c_megamrbc_draw_top_right(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(0,1,0,1, TILE_USERINDEX+tl), x, y);
}

static void c_megamrbc_draw_vertical(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+vert, x, y);
}

static void c_megamrbc_draw_bottom_left(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(0,1,1,0, TILE_USERINDEX+tl), x, y);
}

static void c_megamrbc_draw_bottom_right(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(0,1,1,1, TILE_USERINDEX+tl), x, y);
}

static void c_megamrbc_read_joypad(mrb_vm *vm, mrb_value *v, int argc) {
  uint16_t pad_num = mrbc_integer(v[1]);
  uint16_t value = JOY_readJoypad(pad_num);
  SET_INT_RETURN(value);
}

static void c_megamrbc_wait_vblank(mrb_vm *vm, mrb_value *v, int argc) {
  SYS_doVBlankProcess();
}

static void c_megamrbc_show_cursor(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = x * 8;
  uint16_t y_px = y * 8;
  VDP_setSpriteFull(0, x_px, y_px, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,0,0,TILE_USERINDEX + vert + 1), 1);
  VDP_setSpriteFull(1, x_px+16, y_px, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,0,1,TILE_USERINDEX + vert + 1), 2);
  VDP_setSpriteFull(2, x_px, y_px+16, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,1,0,TILE_USERINDEX + vert + 1), 3);
  VDP_setSprite(3, x_px+16, y_px+16, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,1,1,TILE_USERINDEX + vert + 1));
  VDP_updateSprites(4, 1);
}

static void c_megamrbc_show_tick(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX + vert + 1 + tck, x, y);
}

static void c_megamrbc_draw_green_square(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX + vert + 1 + grn, x, y);
}

static void c_megamrbc_draw_yellow_square(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX + vert + 1 + ylw, x, y);
}

static void c_megamrbc_draw_grey_square(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX + vert + 1 + gry, x, y);
}

static void c_megamrbc_clear_screen(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = 0;
  uint8_t y = 0;
  for(y = 0; y < 28; y++) {
    for(x = 0; x < 40; x++) {
      VDP_setTileMapXY(BG_B, TILE_USERINDEX + vert + 1 + blnk, x, y);
      VDP_setTileMapXY(BG_A, TILE_USERINDEX + vert + 1 + blnk, x, y);
    }
  }
}


void make_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "MegaMrbc", mrbc_class_object);
  mrbc_define_method(vm, cls, "draw_text", c_megamrbc_draw_text);
  mrbc_define_method(vm, cls, "draw_top_left", c_megamrbc_draw_top_left);
  mrbc_define_method(vm, cls, "draw_horizontal", c_megamrbc_draw_horizontal);
  mrbc_define_method(vm, cls, "draw_top_right", c_megamrbc_draw_top_right);
  mrbc_define_method(vm, cls, "draw_vertical", c_megamrbc_draw_vertical);
  mrbc_define_method(vm, cls, "draw_bottom_left", c_megamrbc_draw_bottom_left);
  mrbc_define_method(vm, cls, "draw_bottom_right", c_megamrbc_draw_bottom_right);
  mrbc_define_method(vm, cls, "read_joypad", c_megamrbc_read_joypad);
  mrbc_define_method(vm, cls, "wait_vblank", c_megamrbc_wait_vblank);
  mrbc_define_method(vm, cls, "show_cursor", c_megamrbc_show_cursor);
  mrbc_define_method(vm, cls, "show_tick", c_megamrbc_show_tick);
  mrbc_define_method(vm, cls, "draw_green_square", c_megamrbc_draw_green_square);
  mrbc_define_method(vm, cls, "draw_yellow_square", c_megamrbc_draw_yellow_square);
  mrbc_define_method(vm, cls, "draw_grey_square", c_megamrbc_draw_grey_square);
  mrbc_define_method(vm, cls, "clear_screen", c_megamrbc_clear_screen);
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

void init_screen() {
  // Initialise screen
  VDP_setScreenWidth320();
  VDP_setHInterrupt(0);
  VDP_setHilightShadow(0);
  PAL_setColor(15, 0x0000); // default text colour
  VDP_setTextPalette(0);
  PAL_setColor(0, 0x0EEE);
  VDP_resetSprites();
}

void set_up_colours() {
  VDP_setPaletteColor(1, RGB24_TO_VDPCOLOR(0x222222)); // dark grey
  VDP_setPaletteColor(2, RGB24_TO_VDPCOLOR(0x6AAA64)); // green
  VDP_setPaletteColor(3, RGB24_TO_VDPCOLOR(0xC9B458)); // yellow
  VDP_setPaletteColor(4, RGB24_TO_VDPCOLOR(0x888C8E)); // grey
  VDP_setPaletteColor(5, RGB24_TO_VDPCOLOR(0x2222AA)); // blue
}

int main(void) {
  init_screen();

  set_up_colours();
  load_tiles();

  if( mrbsrc == 0 ) return 1;

  mrubyc( mrbsrc );

  return 0;
}
