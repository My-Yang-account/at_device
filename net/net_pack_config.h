/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_PACK_CONFIG_H_
#define NET_PACK_NET_PACK_CONFIG_H_

#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "time.h"
#include <rtthread.h>

#define NET_DESIGNATE_REGION                                  /* 变量定义到指定区 */
#define NET_USING_NET_PACK                                    /* 联网 */

#ifdef NET_USING_NET_PACK

#define NET_SYSTEM_GUN_NUMBER                  2              /* 单板枪总数 */

#define NET_INCLUDE_OTA

//#define NET_PACK_USING_THA                                    /* 使用钛享协议 */
#define NET_PACK_USING_YKC                                    /* 使用云快充协议 */
#define NET_PACK_USING_YKC_MONITOR                            /* 使用云快充协议(监控) */
//#define NET_PACK_USING_YCP                                    /* 使用越城协议 */
//#define NET_PACK_USING_YND                                    /* 使用一电、南电协议 */
//#define NET_PACK_USING_XJ                                     /* 使用小桔协议 */
//#define NET_PACK_USING_SL                                     /* 使用阳光乐通协议 */
//#define NET_PACK_USING_SGCC                                   /* 使用国网协议 */

/*************************************************** 钛享协议 **********************************************************/
#ifdef NET_PACK_USING_THA
#define NET_THA_AS_MONITOR                                        /* 钛享平台作为监控平台 */
#define NET_THA_AS_TARGET                                        /* 钛享平台作为目标平台 */

#ifdef NET_THA_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000001  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000001  /* 目标平台ID */
#endif /* NET_THA_AS_MONITOR */

#define NET_THA_PRO_USING_DC                                      /* 钛享协议使用直流部分 */
#define NET_THA_PRO_USING_AC                                      /* 钛享协议使用交流部分 */
#endif /* NET_PACK_USING_THA */
#define NET_THA_PRO_ID                                0x00000001 /* 钛享协议ID */
/*************************************************** 云快充协议 **********************************************************/
#ifdef NET_PACK_USING_YKC
//#define NET_YKC_AS_MONITOR                                        /* 云快充平台作为监控平台 */
#define NET_YKC_AS_TARGET                                        /* 云快充平台作为目标平台 */

#ifdef NET_YKC_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000002  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000002  /* 目标平台ID */
#endif /* NET_YKC_AS_MONITOR */

#define NET_YKC_PRO_USING_DC                                      /* 云快充协议使用直流部分 */
#define NET_YKC_PRO_USING_AC                                      /* 云快充协议使用交流部分 */
#endif /* NET_PACK_USING_YKC */
#define NET_YKC_PRO_ID                                0x00000002 /* 云快充协议ID */
/*************************************************** 云快充监控协议 **********************************************************/
#ifdef NET_PACK_USING_YKC_MONITOR
#define NET_YKC_MONITOR_AS_MONITOR                                /* 云快充平台作为监控平台 */
//#define NET_YKC_MONITOR_AS_TARGET                                 /* 云快充平台作为目标平台 */

#ifdef NET_YKC_MONITOR_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000004  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000004  /* 目标平台ID */
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#define NET_YKC_MONITOR_PRO_USING_DC                                /* 云快充协议使用直流部分 */
#define NET_YKC_MONITOR_PRO_USING_AC                                /* 云快充协议使用交流部分 */
#endif /* NET_PACK_USING_YKC_MONITOR */
#define NET_YKC_MONITOR_PRO_ID                        0x00000004 /* 云快充监控协议ID */
/*************************************************** 越城协议 **********************************************************/
#ifdef NET_PACK_USING_YCP
//#define NET_YCP_AS_MONITOR                                        /* 越城平台作为监控平台 */
#define NET_YCP_AS_TARGET                                         /* 越城平台作为目标平台 */

#ifdef NET_YCP_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000008  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000008  /* 目标平台ID */
#endif /* NET_YCP_AS_MONITOR */

#define NET_YCP_PRO_USING_DC                                       /* 越城协议使用直流部分 */
#define NET_YCP_PRO_USING_AC                                       /* 越城协议使用交流部分 */
#endif /* NET_PACK_USING_YCP */
#define NET_YCP_PRO_ID                                0x00000008 /* 越城协议ID */
/*************************************************** 一电、南电协议 **********************************************************/
#ifdef NET_PACK_USING_YND
//#define NET_YND_AS_MONITOR                                        /* 一电、南电平台作为监控平台 */
#define NET_YND_AS_TARGET                                         /* 一电、南电平台作为目标平台 */

#ifdef NET_YND_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000010  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000010  /* 目标平台ID */
#endif /* NET_YND_AS_MONITOR */

#define NET_YND_PRO_USING_DC                                       /* 一电、南电协议使用直流部分 */
#define NET_YND_PRO_USING_AC                                       /* 一电、南电协议使用交流部分 */
#endif /* NET_PACK_USING_YND */
#define NET_YND_PRO_ID                                0x00000010 /* 一电、南电协议ID */
/*************************************************** 小桔协议 **********************************************************/
#ifdef NET_PACK_USING_XJ
//#define NET_XJ_AS_MONITOR                                        /* 小桔平台作为监控平台 */
//#define NET_XJ_AS_TARGET                                        /* 小桔平台作为目标平台 */

