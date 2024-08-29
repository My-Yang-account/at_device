#include <rtthread.h>

#include <stdlib.h>
#include <stdarg.h>

#include "sys/socket.h"
#include "sys/errno.h"
#include "netdb.h"
#include "fcntl.h"

#include "mbedtls/ssl.h"
#include "mbedtls/net.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"

#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "wrappers_defs.h"
#include "stdarg.h"

#define DBG_TAG "wrappers"
#define DBG_LVL DBG_WARNING
#include <rtdbg.h>

typedef struct _TLSDataParams {
    mbedtls_ssl_context ssl;
    mbedtls_net_context fd;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacertl;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
} TLSDataParams_t, *TLSDataParams_pt;

static uint64_t time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

/**
 *
 * 函数 HAL_Firmware_Persistence_Start() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 远程升级开始的实现
 * 
 */
void HAL_Firmware_File_Size(unsigned int file_size)
{
    extern void sgcc_firmware_file_size(uint32_t file_size);
    sgcc_firmware_file_size(file_size);
}

/**
 *
 * 函数 HAL_Firmware_Persistence_Start() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 远程升级开始的实现
 * 
 */
void HAL_Firmware_Persistence_Start(void)
{
    extern int sgcc_firmware_start(void);
    return sgcc_firmware_start();
}

/**
 *
 * 函数 HAL_Firmware_Persistence_Write() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 远程升级完成的实现
 * 
 */
int HAL_Firmware_Persistence_Write(char *buffer, unsigned int length)
{
    extern int sgcc_firmware_write(char *buffer, uint32_t length);
    return sgcc_firmware_write(buffer, length);
}

/**
 *
 * 函数 HAL_Firmware_Persistence_Stop() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 远程升级过程中的实现
 * 
 */
int HAL_Firmware_Persistence_Stop(int process)
{
    extern int sgcc_firmware_stop(void);
    return sgcc_firmware_stop();
}

/**
 *
 * 函数 HAL_GetFirmwareVersion() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 获取固件版本的实现
 * 
 */
/**
 * @brief Get firmware version
 *
 * @param [ou] version: array to store firmware version, max length is IOTX_FIRMWARE_VER_LEN
 * @return the actual length of firmware version
 */
int HAL_GetFirmwareVersion(char *version)
{
    int sgcc_firmware_version(char *version);
    return sgcc_firmware_version(version);
}

/**
 *
 * 函数 HAL_Kv_Get() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 不需要实现
 * 
 */
int HAL_Kv_Get(const char *key, void *val, int *buffer_len)
{
    return (int)1;
}

/**
 *
 * 函数 HAL_Kv_Set() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 不需要实现
 * 
 */
int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    return (int)1;
}

void *HAL_Malloc(unsigned int size)
{
    return rt_malloc(size);
}

void HAL_Free(void *ptr)
{
    rt_free(ptr);
}

/**
 * @brief Create a mutex.
 *
 * @retval NULL : Initialize mutex failed.
 * @retval NOT_NULL : The mutex handle.
 * @see None.
 * @note None.
 */
void *HAL_MutexCreate(void)
{
    rt_mutex_t mutex = rt_mutex_create("sdk_mutex", RT_IPC_FLAG_FIFO);

    return mutex;
}

/**
 * @brief Destroy the specified mutex object, it will release related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexDestroy(void *mutex)
{
    if (NULL == mutex) {
        return;
    }

    rt_mutex_delete((rt_mutex_t)mutex);
}

/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexLock(void *mutex)
{
    if (NULL == mutex) {
        return;
    }

    rt_mutex_take((rt_mutex_t)mutex, RT_WAITING_FOREVER);
}

/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexUnlock(void *mutex)
{
    if (NULL == mutex) {
        return;
    }

    rt_mutex_release((rt_mutex_t)mutex);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (uint32_t)(rand() % region) : (uint32_t)(0);
}

void HAL_Srandom(uint32_t seed)
{
    srand((uint32_t)(seed));
}

/**
 *
 * 函数 HAL_SetFirmwareVersion() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 设置固件版本的实现
 * 
 */
int HAL_SetFirmwareVersion(const char *version)
{
    //-------------------------//
    //在此处实现。。。
    //-------------------------//

    return (int)1;
}

/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint64_t HAL_UptimeMs(void)
{
    return (uint64_t)(rt_tick_get());
}

void HAL_SleepMs(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = rt_vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
    return rt_vsnprintf(str, len, format, ap);
}

static int ssl_random(void *p_rng, unsigned char *output, size_t output_len)
{
    uint32_t rnglen = output_len;
    uint8_t rngoffset = 0;

    while (rnglen > 0) {
        *(output + rngoffset) = (((unsigned int)rand() << 16) + rand());
        rngoffset++;
        rnglen--;
    }

    return 0;
}

