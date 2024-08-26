/**
  ******************************************************************************
  * @file
  * @author
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __VERSION_H
#define __VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"

/*
 * V8-Software version information
 */
#define SOFTWARE_VERSION       1L
#define SOFTWARE_SUBVERSION    2L
#define SOFTWARE_REVISION      9L   //  ([1.2.5 (修复四组模块的柔性切换模式下，一组继电器被误断问题)])

#ifdef HMI_T5UIC1_ENABLE
#define SOFTWARE_HMIION       'B'
#else
#define SOFTWARE_HMIION       'A'
#endif

#define SOFTWARE_RCVERSION     0L

/*
 * V8-Hardware version information
 */
#define HARDWARE_VERSION       '8'
#define HARDWARE_SUBVERSION    '8'
#define HARDWARE_REVISION1     '2'
#define HARDWARE_REVISION2     '7'

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
