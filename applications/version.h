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

#include "app_ofsm.h"

/*
 * V8-Software version information
 */
#define SOFTWARE_VERSION       1L
#define SOFTWARE_SUBVERSION    5L
#define SOFTWARE_REVISION      9L   /*  ([1.2.5 (修复四组模块的柔性切换模式下，一组继电器被误断问题)]
                                         [1.3.7 为能佳新板子将能佳默认IP、端口、二维码前缀写入flash]
                                         [1.3.8为能佳修改并充时CCS输出电流值]
                                         [1.4.1 为模块离线或故障也不踢出，去掉CCS与BCS电流比较，保留CCS与BCL电流比较]
                                         [1.4.2 为模块离线或故障也不踢出，去掉CCS与BCS电流比较，去掉CCS与BCL电流比较]
                                         [1.4.3 为模块离线或故障则踢出，CCS电流与模块电流比较，模块电流与BCL电流比较，仅用于测试]
                                         [1.4.8 为打印开log查灯全亮和费率不显示问题，最后提交日期：20241016 16:03]
                                         [1.4.9 为打印开log费率不显示问题(增加打印信息)]
                                         [1.5.1 为打开电流比较，测试版本]) */

#ifdef APP_USING_DOUBLEGUN
#define SOFTWARE_MODULE                 "YKC7103"          /* 软件型号 */
//#define SOFTWARE_VERSION       1L
//#define SOFTWARE_SUBVERSION    5L
//#define SOFTWARE_REVISION      8L

///* offline billing */
#define SOFTWARE_VERSION       1L
#define SOFTWARE_SUBVERSION    5L
#define SOFTWARE_REVISION      9L

#else
#define SOFTWARE_MODULE                 "YKC7101"          /* 软件型号 */
#define SOFTWARE_VERSION       1L
#define SOFTWARE_SUBVERSION    5L
#define SOFTWARE_REVISION      8L

///* offline billing */
//#define SOFTWARE_VERSION       1L
//#define SOFTWARE_SUBVERSION    5L
//#define SOFTWARE_REVISION      8L

#endif /* APP_USING_DOUBLEGUN */

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