static void ssl_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void) level);

    rt_kprintf("%s:%04d: %s \r\n", file, line, str);
}

void mbedtls_net_init(mbedtls_net_context *ctx)
{
    if (NULL == ctx) {
        return;
    }

    LOG_I("net init \n");

    ctx->fd = -1;
}

int mbedtls_net_connect(mbedtls_net_context *ctx, const char *host, const char *port, int proto)
{
    int rc = 0, fd = -1;
    struct addrinfo hints;
    struct addrinfo *addr_info = NULL;

    LOG_I("net connect --- host: %s, port: %s \n", host, port);

    memset((void *)&(hints), 0x00, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (0 != getaddrinfo(host, port, &hints, &addr_info)) {
        rc = -1;
        goto __exit;
    }

    fd = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (0 > fd) {
        rc = -2;
        goto __exit;
    }

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);

    if (0 != connect(fd, addr_info->ai_addr, addr_info->ai_addrlen)) {
        closesocket(fd);
        rc = -3;
    }

__exit:

    freeaddrinfo(addr_info);

    if (0 == rc) {
        ctx->fd = fd;
        LOG_D("net establish --- success, fd: %d \n", fd);
    } else {
        LOG_D("net establish --- failed, code: %d \n", rc);
    }

    return (0 == rc) ? 0 : (-1);
}

void mbedtls_net_free(mbedtls_net_context *ctx)
{
    LOG_I("net destroy, fd: %d \n", ctx->fd);

    closesocket(ctx->fd);
}

int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int res, n, fd = ((mbedtls_net_context *)ctx)->fd;

    LOG_I("net send --- fd: %d, len: %d \n", fd, len);

    n = send(fd, buf, len, 0);
    if (0 < n) {
        res = n;
    } else if (-1 == n) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            res = 0;
            LOG_W("net send --- fail \n");
        } else {
            res = -1;
            LOG_W("net send --- failed, code: %d \n", errno);
        }
    } else {
        res = -1;
        LOG_W("net send --- close \n");
    }

    return res;
}

int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int res, n, fd = ((mbedtls_net_context *)ctx)->fd;

    LOG_I("net recv --- fd: %d, len: %d \n", fd, len);

    n = recv(fd, (void *)buf, len, 0);
    if (0 < n) {
        res = n;
    } else if (-1 == n) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            res = 0;
            LOG_W("net recv --- fail \n");
        } else {
            res = -1;
            LOG_W("net recv --- failed, code: %d \n", errno);
        }
    } else {
        res = -2;
        LOG_W("net recv --- close \n");
    }

    return res;
}

int mbedtls_net_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout_ms)
{
    (void)timeout_ms;

    int res = 0, l = 0, n = 0, fd = ((mbedtls_net_context *)ctx)->fd;;
    uint64_t t_left = 0, t_end = HAL_UptimeMs() + timeout_ms;

    LOG_I("net read timeout --- fd: %d, len: %d, timeout: %d \n", fd, len, timeout_ms);

    if (0 == len) {
        return 0;
    }

    do {
        if (0 < timeout_ms) {
            t_left = time_left(t_end, HAL_UptimeMs());
            if (0 == t_left) {
                res = 0;
                LOG_W("net read timeout --- timeout \n");
                break;
            }
        }

        n = recv(fd, (void *)(buf + l), len - l, 0);
        if (0 < n) {
            l += n;
            res = l;
        } else if (-1 == n) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_W("net read timeout --- continue \n");
                continue;
            } else {
                res = -1;
                LOG_W("net read timeout --- failed, code: %d \n", errno);
                break;
            }
        } else {
            res = -2;
            LOG_W("net read timeout --- close \n");
            break;
        }
    } while (l < len);

    return res;
}

