/***********************************************************************************************************************
 * SIBROS TECHNOLOGIES, INC. CONFIDENTIAL
 * Copyright (c) 2018 Sibros Technologies, Inc.
 * All Rights Reserved.
 * NOTICE: All information contained herein is, and remains the property of Sibros Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained herein are proprietary to Sibros Technologies, Inc. and its
 * suppliers and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade secret or
 * copyright law. Dissemination of this information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from Sibros Technologies, Inc.
 **********************************************************************************************************************/

/**
 * @file
 * Enumeration for specifying the format of a firmware file.
 */

#ifndef SIBROS__SL_FILE_TYPES_H
#define SIBROS__SL_FILE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 *
 *                                                  I N C L U D E S
 *
 **********************************************************************************************************************/
/* Standard Includes */

/* External Includes */

/* Module Includes */

/***********************************************************************************************************************
 *
 *                                                   D E F I N E S
 *
 **********************************************************************************************************************/

#ifndef SL_FILE__PATH_BUFFER_SIZE
#define SL_FILE__PATH_BUFFER_SIZE 512U
#endif

/***********************************************************************************************************************
 *
 *                                                  T Y P E D E F S
 *
 **********************************************************************************************************************/

typedef struct {
  /* The size of this array is picked arbitrarily. PATH_MAX exists however it is only for POSIX systems and does not
   * seem to be part of the POSIX standard so it would not be portable.*/
  char cstring[SL_FILE__PATH_BUFFER_SIZE];
} sl_file__path_buffer_s;

typedef struct {
  char cstring[64U];
} sl_file__name_buffer_s;

typedef enum {
  SL_FILE__TYPE_HEX = 0,
  SL_FILE__TYPE_BIN,
  SL_FILE__TYPE_SREC,
  SL_FILE__TYPE_BBMS,

  SL_FILE__TYPE_COUNT,
  SL_FILE__TYPE_INVALID,
} sl_file__type_e;

/***********************************************************************************************************************
 *
 *                                     F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifdef SIBROS__SL_FILE_TYPES_H */
