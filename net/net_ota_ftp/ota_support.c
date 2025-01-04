/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#include "ota_support.h"
#include "ftplib.h"
#include "net_pack_config.h"

struct _ftp_user_arg_cb {
    uint8_t *data;
};

typedef int32_t (*parse)(const uint8_t *data, uint32_t len);

NET_DEF_SRAM2 static struct _ftp_user_arg_cb s_ftp_user_arg_cb = {0};

NET_DEF_SRAM2 static parse s_parse_cb = NULL;

NET_DEF_SRAM2 static netbuf *s_conn = NULL;
NET_DEF_SRAM2 static netbuf *s_access = NULL;
NET_DEF_SRAM2 static FtpCallbackOptions s_ftp_opt = {0};

NET_DEF_SRAM2 static uint32_t s_ftp_file_size = 0;
NET_DEF_SRAM2 static uint8_t s_data[FTPLIB_CB_BUFFER] = {0};

static int ftp_user_func_cb(netbuf *nControl, fsz_t xfered, void *arg)
{
    static uint32_t size = 0;

    fsz_t len = xfered - size;
    struct _ftp_user_arg_cb *arg_cb = arg;

    size = xfered;

    return s_parse_cb(arg_cb->data, len);
}

int32_t ftp_read(void)
{
    return FtpRead(s_data, sizeof(s_data), s_access);
}

void ftp_callback_init(int32_t (*callback)(const uint8_t *data, uint32_t len))
{
#ifdef NET_DESIGNATE_REGION
    s_parse_cb = NULL;
    s_conn = NULL;
    s_access = NULL;
    s_ftp_file_size = 0;

    memset(&s_ftp_user_arg_cb, 0x00, sizeof(s_ftp_user_arg_cb));
    memset(&s_ftp_opt, 0x00, sizeof(s_ftp_opt));
    memset(s_data, 0x00, sizeof(s_data));
#endif /* NET_DESIGNATE_REGION */
    s_parse_cb = callback;
}

int32_t ftp_linkkie_new(const char *host, const char *user, const char *password, const char *path)
{
    int32_t result = 0;

    result = FtpConnect(host, &s_conn);
    if (1 != result) {
        return -1;
    }

    result = FtpLogin(user, password, s_conn);
    if (1 != result) {
        return -2;
    }

    result = FtpSize(path, (unsigned int *)&(s_ftp_file_size), FTPLIB_IMAGE, s_conn);
    if (1 != result) {
        s_ftp_file_size = 0;
        return -3;
    }

    result = FtpAccess(path, FTPLIB_FILE_READ, FTPLIB_IMAGE, s_conn, &s_access);
    if (1 != result) {
        s_ftp_file_size = 0;
        return -4;
    }

    (void)FtpClearCallback(s_conn);

    s_ftp_user_arg_cb.data = s_data;
    s_ftp_opt.cbFunc = ftp_user_func_cb;
    s_ftp_opt.cbArg = &s_ftp_user_arg_cb;
    s_ftp_opt.idleTime = 10000; /* Specifies the socket idle time in milliseconds that triggers calling the user's callback routine. */
    s_ftp_opt.bytesXferred = 1; /* Specifies the number of bytes to transfer between calls to the user's callback routine. */
    (void)FtpSetCallback(&s_ftp_opt, s_access);

    return result;
}

uint32_t ftp_file_size(void)
{
    return s_ftp_file_size;
}

void ftp_free()
{
    if (s_access) {
        FtpClose(s_access);
    }

    if (NULL != s_conn) {
        FtpQuit(s_conn);
    }
}
