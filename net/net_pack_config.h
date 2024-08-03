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

#define NET_USING_NET_PACK                                    /* 联网 */

#ifdef NET_USING_NET_PACK

#define NET_SYSTEM_GUN_NUMBER                  2              /* 单板枪总数 */

#define NET_INCLUDE_OTA

//#define NET_PACK_USING_THA                                    /* 使用钛享协议 */
//#define NET_PACK_USING_YKC                                    /* 使用云快充协议 */
//#define NET_PACK_USING_YKC_MONITOR                              /* 使用云快充协议(监控) */
//#define NET_PACK_USING_YCP                                     /* 使用越城协议 */
//#define NET_PACK_USING_YND                                     /* 使用一电、南电协议 */
//#define NET_PACK_USING_XJ                                     /* 使用小桔协议 */
//#define NET_PACK_USING_SL                                     /* 使用阳光乐通协议 */
#define NET_PACK_USING_SGCC                                     /* 使用国网协议 */

#ifdef NET_PACK_USING_THA
#define NET_THA_AS_MONITOR                                        /* 钛享平台作为监控平台 */
#define NET_THA_AS_TARGET                                        /* 钛享平台作为目标平台 */
#define NET_THA_PRO_USING_DC                                      /* 钛享协议使用直流部分 */
#define NET_THA_PRO_USING_AC                                      /* 钛享协议使用交流部分 */
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
//#define NET_YKC_AS_MONITOR                                        /* 云快充平台作为监控平台 */
#define NET_YKC_AS_TARGET                                        /* 云快充平台作为目标平台 */
#define NET_YKC_PRO_USING_DC                                      /* 云快充协议使用直流部分 */
#define NET_YKC_PRO_USING_AC                                      /* 云快充协议使用交流部分 */
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
#define NET_YKC_MONITOR_AS_MONITOR                                /* 云快充平台作为监控平台 */
//#define NET_YKC_MONITOR_AS_TARGET                                 /* 云快充平台作为目标平台 */
#define NET_YKC_MONITOR_PRO_USING_DC                                /* 云快充协议使用直流部分 */
#define NET_YKC_MONITOR_PRO_USING_AC                                /* 云快充协议使用交流部分 */
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
//#define NET_YCP_AS_MONITOR                                        /* 越城平台作为监控平台 */
#define NET_YCP_AS_TARGET                                         /* 越城平台作为目标平台 */
#define NET_YCP_PRO_USING_DC                                       /* 越城协议使用直流部分 */
#define NET_YCP_PRO_USING_AC                                       /* 越城协议使用交流部分 */
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_YND
//#define NET_YND_AS_MONITOR                                        /* 一电、南电平台作为监控平台 */
#define NET_YND_AS_TARGET                                         /* 一电、南电平台作为目标平台 */
#define NET_YND_PRO_USING_DC                                       /* 一电、南电协议使用直流部分 */
#define NET_YND_PRO_USING_AC                                       /* 一电、南电协议使用交流部分 */
#endif /* NET_PACK_USING_YND */

#ifdef NET_PACK_USING_XJ
//#define NET_XJ_AS_MONITOR                                        /* 小桔平台作为监控平台 */
//#define NET_XJ_AS_TARGET                                        /* 小桔平台作为目标平台 */
#define NET_XJ_PRO_USING_DC                                       /* 小桔协议使用直流部分 */
#define NET_XJ_PRO_USING_AC                                       /* 小桔协议使用交流部分 */
#endif /* NET_PACK_USING_XJ */

#ifdef NET_PACK_USING_SL
//#define NET_SL_AS_MONITOR                                        /* 阳光乐通平台作为监控平台 */
//#define NET_SL_AS_TARGET                                        /* 阳光乐通平台作为目标平台 */
#define NET_SL_PRO_USING_DC                                       /* 阳光乐通协议使用直流部分 */
#define NET_SL_PRO_USING_AC                                       /* 阳光乐通协议使用交流部分 */
#endif /* NET_PACK_USING_SL */

#ifdef NET_PACK_USING_SGCC
//#define NET_SGCC_AS_MONITOR                                      /* 国网平台作为监控平台 */
#define NET_SGCC_AS_TARGET                                        /* 国网平台作为目标平台 */
#define NET_SGCC_PRO_USING_DC                                      /* 国网协议使用直流部分 */
#define NET_SGCC_PRO_USING_AC                                      /* 国网协议使用交流部分 */
#endif /* NET_PACK_USING_SGCC */

/* OTA 相关配置 */
#ifdef NET_INCLUDE_OTA
#define NET_INCLUDE_BREAKPOINT_RESUME                         /* 支持断点续传 */

#define NET_OTA_THREAD_PRIORITY                16             /* OTA 线程优先级(网络包中所有OTA线程优先级需一样，以防止OTA状态出错) */
#define NET_OTA_SEGMENT_LEN                    1024
#define NET_OTA_INFO_ADDR                      0x10000     /* OTA信息保存地址 */
#define NET_OTA_INFO_REGION_SIZE               4096           /* OTA信息区域大小 */
#define NET_OTA_DATA_ADDR                      0x20000     /* OTA数据保存地址 */
#define NET_OTA_DATA_REGION_SIZE               2 *1024 *1024  /* OTA数据区域大小 */

#ifdef NET_INCLUDE_BREAKPOINT_RESUME
#define NET_OTA_BREAKPOINT_RESUME_INFO_ADDR         0x20000000/* OTA断点续传信息保存地址 */
#define NET_OTA_BREAKPOINT_RESUME_INFO_REGION_SIZE  4096      /* OTA断点续传信息区域大小 */
#endif /* NET_INCLUDE_BREAKPOINT_RESUME */

#endif /* NET_INCLUDE_OTA */

enum platform{
#ifdef NET_PACK_USING_THA
    NET_PLATFORM_THA,
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    NET_PLATFORM_YKC,
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_XJ
    NET_PLATFORM_XJ,
#endif /* NET_PACK_USING_XJ */
    NET_PLATFORM_SIZE,
};

#endif /* NET_USING_NET_PACK */

#endif /* NET_PACK_NET_PACK_CONFIG_H_ */