static int network_ssl_connect(TLSDataParams_t *pTlsData, const char *addr, const char *port,
                              const char *ca_crt, size_t ca_crt_len,
                              const char *client_crt, size_t client_crt_len,
                              const char *client_key, size_t client_key_len,
                              const char *client_pwd, size_t client_pwd_len)
{
    int ret = -1;

    mbedtls_net_init(&(pTlsData->fd));
    rt_kprintf("network_ssl_connect(%d, %s, %s)\n", pTlsData->fd, addr, port);
    if (0 != (ret = mbedtls_net_connect(&(pTlsData->fd), addr, port, MBEDTLS_NET_PROTO_TCP))) {
        mbedtls_net_free(&(pTlsData->fd));
        LOG_E("failed! ssl connect failed, returned -0x%04x \n", -ret);
        return ret;
    }

    mbedtls_ssl_init(&(pTlsData->ssl));
    mbedtls_ssl_config_init(&(pTlsData->conf));
    mbedtls_x509_crt_init(&(pTlsData->cacertl));
    mbedtls_x509_crt_init(&(pTlsData->clicert));
    mbedtls_pk_init(&(pTlsData->pkey));

    if (NULL != ca_crt) {
        if (0 != (ret = mbedtls_x509_crt_parse(&(pTlsData->cacertl), (const unsigned char *)ca_crt, ca_crt_len))) {
            LOG_E("failed! parse ca-cert failed, returned -0x%04x \n", -ret);
            return ret;
        }
    }

    if ((ret = mbedtls_ssl_config_defaults(&(pTlsData->conf), MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        LOG_E("failed! ssl config failed, returned -0x%04x \n", -ret);
        return ret;
    }

    mbedtls_ssl_conf_max_version(&pTlsData->conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_min_version(&pTlsData->conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_1);
    mbedtls_ssl_conf_authmode(&(pTlsData->conf), MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(&(pTlsData->conf), &(pTlsData->cacertl), NULL);

    if ((ret = mbedtls_ssl_conf_own_cert(&(pTlsData->conf), &(pTlsData->clicert), &(pTlsData->pkey))) != 0) {
        LOG_E("failed! ssl config own cert failed, returned -0x%04x \n", -ret);
        return ret;
    }

    mbedtls_ssl_conf_rng(&(pTlsData->conf), ssl_random, NULL);
    mbedtls_ssl_conf_dbg(&(pTlsData->conf), ssl_debug, NULL);

    if ((ret = mbedtls_ssl_setup(&(pTlsData->ssl), &(pTlsData->conf))) != 0) {
        LOG_E("failed! ssl setup failed, returned -0x%04x \n", -ret);
        return ret;
    }

    if ((ret = mbedtls_ssl_set_hostname(&(pTlsData->ssl), addr)) != 0 ) {
        LOG_E("failed! ssl set hostname, returned -0x%04x \n", -ret);
        return ret;
    }

    mbedtls_ssl_set_bio(&(pTlsData->ssl), &(pTlsData->fd), mbedtls_net_send, mbedtls_net_recv, NULL);

    while ((ret = mbedtls_ssl_handshake(&(pTlsData->ssl))) != 0) {
        if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
            LOG_E("failed! ssl handshake returned -0x%04x \n", -ret);
            if(ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
                LOG_E("failed! Unable to verify the server's certificate \n");
            } else if(ret == MBEDTLS_ERR_SSL_ALLOC_FAILED) {
                LOG_E("failed! Memory allocation failed \n");
            }
            return ret;
        }
    }

    return 0;
}

static void network_ssl_disconnect(TLSDataParams_t *pTlsData)
{
    LOG_I("net ssl disconnect \n");

    mbedtls_ssl_close_notify(&(pTlsData->ssl));

    if((pTlsData->fd.fd) >= 0) {
        mbedtls_net_free(&(pTlsData->fd));
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_free(&(pTlsData->cacertl));
    if ((pTlsData->pkey).pk_info != NULL) {
#if defined(MBEDTLS_CERTS_C)
        mbedtls_x509_crt_free(&(pTlsData->clicert));
        mbedtls_pk_free(&(pTlsData->pkey));
#endif
    }
#endif

    mbedtls_ssl_free(&(pTlsData->ssl));
    mbedtls_ssl_config_free(&(pTlsData->conf));
}

/**
 *
 * 函数 HAL_SSL_Establish() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * SSL连接的实现
 * 
 */
uintptr_t HAL_SSL_Establish(const char *host, uint16_t port, const char *ca_crt, uint32_t ca_crt_len)
{
    char port_str[6] = {0};
    TLSDataParams_pt p;

    LOG_I("ssl establish --- host: %s, port: %d, cert len: %d \n", host, port, ca_crt_len);

    p = HAL_Malloc(sizeof(TLSDataParams_t));
    if (NULL == p) {
        return (uintptr_t)NULL;
    }
    memset(p, 0x0, sizeof(TLSDataParams_t));

    sprintf(port_str, "%u", port);

    if (0 != network_ssl_connect(p, host, port_str, ca_crt, ca_crt_len, NULL, 0, NULL, 0, NULL, 0)) {
        network_ssl_disconnect(p);
        HAL_Free((void *)p);
        return (uintptr_t)NULL;
    }

    LOG_D("ssl establish --- success \n");

    return (uintptr_t)p;
}

/**
 *
 * 函数 HAL_SSL_Destroy() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * SSL断开的实现
 * 
 */
int32_t HAL_SSL_Destroy(uintptr_t handle)
{
    LOG_I("ssl destroy \n");

    if ((uintptr_t)NULL == handle) {
        return 0;
    }

    network_ssl_disconnect((TLSDataParams_t *)handle);
    HAL_Free((void *)handle);

    return 0;
}

/**
 *
 * 函数 HAL_SSL_Read() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * SSL读的实现
 * 
 */
int HAL_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms)
{
    (void)timeout_ms;

    int res = 0, l = 0, n = 0;
    TLSDataParams_t *h = (TLSDataParams_t *)handle;
    uint64_t t_left = 0, t_end = HAL_UptimeMs() + timeout_ms;

    LOG_I("ssl read --- len: %d, timeout: %d \n", len, timeout_ms);

    if (0 == len) {
        return 0;
    }

    mbedtls_ssl_conf_read_timeout(&(h->conf), timeout_ms);
    do {
        t_left = time_left(t_end, HAL_UptimeMs());
        if (0 == t_left) {
            res = 0;
            LOG_D("ssl read --- timeout \n");
            break;
        }

        n = mbedtls_ssl_read(&(h->ssl), (unsigned char *)(buf + l), len - l);
        if (0 < n) {
            l += n;
            res = l;
        } else {
            if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) {
                res = -1;
                LOG_W("ssl read --- fail, code: -0x%04x \n", n);
            } else {
                res = -2;
                LOG_W("ssl read --- failed, code: -0x%04x \n", n);
            }
            break;
        }
    } while (l < len);

    return res;
}

/**
 *
 * 函数 HAL_SSL_Write() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * SSL写的实现
 * 
 */
int HAL_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms)
{
    (void)timeout_ms;

    int res = 0, l = 0, n = 0;
    TLSDataParams_t *h = (TLSDataParams_t *)handle;

    LOG_I("ssl write --- len: %d, timeout: %d \n", len, timeout_ms);

    if (0 == len) {
        return 0;
    }

    while (l < len) {
        n = mbedtls_ssl_write(&(h->ssl), (const unsigned char *)(buf + l), len - l);
        if (0 < n) {
            l += n;
            res = l;
        } else {
            if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) {
                res = -1;
                LOG_W("ssl write --- fail, code: -0x%04x \n", -n);
            } else {
                res = -2;
                LOG_W("ssl write --- failed, code: -0x%04x \n", -n);
            }
            break;
        }
    }

    return res;
}

