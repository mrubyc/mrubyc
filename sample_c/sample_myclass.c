/*
 * This sample program executes a mruby/c program.
 * The class "MyClass" is defined before mruby/c program execution.
 *
 * You can add your own classes and methods by this way.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mrubyc.h"

#if !defined(MRBC_MEMORY_SIZE)
#define MRBC_MEMORY_SIZE (1024*40)
#endif
static uint8_t memory_pool[MRBC_MEMORY_SIZE];

uint8_t * load_mrb_file(const char *filename)
{
  FILE *fp = fopen(filename, "rb");

  if( fp == NULL ) {
    fprintf(stderr, "File not found (%s)\n", filename);
    return NULL;
  }

  // get filesize
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // allocate memory
  uint8_t *p = malloc(size);
  if( p != NULL ) {
    fread(p, sizeof(uint8_t), size, fp);
  } else {
    fprintf(stderr, "Memory allocate error.\n");
  }
  fclose(fp);

  return p;
}


// Sample code for making a mruby/c method.
static void c_myclass_method1(mrb_vm *vm, mrb_value v[], int argc)
{
  mrbc_printf("MyClass.method1 argument list.\n");
  for( int i = 0; i <= argc; i++ ) {
    mrbc_printf("  v[%d]=", i);
    mrbc_p( &v[i] );
  }

  // return values. defined in value.h
  SET_INT_RETURN(1);
}


int main(int argc, char *argv[])
{
  if( argc != 2 ) {
    printf("Usage: %s <xxxx.mrb>\n", argv[0]);
    return 1;
  }

  uint8_t *mrbbuf = load_mrb_file( argv[1] );
  if( mrbbuf == 0 ) return 1;

  /*
    start mruby/c with rrt0 scheduler.
  */
  mrbc_init(memory_pool, MRBC_MEMORY_SIZE);


  // Define your own class.
  mrb_class *my_cls = mrbc_define_class(0, "MyClass", MRBC_CLASS(Object));
  mrbc_define_method(0, my_cls, "method1", c_myclass_method1);


  if( !mrbc_create_task(mrbbuf, NULL) ) return 1;
  int ret = mrbc_run();

  /*
    Done
  */
  return ret == 1 ? 0 : ret;
}
