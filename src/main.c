#include "mrubyc.h"
#include <stdint.h>
#include <genesis.h>
#include <bmp.h> // drawline

#define int8_t s8

typedef char s8;

// Sprites for rectangles

enum rect_parts { tl, tc, tr, l, c, r, bl, bc, br };

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

const u32 top_centre[8] =
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

const u32 top_right[8] =
  {
    0x00000000,
    0x00000000,
    0x00000000,
    0x11111000,
    0x11111000,
    0x00011000,
    0x00011000,
    0x00011000
  };

const u32 left[8] =
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

const u32 centre[8] =
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

const u32 *right = left;

const u32 bottom_left[8] =
  {
    0x00011000,
    0x00011000,
    0x00011000,
    0x00011111,
    0x00011111,
    0x00000000,
    0x00000000,
    0x00000000
  };

const u32 *bottom_centre = top_centre;

const u32 bottom_right[8] =
  {
    0x00011000,
    0x00011000,
    0x00011000,
    0x11111000,
    0x11111000,
    0x00000000,
    0x00000000,
    0x00000000
  };

const u32 cursor[8] =
  {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000055,
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

void load_tiles() {
  VDP_loadTileData(top_left, TILE_USERINDEX + tl, 1, 0);
  VDP_loadTileData(top_centre, TILE_USERINDEX + tc, 1, 0);
  VDP_loadTileData(top_right, TILE_USERINDEX + tr, 1, 0);
  VDP_loadTileData(left, TILE_USERINDEX + l, 1, 0);
  VDP_loadTileData(centre, TILE_USERINDEX + c, 1, 0);
  VDP_loadTileData(right, TILE_USERINDEX + r, 1, 0);
  VDP_loadTileData(bottom_left, TILE_USERINDEX + bl, 1, 0);
  VDP_loadTileData(bottom_centre, TILE_USERINDEX + bc, 1, 0);
  VDP_loadTileData(bottom_right, TILE_USERINDEX + br, 1, 0);

  // this one is after the parts...
  VDP_loadTileData( cursor, TILE_USERINDEX + br + 1, 1, 0);
  VDP_loadTileData( tick, TILE_USERINDEX + br + 2, 1, 0);
  VDP_loadTileData( green, TILE_USERINDEX + br + 3, 1, 0);
  VDP_loadTileData( yellow, TILE_USERINDEX + br + 4, 1, 0);
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

static void c_megamrbc_draw_top_centre(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+tc, x, y);
}

static void c_megamrbc_draw_top_right(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+tr, x, y);
}

static void c_megamrbc_draw_left(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+l, x, y);
}

static void c_megamrbc_draw_right(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+r, x, y);
}

static void c_megamrbc_draw_bottom_left(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+bl, x, y);
}

static void c_megamrbc_draw_bottom_centre(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+bc, x, y);
}

static void c_megamrbc_draw_bottom_right(mrb_vm *vm, mrb_value *v, int argc)
{
  char x = mrbc_integer(v[1]);
  char y = mrbc_integer(v[2]);
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+br, x, y);
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
  VDP_setSpriteFull(0, x_px, y_px, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,0,0,TILE_USERINDEX + br + 1), 1);
  VDP_setSpriteFull(1, x_px+16, y_px, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,0,1,TILE_USERINDEX + br + 1), 2);
  VDP_setSpriteFull(2, x_px, y_px+16, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,1,0,TILE_USERINDEX + br + 1), 3);
  VDP_setSprite(3, x_px+16, y_px+16, SPRITE_SIZE(1,1), TILE_ATTR_FULL(0,1,1,1,TILE_USERINDEX + br + 1));
  VDP_updateSprites(4, 1);
}

static void c_megamrbc_show_tick(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+br+2, x, y);
}

static void c_megamrbc_draw_green_square(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+br+3, x, y);
}

static void c_megamrbc_draw_yellow_square(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t x = mrbc_integer(v[1]);
  uint8_t y = mrbc_integer(v[2]);
  uint16_t x_px = (x + 1) * 8;
  uint16_t y_px = (y + 1) * 8;
  VDP_setTileMapXY(BG_B, TILE_USERINDEX+br+4, x, y);
}

void make_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "MegaMrbc", mrbc_class_object);
  mrbc_define_method(vm, cls, "draw_text", c_megamrbc_draw_text);
  mrbc_define_method(vm, cls, "draw_top_left", c_megamrbc_draw_top_left);
  mrbc_define_method(vm, cls, "draw_top_centre", c_megamrbc_draw_top_centre);
  mrbc_define_method(vm, cls, "draw_top_right", c_megamrbc_draw_top_right);
  mrbc_define_method(vm, cls, "draw_left", c_megamrbc_draw_left);
  mrbc_define_method(vm, cls, "draw_right", c_megamrbc_draw_right);
  mrbc_define_method(vm, cls, "draw_bottom_left", c_megamrbc_draw_bottom_left);
  mrbc_define_method(vm, cls, "draw_bottom_centre", c_megamrbc_draw_bottom_centre);
  mrbc_define_method(vm, cls, "draw_bottom_right", c_megamrbc_draw_bottom_right);
  mrbc_define_method(vm, cls, "read_joypad", c_megamrbc_read_joypad);
  mrbc_define_method(vm, cls, "wait_vblank", c_megamrbc_wait_vblank);
  mrbc_define_method(vm, cls, "show_cursor", c_megamrbc_show_cursor);
  mrbc_define_method(vm, cls, "show_tick", c_megamrbc_show_tick);
  mrbc_define_method(vm, cls, "draw_green_square", c_megamrbc_draw_green_square);
  mrbc_define_method(vm, cls, "draw_yellow_square", c_megamrbc_draw_yellow_square);
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

void set_up_colours() {
  VDP_setPaletteColor(1, RGB24_TO_VDPCOLOR(0x222222)); // dark grey
  VDP_setPaletteColor(2, RGB24_TO_VDPCOLOR(0x6AAA64)); // green
  VDP_setPaletteColor(3, RGB24_TO_VDPCOLOR(0xC9B458)); // yellow
  VDP_setPaletteColor(4, RGB24_TO_VDPCOLOR(0x787C7E)); // grey
  VDP_setPaletteColor(5, RGB24_TO_VDPCOLOR(0x2222AA)); // blue
}

int main(void) {
  // Initialise screen
  VDP_setScreenWidth320();
  VDP_setHInterrupt(0);
  VDP_setHilightShadow(0);
  PAL_setColor(15, 0x0222); // default text colour
  VDP_setTextPalette(0);
  PAL_setColor(0, 0x0EEE);
  VDP_resetSprites();

  set_up_colours();
  load_tiles();

  if( mrbsrc == 0 ) return 1;

  mrubyc( mrbsrc );

  return 0;
}
