/*! @file
  @brief


  <pre>
  Copyright (C) 2015 Kyushu Institute of Technology.
  Copyright (C) 2015 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#ifndef MRBC_SRC_ERRORCODE_H_
#define MRBC_SRC_ERRORCODE_H_

#ifdef __cplusplus
extern "C" {
#endif


/* no error */
#define NO_ERROR 0

/* unknown */
#define UNKNOWN_ERROR 0x0000ffff

/* load_file */
#define LOAD_FILE_ERROR ((uint32_t)0x0100 << 16)
#define LOAD_FILE_ERROR_MALLOC (LOAD_FILE_ERROR | 0x0001)
#define LOAD_FILE_ERROR_NOFILE (LOAD_FILE_ERROR | 0x0002)

#define LOAD_FILE_HEADER_ERROR ((uint32_t)0x0101 << 16)
#define LOAD_FILE_HEADER_ERROR_RITE (LOAD_FILE_HEADER_ERROR) | 0x0001)
#define LOAD_FILE_HEADER_ERROR_VERSION (LOAD_FILE_HEADER_ERROR | 0x0002)
#define LOAD_FILE_HEADER_ERROR_CRC (LOAD_FILE_HEADER_ERROR | 0x0003)
#define LOAD_FILE_HEADER_ERROR_MATZ (LOAD_FILE_HEADER_ERROR | 0x0004)

#define LOAD_FILE_IREP_ERROR ((uint32_t)0x0102 << 16)
#define LOAD_FILE_IREP_ERROR_IREP (LOAD_FILE_IREP_ERROR | 0x0001)
#define LOAD_FILE_IREP_ERROR_VERSION (LOAD_FILE_IREP_ERROR | 0x0002)
#define LOAD_FILE_IREP_ERROR_ALLOCATION (LOAD_FILE_IREP_ERROR | 0x0003)

/* VM execution */
#define VM_EXEC_ERROR ((uint32_t)0x1000 << 16)
#define VM_EXEC_STATIC_OVWEFLOW_VM (VM_EXEC_ERROR | 0x0001)
#define VM_EXEC_STATIC_OVWEFLOW_IREP (VM_EXEC_ERROR | 0x0002)
#define VM_EXEC_STATIC_OVWEFLOW_CALLINFO (VM_EXEC_ERROR | 0x0003)


#ifdef __cplusplus
}
#endif
#endif