#ifdef NET_XJ_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000020  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000020  /* 目标平台ID */
#endif /* NET_XJ_AS_MONITOR */

#define NET_XJ_PRO_USING_DC                                       /* 小桔协议使用直流部分 */
#define NET_XJ_PRO_USING_AC                                       /* 小桔协议使用交流部分 */
#endif /* NET_PACK_USING_XJ */
#define NET_XJ_PRO_ID                                 0x00000020 /* 小桔协议ID */
/*************************************************** 阳关乐通协议 **********************************************************/
#ifdef NET_PACK_USING_SL
//#define NET_SL_AS_MONITOR                                        /* 阳光乐通平台作为监控平台 */
//#define NET_SL_AS_TARGET                                        /* 阳光乐通平台作为目标平台 */

#ifdef NET_SL_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000040  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000040  /* 目标平台ID */
#endif /* NET_SL_AS_MONITOR */

#define NET_SL_PRO_USING_DC                                       /* 阳光乐通协议使用直流部分 */
#define NET_SL_PRO_USING_AC                                       /* 阳光乐通协议使用交流部分 */
#endif /* NET_PACK_USING_SL */
#define NET_SL_PRO_ID                                 0x00000040 /* 阳光乐通协议ID */
/*************************************************** 国网协议 **********************************************************/
#ifdef NET_PACK_USING_SGCC
//#define NET_SGCC_AS_MONITOR                                      /* 国网平台作为监控平台 */
#define NET_SGCC_AS_TARGET                                        /* 国网平台作为目标平台 */

#ifdef NET_SGCC_AS_MONITOR
#define NET_INCLUDE_MONITOR_PLATFORM                              /* 包含监控平台 */
#define NET_MONITOR_PLATFORM_ID                       0x00000080  /* 监控平台ID */
#else
#define NET_INCLUDE_TARGET_PLATFORM                               /* 包含目标平台 */
#define NET_TARGET_PLATFORM_ID                        0x00000080  /* 目标平台ID */
#endif /* NET_SGCC_AS_MONITOR */

#define NET_SGCC_PRO_USING_DC                                      /* 国网协议使用直流部分 */
//#define NET_SGCC_PRO_USING_AC                                      /* 国网协议使用交流部分 */
#endif /* NET_PACK_USING_SGCC */
#define NET_SGCC_PRO_ID                               0x00000080 /* 国网协议ID */
/***********************************************************************************************************************/

/* OTA 相关配置 */
#ifdef NET_INCLUDE_OTA
#define NET_INCLUDE_BREAKPOINT_RESUME                            /* 支持断点续传 */

#define NET_OTA_THREAD_PRIORITY                       16         /* OTA 线程优先级(网络包中所有OTA线程优先级需一样，以防止OTA状态出错) */
#define NET_OTA_SEGMENT_LEN                           1024
#define NET_OTA_INFO_ADDR                             0x10000    /* OTA信息保存地址 */
#define NET_OTA_INFO_REGION_SIZE                      4096       /* OTA信息区域大小 */
#define NET_OTA_DATA_ADDR                             0x20000    /* OTA数据保存地址 */
#define NET_OTA_DATA_REGION_SIZE                      2 *1024 *1024  /* OTA数据区域大小 */

#ifdef NET_INCLUDE_BREAKPOINT_RESUME
#define NET_OTA_BREAKPOINT_RESUME_INFO_ADDR           0x20000000 /* OTA断点续传信息保存地址 */
#define NET_OTA_BREAKPOINT_RESUME_INFO_REGION_SIZE    4096       /* OTA断点续传信息区域大小 */
#endif /* NET_INCLUDE_BREAKPOINT_RESUME */

#endif /* NET_INCLUDE_OTA */

#define NET_SYSTEM_RECORD_STORAGE_NUM_MAX             100        /* 系统存储记录数量最大值 */

#ifdef NET_DESIGNATE_REGION
#define NET_DEF_TCMRAM __attribute__((section(".TCM_RAM")))      /* 将变量定义在TCMRAM区，注：对于GD32F470ZGT6 TCMRAM 不能存放代码，不能被任何 DMA 访问，可以将一些变量定义在该地址空间；定义的变量初始值是未知的 */
#define NET_DEF_SRAM0  __attribute__((section(".SRAM0_RAM")))    /* 将变量定义在SRAM0区，注：对于GD32F470ZGT6 SRAM0 可以存放代码，也可以存放变量，也可以作为线程的栈地址空间，可以被 DMA 访问；定义的变量初始值是未知的 */
#define NET_DEF_SRAM1  __attribute__((section(".SRAM1_RAM")))    /* 将变量定义在SRAM1区，注：对于GD32F470ZGT6 SRAM1 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#define NET_DEF_SRAM2  __attribute__((section(".SRAM2_RAM")))    /* 将变量定义在SRAM2区，注：对于GD32F470ZGT6 SRAM2 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#else
#define NET_DEF_TCMRAM
#define NET_DEF_SRAM0
#define NET_DEF_SRAM1
#define NET_DEF_SRAM2
#endif /* NET_DESIGNATE_REGION */

#endif /* NET_USING_NET_PACK */

#endif /* NET_PACK_NET_PACK_CONFIG_H_ */
