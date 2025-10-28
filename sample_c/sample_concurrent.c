/*
 * This sample program executes multiple mruby/c programs concurrently.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mrubyc.h"

#if !defined(MRBC_MEMORY_SIZE)
#define MRBC_MEMORY_SIZE (1024*60)
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


int main(int argc, char *argv[])
{
  int vm_cnt = argc-1;
  if( vm_cnt < 1 || vm_cnt > MAX_VM_COUNT ) {
    printf("Usage: %s <xxxx.mrb> <xxxx.mrb> ... \n", argv[0]);
    printf("  Maximum number of mrb file: %d\n", MAX_VM_COUNT );
    return 1;
  }

  /*
    start mruby/c with rrt0 scheduler.
  */
  mrbc_init(memory_pool, MRBC_MEMORY_SIZE);

  // create each task.
  for( int i = 0; i < vm_cnt; i++ ) {
    fprintf( stderr, "Loading: '%s'\n", argv[i+1] );
    uint8_t *mrbbuf = load_mrb_file( argv[i+1] );
    if( mrbbuf == 0 ) return 1;
    if( !mrbc_create_task( mrbbuf, NULL ) ) {
      free(mrbbuf);
      return 1;
    }
  }

  // and execute all.
  int ret = mrbc_run();

  return ret == 1 ? 0 : ret;
}
