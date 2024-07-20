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
#define SOFTWARE_VERSION       1L   // 6
#define SOFTWARE_SUBVERSION    1L   // 5
#define SOFTWARE_REVISION      5L   // 6    (7.1.5 版本仅是在7.1.4基础上更换充电库和驱动库【测试协议一致性】)

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