/**
 *
 * 函数 HAL_TCP_Establish() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * TCP连接的实现
 * 
 */
/**
 * @brief Establish a TCP connection.
 *
 * @param [in] host: @n Specify the hostname(IP) of the TCP server
 * @param [in] port: @n Specify the TCP port of TCP server
 *
 * @return The handle of TCP connection.
   @retval (uintptr_t)(-1): Fail.
   @retval All other values(0 included): Success, the value is handle of this TCP connection.
 */
uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    int rc = 0, fd = -1;
    char servname[6] = {0};
    struct addrinfo hints;
    struct addrinfo *addr_info = NULL;

    LOG_I("tcp establish --- host: %s, port: %d \n", host, port);

    memset((void *)&(servname), 0x00, sizeof(servname));
    memset((void *)&(hints), 0x00, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(servname, "%u", port);

    if (0 != getaddrinfo(host, servname, &hints, &addr_info)) {
        rc = -1;
        goto __exit;
    }

    fd = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (0 > fd) {
        rc = -2;
        goto __exit;
    }

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);

    if (0 != connect(fd, addr_info->ai_addr, addr_info->ai_addrlen)) {
        closesocket(fd);
        rc = -3;
    }

__exit:

    freeaddrinfo(addr_info);

    if (0 == rc) {
        LOG_D("tcp establish --- success, fd: %d \n", fd);
    } else {
        LOG_D("tcp establish --- failed, code: %d \n", rc);
    }

    return fd;
//    return (0 == rc) ? 0 : (-1);
}

/**
 *
 * 函数 HAL_TCP_Destroy() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * TCP断开的实现
 * 
 */
/**
 * @brief Destroy the specific TCP connection.
 *
 * @param [in] fd: @n Specify the TCP connection by handle.
 *
 * @return The result of destroy TCP connection.
 * @retval < 0 : Fail.
 * @retval   0 : Success.
 */
int HAL_TCP_Destroy(uintptr_t fd)
{
    int s = fd;

    LOG_I("tcp destroy, fd: %d \n", s);

    if (0 != closesocket(s)) {
        return -1;
    }

    return 0;
}

