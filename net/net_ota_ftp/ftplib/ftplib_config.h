//
// Custom Configured
//

#ifndef FTPLIB_CONFIG_H
#define FTPLIB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTTHREAD
#define RTTHREAD
#endif

#ifdef RTTHREAD
#include "rtthread.h"
#if (RTTHREAD_VERSION >= 40100)
#include "sys/select.h"
#endif
#endif

#ifdef RTTHREAD
#define ftplib_free rt_free
#define ftplib_calloc rt_calloc
#define ftplib_malloc rt_malloc
#else
#define ftplib_free free
#define ftplib_calloc calloc
#define ftplib_malloc malloc
#endif

#define FTPLIB_CB_BUFFER (256 + 256)
#define FTPLIB_BUFSIZ    (FTPLIB_CB_BUFFER + 64) /* 抓包数据分析（应用层数据接收BUF大小） */
#define RESPONSE_BUFSIZ  (128 + 16) /* 抓包数据分析 */
#define TMP_BUFSIZ       (128)      /* 抓包数据分析 */

#ifdef __cplusplus
}
#endif

#endif  // FTPLIB_CONFIG_H
