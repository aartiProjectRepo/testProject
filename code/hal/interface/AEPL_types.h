/**
 * @file        AEPL_types.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        19 Oct 2023
 * @author      Tejas Kamod
 *
 * @brief       AEPL type header file.
 */

#ifndef __AEPL_TYPES_H__
#define __AEPL_TYPES_H__

typedef unsigned char   U8;
typedef char            S8;
typedef unsigned int    U16;
typedef int             S16;
typedef unsigned long   U32;
typedef long            S32;

typedef float           F32;

typedef enum
{
    FALSE = 0,
    TRUE = !FALSE
}BOOL_T;

#endif