/**
 *
 * 函数 HAL_TCP_Read() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * TCP读的实现
 * 
 */
/**
 * @brief Read data from the specific TCP connection with timeout parameter.
 *        The API will return immediately if 'len' be received from the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a TCP connection.
 * @param [out] buf @n A pointer to a buffer to receive incoming data.
 * @param [out] len @n The length, in bytes, of the data pointed to by the 'buf' parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block 'timeout_ms' millisecond maximumly.
 *
 * @retval       -2 : TCP connection error occur.
 * @retval       -1 : TCP connection be closed by remote server.
 * @retval        0 : No any data be received in 'timeout_ms' timeout period.
 * @retval (0, len] : The total number of bytes be received in 'timeout_ms' timeout period.

 * @see None.
 */
int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    (void)timeout_ms;

    int res = 0, l = 0, n = 0;
    uint64_t t_left = 0, t_end = HAL_UptimeMs() + timeout_ms;

    LOG_I("tcp read --- fd: %d, len: %d, timeout: %d \n", fd, len, timeout_ms);

    if (0 == len) {
        return 0;
    }

    do {
        if (0 < timeout_ms) {
            t_left = time_left(t_end, HAL_UptimeMs());
            if (0 == t_left) {
                res = 0;
                LOG_D("tcp read --- timeout \n");
                break;
            }
        }

        n = recv(fd, (void *)(buf + l), len - l, 0);
        if (0 < n) {
            l += n;
            res = l;
        } else if (-1 == n) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_D("tcp read --- continue \n");
                continue;
            } else {
                res = -1;
                LOG_D("tcp read --- failed, code: %d \n", errno);
                break;
            }
        } else {
            res = -2;
            LOG_D("tcp read --- close \n");
            break;
        }
    } while (l < len);

    return res;
}

/**
 *
 * 函数 HAL_TCP_Write() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * TCP写的实现
 * 
 */
/**
 * @brief Write data into the specific TCP connection.
 *        The API will return immediately if 'len' be written into the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a connection.
 * @param [in] buf @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] len @n The length, in bytes, of the data pointed to by the 'buf' parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block 'timeout_ms' millisecond maximumly.
 *
 * @retval      < 0 : TCP connection error occur..
 * @retval        0 : No any data be write into the TCP connection in 'timeout_ms' timeout period.
 * @retval (0, len] : The total number of bytes be written in 'timeout_ms' timeout period.

 * @see None.
 */
int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    (void)timeout_ms;

    int res = 0, l = 0, n = 0;

    LOG_I("tcp write --- fd: %d, len: %d, timeout: %d \n", fd, len, timeout_ms);

    if (NULL == buf || 0 == len) {
        return -1;
    }

    while (l < len) {
        n = send(fd, (const void *)(buf + l), len - l, 0);
        if (0 < n) {
            l += n;
            res = l;
        } else if (-1 == n) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_D("tcp write --- continue \n");
                continue;
            } else {
                res = -1;
                LOG_D("tcp write --- failed, code: %d \n", errno);
                break;
            }
        } else {
            res = -2;
            LOG_D("tcp write --- close \n");
            break;
        }
    }

    return res;
}

/**
 *
 * 函数 HAL_UTC_Get() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 不需要实现
 * 
 */
long HAL_UTC_Get(void)
{
    return (long)1;
}

/**
 *
 * 函数 HAL_UTC_Set() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 不需要实现
 * 
 */
int HAL_UTC_Set(long s)
{
    (void)s;

    return (int)1;
}

/**
 *
 * 函数 HAL_Read() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 不需要实现
 * 
 */
/**
 * @brief write data to terminal
 *
 * @param[in] fd @n device descriptor.
 * @param[out]  data pointer to the buffer which will store incoming data
 * @param[out]  count number of bytes received
 * @return count number of bytes received.
 * @see None.
 * @note None.
 */
int HAL_Read(int fd, void *buf, int len)
{
    (void)fd, (void)buf, (void)len;

    return (int)1;
}

/**
 *
 * 函数 HAL_Write() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * 不需要实现
 * 
 */
/**
 * @brief write data to terminal
 *
 * @param[in] fd @n device descriptor.
 * @param[in] data pointer to the start of data
 * @param[in] size number of bytes to transmit
 * @return count number of bytes received.
 * @see None.
 * @note None.
 */
int HAL_Write(int fd, const void *buf, int len)
{
    (void)fd, (void)buf, (void)len;

    return (int)1;
}

int HAL_Get_SocketFd(uintptr_t handle)
{
    TLSDataParams_pt p = (TLSDataParams_pt)handle;

    return p->fd.fd;
}
