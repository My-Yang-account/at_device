/*
 * Copyright (C) 2019-2025 SGCC Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "infra_config.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "infra_state.h"
#include "dev_model_api.h"
#include "wrappers.h"
#include "cJSON.h"

#ifdef ATM_ENABLED
#include "at_api.h"
#endif

#include "protocol.h"
#include "interface.h"

#ifdef DYNAMIC_REGISTER
#include "dynreg_api.h"
#endif

#ifdef HTTP2_COMM_ENABLED
#include "http2_upload_api.h"
#define UPLOAD_RETRY_TIME 50
#define HTTP2_ONLINE_SERVER_URL "a1IgnOND7vI.iot-as-http2.cn-shanghai.aliyuncs.com"
#define HTTP2_ONLINE_SERVER_PORT 443
#endif

static int upload_result = 1;
static char g_upload_id[50] = {0};
static unsigned char is_connected = 0;
static int main_loop_step = EVS_LINKKIT_OPEN;

#define USER_IS_DEBUG

#if defined(PLATFORM_IS_DEBUG) && defined(USER_IS_DEBUG)
#define PROTOCOL_TRACE(...)                                     \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;34m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)
#else
#define PROTOCOL_TRACE(...)
#endif

#define EVS_YIELD_TIMEOUT_MS (200)

unsigned int strtoint(char s[])
{
    unsigned int i;
    unsigned int num = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
    {
        num = 10 * num + (s[i] - '0');
    }
    return num;
}

void bcd2str(unsigned char byte[], char str[], int len)
{
    int i, index = 0;
    for (i = 0; i < len; i++)
    {
        char hex1;
        char hex2;
        int value = byte[i];
        int v1 = value / 16;
        int v2 = value % 16;
        if (v1 >= 0 && v1 <= 9)
            hex1 = (char)(48 + v1);
        else
            hex1 = (char)(55 + v1);
        if (v2 >= 0 && v2 <= 9)
            hex2 = (char)(48 + v2);
        else
            hex2 = (char)(55 + v2);

        str[index++] = hex1;
        str[index++] = hex2;
    }
}

int stringToHex(char *str, unsigned char *hex)
{
    char *p = NULL;
    char High = 0;
    char Low = 0;
    int Len = 0;
    int count = 0;

    p = str;
    Len = strlen(p);

    while (count < (Len / 2))
    {
        High = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        Low = (*(++p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        hex[count] = ((High & 0x0f) << 4 | (Low & 0x0f));
        p++;
        count++;
    }

    if (0 != Len % 2)
    {
        hex[count++] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
    }

    return Len / 2 + Len % 2;
}

typedef struct
{
    int master_devid;
    int cloud_connected;
    int master_initialized;
} evs_user_ctx_t;

static evs_user_ctx_t evs_g_user_ctx;
iotx_linkkit_dev_meta_info_t master_meta_info;

typedef struct
{
    unsigned char gunNo;
    unsigned char getTerminalFlag;
    unsigned char getServerAuthFlag;
    unsigned char getAuthChargeFlag;
    unsigned char PKid;
    unsigned char MAC1[4];
    unsigned char MAC2[4];
    unsigned char TAC[4];
    unsigned char randomC[4];
    unsigned char cardBalance[4];
    unsigned char tradeSn[2];
    unsigned char tradeTime[7];
    unsigned char terminalSn[6];
    unsigned char authVer;
    unsigned char authFlag;
    unsigned char usr_id[10];
    unsigned char psy_id[8];
    unsigned char usrId[8];
} evs_smart_gun_auth_process_data;

static evs_smart_gun_auth_process_data gun_auth_process_data[EVS_MAX_PORT_NUM];

/**
 * @brief 解析所有属性设置的值
 * @param request 指向属性设置请求payload的指针
 * @param request_len 属性设置请求的payload长度
 * @return 解析成功: 0, 解析失败: <0
 */
int app_parse_property(const char *request, unsigned int request_len)
{
    cJSON *structcnt = NULL;

    cJSON *req = cJSON_Parse(request);
    if (req == NULL || !cJSON_IsObject(req))
    {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }

    structcnt = cJSON_GetObjectItem(req, "structCnt");
    if (structcnt != NULL && cJSON_IsObject(structcnt))
    {
        PROTOCOL_TRACE("struct property id: structCnt");
    }

    cJSON_Delete(req);
    return 0;
}

void upload_file_result(const char *file_path, int result, const char *store_id, void *user_data)
{
    upload_result = result;

    PROTOCOL_TRACE("=========== file_path = %s, result = %d ===========", file_path, upload_result);
}

void upload_id_received_handle(const char *file_path, const char *upload_id, void *user_data)
{
    PROTOCOL_TRACE("=========== file_path = %s, upload_id = %s ===========", file_path, upload_id);

    if (upload_id != NULL)
    {
        memset(g_upload_id, 0, sizeof(g_upload_id));
        strncpy(g_upload_id, upload_id, sizeof(g_upload_id) - 1);
    }
}

/** http建立连成功事件回调 */
static void _on_http2_reconnect(void)
{
    PROTOCOL_TRACE("http2 reconnected");

    is_connected = 1;
}

/** http断开连接事件回调 */
static void _on_http2_disconnect(void)
{
    PROTOCOL_TRACE("http2 disconnected");

    is_connected = 0;
}

/** IOT建立连成功事件回调 */
static int user_connected_event_handler(void)
{
    void *callback;
    evs_g_user_ctx.cloud_connected = 1;
    callback = evs_service_callback(EVS_CONNECT_SUCC);
    if (callback)
    {
        ((int (*)(void))callback)();
    }
    return 0;
}

/** IOT断开连接事件回调 */
static int user_disconnected_event_handler(void)
{
    void *callback;
    evs_g_user_ctx.cloud_connected = 0;
    callback = evs_service_callback(EVS_DISCONNECTED);
    if (callback)
    {
        ((int (*)(void))callback)();
    }
    return 0;
}

/* 设备初始化成功事件回调 */
static int user_initialized(const int devid)
{
    evs_g_user_ctx.master_initialized = 1;
    return 0;
}

/** 事件回调：接收到云端回复属性上报应答 **/
static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
                                           const int reply_len)
{
    void *callback;

    callback = evs_service_callback(EVS_REPORT_REPLY);
    if (callback)
    {
        ((int (*)(const int msgid, const int code, const char *reply,
                  const int reply_len))callback)(msgid, code, reply, reply_len);
    }
    return 0;
}

/** 事件回调：接收到云端回复的事件上报应答 **/
static int user_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
                                                  const int eventid_len, const char *message, const int message_len)
{
    void *callback;

    callback = evs_service_callback(EVS_TRIGGER_EVENT_REPLY);
    if (callback)
    {
        ((int (*)(const int msgid, const int code, const char *eventid,
                  const int eventid_len, const char *message, const int message_len))callback)(msgid, code, eventid, eventid_len, message, message_len);
    }
    return 0;
}

/** 事件回调：接收到云端下发的属性设置 **/
static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    PROTOCOL_TRACE("Property Set Received, Request: %s", request);

    app_parse_property(request, request_len);

    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    PROTOCOL_TRACE("Post Property return: %d", res);

    return res;
}

/** 事件回调：接收到云端回复的时间戳 **/
static int user_timestamp_reply_event_handler(const char *timestamp)
{
    PROTOCOL_TRACE("Current Timestamp: %s", timestamp);

    char time_buf[11] = {0};
    memcpy(time_buf, timestamp, sizeof(time_buf) - 1);

    unsigned int time = strtoint(time_buf);

    void *callback;
    callback = evs_service_callback(EVS_TIME_SYNC);
    if (callback)
    {
        ((int (*)(unsigned int))callback)(time);
    }
    return 0;
}

/** FOTA事件回调处理 **/
static int user_fota_event_handler(int type, const char *version)
{

    /* 0 - new firmware exist, query the new firmware */
    if (type == 0)
    {
        PROTOCOL_TRACE("New Firmware Version: %s", version);
        void *callback;
        callback = evs_service_callback(EVS_OTA_UPDATE);
        if (callback)
        {
            ((int (*)(const char *))callback)(version);
        }
    }
    return 0;
}

/** 事件回调：接收到云端错误信息 **/
static int user_cloud_error_handler(const int code, const char *data, const char *detail)
{
    PROTOCOL_TRACE("code =%d ,data=%s, detail=%s", code, data, detail);
    return 0;
}

/** 事件回调：通过动态注册获取到DeviceSecret **/
static int dynreg_device_secret(const char *device_secret)
{
    PROTOCOL_TRACE("device secret: %s", device_secret);
    return 0;
}

/** 事件回调: SDK内部运行状态打印 **/
static int user_sdk_state_dump(int ev, const char *msg)
{
    void *callback;
    PROTOCOL_TRACE("received state event, -0x%04x(%s)\n", -ev, msg);
    callback = evs_service_callback(EVS_STATE_EVERYTHING);
    if (callback)
    {
        ((int (*)(int ev, const char *msg))callback)(ev, msg);
    }
    return 0;
}

static int evs_service_get_config_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    int i = 0;
    evs_data_dev_config service_dev_feedback_config_data;

    callback = evs_service_callback(EVS_CONF_GET_SRV);
    if (callback)
    {
        ((int (*)(const evs_data_dev_config *))callback)(&service_dev_feedback_config_data);
    }

    cJSON *response_root = cJSON_CreateObject();
    cJSON *qrCodeArray;

    cJSON_AddNumberToObject(response_root, "equipParamFreq", service_dev_feedback_config_data.equipParamFreq);
    cJSON_AddNumberToObject(response_root, "gunElecFreq", service_dev_feedback_config_data.gunElecFreq);
    cJSON_AddNumberToObject(response_root, "nonElecFreq", service_dev_feedback_config_data.nonElecFreq);
    cJSON_AddNumberToObject(response_root, "faultWarnings", service_dev_feedback_config_data.faultWarnings);
    //cJSON_AddNumberToObject(response_root, "acMeterFreq", service_dev_feedback_config_data.acMeterFreq);
    //cJSON_AddNumberToObject(response_root, "dcMeterFreq", service_dev_feedback_config_data.dcMeterFreq);
    cJSON_AddNumberToObject(response_root, "offlinChaLen", service_dev_feedback_config_data.offlinChaLen);
    cJSON_AddNumberToObject(response_root, "grndLock", service_dev_feedback_config_data.grndLock);
    cJSON_AddNumberToObject(response_root, "doorLock", service_dev_feedback_config_data.doorLock);

    cJSON_AddItemToObject(response_root, "qrCode", qrCodeArray = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_PORT_NUM; i++)
    {
        // cJSON_AddStringToArray(qrCodeArray, service_dev_feedback_config_data.qrCode[i]);
        cJSON_AddItemToArray(qrCodeArray, cJSON_CreateString(service_dev_feedback_config_data.qrCode[i]));
    }

    cJSON_AddNumberToObject(response_root, "s2CloseTime", service_dev_feedback_config_data.s2CloseTime);
    cJSON_AddNumberToObject(response_root, "s2OpenTime", service_dev_feedback_config_data.s2OpenTime);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}
static int evs_service_get_Maintain_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    evs_service_feedback_maintain_query service_feedback_maintain_status;

    callback = evs_service_callback(EVS_MAINTAIN_RESULT_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_feedback_maintain_query *))callback)(&service_feedback_maintain_status);
    }

    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "ctrlType", service_feedback_maintain_status.ctrlType);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_maintain_status.result);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}
static int evs_service_update_config_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_data_dev_config service_dev_config_data;
    int feedback_data;
    int i = 0;
    memset(&service_dev_config_data, 0, sizeof(service_dev_config_data));
    cJSON *item_equipParamFreq = cJSON_GetObjectItem(root, "equipParamFreq");
    if (item_equipParamFreq != NULL && cJSON_IsNumber(item_equipParamFreq))
    {
        service_dev_config_data.equipParamFreq = item_equipParamFreq->valueint;
    }

    cJSON *item_gunElecFeeq = cJSON_GetObjectItem(root, "gunElecFreq");
    if (item_gunElecFeeq != NULL && cJSON_IsNumber(item_gunElecFeeq))
    {
        service_dev_config_data.gunElecFreq = item_gunElecFeeq->valueint;
    }

    cJSON *item_nonElecFreq = cJSON_GetObjectItem(root, "nonElecFreq");
    if (item_nonElecFreq != NULL && cJSON_IsNumber(item_nonElecFreq))
    {
        service_dev_config_data.nonElecFreq = item_nonElecFreq->valueint;
    }

    cJSON *item_faultWarnings = cJSON_GetObjectItem(root, "faultWarnings");
    if (item_faultWarnings != NULL && cJSON_IsNumber(item_faultWarnings))
    {
        service_dev_config_data.faultWarnings = item_faultWarnings->valueint;
    }

    /*cJSON *item_acMeterFreq = cJSON_GetObjectItem(root, "acMeterFreq");
    if (item_acMeterFreq != NULL && cJSON_IsNumber(item_acMeterFreq))
    {
        service_dev_config_data.acMeterFreq = item_acMeterFreq->valueint;
    }

    cJSON *item_dcMeterFreq = cJSON_GetObjectItem(root, "dcMeterFreq");
    if (item_dcMeterFreq != NULL && cJSON_IsNumber(item_dcMeterFreq))
    {
        service_dev_config_data.dcMeterFreq = item_dcMeterFreq->valueint;
    }*/

    cJSON *item_offlinChaLen = cJSON_GetObjectItem(root, "offlinChaLen");
    if (item_offlinChaLen != NULL && cJSON_IsNumber(item_offlinChaLen))
    {
        service_dev_config_data.offlinChaLen = item_offlinChaLen->valueint;
    }

    cJSON *item_grndLock = cJSON_GetObjectItem(root, "grndLock");
    if (item_grndLock != NULL && cJSON_IsNumber(item_grndLock))
    {
        service_dev_config_data.grndLock = item_grndLock->valueint;
    }

    cJSON *item_doorLock = cJSON_GetObjectItem(root, "doorLock");
    if (item_doorLock != NULL && cJSON_IsNumber(item_doorLock))
    {
        service_dev_config_data.doorLock = item_doorLock->valueint;
    }

    cJSON *item_qrCode = cJSON_GetObjectItem(root, "qrCode");
    if (item_qrCode != NULL && cJSON_IsArray(item_qrCode))
    {
        cJSON *item_arrayData;
        int qrcode_len = 0;
        int qrcode_num = cJSON_GetArraySize(item_qrCode);
        qrcode_num = (qrcode_num <= EVS_MAX_PORT_NUM) ? qrcode_num : EVS_MAX_PORT_NUM;

        for (i = 0; i < qrcode_num; i++)
        {
            item_arrayData = cJSON_GetArrayItem(item_qrCode, i);
            qrcode_len = strlen(item_arrayData->valuestring);
            if (qrcode_len < EVS_MAX_QRCODE_LEN)
            {
                memcpy(service_dev_config_data.qrCode[i], item_arrayData->valuestring, qrcode_len);
            }
            else
            {
                PROTOCOL_TRACE("QRCode is too big!");
            }
        }
    }

    cJSON *item_s2CloseTime = cJSON_GetObjectItem(root, "s2CloseTime");
    if (item_s2CloseTime != NULL && cJSON_IsNumber(item_s2CloseTime))
    {
        service_dev_config_data.s2CloseTime = item_s2CloseTime->valueint;
    }

    cJSON *item_s2OpenTime = cJSON_GetObjectItem(root, "s2OpenTime");
    if (item_s2OpenTime != NULL && cJSON_IsNumber(item_s2OpenTime))
    {
        service_dev_config_data.s2OpenTime = item_s2OpenTime->valueint;
    }

    callback = evs_service_callback(EVS_CONF_UPDATE_SRV);
    if (callback)
    {
        ((int (*)(const evs_data_dev_config *, int *))callback)(&service_dev_config_data, &feedback_data);
    }

    cJSON *response_root = cJSON_CreateObject();
    if ((feedback_data != 10) && (feedback_data != 11)) // 必须处理返回值，并返回结果，默认回复失败
    {
        cJSON_AddNumberToObject(response_root, "resCode", 11);
    }
    else
    {
        cJSON_AddNumberToObject(response_root, "resCode", feedback_data);
    }

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_issue_feeModel_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_issue_feeModel service_feeModel_data;
    evs_service_feedback_feeModel service_feedback_feeModel_data;
    int i = 0;
    memset(&service_feeModel_data, 0, sizeof(service_feeModel_data));
    cJSON *item_feeModelId = cJSON_GetObjectItem(root, "feeModelId");
    if (item_feeModelId != NULL && cJSON_IsString(item_feeModelId))
    {
        char *feeModelId = item_feeModelId->valuestring;
        memcpy(service_feeModel_data.feeModelId, feeModelId, strlen(feeModelId));
    }

    cJSON *item_eleTimeNum = cJSON_GetObjectItem(root, "timeNum");
    if (item_eleTimeNum != NULL && cJSON_IsNumber(item_eleTimeNum))
    {
        service_feeModel_data.timeNum = item_eleTimeNum->valueint;
    }

    cJSON *item_TimeSeg = cJSON_GetObjectItem(root, "timeSeg");
    cJSON *array_timeSeg = NULL;
    if (item_TimeSeg != NULL && cJSON_IsArray(item_TimeSeg))
    {

        for (i = 0; i < service_feeModel_data.timeNum; i++)
        {
            array_timeSeg = cJSON_GetArrayItem(item_TimeSeg, i);
            memcpy(service_feeModel_data.timeSeg[i], array_timeSeg->valuestring, strlen(array_timeSeg->valuestring));
        }
    }

    cJSON *item_chargeFee = cJSON_GetObjectItem(root, "chargeFee");
    cJSON *arrary_chargeFee = NULL;
    if (item_chargeFee != NULL && cJSON_IsArray(item_chargeFee))
    {
        for (i = 0; i < service_feeModel_data.timeNum; i++)
        {
            arrary_chargeFee = cJSON_GetArrayItem(item_chargeFee, i);
            service_feeModel_data.chargeFee[i] = arrary_chargeFee->valueint;
        }
    }

    cJSON *item_serviceFee = cJSON_GetObjectItem(root, "serviceFee");
    cJSON *arrary_serviceFee = NULL;
    if (item_serviceFee != NULL && cJSON_IsArray(item_serviceFee))
    {
        for (i = 0; i < service_feeModel_data.timeNum; i++)
        {
            arrary_serviceFee = cJSON_GetArrayItem(item_serviceFee, i);
            service_feeModel_data.serviceFee[i] = arrary_serviceFee->valueint;
        }
    }

    callback = evs_service_callback(EVS_FEE_MODEL_UPDATE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_issue_feeModel *, evs_service_feedback_feeModel *))callback)(&service_feeModel_data, &service_feedback_feeModel_data);
    }
    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddStringToObject(response_root, "feeModelId", service_feedback_feeModel_data.feeModelId);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_feeModel_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_startCharge_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }
    evs_service_startCharge service_charge_data;
    evs_service_feedback_startCharge service_feedback_startCharge_data;
    memset(&service_charge_data, 0, sizeof(service_charge_data));
    /* Parse json */
    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_charge_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_preTradeNo = cJSON_GetObjectItem(root, "preTradeNo");
    if (item_preTradeNo != NULL && cJSON_IsString(item_preTradeNo))
    {
        memcpy(service_charge_data.preTradeNo, item_preTradeNo->valuestring, strlen(item_preTradeNo->valuestring));
    }

    cJSON *item_startType = cJSON_GetObjectItem(root, "startType");
    if (item_startType != NULL && cJSON_IsNumber(item_startType))
    {
        service_charge_data.startType = item_startType->valueint;
    }

    cJSON *item_chargeMode = cJSON_GetObjectItem(root, "chargeMode");
    if (item_chargeMode != NULL && cJSON_IsNumber(item_chargeMode))
    {
        service_charge_data.chargeMode = item_chargeMode->valueint;
    }

    cJSON *item_limitData = cJSON_GetObjectItem(root, "limitData");
    if (item_limitData != NULL && cJSON_IsNumber(item_limitData))
    {
        service_charge_data.limitData = item_limitData->valueint;
    }

    cJSON *item_stopCode = cJSON_GetObjectItem(root, "stopCode");
    if (item_stopCode != NULL && cJSON_IsNumber(item_stopCode))
    {
        service_charge_data.stopCode = item_stopCode->valueint;
    }

    cJSON *item_startMode = cJSON_GetObjectItem(root, "startMode");
    if (item_startMode != NULL && cJSON_IsNumber(item_startMode))
    {
        service_charge_data.startMode = item_startMode->valueint;
    }

    cJSON *item_insertGunTime = cJSON_GetObjectItem(root, "insertGunTime");
    if (item_insertGunTime != NULL && cJSON_IsNumber(item_insertGunTime))
    {
        service_charge_data.insertGunTime = item_insertGunTime->valueint;
    }

    callback = evs_service_callback(EVS_START_CHARGE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_startCharge *, evs_service_feedback_startCharge *))callback)(&service_charge_data, &service_feedback_startCharge_data);
    }

    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddStringToObject(response_root, "preTradeNo", service_feedback_startCharge_data.preTradeNo);
    cJSON_AddStringToObject(response_root, "tradeNo", service_feedback_startCharge_data.tradeNo);
    cJSON_AddNumberToObject(response_root, "gunNo", service_feedback_startCharge_data.gunNo);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_authCharge_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }
    evs_service_authCharge service_authCharge_data;
    evs_service_feedback_authCharge service_feedback_authCharge_data;

    memset(&service_authCharge_data, 0, sizeof(service_authCharge_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_authCharge_data.gunNo = item_gunNo->valueint;
        gun_auth_process_data[service_authCharge_data.gunNo - 1].getAuthChargeFlag = 1;
    }

    cJSON *item_preTradeNo = cJSON_GetObjectItem(root, "preTradeNo");
    if (item_preTradeNo != NULL && cJSON_IsString(item_preTradeNo))
    {
        char *tradeNo = item_preTradeNo->valuestring;
        memcpy(service_authCharge_data.preTradeNo, tradeNo, strlen(tradeNo));
    }

    cJSON *item_tradeNo = cJSON_GetObjectItem(root, "tradeNo");
    if (item_tradeNo != NULL && cJSON_IsString(item_tradeNo))
    {
        char *devTradeNo = item_tradeNo->valuestring;
        memcpy(service_authCharge_data.tradeNo, devTradeNo, strlen(devTradeNo));
    }

    cJSON *item_startType = cJSON_GetObjectItem(root, "startType");
    if (item_startType != NULL && cJSON_IsNumber(item_startType))
    {
        service_authCharge_data.startType = item_startType->valueint;
    }

    cJSON *item_authCode = cJSON_GetObjectItem(root, "authCode");
    if (item_authCode != NULL && cJSON_IsString(item_authCode))
    {
        char *authCode = item_authCode->valuestring;
        memcpy(service_authCharge_data.authCode, authCode, strlen(authCode));
    }

    cJSON *item_result = cJSON_GetObjectItem(root, "result");
    if (item_result != NULL && cJSON_IsNumber(item_result))
    {
        service_authCharge_data.result = item_result->valueint;
    }

    cJSON *item_chargeMode = cJSON_GetObjectItem(root, "chargeMode");
    if (item_chargeMode != NULL && cJSON_IsNumber(item_chargeMode))
    {
        service_authCharge_data.chargeMode = item_chargeMode->valueint;
    }

    cJSON *item_limitData = cJSON_GetObjectItem(root, "limitData");
    if (item_limitData != NULL && cJSON_IsNumber(item_limitData))
    {
        service_authCharge_data.limitData = item_limitData->valueint;
    }

    cJSON *item_stopCode = cJSON_GetObjectItem(root, "stopCode");
    if (item_stopCode != NULL && cJSON_IsNumber(item_stopCode))
    {
        service_authCharge_data.stopCode = item_stopCode->valueint;
    }

    cJSON *item_startMode = cJSON_GetObjectItem(root, "startMode");
    if (item_startMode != NULL && cJSON_IsNumber(item_startMode))
    {
        service_authCharge_data.startMode = item_startMode->valueint;
    }

    cJSON *item_insertGunTime = cJSON_GetObjectItem(root, "insertGunTime");
    if (item_insertGunTime != NULL && cJSON_IsNumber(item_insertGunTime))
    {
        service_authCharge_data.insertGunTime = item_insertGunTime->valueint;
    }

    callback = evs_service_callback(EVS_AUTH_RESULT_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_authCharge *, evs_service_feedback_authCharge *))callback)(&service_authCharge_data, &service_feedback_authCharge_data);
    }
    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddStringToObject(response_root, "preTradeNo", service_feedback_authCharge_data.preTradeNo);
    cJSON_AddStringToObject(response_root, "tradeNo", service_feedback_authCharge_data.tradeNo);
    cJSON_AddNumberToObject(response_root, "gunNo", service_feedback_authCharge_data.gunNo);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_stopCharge_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_stopCharge service_stopCharge_data;
    evs_service_feedback_stopCharge service_feedback_stopCharge_data;

    memset(&service_stopCharge_data, 0, sizeof(service_stopCharge_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_stopCharge_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_preTradeNo = cJSON_GetObjectItem(root, "preTradeNo");
    if (item_preTradeNo != NULL && cJSON_IsString(item_preTradeNo))
    {
        char *tradeNo = item_preTradeNo->valuestring;
        memcpy(service_stopCharge_data.preTradeNo, tradeNo, strlen(tradeNo));
    }

    cJSON *item_tradeNo = cJSON_GetObjectItem(root, "tradeNo");
    if (item_tradeNo != NULL && cJSON_IsString(item_tradeNo))
    {
        char *devTradeNo = item_tradeNo->valuestring;
        memcpy(service_stopCharge_data.tradeNo, devTradeNo, strlen(devTradeNo));
    }

    cJSON *item_stopReason = cJSON_GetObjectItem(root, "stopReason");
    if (item_stopReason != NULL && cJSON_IsNumber(item_stopReason))
    {
        service_stopCharge_data.stopReason = item_stopReason->valueint;
    }

    callback = evs_service_callback(EVS_STOP_CHARGE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_stopCharge *, evs_service_feedback_stopCharge *))callback)(&service_stopCharge_data, &service_feedback_stopCharge_data);
    }
    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddStringToObject(response_root, "preTradeNo", service_feedback_stopCharge_data.preTradeNo);
    cJSON_AddStringToObject(response_root, "tradeNo", service_feedback_stopCharge_data.tradeNo);
    cJSON_AddNumberToObject(response_root, "gunNo", service_feedback_stopCharge_data.gunNo);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_confirmTrade_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }
    evs_service_confirmTrade service_confirmTrade_data;

    memset(&service_confirmTrade_data, 0, sizeof(service_confirmTrade_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_confirmTrade_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_preTradeNo = cJSON_GetObjectItem(root, "preTradeNo");
    if (item_preTradeNo != NULL && cJSON_IsString(item_preTradeNo))
    {
        char *tradeNo = item_preTradeNo->valuestring;
        memcpy(service_confirmTrade_data.preTradeNo, tradeNo, strlen(tradeNo));
    }

    cJSON *item_tradeNo = cJSON_GetObjectItem(root, "tradeNo");
    if (item_tradeNo != NULL && cJSON_IsString(item_tradeNo))
    {
        char *devTradeNo = item_tradeNo->valuestring;
        memcpy(service_confirmTrade_data.tradeNo, devTradeNo, strlen(devTradeNo));
    }

    cJSON *item_errcode = cJSON_GetObjectItem(root, "errcode");
    if (item_errcode != NULL && cJSON_IsNumber(item_errcode))
    {
        service_confirmTrade_data.errcode = item_errcode->valueint;
    }

    callback = evs_service_callback(EVS_ORDER_CHECK_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_confirmTrade *, void *))callback)(&service_confirmTrade_data, NULL);
    }

    cJSON_Delete(root);

    return 0;
}

static int evs_service_query_log_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }
    evs_service_query_log service_query_log_data;
    evs_service_feedback_query_log feedback_query_log_data;
    char temp[16] = {0};

    memset(&service_query_log_data, 0, sizeof(service_query_log_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_query_log_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_startDate = cJSON_GetObjectItem(root, "startDate");
    if (item_startDate != NULL && cJSON_IsString(item_startDate))
    {
        char *startDate = item_startDate->valuestring;
        service_query_log_data.startDate = strtoint(startDate);
    }

    cJSON *item_stopDate = cJSON_GetObjectItem(root, "stopDate");
    if (item_stopDate != NULL && cJSON_IsString(item_stopDate))
    {
        char *stopDate = item_stopDate->valuestring;
        service_query_log_data.stopDate = strtoint(stopDate);
    }

    cJSON *item_askType = cJSON_GetObjectItem(root, "askType");
    if (item_askType != NULL && cJSON_IsNumber(item_askType))
    {
        service_query_log_data.askType = item_askType->valueint;
    }

    cJSON *item_logQueryNo = cJSON_GetObjectItem(root, "logQueryNo");
    if (item_logQueryNo != NULL && cJSON_IsString(item_logQueryNo))
    {
        char *logQueryNo = item_logQueryNo->valuestring;
        memcpy(service_query_log_data.logQueryNo, logQueryNo, strlen(logQueryNo));
    }

    callback = evs_service_callback(EVS_QUE_DATA_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_query_log *, evs_service_feedback_query_log *))callback)(&service_query_log_data, &feedback_query_log_data);
    }
    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(response_root, "gunNo", feedback_query_log_data.gunNo);

    sprintf(temp, "%d", feedback_query_log_data.startDate);
    cJSON_AddStringToObject(response_root, "startDate", temp);

    sprintf(temp, "%d", feedback_query_log_data.stopDate);
    cJSON_AddStringToObject(response_root, "stopDate", temp);

    cJSON_AddNumberToObject(response_root, "askType", feedback_query_log_data.askType);
    cJSON_AddNumberToObject(response_root, "result", feedback_query_log_data.result);
    cJSON_AddStringToObject(response_root, "logQueryNo", feedback_query_log_data.logQueryNo);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_gateLock_ctrl_handler(const char *request, char **response, int *response_len)
{

    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_gateLock_ctrl service_gateLock_ctrl_data;
    evs_service_feedback_gateLock_ctrl service_feedback_gateLock_ctrl_data;

    memset(&service_gateLock_ctrl_data, 0, sizeof(service_gateLock_ctrl_data));

    cJSON *item_lockNo = cJSON_GetObjectItem(root, "lockNo");
    if (item_lockNo != NULL && cJSON_IsNumber(item_lockNo))
    {
        service_gateLock_ctrl_data.lockNo = item_lockNo->valueint;
    }

    cJSON *item_ctrlFlag = cJSON_GetObjectItem(root, "ctrlFlag");
    if (item_ctrlFlag != NULL && cJSON_IsNumber(item_ctrlFlag))
    {
        service_gateLock_ctrl_data.ctrlFlag = item_ctrlFlag->valueint;
    }

    callback = evs_service_callback(EVS_GATE_LOCK_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_gateLock_ctrl *, evs_service_feedback_gateLock_ctrl *))callback)(&service_gateLock_ctrl_data, &service_feedback_gateLock_ctrl_data);
    }

    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(response_root, "lockNo", service_feedback_gateLock_ctrl_data.lockNo);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_gateLock_ctrl_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_groundLock_ctrl_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_groundLock_ctrl service_groundLock_ctrl_data;
    evs_service_feedback_groundLock_ctrl service_feedback_groundLock_ctrl_data;

    memset(&service_groundLock_ctrl_data, 0, sizeof(service_groundLock_ctrl_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_groundLock_ctrl_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_ctrlFlag = cJSON_GetObjectItem(root, "ctrlFlag");
    if (item_ctrlFlag != NULL && cJSON_IsNumber(item_ctrlFlag))
    {
        service_groundLock_ctrl_data.ctrlFlag = item_ctrlFlag->valueint;
    }

    callback = evs_service_callback(EVS_GROUND_LOCK_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_groundLock_ctrl *, evs_service_feedback_groundLock_ctrl *))callback)(&service_groundLock_ctrl_data, &service_feedback_groundLock_ctrl_data);
    }
    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(response_root, "gunNo", service_feedback_groundLock_ctrl_data.gunNo);
    cJSON_AddNumberToObject(response_root, "reason", service_feedback_groundLock_ctrl_data.reason);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_groundLock_ctrl_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_lockCtrl_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_lockCtrl service_lockCtrl_data;

    memset(&service_lockCtrl_data, 0, sizeof(service_lockCtrl_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_lockCtrl_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_lockParam = cJSON_GetObjectItem(root, "lockParam");
    if (item_lockParam != NULL && cJSON_IsNumber(item_lockParam))
    {
        service_lockCtrl_data.lockParam = item_lockParam->valueint;
    }

    callback = evs_service_callback(EVS_CTRL_LOCK_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_lockCtrl *))callback)(&service_lockCtrl_data);
    }

    cJSON_Delete(root);
    return 0;
}

static int evs_service_dev_maintain_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_dev_maintain service_dev_maintain;

    memset(&service_dev_maintain, 0, sizeof(service_dev_maintain));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_dev_maintain.gunNo = item_gunNo->valueint;
    }

    cJSON *item_ctrlType = cJSON_GetObjectItem(root, "ctrlType");
    if (item_ctrlType != NULL && cJSON_IsNumber(item_ctrlType))
    {
        service_dev_maintain.ctrlType = item_ctrlType->valueint;
    }

    callback = evs_service_callback(EVS_DEV_MAINTAIN_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_dev_maintain *))callback)(&service_dev_maintain);
    }

    cJSON_Delete(root);
    return 0;
}

static int evs_service_orderCharge_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }
    evs_service_orderCharge service_oderCharge_data;
    evs_service_feedback_orderCharge service_feedback_oderCharge_data;

    memset(&service_oderCharge_data, 0, sizeof(service_oderCharge_data));

    cJSON *item_preTradeNo = cJSON_GetObjectItem(root, "preTradeNo");
    if (item_preTradeNo != NULL && cJSON_IsString(item_preTradeNo))
    {
        char *tradeNo = item_preTradeNo->valuestring;
        memcpy(service_oderCharge_data.preTradeNo, tradeNo, strlen(tradeNo));
    }

    cJSON *item_validTime = cJSON_GetObjectItem(root, "validTime");
    if (item_validTime != NULL && cJSON_IsArray(item_validTime))
    {
        int item_num = cJSON_GetArraySize(item_validTime);
        if ((item_num > 0))
        {
            if (item_num <= EVS_MAX_SEG_LEN)
            {
                service_oderCharge_data.num = item_num;
            }
            else
            {
                service_oderCharge_data.num = EVS_MAX_SEG_LEN;
            }
            cJSON *array_validTime = NULL;
            int i = 0;
            for (i = 0; i < service_oderCharge_data.num; i++)
            {
                array_validTime = cJSON_GetArrayItem(item_validTime, i);
                memcpy(service_oderCharge_data.validTime[i], array_validTime->valuestring, strlen(array_validTime->valuestring));
            }

            cJSON *item_kw = cJSON_GetObjectItem(root, "kw");
            cJSON *array_kw;
            if (item_kw != NULL && cJSON_IsArray(item_kw))
            {
                for (i = 0; i < service_oderCharge_data.num; i++)
                {
                    array_kw = cJSON_GetArrayItem(item_kw, i);
                    service_oderCharge_data.kw[i] = array_kw->valueint;
                }
            }
        }
    }

    callback = evs_service_callback(EVS_ORDERLY_CHARGE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_orderCharge *, evs_service_feedback_orderCharge *))callback)(&service_oderCharge_data, &service_feedback_oderCharge_data);
    }
    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddStringToObject(response_root, "preTradeNo", service_feedback_oderCharge_data.preTradeNo);
    cJSON_AddNumberToObject(response_root, "reason", service_feedback_oderCharge_data.reason);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_oderCharge_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_terminalSn_handler(const char *request, char **response, int *response_len)
{
    const char *response_fmt = "{\"result\": %d}";
    char terminalStr[13] = {0};

    int gunNo = 0;

    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        gunNo = item_gunNo->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'gunNo' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_terminalSn = cJSON_GetObjectItem(root, "terminalSn");
    if (item_terminalSn != NULL && cJSON_IsString(item_terminalSn))
    {
        memcpy(terminalStr, item_terminalSn->valuestring, sizeof(terminalStr));
        stringToHex(terminalStr, gun_auth_process_data[gunNo - 1].terminalSn);
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'terminalSn' Error");
        cJSON_Delete(root);
        return -1;
    }

    gun_auth_process_data[gunNo - 1].getTerminalFlag = 1;

    /* 服务应答数据，数据长度通过response，response_len参数传给SDK */
    *response_len = strlen(response_fmt) + 10 + 1;
    *response = (char *)HAL_Malloc(*response_len);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Memory Not Enough");
        return -1;
    }

    memset(*response, 0, *response_len);
    HAL_Snprintf(*response, *response_len, response_fmt, 10);
    *response_len = strlen(*response);

    cJSON_Delete(root);

    return 0;
}

static int evs_service_srvAuthCode_handler(const char *request, char **response, int *response_len)
{

    char tv[15] = {0};
    char strMac2[9] = {0};
    int gunNo = 0;

    const char *response_fmt = "{\"result\": %d}";

    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        gunNo = item_gunNo->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'gunNo' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_timeStr = cJSON_GetObjectItem(root, "timeStr");
    if (item_timeStr != NULL && cJSON_IsString(item_timeStr))
    {
        memcpy(tv, item_timeStr->valuestring, sizeof(tv));
        stringToHex(tv, gun_auth_process_data[gunNo - 1].tradeTime);
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'timeStr' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_mac2 = cJSON_GetObjectItem(root, "tradeMac2");
    if (item_mac2 != NULL && cJSON_IsString(item_mac2))
    {
        memcpy(strMac2, item_mac2->valuestring, sizeof(strMac2));
        stringToHex(strMac2, gun_auth_process_data[gunNo - 1].MAC2);
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'tradeMac2' Error");
        cJSON_Delete(root);
        return -1;
    }

    gun_auth_process_data[gunNo - 1].getServerAuthFlag = 1;

    /* 服务应答数据，数据长度通过response，response_len参数传给SDK */
    *response_len = strlen(response_fmt) + 10 + 1;
    *response = (char *)HAL_Malloc(*response_len);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Memory Not Enough");
        return -1;
    }

    memset(*response, 0, *response_len);
    HAL_Snprintf(*response, *response_len, response_fmt, 10);

    *response_len = strlen(*response);

    cJSON_Delete(root);

    return 0;
}

static int evs_service_update_fun_config_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    evs_service_feedback_dev_fun_config feedback_data = {0};

    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_dev_fun_config service_dev_fun_config_data;
    memset(&service_dev_fun_config_data, 0, sizeof(service_dev_fun_config_data));

    cJSON *item_funCode = cJSON_GetObjectItem(root, "funCode");
    if (item_funCode != NULL && cJSON_IsNumber(item_funCode))
    {
        service_dev_fun_config_data.funCode = item_funCode->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'funCode' Error");
        cJSON_Delete(root);
        return -1;
    }

    if (service_dev_fun_config_data.funCode == 12 || service_dev_fun_config_data.funCode == 13 || service_dev_fun_config_data.funCode == 20 || service_dev_fun_config_data.funCode == 22)
    {
        cJSON *item_confString = cJSON_GetObjectItem(root, "confString");
        if (item_confString != NULL && cJSON_IsString(item_confString))
        {
            char *confString = item_confString->valuestring;
            memcpy(service_dev_fun_config_data.confString, confString, strlen(confString));
        }
        else
        {
            PROTOCOL_TRACE("JSON Parse 'confString' Error");
            cJSON_Delete(root);
            return -1;
        }
    }
    else
    {
        cJSON *item_confInt = cJSON_GetObjectItem(root, "confInt");
        if (item_confInt != NULL && cJSON_IsNumber(item_confInt))
        {
            service_dev_fun_config_data.confInt = item_confInt->valueint;
        }
        else
        {
            PROTOCOL_TRACE("JSON Parse 'confInt' Error");
            cJSON_Delete(root);
            return -1;
        }
    }

    cJSON *item_optSn = cJSON_GetObjectItem(root, "optSn");
    if (item_optSn != NULL && cJSON_IsString(item_optSn))
    {
        char *optString = item_optSn->valuestring;
        memcpy(service_dev_fun_config_data.optSn, optString, strlen(optString));
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'optSn' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_FUN_CONF_UPDATE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_dev_fun_config *, evs_service_feedback_dev_fun_config *))callback)(&service_dev_fun_config_data, &feedback_data);
    }

    cJSON *response_root = cJSON_CreateObject();
    if ((feedback_data.resCode != 10) && (feedback_data.resCode != 11)) // 必须处理返回值，并返回结果，默认回复失败
    {
        cJSON_AddNumberToObject(response_root, "resCode", 11);
    }
    else
    {
        cJSON_AddNumberToObject(response_root, "resCode", feedback_data.resCode);
    }

    cJSON_AddNumberToObject(response_root, "funCode", feedback_data.funCode);
    cJSON_AddStringToObject(response_root, "optSn", feedback_data.optSn);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_get_fun_config_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    evs_service_get_dev_fun_config service_get_fun_config_data;
    evs_service_dev_fun_config service_dev_feedback_fun_config_data;

    memset(&service_get_fun_config_data, 0, sizeof(service_get_fun_config_data));
    memset(&service_dev_feedback_fun_config_data, 0, sizeof(service_dev_feedback_fun_config_data));

    cJSON *item_funCode = cJSON_GetObjectItem(root, "funCode");
    if (item_funCode != NULL && cJSON_IsNumber(item_funCode))
    {
        service_get_fun_config_data.funCode = item_funCode->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'funCode' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_FUN_CONF_GET_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_get_dev_fun_config *, evs_service_dev_fun_config *))callback)(&service_get_fun_config_data, &service_dev_feedback_fun_config_data);
    }

    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "funCode", service_dev_feedback_fun_config_data.funCode);
    cJSON_AddNumberToObject(response_root, "confInt", service_dev_feedback_fun_config_data.confInt);
    cJSON_AddStringToObject(response_root, "confString", service_dev_feedback_fun_config_data.confString);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_feeModel_query_handler(const char *request, char **response, int *response_len)
{
    void *callback = NULL;
    cJSON *root = cJSON_Parse(request);
    cJSON *TimeSegArray, *chargeFeeArray, *serviceFeeArray;
    int i = 0;
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_feedback_feeModel_qurey service_feedback_feeModel_query_data;
    memset(&service_feedback_feeModel_query_data, 0, sizeof(service_feedback_feeModel_query_data));

    callback = evs_service_callback(EVS_FEE_MODEL_QUERY_SRV);
    if (callback)
    {
        ((int (*)(evs_service_feedback_feeModel_qurey *))callback)(&service_feedback_feeModel_query_data);
    }
    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddStringToObject(response_root, "feeModelId", service_feedback_feeModel_query_data.feeModelId);
    cJSON_AddNumberToObject(response_root, "timeNum", service_feedback_feeModel_query_data.timeNum);

    cJSON_AddItemToObject(response_root, "timeSeg", TimeSegArray = cJSON_CreateArray());
    for (i = 0; i < service_feedback_feeModel_query_data.timeNum; i++)
    {
        cJSON_AddItemToArray(TimeSegArray, cJSON_CreateString(service_feedback_feeModel_query_data.timeSeg[i]));
    }

    cJSON_AddItemToObject(response_root, "chargeFee", chargeFeeArray = cJSON_CreateArray());
    for (i = 0; i < service_feedback_feeModel_query_data.timeNum; i++)
    {
        cJSON_AddItemToArray(chargeFeeArray, cJSON_CreateNumber(service_feedback_feeModel_query_data.chargeFee[i]));
    }

    cJSON_AddItemToObject(response_root, "serviceFee", serviceFeeArray = cJSON_CreateArray());
    for (i = 0; i < service_feedback_feeModel_query_data.timeNum; i++)
    {
        cJSON_AddItemToArray(serviceFeeArray, cJSON_CreateNumber(service_feedback_feeModel_query_data.serviceFee[i]));
    }

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_reserve_charge_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_rsvCharge service_reserve_charge_data;
    evs_service_feedback_rsvCharge service_feedback_reserve_charge_data;
    memset(&service_reserve_charge_data, 0, sizeof(service_reserve_charge_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_reserve_charge_data.gunNo = item_gunNo->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'gunNo' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_appoMethod = cJSON_GetObjectItem(root, "appoMethod");
    if (item_appoMethod != NULL && cJSON_IsNumber(item_appoMethod))
    {
        service_reserve_charge_data.appoMethod = item_appoMethod->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'appoMethod' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_appoDelay = cJSON_GetObjectItem(root, "appoDelay");
    if (item_appoDelay != NULL && cJSON_IsNumber(item_appoDelay))
    {
        service_reserve_charge_data.appoDelay = item_appoDelay->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'appoDelay' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_appoData = cJSON_GetObjectItem(root, "appoData");
    if (item_appoData != NULL && cJSON_IsNumber(item_appoDelay))
    {
        service_reserve_charge_data.appoData = item_appoData->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'appoData' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_startChargeTime = cJSON_GetObjectItem(root, "startChargeTime");
    if (item_startChargeTime != NULL && cJSON_IsString(item_startChargeTime))
    {
        char *startChargeTime = item_startChargeTime->valuestring;
        service_reserve_charge_data.startChargeTime = strtoint(startChargeTime);
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'startChargeTime' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_stopChargeTime = cJSON_GetObjectItem(root, "stopChargeTime");
    if (item_stopChargeTime != NULL && cJSON_IsString(item_stopChargeTime))
    {
        char *stopChargeTime = item_stopChargeTime->valuestring;
        service_reserve_charge_data.stopChargeTime = strtoint(stopChargeTime);
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'stopChargeTime' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_RSV_CHARGE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_rsvCharge *, evs_service_feedback_rsvCharge *))callback)(&service_reserve_charge_data, &service_feedback_reserve_charge_data);
    }
    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "gunNo", service_feedback_reserve_charge_data.gunNo);
    cJSON_AddNumberToObject(response_root, "appoMethod", service_feedback_reserve_charge_data.appoMethod);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_reserve_charge_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_ble_secret_update_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_blesecret_update service_ble_secret_data;
    evs_service_feedback_blesecret_update service_feedback_ble_secret_data;
    memset(&service_ble_secret_data, 0, sizeof(service_ble_secret_data));
    memset(&service_feedback_ble_secret_data, 0, sizeof(service_feedback_ble_secret_data));

    cJSON *item_bleSecretKey = cJSON_GetObjectItem(root, "bleSecretKey");
    if (item_bleSecretKey != NULL && cJSON_IsString(item_bleSecretKey))
    {
        char *bleSecretKey = item_bleSecretKey->valuestring;
        memcpy(service_ble_secret_data.bleSecretKey, bleSecretKey, strlen(bleSecretKey));
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'bleSecretKey' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_updateTime = cJSON_GetObjectItem(root, "updateTime");
    if (item_updateTime != NULL && cJSON_IsString(item_updateTime))
    {
        char *updateTime = item_updateTime->valuestring;
        service_ble_secret_data.updateTime = strtoint(updateTime);
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'updateTime' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_BLE_SECRET_UPDATE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_blesecret_update *, evs_service_feedback_blesecret_update *))callback)(&service_ble_secret_data, &service_feedback_ble_secret_data);
    }
    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddStringToObject(response_root, "bleSecretKey", service_feedback_ble_secret_data.bleSecretKey);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_ble_secret_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_ble_auth_list_get_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    cJSON *macListArray = NULL;
    int i = 0;

    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_event_ble_plug_charge_info service_feedback_ble_plugchg_data;
    memset(&service_feedback_ble_plugchg_data, 0, sizeof(service_feedback_ble_plugchg_data));

    callback = evs_service_callback(EVS_BLE_PLUG_CHG_INFO_GET_SRV);
    if (callback)
    {
        ((int (*)(evs_event_ble_plug_charge_info *))callback)(&service_feedback_ble_plugchg_data);
    }
    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "plugChgStatus", service_feedback_ble_plugchg_data.plugChgStatus);
    cJSON_AddNumberToObject(response_root, "macNum", service_feedback_ble_plugchg_data.macNum);

    cJSON_AddItemToObject(response_root, "macList", macListArray = cJSON_CreateArray());
    for (i = 0; i < service_feedback_ble_plugchg_data.macNum; i++)
    {
        cJSON_AddItemToArray(macListArray, cJSON_CreateString(service_feedback_ble_plugchg_data.macList[i]));
    }

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_ble_auth_list_confirm_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);

    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_ble_plug_charge_info_conf service_ble_plugchg_confirm;
    memset(&service_ble_plugchg_confirm, 0, sizeof(service_ble_plugchg_confirm));

    cJSON *item_macNum = cJSON_GetObjectItem(root, "macNum");
    if (item_macNum != NULL && cJSON_IsNumber(item_macNum))
    {
        service_ble_plugchg_confirm.macNum = item_macNum->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'macNum' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_BLE_PLUG_CHG_INFO_CONF_SRV);
    if (callback)
    {
        ((int (*)(evs_service_ble_plug_charge_info_conf *))callback)(&service_ble_plugchg_confirm);
    }

    cJSON_Delete(root);

    return 0;
}

static int evs_service_ble_auth_list_clean_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);

    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_blelist_clean service_blelist_clean_data;
    evs_service_feedback_blelist_clean service_feedback_blelist_clean_data;
    memset(&service_blelist_clean_data, 0, sizeof(service_blelist_clean_data));
    memset(&service_feedback_blelist_clean_data, 0, sizeof(service_feedback_blelist_clean_data));

    cJSON *item_cleanType = cJSON_GetObjectItem(root, "cleanType");
    if (item_cleanType != NULL && cJSON_IsNumber(item_cleanType))
    {
        service_blelist_clean_data.cleanType = item_cleanType->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'cleanType' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_macInfo = cJSON_GetObjectItem(root, "macInfo");
    if (item_macInfo != NULL && cJSON_IsString(item_macInfo))
    {
        char *macInfo = item_macInfo->valuestring;
        memcpy(service_blelist_clean_data.macInfo, macInfo, strlen(macInfo));
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'macInfo' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_BLE_AUTH_LIST_CLEAN_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_blelist_clean *, evs_service_feedback_blelist_clean *))callback)(&service_blelist_clean_data, &service_feedback_blelist_clean_data);
    }

    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "cleanType", service_feedback_blelist_clean_data.cleanType);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_blelist_clean_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }
    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_ble_reset_handler(const char *request, char **response, int *response_len)
{
    void *callback;

    evs_service_feedback_ble_reset service_feedback_ble_reset_data;
    memset(&service_feedback_ble_reset_data, 0, sizeof(service_feedback_ble_reset_data));

    callback = evs_service_callback(EVS_BLE_RESET_SRV);
    if (callback)
    {
        ((int (*)(evs_service_feedback_ble_reset *))callback)(&service_feedback_ble_reset_data);
    }
    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "result", service_feedback_ble_reset_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_get_parts_config_handler(const char *request, char **response, int *response_len)
{

    void *callback;
    cJSON *root = cJSON_Parse(request);
    evs_service_parts_config_get service_parts_config_data;
    evs_service_feedback_config_parts_get service_feedback_get_config_data;

    memset(&service_parts_config_data, 0, sizeof(service_parts_config_data));
    memset(&service_feedback_get_config_data, 0, sizeof(service_feedback_get_config_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_parts_config_data.gunNo = item_gunNo->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'gunNo' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_partsType = cJSON_GetObjectItem(root, "partsType");
    if (item_partsType != NULL && cJSON_IsNumber(item_partsType))
    {
        service_parts_config_data.partsType = item_partsType->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'partsType' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_funCode = cJSON_GetObjectItem(root, "funCode");
    if (item_funCode != NULL && cJSON_IsNumber(item_funCode))
    {
        service_parts_config_data.funCode = item_funCode->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'funCode' Error");
        cJSON_Delete(root);
        return -1;
    }

    callback = evs_service_callback(EVS_PILE_PARTS_CONF_GET_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_parts_config_get *, evs_service_feedback_config_parts_get *))callback)(&service_parts_config_data, &service_feedback_get_config_data);
    }

    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "result", service_feedback_get_config_data.result);
    cJSON_AddNumberToObject(response_root, "confInt", service_feedback_get_config_data.confInt);
    cJSON_AddStringToObject(response_root, "confString", service_feedback_get_config_data.confString);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_parts_config_handler(const char *request, char **response, int *response_len)
{
    PROTOCOL_TRACE("recv parts config");
    void *callback;

    cJSON *root = cJSON_Parse(request);

    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_config_parts service_parts_config_data;
    evs_service_feedback_config_parts result = {0};
    memset(&service_parts_config_data, 0, sizeof(evs_service_config_parts));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_parts_config_data.gunNo = item_gunNo->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'gunNo' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_opSn = cJSON_GetObjectItem(root, "optSn");
    if (item_opSn != NULL && cJSON_IsString(item_opSn))
    {
        memcpy(service_parts_config_data.optSn, item_opSn->valuestring, strlen(item_opSn->valuestring));
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'optSn' Error");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *item_funCode = cJSON_GetObjectItem(root, "funCode");
    if (item_funCode != NULL && cJSON_IsNumber(item_funCode))
    {
        service_parts_config_data.funCode = item_funCode->valueint;
    }
    else
    {
        PROTOCOL_TRACE("JSON Parse 'funCode' Error");
        cJSON_Delete(root);
        return -1;
    }

    // cJSON *item_conf = cJSON_GetObjectItem(root, "confInfo");
    // if (item_conf != NULL && cJSON_IsObject(item_conf))
    // {
    //     cJSON *item_power = cJSON_GetObjectItem(item_conf, "powerAlloc");
    //     if (item_power != NULL && cJSON_IsNumber(item_power))
    //     {
    //         service_parts_config_data.powerAlloc = item_power->valueint;
    //     }

    //     cJSON *item_partsType = cJSON_GetObjectItem(item_conf, "partsType");
    //     if (item_partsType != NULL && cJSON_IsNumber(item_partsType))
    //     {
    //         service_parts_config_data.partsType = item_partsType->valueint;
    //     }

    //     cJSON *item_sendMode = cJSON_GetObjectItem(item_conf, "sendMode");
    //     if (item_sendMode != NULL && cJSON_IsNumber(item_sendMode))
    //     {
    //         service_parts_config_data.sendMode = item_sendMode->valueint;
    //     }

    //     cJSON *item_ccuPeriod = cJSON_GetObjectItem(item_conf, "ccuPeriod");
    //     if (item_ccuPeriod != NULL && cJSON_IsNumber(item_ccuPeriod))
    //     {
    //         service_parts_config_data.ccuPeriod = item_ccuPeriod->valueint;
    //     }

    //     cJSON *item_tcuPeriod = cJSON_GetObjectItem(item_conf, "tcuPeriod");
    //     if (item_tcuPeriod != NULL && cJSON_IsNumber(item_tcuPeriod))
    //     {
    //         service_parts_config_data.tcuPeriod = item_tcuPeriod->valueint;
    //     }
    // }
    // else
    // {
    //     PROTOCOL_TRACE("JSON Parse 'confInfo' Error");
    //     cJSON_Delete(root);
    //     return -1;
    // }

    if (service_parts_config_data.funCode == 11)
    {
        cJSON *item_confString = cJSON_GetObjectItem(root, "confString");
        if (item_confString != NULL && cJSON_IsString(item_confString))
        {
            char *confString = item_confString->valuestring;
            memcpy(service_parts_config_data.confString, confString, strlen(confString));
        }
        else
        {
            PROTOCOL_TRACE("JSON Parse 'confString' Error");
            cJSON_Delete(root);
            return -1;
        }
    }
    else if (service_parts_config_data.funCode == 10)
    {
        cJSON *item_confInt = cJSON_GetObjectItem(root, "confInt");
        if (item_confInt != NULL && cJSON_IsNumber(item_confInt))
        {
            service_parts_config_data.confInt = item_confInt->valueint;
        }
        else
        {
            PROTOCOL_TRACE("JSON Parse 'confInt' Error");
            cJSON_Delete(root);
            return -1;
        }
    }

    callback = evs_service_callback(EVS_PILE_PARTS_CONF_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_config_parts *, evs_service_feedback_config_parts *))callback)(&service_parts_config_data, &result);
    }

    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(response_root, "resCode", result.resCode);
    cJSON_AddStringToObject(response_root, "optSn", result.optSn);
    cJSON_AddNumberToObject(response_root, "funCode", result.funCode);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_vin_whitelist_handler(const char *request, char **response, int *response_len)
{
    void *callback;
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    evs_service_vinList_update service_vinList_data;
    evs_service_feedback_vinList_update service_feedback_vinList_data;
    int i = 0;
    memset(&service_vinList_data, 0, sizeof(evs_service_vinList_update));

    cJSON *item_updateSN = cJSON_GetObjectItem(root, "updateSN");
    if (item_updateSN != NULL && cJSON_IsString(item_updateSN))
    {
        char *updateSN = item_updateSN->valuestring;
        memcpy(service_vinList_data.updateSN, updateSN, strlen(updateSN));
    }

    cJSON *item_optType = cJSON_GetObjectItem(root, "optType");
    if (item_optType != NULL && cJSON_IsNumber(item_optType))
    {
        service_vinList_data.optType = item_optType->valueint;
    }

    cJSON *item_num = cJSON_GetObjectItem(root, "totalVINCnt");
    if (item_num != NULL && cJSON_IsNumber(item_num))
    {
        service_vinList_data.totalVINCnt = item_num->valueint;
    }

    item_num = cJSON_GetObjectItem(root, "packTotalCnt");
    if (item_num != NULL && cJSON_IsNumber(item_num))
    {
        service_vinList_data.packTotalCnt = item_num->valueint;
    }

    item_num = cJSON_GetObjectItem(root, "packCnt");
    if (item_num != NULL && cJSON_IsNumber(item_num))
    {
        service_vinList_data.packCnt = item_num->valueint;
    }

    item_num = cJSON_GetObjectItem(root, "packVINCnt");
    if (item_num != NULL && cJSON_IsNumber(item_num))
    {
        service_vinList_data.packVINCnt = item_num->valueint;
    }

    cJSON *item_vinList = cJSON_GetObjectItem(root, "vinList");
    cJSON *array_vinList = NULL;
    if (item_vinList != NULL && cJSON_IsArray(item_vinList))
    {

        for (i = 0; i < service_vinList_data.packVINCnt; i++)
        {
            array_vinList = cJSON_GetArrayItem(array_vinList, i);
            memcpy(service_vinList_data.vinList[i], array_vinList->valuestring, strlen(array_vinList->valuestring));
        }
    }

    callback = evs_service_callback(EVS_VIN_LIST_UPDATE_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_vinList_update *, evs_service_feedback_vinList_update *))callback)(&service_vinList_data, &service_feedback_vinList_data);
    }

    cJSON *response_root = cJSON_CreateObject();
    cJSON_AddStringToObject(response_root, "updateSN", service_feedback_vinList_data.updateSN);
    cJSON_AddNumberToObject(response_root, "optType", service_feedback_vinList_data.optType);
    cJSON_AddNumberToObject(response_root, "result", service_feedback_vinList_data.result);

    *response = cJSON_PrintUnformatted(response_root);

    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_get_trade_handler(const char *request, char **response, int *response_len)
{

    void *callback;
    cJSON *root = cJSON_Parse(request);
    evs_service_trade_get service_get_trade_data;
    evs_service_feedback_trade_get service_dev_feedback_trade_data;

    memset(&service_get_trade_data, 0, sizeof(service_get_trade_data));
    memset(&service_dev_feedback_trade_data, 0, sizeof(service_dev_feedback_trade_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_get_trade_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_askType = cJSON_GetObjectItem(root, "askType");
    if (item_askType != NULL && cJSON_IsNumber(item_askType))
    {
        service_get_trade_data.askType = item_askType->valueint;
    }

    cJSON *item_preTradeNo = cJSON_GetObjectItem(root, "preTradeNo");
    if (item_preTradeNo != NULL && cJSON_IsString(item_preTradeNo))
    {
        char *tradeNo = item_preTradeNo->valuestring;
        memcpy(service_get_trade_data.preTradeNo, tradeNo, strlen(tradeNo));
    }

    cJSON *item_tradeNo = cJSON_GetObjectItem(root, "tradeNo");
    if (item_tradeNo != NULL && cJSON_IsString(item_tradeNo))
    {
        char *tradeNo = item_tradeNo->valuestring;
        memcpy(service_get_trade_data.tradeNo, tradeNo, strlen(tradeNo));
    }

    cJSON *item_startDate = cJSON_GetObjectItem(root, "startDate");
    if (item_startDate != NULL && cJSON_IsString(item_startDate))
    {
        char *startDate = item_startDate->valuestring;
        memcpy(service_get_trade_data.startDate, startDate, strlen(startDate));
    }

    cJSON *item_stopDate = cJSON_GetObjectItem(root, "stopDate");
    if (item_stopDate != NULL && cJSON_IsString(item_stopDate))
    {
        char *stopDate = item_stopDate->valuestring;
        memcpy(service_get_trade_data.stopDate, stopDate, strlen(stopDate));
    }

    callback = evs_service_callback(EVS_ORDER_ASK_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_trade_get *, evs_service_feedback_trade_get *))callback)(&service_get_trade_data, &service_dev_feedback_trade_data);
    }

    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "gunNo", service_dev_feedback_trade_data.gunNo);
    cJSON_AddNumberToObject(response_root, "askType", service_dev_feedback_trade_data.askType);
    cJSON_AddStringToObject(response_root, "preTradeNo", service_dev_feedback_trade_data.preTradeNo);
    cJSON_AddStringToObject(response_root, "tradeNo", service_dev_feedback_trade_data.tradeNo);
    cJSON_AddStringToObject(response_root, "startDate", service_dev_feedback_trade_data.startDate);
    cJSON_AddStringToObject(response_root, "stopDate", service_dev_feedback_trade_data.stopDate);
    cJSON_AddNumberToObject(response_root, "askResult", service_dev_feedback_trade_data.askResult);
    cJSON_AddNumberToObject(response_root, "tradeCnt", service_dev_feedback_trade_data.tradeCnt);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_get_meter_handler(const char *request, char **response, int *response_len)
{

    void *callback;
    cJSON *root = cJSON_Parse(request);
    evs_service_meter_get service_get_meter_data;
    evs_service_feedback_meter_get service_dev_feedback_meter_data;

    memset(&service_get_meter_data, 0, sizeof(service_get_meter_data));
    memset(&service_dev_feedback_meter_data, 0, sizeof(service_dev_feedback_meter_data));

    cJSON *item_gunNo = cJSON_GetObjectItem(root, "gunNo");
    if (item_gunNo != NULL && cJSON_IsNumber(item_gunNo))
    {
        service_get_meter_data.gunNo = item_gunNo->valueint;
    }

    cJSON *item_askDate = cJSON_GetObjectItem(root, "askDate");
    if (item_askDate != NULL && cJSON_IsString(item_askDate))
    {
        char *askDate = item_askDate->valuestring;
        memcpy(service_get_meter_data.askDate, askDate, strlen(askDate));
    }

    cJSON *item_askType = cJSON_GetObjectItem(root, "askType");
    if (item_askType != NULL && cJSON_IsNumber(item_askType))
    {
        service_get_meter_data.askType = item_askType->valueint;
    }

    callback = evs_service_callback(EVS_Meter_ASK_SRV);
    if (callback)
    {
        ((int (*)(const evs_service_meter_get *, evs_service_feedback_meter_get *))callback)(&service_get_meter_data, &service_dev_feedback_meter_data);
    }

    cJSON *response_root = cJSON_CreateObject();

    cJSON_AddNumberToObject(response_root, "gunNo", service_dev_feedback_meter_data.gunNo);
    cJSON_AddStringToObject(response_root, "askDate", service_dev_feedback_meter_data.askDate);
    cJSON_AddNumberToObject(response_root, "askType", service_dev_feedback_meter_data.askType);
    cJSON_AddNumberToObject(response_root, "askResult", service_dev_feedback_meter_data.askResult);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL)
    {
        PROTOCOL_TRACE("Json Not Exist!");
        return -1;
    }
    else
    {
        *response_len = strlen(*response);
    }

    cJSON_Delete(response_root);

    return 0;
}

static int evs_service_time_sync_handler(const char *request, char **response, int *response_len)
{
    char time_buf[20] = {0};
    cJSON *root = cJSON_Parse(request);

    cJSON *item_time = cJSON_GetObjectItem(root, "srvTime");
    if (item_time != NULL && cJSON_IsString(item_time))
    {
        memcpy(time_buf, item_time->valuestring, strlen(item_time->valuestring));
    }

    PROTOCOL_TRACE("Current Timestamp: %s", time_buf);
    unsigned int time = strtoint(time_buf);

    void *callback;
    callback = evs_service_callback(EVS_TIME_SYNC);
    if (callback)
    {
        ((int (*)(unsigned int))callback)(time);
    }

    return 0;
}

/** 事件回调：接收到云端下发的服务请求 **/
static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
                                              const char *request, const int request_len,
                                              char **response, int *response_len)
{
    int add_result = 0;

    const char *response_fmt = "{\"Result\": %d}";

    PROTOCOL_TRACE("Service Request Received, Service ID: %.*s, Payload: %s", serviceid_len, serviceid, request);

    if (strlen("feeModelTwUpdateSrv") == serviceid_len && memcmp("feeModelTwUpdateSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_issue_feeModel_handler(request, response, response_len);
    }
    if (strlen("startChargeSrv") == serviceid_len && memcmp("startChargeSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_startCharge_handler(request, response, response_len);
    }

    if (strlen("authResultSrv") == serviceid_len && memcmp("authResultSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_authCharge_handler(request, response, response_len);
    }

    if (strlen("stopChargeSrv") == serviceid_len && memcmp("stopChargeSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_stopCharge_handler(request, response, response_len);
    }

    if (strlen("orderCheckSrv") == serviceid_len && memcmp("orderCheckSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_confirmTrade_handler(request, response, response_len);
    }
    if (strlen("queDataSrv") == serviceid_len && memcmp("queDataSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_query_log_handler(request, response, response_len);
    }
    if (strlen("gateLockSrv") == serviceid_len && memcmp("gateLockSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_gateLock_ctrl_handler(request, response, response_len);
    }
    if (strlen("groundLockSrv") == serviceid_len && memcmp("groundLockSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_groundLock_ctrl_handler(request, response, response_len);
    }
    if (strlen("ctrlLockSrv") == serviceid_len && memcmp("ctrlLockSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_lockCtrl_handler(request, response, response_len);
    }
    if (strlen("devMaintainCtrlSrv") == serviceid_len && memcmp("devMaintainCtrlSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_dev_maintain_handler(request, response, response_len);
    }
    if (strlen("acOrderlyChargeSrv") == serviceid_len && memcmp("acOrderlyChargeSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_orderCharge_handler(request, response, response_len);
    }
    if (strlen("dcOrderlyChargeSrv") == serviceid_len && memcmp("dcOrderlyChargeSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_orderCharge_handler(request, response, response_len);
    }
    if (strlen("confUpdateCtrlSrv") == serviceid_len && memcmp("confUpdateCtrlSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_update_config_handler(request, response, response_len);
    }
    if (strlen("getDevConfSrv") == serviceid_len && memcmp("getDevConfSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_get_config_handler(request, response, response_len);
    }
    if (strlen("devMaintainQuerySrv") == serviceid_len && memcmp("devMaintainQuerySrv", serviceid, serviceid_len) == 0)
    {
        evs_service_get_Maintain_handler(request, response, response_len);
    }
    if (strlen("srvAuthCodeSrv") == serviceid_len && memcmp("srvAuthCodeSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_srvAuthCode_handler(request, response, response_len);
    }
    if (strlen("termSnSrv") == serviceid_len && memcmp("termSnSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_terminalSn_handler(request, response, response_len);
    }
    if (strlen("funConfUpdateDataSrv") == serviceid_len && memcmp("funConfUpdateDataSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_update_fun_config_handler(request, response, response_len);
    }
    if (strlen("getFunConfSrv") == serviceid_len && memcmp("getFunConfSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_get_fun_config_handler(request, response, response_len);
    }
    if (strlen("feeModelTwQuerySrv") == serviceid_len && memcmp("feeModelTwQuerySrv", serviceid, serviceid_len) == 0)
    {
        evs_service_feeModel_query_handler(request, response, response_len);
    }
    if (strlen("rsvChargeSrv") == serviceid_len && memcmp("rsvChargeSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_reserve_charge_handler(request, response, response_len);
    }
    if (strlen("bleSecretUpdateSrv") == serviceid_len && memcmp("bleSecretUpdateSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_ble_secret_update_handler(request, response, response_len);
    }
    if (strlen("getBlePlugChgInfoSrv") == serviceid_len && memcmp("getBlePlugChgInfoSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_ble_auth_list_get_handler(request, response, response_len);
    }
    if (strlen("blePlugChgConfSrv") == serviceid_len && memcmp("blePlugChgConfSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_ble_auth_list_confirm_handler(request, response, response_len);
    }
    if (strlen("bleAuthListCleanSrv") == serviceid_len && memcmp("bleAuthListCleanSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_ble_auth_list_clean_handler(request, response, response_len);
    }
    if (strlen("bleResetSrv") == serviceid_len && memcmp("bleResetSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_ble_reset_handler(request, response, response_len);
    }

    if (strlen("partsConfUpdateSrv") == serviceid_len && memcmp("partsConfUpdateSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_parts_config_handler(request, response, response_len);
    }

    if (strlen("getPartsConfSrv") == serviceid_len && memcmp("getPartsConfSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_get_parts_config_handler(request, response, response_len);
    }

    if (strlen("vinListUpdateSrv") == serviceid_len && memcmp("vinListUpdateSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_vin_whitelist_handler(request, response, response_len);
    }

    if (strlen("tradeRecordAskSrv") == serviceid_len && memcmp("tradeRecordAskSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_get_trade_handler(request, response, response_len);
    }

    if (strlen("meterRecordAskSrv") == serviceid_len && memcmp("meterRecordAskSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_get_meter_handler(request, response, response_len);
    }

    if (strlen("timeSyncSrv") == serviceid_len && memcmp("timeSyncSrv", serviceid, serviceid_len) == 0)
    {
        evs_service_time_sync_handler(request, response, response_len);
    }

    if (*response == NULL)
    {
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = (char *)HAL_Malloc(*response_len);
        memset(*response, 0, *response_len);
        HAL_Snprintf(*response, *response_len, response_fmt, add_result);
        *response_len = strlen(*response);
    }

    return 0;
}

static int send_property_dcPile(evs_property_dcPile *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "dcDeRealItyTwData", body = cJSON_CreateObject());

    cJSON_AddNumberToObject(body, "netType", data->netType);
    cJSON_AddNumberToObject(body, "sigVal", data->sigVal);

    cJSON_AddNumberToObject(body, "netId", data->netId);

    cJSON_AddNumberToObject(body, "acVolA", data->acVolA);
    cJSON_AddNumberToObject(body, "acCurA", data->acCurA);

    cJSON_AddNumberToObject(body, "acVolB", data->acVolB);
    cJSON_AddNumberToObject(body, "acCurB", data->acCurB);

    cJSON_AddNumberToObject(body, "acVolC", data->acVolC);
    cJSON_AddNumberToObject(body, "acCurC", data->acCurC);

    cJSON_AddNumberToObject(body, "caseTemp", data->caseTemp);

    cJSON_AddStringToObject(body, "feeModelId", data->feeModelId);

    cJSON_AddNumberToObject(body, "outletTemp", data->outletTemp);
    cJSON_AddNumberToObject(body, "inletTemp", data->inletTemp);

    cJSON_AddNumberToObject(body, "caseHumidity", data->caseHumidity);
    cJSON_AddNumberToObject(body, "totalRam", data->totalRam);
    cJSON_AddNumberToObject(body, "ramUseRate", data->ramUseRate);

    cJSON_AddNumberToObject(body, "totalRom", data->totalRom);
    cJSON_AddNumberToObject(body, "romUseRate", data->romUseRate);

    cJSON_AddNumberToObject(body, "cpuUseRate", data->cpuUseRate);

    // sprintf(time, "%d", data->sysStartTime);
    // cJSON_AddStringToObject(body, "sysStartTime", time);
    cJSON_AddNumberToObject(body, "sysStartTime", data->sysStartTime);
    cJSON_AddNumberToObject(body, "sysWorkTime", data->sysWorkTime);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_property_acPile(evs_property_acPile *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "acDeRealItyData", body = cJSON_CreateObject());

    cJSON_AddNumberToObject(body, "netType", data->netType);
    cJSON_AddNumberToObject(body, "sigVal", data->sigVal);
    cJSON_AddNumberToObject(body, "netId", data->netId);

    cJSON_AddNumberToObject(body, "acVolA", data->acVolA);
    cJSON_AddNumberToObject(body, "acCurA", data->acCurA);

    cJSON_AddNumberToObject(body, "acVolB", data->acVolB);
    cJSON_AddNumberToObject(body, "acCurB", data->acCurB);

    cJSON_AddNumberToObject(body, "acVolC", data->acVolC);
    cJSON_AddNumberToObject(body, "acCurC", data->acCurC);

    cJSON_AddNumberToObject(body, "caseTemp", data->caseTemp);

    cJSON_AddStringToObject(body, "feeModelId", data->feeModelId);

    cJSON_AddNumberToObject(body, "totalRam", data->totalRam);
    cJSON_AddNumberToObject(body, "ramUseRate", data->ramUseRate);
    cJSON_AddNumberToObject(body, "totalRom", data->totalRom);
    cJSON_AddNumberToObject(body, "romUseRate", data->romUseRate);
    cJSON_AddNumberToObject(body, "cpuUseRate", data->cpuUseRate);

    cJSON_AddStringToObject(body, "netSigQua", data->netSigQua);
    cJSON_AddStringToObject(body, "devRunSampVal", data->devRunSampVal);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 交流非充电过程实时监测属性
static int send_property_ac_nonwork(evs_property_ac_nonWork *data)
{
    int res = 0;
    char *payload = NULL;
    char Temp[20] = {0};
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "acGunIdleItyData", body = cJSON_CreateObject());
    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(body, "workStatus", data->workStatus);

    cJSON_AddNumberToObject(body, "conStatus", data->conStatus);
    cJSON_AddNumberToObject(body, "outRelayStatus", data->outRelayStatus);
    cJSON_AddNumberToObject(body, "eLockStatus", data->eLockStatus);
    cJSON_AddNumberToObject(body, "gunTemp", data->gunTemp);

    cJSON_AddNumberToObject(body, "acVolA", data->acVolA);
    cJSON_AddNumberToObject(body, "acCurA", data->acCurA);

    cJSON_AddNumberToObject(body, "acVolB", data->acVolB);
    cJSON_AddNumberToObject(body, "acCurB", data->acCurB);

    cJSON_AddNumberToObject(body, "acVolC", data->acVolC);
    cJSON_AddNumberToObject(body, "acCurC", data->acCurC);

    sprintf(Temp, "%lld", data->sumMeter);
    cJSON_AddStringToObject(body, "sumMeter", Temp);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 交流充电过程实时监测属性
static int send_property_ac_work(evs_property_ac_work *data)
{
    int res = 0;
    int i = 0;
    char *payload = NULL;
    char Temp[20] = {0};
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "acGunRunItyData", body = cJSON_CreateObject());

    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(body, "workStatus", data->workStatus);
    cJSON_AddNumberToObject(body, "conStatus", data->conStatus);
    cJSON_AddNumberToObject(body, "eLockStatus", data->eLockStatus);
    cJSON_AddNumberToObject(body, "outRelayStatus", data->outRelayStatus);
    cJSON_AddNumberToObject(body, "gunTemp", data->gunTemp);

    cJSON_AddNumberToObject(body, "acVolA", data->acVolA);
    cJSON_AddNumberToObject(body, "acCurA", data->acCurA);

    cJSON_AddNumberToObject(body, "acVolB", data->acVolB);
    cJSON_AddNumberToObject(body, "acCurB", data->acCurB);

    cJSON_AddNumberToObject(body, "acVolC", data->acVolC);
    cJSON_AddNumberToObject(body, "acCurC", data->acCurC);

    cJSON_AddStringToObject(body, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(body, "tradeNo", data->tradeNo);

    cJSON_AddNumberToObject(body, "realPower", data->realPower);
    cJSON_AddNumberToObject(body, "chgTime", data->chgTime);

    cJSON_AddNumberToObject(body, "PwmDutyRadio", data->PwmDutyRadio);
    cJSON_AddNumberToObject(body, "s2SwhActNum", data->s2SwhActNum);

    memset(Temp, 0, 20);
    sprintf(Temp, "%lld", data->meterStartVal);
    cJSON_AddStringToObject(body, "meterStartVal", Temp);

    memset(Temp, 0, 20);
    sprintf(Temp, "%lld", data->meterRealVal);
    cJSON_AddStringToObject(body, "meterRealVal", Temp);

    cJSON_AddNumberToObject(body, "totalElect", data->totalElect);
    cJSON_AddNumberToObject(body, "totalCost", data->totalCost);
    cJSON_AddNumberToObject(body, "totalPowerCost", data->totalPowerCost);
    cJSON_AddNumberToObject(body, "totalServCost", data->totalServCost);

    cJSON_AddNumberToObject(body, "timeNum", data->timeNum);

    int segCnt = data->timeNum;
    if (segCnt > 0 && segCnt < EVS_MAX_MODEL_DEVSEG + 1)
    {
        int str_len = 0;

        char *tempTarriff = (char *)HAL_Malloc(8 * segCnt);
        memset(tempTarriff, 0, 8 * segCnt);

        for (i = 0; i < segCnt; i++)
        {
            sprintf(tempTarriff + str_len, "%d,", data->partElect[i]);
            str_len = strlen(tempTarriff);
        }

        tempTarriff[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "partElect", tempTarriff);

        str_len = 0;
        memset(tempTarriff, 0, 8 * segCnt);
        for (i = 0; i < segCnt; i++)
        {
            sprintf(tempTarriff + str_len, "%d,", data->chargeFee[i]);
            str_len = strlen(tempTarriff);
        }

        tempTarriff[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "chargeFee", tempTarriff);

        str_len = 0;
        memset(tempTarriff, 0, 8 * segCnt);
        for (i = 0; i < segCnt; i++)
        {
            sprintf(tempTarriff + str_len, "%d,", data->serviceFee[i]);
            str_len = strlen(tempTarriff);
        }

        tempTarriff[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "serviceFee", tempTarriff);

        HAL_Free(tempTarriff);
    }

    cJSON_AddNumberToObject(body, "startPoint", data->startPoint);
    cJSON_AddNumberToObject(body, "crossPoints", data->crossPoints);

    int pointCnt = data->crossPoints;
    if (pointCnt > 0 && pointCnt < EVS_MAX_MODEL_DEVSEG + 1)
    {
        int str_len = 0;

        char *tempPoint = (char *)HAL_Malloc(8 * pointCnt);
        memset(tempPoint, 0, 8 * pointCnt);

        for (i = 0; i < pointCnt; i++)
        {
            sprintf(tempPoint + str_len, "%d,", data->pointsElect[i]);
            str_len = strlen(tempPoint);
        }

        tempPoint[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "pointsElect", tempPoint);

        HAL_Free(tempPoint);
    }

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 直流充电过程BMS实时监测属性
static int send_property_bms(evs_property_BMS *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "dcBmsRunItyData", body = cJSON_CreateObject());

    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);
    cJSON_AddStringToObject(body, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(body, "tradeNo", data->tradeNo);

    cJSON_AddNumberToObject(body, "socVal", data->socVal);
    cJSON_AddNumberToObject(body, "BMSVer", data->BMSVer);

    cJSON_AddNumberToObject(body, "BMSMaxVol", data->BMSMaxVol);
    cJSON_AddNumberToObject(body, "batType", data->batType);
    cJSON_AddNumberToObject(body, "batRatedCap", data->batRatedCap);

    cJSON_AddNumberToObject(body, "batRatedTotalVol", data->batRatedTotalVol);
    cJSON_AddNumberToObject(body, "singlBatMaxAllowVol", data->singlBatMaxAllowVol);

    cJSON_AddNumberToObject(body, "maxAllowCur", data->maxAllowCur);
    cJSON_AddNumberToObject(body, "battotalEnergy", data->battotalEnergy);

    cJSON_AddNumberToObject(body, "maxVol", data->maxVol);
    cJSON_AddNumberToObject(body, "maxTemp", data->maxTemp);
    cJSON_AddNumberToObject(body, "batCurVol", data->batCurVol);

    cJSON_AddStringToObject(body, "batManu", data->batManufacturer);
    cJSON_AddStringToObject(body, "batSN", data->batSN);

    cJSON_AddStringToObject(body, "batMadeDay", data->batMadeDay);
    cJSON_AddNumberToObject(body, "chargeTimes", data->chargeTimes);
    cJSON_AddNumberToObject(body, "batProperty", data->batProperty);
    cJSON_AddStringToObject(body, "bmsSoftVer", data->bmsSoftVer);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 直流非充电过程实时监测属性
static int send_property_dc_nonwork(evs_property_dc_nonWork *data)
{
    int res = 0;
    char *payload = NULL;
    char Temp[20] = {0};
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "dcGunIdleItyFoData", body = cJSON_CreateObject());

    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(body, "workStatus", data->workStatus);

    cJSON_AddNumberToObject(body, "gunStatus", data->gunStatus);
    cJSON_AddNumberToObject(body, "eLockStatus", data->eLockStatus);
    cJSON_AddNumberToObject(body, "DCK1Status", data->DCK1Status);
    cJSON_AddNumberToObject(body, "DCK2Status", data->DCK2Status);

    cJSON_AddNumberToObject(body, "DCPlusFuseStatus", data->DCPlusFuseStatus);
    cJSON_AddNumberToObject(body, "DCMinusFuseStatus", data->DCMinusFuseStatus);

    cJSON_AddNumberToObject(body, "conTemp1", data->conTemp1);
    cJSON_AddNumberToObject(body, "conTemp2", data->conTemp2);

    cJSON_AddNumberToObject(body, "dcVol", data->dcVol);
    cJSON_AddNumberToObject(body, "dcCur", data->dcCur);

    cJSON_AddNumberToObject(body, "chargeCnt", data->chargeCnt);
    cJSON_AddNumberToObject(body, "chargeTime", data->chargeTime);

    cJSON_AddNumberToObject(body, "k1Cnt", data->k1Cnt);
    cJSON_AddNumberToObject(body, "k2Cnt", data->k2Cnt);
    cJSON_AddNumberToObject(body, "acContactorStatus", data->acContactorState);

    cJSON_AddNumberToObject(body, "guidanceVal", data->guidanceVal);
    cJSON_AddNumberToObject(body, "auxiliaryPowerStatus", data->auxiliaryPowerStatus);
    cJSON_AddNumberToObject(body, "eLockCtrlStatus", data->elockCtrlState);
    cJSON_AddNumberToObject(body, "fanSwitchCtrlStatus", data->fanSwitchCtrlStatus);

    sprintf(Temp, "%lld", data->sumMeter);
    cJSON_AddStringToObject(body, "sumMeter", Temp);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 直流充电过程实时监测属性
static int send_property_dc_work(evs_property_dc_work *data)
{
    int res = 0;
    int i = 0;
    char *payload = NULL;
    cJSON *root = NULL;
    cJSON *body = NULL;

    const char *ctrlState_fmt = "%d,%d,%d,%d,%d,%d";
    char temp[512];

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "dcGunRunItyTwData", body = cJSON_CreateObject());

    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(body, "workStatus", data->workStatus);

    cJSON_AddNumberToObject(body, "gunStatus", data->gunStatus);
    cJSON_AddNumberToObject(body, "eLockStatus", data->eLockStatus);

    cJSON_AddNumberToObject(body, "DCK1Status", data->DCK1Status);
    cJSON_AddNumberToObject(body, "DCK2Status", data->DCK2Status);

    cJSON_AddNumberToObject(body, "DCPlusFuseStatus", data->DCPlusFuseStatus);
    cJSON_AddNumberToObject(body, "DCMinusFuseStatus", data->DCMinusFuseStatus);

    cJSON_AddNumberToObject(body, "conTemp1", data->conTemp1);
    cJSON_AddNumberToObject(body, "conTemp2", data->conTemp2);

    cJSON_AddNumberToObject(body, "dcVol", data->dcVol);
    cJSON_AddNumberToObject(body, "dcCur", data->dcCur);
    cJSON_AddStringToObject(body, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(body, "tradeNo", data->tradeNo);

    cJSON_AddNumberToObject(body, "chgType", data->chgType);

    cJSON_AddNumberToObject(body, "realPower", data->realPower);
    cJSON_AddNumberToObject(body, "chgTime", data->chgTime);
    cJSON_AddNumberToObject(body, "remainT", data->remainT);

    cJSON_AddNumberToObject(body, "socVal", data->socVal);

    cJSON_AddNumberToObject(body, "needVol", data->needVol);
    cJSON_AddNumberToObject(body, "needCur", data->needCur);

    cJSON_AddNumberToObject(body, "chargeMode", data->chargeMode);

    cJSON_AddNumberToObject(body, "bmsVol", data->bmsVol);
    cJSON_AddNumberToObject(body, "bmsCur", data->bmsCur);

    cJSON_AddNumberToObject(body, "SingleMHV", data->SingleMHV);
    cJSON_AddNumberToObject(body, "SingleMLV", data->SingleMLV);

    cJSON_AddNumberToObject(body, "MHTemp", data->MHTemp);
    cJSON_AddNumberToObject(body, "MLTemp", data->MLTemp);

    cJSON_AddNumberToObject(body, "SingleMHVNo", data->SingleMHVNo);

    cJSON_AddNumberToObject(body, "MHTempNo", data->MHTempNo);
    cJSON_AddNumberToObject(body, "MLTempNo", data->MLTempNo);

    cJSON_AddNumberToObject(body, "acContactorStatus", data->acInputContactorState);

    cJSON_AddNumberToObject(body, "guidanceVal", data->guidanceVol);

    HAL_Snprintf(temp, sizeof(temp), ctrlState_fmt, data->k1CtrlState, data->k2CtrlState, data->acInputContactorCtrlState, data->apsSwtichCtrlState, data->elockCtrlState, data->carbinFanCtrlState);
    cJSON_AddStringToObject(body, "componentsCtrlStatus", temp);

    memset(temp, 0, 20);
    sprintf(temp, "%lld", data->meterStartVal);
    cJSON_AddStringToObject(body, "meterStartVal", temp);

    memset(temp, 0, 20);
    sprintf(temp, "%lld", data->meterRealVal);
    cJSON_AddStringToObject(body, "meterRealVal", temp);

    cJSON_AddNumberToObject(body, "totalElect", data->totalElect);
    cJSON_AddNumberToObject(body, "totalCost", data->totalCost);
    cJSON_AddNumberToObject(body, "totalPowerCost", data->totalPowerCost);
    cJSON_AddNumberToObject(body, "totalServCost", data->totalServCost);

    cJSON_AddNumberToObject(body, "timeNum", data->timeNum);

    int segCnt = data->timeNum;
    if (segCnt > 0 && segCnt < EVS_MAX_MODEL_DEVSEG + 1)
    {
        int str_len = 0;

        char *tempTarriff = (char *)HAL_Malloc(8 * segCnt);
        memset(tempTarriff, 0, 8 * segCnt);

        for (i = 0; i < segCnt; i++)
        {
            sprintf(tempTarriff + str_len, "%d,", data->partElect[i]);
            str_len = strlen(tempTarriff);
        }

        tempTarriff[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "partElect", tempTarriff);

        str_len = 0;
        memset(tempTarriff, 0, 8 * segCnt);
        for (i = 0; i < segCnt; i++)
        {
            sprintf(tempTarriff + str_len, "%d,", data->chargeFee[i]);
            str_len = strlen(tempTarriff);
        }

        tempTarriff[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "chargeFee", tempTarriff);

        str_len = 0;
        memset(tempTarriff, 0, 8 * segCnt);
        for (i = 0; i < segCnt; i++)
        {
            sprintf(tempTarriff + str_len, "%d,", data->serviceFee[i]);
            str_len = strlen(tempTarriff);
        }

        tempTarriff[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "serviceFee", tempTarriff);

        HAL_Free(tempTarriff);
    }

    cJSON_AddNumberToObject(body, "startPoint", data->startPoint);
    cJSON_AddNumberToObject(body, "crossPoints", data->crossPoints);

    int pointCnt = data->crossPoints;
    if (pointCnt > 0 && pointCnt < EVS_MAX_MODEL_DEVSEG + 1)
    {
        int str_len = 0;

        char *tempPoint = (char *)HAL_Malloc(8 * pointCnt);
        memset(tempPoint, 0, 8 * pointCnt);

        for (i = 0; i < pointCnt; i++)
        {
            sprintf(tempPoint + str_len, "%d,", data->pointsElect[i]);
            str_len = strlen(tempPoint);
        }

        tempPoint[str_len - 1] = '\0';
        cJSON_AddStringToObject(body, "pointsElect", tempPoint);

        HAL_Free(tempPoint);
    }

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 直流充电设备交流输入电表底值监测属性表
static int send_property_dc_input_meter(evs_property_dc_input_meter *data)
{
    int res = 0;
    char *payload = NULL;
    char Temp[20] = {0};
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "dcSysMeterItyData", body = cJSON_CreateObject());
    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);

    cJSON_AddStringToObject(body, "acqTime", data->acqTime);

    bcd2str(data->mailAddr, Temp, sizeof(data->mailAddr));
    cJSON_AddStringToObject(body, "mailAddr", Temp);

    bcd2str(data->meterNo, Temp, sizeof(data->meterNo));
    cJSON_AddStringToObject(body, "meterNo", Temp);
    cJSON_AddStringToObject(body, "assetId", data->assetId);

    sprintf(Temp, "%lld", data->sumMeter);
    cJSON_AddStringToObject(body, "sumMeter", Temp);
    sprintf(Temp, "%d", data->ApElect);
    cJSON_AddStringToObject(body, "ApElect", Temp);
    sprintf(Temp, "%d", data->BpElect);
    cJSON_AddStringToObject(body, "BpElect", Temp);
    sprintf(Temp, "%d", data->CpElect);
    cJSON_AddStringToObject(body, "CpElect", Temp);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 充电设备输出电表底值监测属性表
static int send_property_dc_outmeter(evs_property_meter *data)
{
    int res = 0;
    char *payload = NULL;
    char Temp[20] = {0};
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "dcOutMeterIty", body = cJSON_CreateObject());
    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);

    cJSON_AddStringToObject(body, "acqTime", data->acqTime);

    bcd2str(data->mailAddr, Temp, sizeof(data->mailAddr));
    cJSON_AddStringToObject(body, "mailAddr", Temp);

    bcd2str(data->meterNo, Temp, sizeof(data->meterNo));
    cJSON_AddStringToObject(body, "meterNo", Temp);

    cJSON_AddStringToObject(body, "assetId", data->assetId);

    sprintf(Temp, "%lld", data->sumMeter);
    cJSON_AddStringToObject(body, "sumMeter", Temp);

    cJSON_AddStringToObject(body, "lastTrade", data->lastTrade);
    cJSON_AddNumberToObject(body, "power", data->elec);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

// 充电设备输出电表底值监测属性表
static int send_property_ac_outmeter(evs_property_meter *data)
{
    int res = 0;
    char *payload = NULL;
    char Temp[20] = {0};
    cJSON *root = NULL;
    cJSON *body = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "acOutMeterItyData", body = cJSON_CreateObject());
    cJSON_AddNumberToObject(body, "gunNo", data->gunNo);

    cJSON_AddStringToObject(body, "acqTime", data->acqTime);

    bcd2str(data->mailAddr, Temp, sizeof(data->mailAddr));
    cJSON_AddStringToObject(body, "mailAddr", Temp);

    bcd2str(data->meterNo, Temp, sizeof(data->meterNo));
    cJSON_AddStringToObject(body, "meterNo", Temp);

    cJSON_AddStringToObject(body, "assetId", data->assetId);

    sprintf(Temp, "%lld", data->sumMeter);
    cJSON_AddStringToObject(body, "sumMeter", Temp);

    cJSON_AddStringToObject(body, "lastTrade", data->lastTrade);
    cJSON_AddNumberToObject(body, "power", data->elec);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_Report(evs_g_user_ctx.master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Property Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_firmware_info(evs_event_firmware_info *data)
{
    int res = 0;
    char *payload = NULL;
    char meterAddr[13] = {0};
    char meter[EVS_MAX_METER_ADDR_LEN] = {0};
    cJSON *root = NULL;
    cJSON *inMeterArray = NULL;
    cJSON *outMeterArray = NULL;

    unsigned char i = 0, j = 0, k = 0;

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "simNo", data->simNo);
    cJSON_AddStringToObject(root, "feeModelId", data->feeModelId);
    cJSON_AddStringToObject(root, "stakeModel", data->stakeModel);
    cJSON_AddStringToObject(root, "devSn", data->devSn);

    cJSON_AddStringToObject(root, "simMac", data->simMac);
    cJSON_AddStringToObject(root, "btMac", data->btMac);

    cJSON_AddNumberToObject(root, "vendorCode", data->vendorCode);
    cJSON_AddNumberToObject(root, "devType", data->devType);
    cJSON_AddNumberToObject(root, "portNum", data->portNum);

    cJSON_AddNumberToObject(root, "longitude", data->longitude);
    cJSON_AddNumberToObject(root, "latitude", data->latitude);
    cJSON_AddNumberToObject(root, "height", data->height);

    cJSON_AddNumberToObject(root, "gridType", data->gridType);
    cJSON_AddNumberToObject(root, "meaType", data->meaType);
    cJSON_AddNumberToObject(root, "otRate", data->otRate);

    cJSON_AddNumberToObject(root, "otMinVol", data->otMinVol);
    cJSON_AddNumberToObject(root, "otMaxVol", data->otMaxVol);
    cJSON_AddNumberToObject(root, "otCur", data->otCur);

    cJSON_AddNumberToObject(root, "CT", data->CT);
    cJSON_AddNumberToObject(root, "isGateLock", data->isGateLock);
    cJSON_AddNumberToObject(root, "isGroundLock", data->isGroundLock);
    cJSON_AddNumberToObject(root, "mutliChargingMode", data->mutliChargingMode);

    cJSON_AddItemToObject(root, "inMeter", inMeterArray = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_INPUT_METER_NUM; i++)
    {
        if (memcmp(data->inMeter[i], meter, EVS_MAX_METER_ADDR_LEN) != 0)
        {
            for (j = 0, k = 0; j < EVS_MAX_METER_ADDR_LEN; k++, j++)
            {
                meterAddr[k] = ((data->inMeter[i][j] >> 4) + '0');
                meterAddr[++k] = (data->inMeter[i][j] & 0x0f) + '0';
            }
            // cJSON_AddStringToArray(inMeterArray, meterAddr);
            cJSON_AddItemToArray(inMeterArray, cJSON_CreateString(meterAddr));
        }
    }

    cJSON_AddItemToObject(root, "outMeter", outMeterArray = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_PORT_NUM; i++)
    {
        if (memcmp(data->outMeter[i], meter, EVS_MAX_METER_ADDR_LEN) != 0)
        {
            for (j = 0, k = 0; j < EVS_MAX_METER_ADDR_LEN; k++, j++)
            {
                meterAddr[k] = ((data->outMeter[i][j] >> 4) + '0');
                meterAddr[++k] = (data->outMeter[i][j] & 0x0f) + '0';
            }
            // cJSON_AddStringToArray(outMeterArray, meterAddr);
            cJSON_AddItemToArray(outMeterArray, cJSON_CreateString(meterAddr));
        }
    }

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "firmwareTwEvt", strlen("firmwareTwEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_ver_info(evs_event_ver_info *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "devRegMethod", data->devRegMethod);
    cJSON_AddStringToObject(root, "pileSoftwareVer", data->pileSoftwareVer);
    cJSON_AddStringToObject(root, "pileHardwareVer", data->pileHardwareVer);
    cJSON_AddStringToObject(root, "sdkVer", "SDK_v1.1.10");

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "verInfoEvt", strlen("verInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_dev_maintain_result(evs_event_feedback_dev_maintain *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "ctrlType", data->ctrlType);
    cJSON_AddNumberToObject(root, "reason", data->reason);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "devMaintainRetEvt", strlen("devMaintainRetEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_lock_ctrl_result(evs_event_feedback_lockCtrl *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "lockStatus", data->lockStatus);
    cJSON_AddNumberToObject(root, "resCode", data->resCode);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "ctrlLockRetEvt", strlen("ctrlLockRetEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_logquery_result(evs_event_logQuery_Result *data)
{
    int res = 0;
    int i = 0;
    char *payload = NULL;
    char temp[20] = {0};

    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);

    sprintf(temp, "%d", data->startDate);
    cJSON_AddStringToObject(root, "startDate", temp);

    sprintf(temp, "%d", data->stopDate);
    cJSON_AddStringToObject(root, "stopDate", temp);

    cJSON_AddNumberToObject(root, "askType", data->askType);
    cJSON_AddNumberToObject(root, "result", data->result);
    cJSON_AddStringToObject(root, "logQueryNo", data->logQueryNo);
    cJSON_AddNumberToObject(root, "retType", data->retType);
    cJSON_AddNumberToObject(root, "logQueryEvtSum", data->logQueryEvtSum);
    cJSON_AddNumberToObject(root, "logQueryEvtNo", data->logQueryEvtNo);

    if (data->retType == 10)
    {
        if (data->askType == 10) // 交易记录
        {
            cJSON *partElecArray = NULL;
            cJSON *partChargeFeeArray = NULL;
            cJSON *partServiceFeeArray = NULL;
            cJSON *pointElecArray = NULL;

            cJSON *tradeRoot = cJSON_CreateObject();
            cJSON_AddNumberToObject(tradeRoot, "gunNo", data->dataArea.tradeInfo.gunNo);

            cJSON_AddStringToObject(tradeRoot, "preTradeNo", data->dataArea.tradeInfo.preTradeNo);
            cJSON_AddStringToObject(tradeRoot, "tradeNo", data->dataArea.tradeInfo.tradeNo);
            cJSON_AddStringToObject(tradeRoot, "vinCode", data->dataArea.tradeInfo.vinCode);

            cJSON_AddNumberToObject(tradeRoot, "timeDivType", data->dataArea.tradeInfo.timeDivType);

            sprintf(temp, "%d", data->dataArea.tradeInfo.chargeStartTime);
            cJSON_AddStringToObject(tradeRoot, "chargeStartTime", temp);
            sprintf(temp, "%d", data->dataArea.tradeInfo.chargeEndTime);
            cJSON_AddStringToObject(tradeRoot, "chargeEndTime", temp);

            cJSON_AddNumberToObject(tradeRoot, "startSoc", data->dataArea.tradeInfo.startSoc);
            cJSON_AddNumberToObject(tradeRoot, "endSoc", data->dataArea.tradeInfo.endSoc);
            cJSON_AddNumberToObject(tradeRoot, "reason", data->dataArea.tradeInfo.reason);

            cJSON_AddNumberToObject(tradeRoot, "totalElect", data->dataArea.tradeInfo.totalElect);
            cJSON_AddStringToObject(tradeRoot, "feeModelId", data->dataArea.tradeInfo.feeModelId);

            sprintf(temp, "%lld", data->dataArea.tradeInfo.sumStart);
            cJSON_AddStringToObject(tradeRoot, "sumStart", temp);
            sprintf(temp, "%lld", data->dataArea.tradeInfo.sumEnd);
            cJSON_AddStringToObject(tradeRoot, "sumEnd", temp);

            cJSON_AddNumberToObject(tradeRoot, "totalElect", data->dataArea.tradeInfo.totalElect);
            cJSON_AddNumberToObject(tradeRoot, "totalPowerCost", data->dataArea.tradeInfo.totalPowerCost);
            cJSON_AddNumberToObject(tradeRoot, "totalServCost", data->dataArea.tradeInfo.totalServCost);
            cJSON_AddNumberToObject(tradeRoot, "timeNum", data->dataArea.tradeInfo.timeNum);

            int segCnt = data->dataArea.tradeInfo.timeNum;
            if (segCnt > 0 && segCnt < EVS_MAX_MODEL_DEVSEG + 1)
            {
                cJSON_AddItemToObject(tradeRoot, "partElect", partElecArray = cJSON_CreateArray());
                cJSON_AddItemToObject(tradeRoot, "chargeFee", partChargeFeeArray = cJSON_CreateArray());
                cJSON_AddItemToObject(tradeRoot, "serviceFee", partServiceFeeArray = cJSON_CreateArray());

                for (i = 0; i < segCnt; i++)
                {
                    cJSON_AddItemToArray(partElecArray, cJSON_CreateNumber(data->dataArea.tradeInfo.partElect[i]));
                    cJSON_AddItemToArray(partChargeFeeArray, cJSON_CreateNumber(data->dataArea.tradeInfo.chargeFee[i]));
                    cJSON_AddItemToArray(partServiceFeeArray, cJSON_CreateNumber(data->dataArea.tradeInfo.serviceFee[i]));
                }
            }

            cJSON_AddNumberToObject(tradeRoot, "startPoint", data->dataArea.tradeInfo.startPoint);
            cJSON_AddNumberToObject(tradeRoot, "crossPoints", data->dataArea.tradeInfo.crossPoints);

            int pointCnt = data->dataArea.tradeInfo.crossPoints;
            if (pointCnt > 0 && pointCnt < EVS_MAX_MODEL_DEVSEG + 1)
            {
                cJSON_AddItemToObject(tradeRoot, "pointsElect", pointElecArray = cJSON_CreateArray());
                for (i = 0; i < pointCnt; i++)
                {
                    cJSON_AddItemToArray(pointElecArray, cJSON_CreateNumber(data->dataArea.tradeInfo.partElect[i]));
                }
            }

            char *tradePayload = cJSON_PrintUnformatted(tradeRoot);
            cJSON_AddStringToObject(root, "dataArea", tradePayload);

            if (tradeRoot != NULL)
            {
                cJSON_Delete(tradeRoot);
            }

            if (tradePayload != NULL)
            {
                HAL_Free(tradePayload);
            }
        }
        else if (data->askType == 11) // 电表底值
        {
            cJSON *meterRoot, *body;

            meterRoot = cJSON_CreateObject();
            cJSON_AddItemToObject(meterRoot, "outMeterItyData", body = cJSON_CreateObject());
            cJSON_AddNumberToObject(body, "gunNo", data->dataArea.meterData.gunNo);

            cJSON_AddStringToObject(body, "acqTime", data->dataArea.meterData.acqTime);

            bcd2str(data->dataArea.meterData.mailAddr, temp, 6);
            cJSON_AddStringToObject(body, "mailAddr", temp);

            bcd2str(data->dataArea.meterData.meterNo, temp, 6);
            cJSON_AddStringToObject(body, "meterNo", temp);

            cJSON_AddStringToObject(body, "assetId", data->dataArea.meterData.assetId);

            sprintf(temp, "%lld", data->dataArea.meterData.sumMeter);
            cJSON_AddStringToObject(body, "sumMeter", temp);

            cJSON_AddStringToObject(body, "lastTrade", data->dataArea.meterData.lastTrade);
            cJSON_AddNumberToObject(body, "power", data->dataArea.meterData.elec);

            char *meterPayload = cJSON_PrintUnformatted(meterRoot);

            cJSON_AddStringToObject(root, "dataArea", meterPayload);

            if (meterRoot != NULL)
            {
                cJSON_Delete(meterRoot);
            }

            if (meterPayload != NULL)
            {
                HAL_Free(meterPayload);
            }
        }
        else if (data->askType == 14) // BMS属性
        {
            cJSON *BMSRoot, *body;

            BMSRoot = cJSON_CreateObject();
            cJSON_AddItemToObject(BMSRoot, "dcBmsRunIty", body = cJSON_CreateObject());
            cJSON_AddNumberToObject(body, "gunNo", data->dataArea.BMSData.gunNo);
            cJSON_AddStringToObject(body, "preTradeNo", data->dataArea.BMSData.preTradeNo);
            cJSON_AddStringToObject(body, "tradeNo", data->dataArea.BMSData.tradeNo);

            cJSON_AddNumberToObject(body, "socVal", data->dataArea.BMSData.socVal);
            cJSON_AddNumberToObject(body, "BMSVer", data->dataArea.BMSData.BMSVer);

            cJSON_AddNumberToObject(body, "BMSMaxVol", data->dataArea.BMSData.BMSMaxVol);
            cJSON_AddNumberToObject(body, "batType", data->dataArea.BMSData.batType);
            cJSON_AddNumberToObject(body, "batRatedCap", data->dataArea.BMSData.batRatedCap);

            cJSON_AddNumberToObject(body, "batRatedTotalVol", data->dataArea.BMSData.batRatedTotalVol);
            cJSON_AddNumberToObject(body, "singlBatMaxAllowVol", data->dataArea.BMSData.singlBatMaxAllowVol);

            cJSON_AddNumberToObject(body, "maxAllowCur", data->dataArea.BMSData.maxAllowCur);
            cJSON_AddNumberToObject(body, "battotalEnergy", data->dataArea.BMSData.battotalEnergy);

            cJSON_AddNumberToObject(body, "maxVol", data->dataArea.BMSData.maxVol);
            cJSON_AddNumberToObject(body, "maxTemp", data->dataArea.BMSData.maxTemp);
            cJSON_AddNumberToObject(body, "batCurVol", data->dataArea.BMSData.batCurVol);

            cJSON_AddStringToObject(body, "batManufacturer", data->dataArea.BMSData.batManufacturer);
            cJSON_AddStringToObject(body, "batSN", data->dataArea.BMSData.batSN);
            cJSON_AddStringToObject(body, "batMadeDay", data->dataArea.BMSData.batMadeDay);
            cJSON_AddNumberToObject(body, "chargeTimes", data->dataArea.BMSData.chargeTimes);
            cJSON_AddNumberToObject(body, "batProperty", data->dataArea.BMSData.batProperty);
            cJSON_AddStringToObject(body, "bmsSoftVer", data->dataArea.BMSData.bmsSoftVer);

            char *BMSPayload = cJSON_PrintUnformatted(BMSRoot);
            cJSON_AddStringToObject(root, "dataArea", BMSPayload);

            if (BMSRoot != NULL)
            {
                cJSON_Delete(BMSRoot);
            }

            if (BMSPayload != NULL)
            {
                HAL_Free(BMSPayload);
            }
        }
        else
        {
            cJSON_AddStringToObject(root, "dataArea", data->dataArea.rawData);
        }
    }
    else
    {
        cJSON_AddStringToObject(root, "dataArea", "");
    }

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "logQueryEvt", strlen("logQueryEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_ask_feeModel(evs_event_ask_feeModel *data)
{
    int res = 0;
    char *payload = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "feeModelId", data->feeModelId);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "askFeeModelTwEvt", strlen("askFeeModelTwEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_start_result(evs_event_startResult *data)
{
    int res = 0;
    char *payload = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "startResult", data->startResult);
    cJSON_AddNumberToObject(root, "faultCode", data->faultCode);
    cJSON_AddStringToObject(root, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(root, "tradeNo", data->tradeNo);
    cJSON_AddStringToObject(root, "vinCode", data->vinCode);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "startChaResEvt", strlen("startChaResEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_auth_start(evs_event_startCharge *data)
{
    int res = 0;
    char *payload = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "startType", data->startType);

    cJSON_AddStringToObject(root, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(root, "tradeNo", data->tradeNo);
    cJSON_AddStringToObject(root, "authCode", data->authCode);

    cJSON_AddNumberToObject(root, "batterySOC", data->batterySOC);
    cJSON_AddNumberToObject(root, "batteryCap", data->batteryCap);

    cJSON_AddNumberToObject(root, "chargeTimes", data->chargeTimes);

    cJSON_AddNumberToObject(root, "batteryVol", data->batteryVol);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "startChargeAuthEvt", strlen("startChargeAuthEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_stopCharge(evs_event_stopCharge *data)
{
    int res = 0;
    char *payload = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "stopResult", data->stopResult);
    cJSON_AddNumberToObject(root, "resultCode", data->resultCode);
    cJSON_AddStringToObject(root, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(root, "tradeNo", data->tradeNo);
    cJSON_AddNumberToObject(root, "stopFailReson", data->stopFailReson);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "stopChaResEvt", strlen("stopChaResEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_groundLock_change(evs_event_groundLock_change *data)
{
    int res = 0;
    char *payload = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "lockState", data->lockState);
    cJSON_AddNumberToObject(root, "powerType", data->powerType);
    cJSON_AddNumberToObject(root, "cellState", data->cellState);
    cJSON_AddNumberToObject(root, "lockerState", data->lockerState);
    cJSON_AddNumberToObject(root, "lockerForced", data->lockerForced);
    cJSON_AddNumberToObject(root, "lowPower", data->lowPower);
    cJSON_AddNumberToObject(root, "soc", data->soc);
    cJSON_AddNumberToObject(root, "openCnt", data->openCnt);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "groundLockEvt", strlen("groundLockEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_gateLock_change(evs_event_gateLock_change *data)
{
    int res = 0;
    char *payload = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "lockNo", data->lockNo);
    cJSON_AddNumberToObject(root, "lockState", data->lockState);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "smartLockEvent", strlen("smartLockEvent"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_tradeInfo(evs_event_tradeInfo *data)
{
    int res = 0;
    int i = 0;
    char *payload = NULL;
    char time[16] = {0};
    char sumPower[20] = {0};
    cJSON *root = NULL;
    cJSON *partElecArray = NULL;
    cJSON *partChargeFeeArray = NULL;
    cJSON *partServiceFeeArray = NULL;
    cJSON *pointElecArray = NULL;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);

    cJSON_AddStringToObject(root, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(root, "tradeNo", data->tradeNo);
    cJSON_AddStringToObject(root, "vinCode", data->vinCode);

    cJSON_AddNumberToObject(root, "timeDivType", data->timeDivType);
    cJSON_AddNumberToObject(root, "startType", data->startType);

    sprintf(time, "%d", data->chargeStartTime);
    cJSON_AddStringToObject(root, "chargeStartTime", time);
    sprintf(time, "%d", data->chargeEndTime);
    cJSON_AddStringToObject(root, "chargeEndTime", time);

    cJSON_AddNumberToObject(root, "startSoc", data->startSoc);
    cJSON_AddNumberToObject(root, "endSoc", data->endSoc);
    cJSON_AddNumberToObject(root, "reason", data->reason);

    cJSON_AddStringToObject(root, "feeModelId", data->feeModelId);

    sprintf(sumPower, "%lld", data->sumStart);
    cJSON_AddStringToObject(root, "sumStart", sumPower);
    sprintf(sumPower, "%lld", data->sumEnd);
    cJSON_AddStringToObject(root, "sumEnd", sumPower);

    cJSON_AddNumberToObject(root, "totalElect", data->totalElect);
    cJSON_AddNumberToObject(root, "totalPowerCost", data->totalPowerCost);
    cJSON_AddNumberToObject(root, "totalServCost", data->totalServCost);
    cJSON_AddNumberToObject(root, "totalCost", data->totalCost);

    cJSON_AddNumberToObject(root, "timeNum", data->timeNum);

    int segCnt = data->timeNum;
    if (segCnt > 0 && segCnt < EVS_MAX_MODEL_DEVSEG + 1)
    {
        cJSON_AddItemToObject(root, "partElect", partElecArray = cJSON_CreateArray());
        cJSON_AddItemToObject(root, "chargeFee", partChargeFeeArray = cJSON_CreateArray());
        cJSON_AddItemToObject(root, "serviceFee", partServiceFeeArray = cJSON_CreateArray());

        for (i = 0; i < segCnt; i++)
        {
            cJSON_AddItemToArray(partElecArray, cJSON_CreateNumber(data->partElect[i]));
            cJSON_AddItemToArray(partChargeFeeArray, cJSON_CreateNumber(data->chargeFee[i]));
            cJSON_AddItemToArray(partServiceFeeArray, cJSON_CreateNumber(data->serviceFee[i]));
        }
    }

    cJSON_AddNumberToObject(root, "startPoint", data->startPoint);
    cJSON_AddNumberToObject(root, "crossPoints", data->crossPoints);

    int pointCnt = data->crossPoints;
    if (pointCnt > 0 && pointCnt < EVS_MAX_MODEL_DEVSEG + 1)
    {
        cJSON_AddItemToObject(root, "pointsElect", pointElecArray = cJSON_CreateArray());
        for (i = 0; i < pointCnt; i++)
        {
            cJSON_AddItemToArray(pointElecArray, cJSON_CreateNumber(data->pointsElect[i]));
        }
    }

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "orderTwUpdateEvt", strlen("orderTwUpdateEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_alarm(evs_event_alarm *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;
    cJSON *faultArray = NULL;
    cJSON *warnArray = NULL;
    unsigned char i = 0;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "faultSum", data->faultSum);
    cJSON_AddNumberToObject(root, "warnSum", data->warnSum);

    cJSON_AddItemToObject(root, "faultValue", faultArray = cJSON_CreateArray());
    for (i = 0; i < data->faultSum; i++)
    {
        // cJSON_AddNumberToArray(faultArray, data->faultValue[i]);
        cJSON_AddItemToArray(faultArray, cJSON_CreateNumber(data->faultValue[i]));
    }
    cJSON_AddItemToObject(root, "warnValue", warnArray = cJSON_CreateArray());
    for (i = 0; i < data->warnSum; i++)
    {
        // cJSON_AddNumberToArray(warnArray, data->warnValue[i]);
        cJSON_AddItemToArray(warnArray, cJSON_CreateNumber(data->warnValue[i]));
    }
    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "totalFaultEvt", strlen("totalFaultEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_ask_dev_config(void)
{
    int res = 0;
    char payload[6] = "{}";

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "askConfigEvt", strlen("askConfigEvt"), (char *)payload, strlen(payload));

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_acPile_change(evs_event_pile_stutus_change *data)
{
    int res = 0;
    char *payload = NULL;
    char time[16] = {0};
    cJSON *root = NULL;

    sprintf(time, "%d", data->yxOccurTime);

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddStringToObject(root, "yxOccurTime", time);
    cJSON_AddNumberToObject(root, "connCheckStatus", data->connCheckStatus);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "acStChEvt", strlen("acStChEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_raw(char *data)
{
    int res = 0;
    char *eventName = NULL;
    char *payload = NULL;

    if (data == NULL)
        return -1;

    cJSON *root = cJSON_Parse(data);
    if (root == NULL || !cJSON_IsObject(root))
    {
        PROTOCOL_TRACE("JSON Parse Error");
        return -1;
    }

    cJSON *item_partsType = cJSON_GetObjectItem(root, "partsType");
    if (item_partsType != NULL && cJSON_IsNumber(item_partsType))
    {

        // 0x01：直流充电控制器（充电控制模块）
        // 0x02：交流充电控制器
        // 0x03：功率控制模块
        // 0x04：充电模块
        // 0x05：开关模块
        // 0x06：环境信息采集模块
        // 0x07：充电模块运行信息统计数据

        int partsType = item_partsType->valueint;
        switch (partsType)
        {
        case 1:
            eventName = "ccuInfoEvt";
            break;
        case 2:
            /* code */
            break;
        case 3:
            eventName = "pcuInfoEvt";
            break;
        case 4:
            eventName = "cuInfoEvt";
            break;
        case 5:
            eventName = "switchInfoEvt";
            break;
        case 6:
            eventName = "evInfoEvt";
            break;
        case 7:
            eventName = "cuWorkInfoEvt";
            break;
        default:
            break;
        }
    }

    cJSON *item_partsInfo = cJSON_GetObjectItem(root, "partsInfo");
    if (item_partsInfo != NULL && cJSON_IsObject(item_partsInfo))
    {
        payload = cJSON_PrintUnformatted(item_partsInfo);
    }

    if (eventName != NULL)
        res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, eventName, strlen(eventName), (char *)payload, strlen(payload));

    PROTOCOL_TRACE("Post Event Message ID: %d", res);

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_dcPile_change(evs_event_pile_stutus_change *data)
{
    int res = 0;
    char *payload = NULL;
    char time[16] = {0};
    cJSON *root = NULL;

    sprintf(time, "%d", data->yxOccurTime);

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddStringToObject(root, "yxOccurTime", time);
    cJSON_AddNumberToObject(root, "connCheckStatus", data->connCheckStatus);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "dcStChEvt", strlen("dcStChEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_car_info(evs_event_car_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "batterySOC", data->batterySOC);
    cJSON_AddNumberToObject(root, "batteryCap", data->batteryCap);
    cJSON_AddStringToObject(root, "vinCode", data->vinCode);
    cJSON_AddNumberToObject(root, "state", data->state);

    payload = cJSON_PrintUnformatted(root);

    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "acCarInfoEvt", strlen("acCarInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_card_info(evs_event_card_info *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    bcd2str(gun_auth_process_data[data->gunNo - 1].psy_id, data->cardPsySn, sizeof(gun_auth_process_data[data->gunNo - 1].psy_id));
    bcd2str(gun_auth_process_data[data->gunNo - 1].usrId, data->cardSn, sizeof(gun_auth_process_data[data->gunNo - 1].usrId));

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddStringToObject(root, "cardSn", data->cardSn);
    cJSON_AddStringToObject(root, "cardPsySn", data->cardPsySn);
    cJSON_AddNumberToObject(root, "startMode", data->startMode);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "tagCardInfoEvt", strlen("tagCardInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }

    return res;
}

static int send_event_card_auth(evs_event_card_auth *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    bcd2str(gun_auth_process_data[data->gunNo - 1].randomC, data->randomC, sizeof(gun_auth_process_data[data->gunNo - 1].randomC));
    bcd2str(gun_auth_process_data[data->gunNo - 1].cardBalance, data->cardBalance, sizeof(gun_auth_process_data[data->gunNo - 1].cardBalance));
    bcd2str(gun_auth_process_data[data->gunNo - 1].tradeSn, data->cardTradeSn, sizeof(gun_auth_process_data[data->gunNo - 1].tradeSn));
    bcd2str(&gun_auth_process_data[data->gunNo - 1].authVer, data->keyVer, sizeof(gun_auth_process_data[data->gunNo - 1].authVer));
    bcd2str(&gun_auth_process_data[data->gunNo - 1].authFlag, data->keyFlag, sizeof(gun_auth_process_data[data->gunNo - 1].authFlag));
    bcd2str(gun_auth_process_data[data->gunNo - 1].MAC1, data->tradeMac1, sizeof(gun_auth_process_data[data->gunNo - 1].MAC1));

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddStringToObject(root, "randomC", data->randomC);
    cJSON_AddStringToObject(root, "cardBalance", data->cardBalance);
    cJSON_AddStringToObject(root, "cardTradeSn", data->cardTradeSn);
    cJSON_AddStringToObject(root, "keyVer", data->keyVer);
    cJSON_AddStringToObject(root, "keyFlag", data->keyFlag);
    cJSON_AddStringToObject(root, "tradeMac1", data->tradeMac1);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "devAuthCodeEvt", strlen("devAuthCodeEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_card_auth_result(evs_event_card_auth_result *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    bcd2str(gun_auth_process_data[data->gunNo - 1].TAC, data->tradeTac, sizeof(gun_auth_process_data[data->gunNo - 1].TAC));
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddStringToObject(root, "preTradeNo", data->preTradeNo);
    cJSON_AddStringToObject(root, "tradeNo", data->tradeNo);
    cJSON_AddStringToObject(root, "tradeTac", data->tradeTac);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "srvAuthConfirmEvt", strlen("srvAuthConfirmEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_card_check_error(evs_event_card_check_error *data)
{
    int res = 0;
    char *payload = NULL;
    cJSON *root = NULL;

    root = cJSON_CreateObject();

    bcd2str(gun_auth_process_data[data->gunNo - 1].psy_id, data->cardPsySn, sizeof(gun_auth_process_data[data->gunNo - 1].psy_id));
    bcd2str(gun_auth_process_data[data->gunNo - 1].usrId, data->cardSn, sizeof(gun_auth_process_data[data->gunNo - 1].usrId));

    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddStringToObject(root, "cardSn", data->cardSn);
    cJSON_AddStringToObject(root, "cardPsySn", data->cardPsySn);
    cJSON_AddNumberToObject(root, "faultValue", data->faultValue);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "cardCheckErrorEvt", strlen("cardCheckErrorEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_devmdu_info(evs_event_devmdu_info *data)
{
    int res = 0, i = 0;
    cJSON *root = NULL;
    char *payload = NULL;
    cJSON *mduInfoIntArray = NULL;
    cJSON *mduInfoStringArray = NULL;

    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "netMduInfo", data->netMduInfo);
    cJSON_AddStringToObject(root, "netMduSoftVer", data->netMduSoftVer);
    cJSON_AddStringToObject(root, "netMduImei", data->netMduImei);
    cJSON_AddNumberToObject(root, "smartGun", data->smartGun);

    cJSON_AddItemToObject(root, "mduInfoInt", mduInfoIntArray = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_MDUINT_NUM; i++)
    {
        cJSON_AddItemToArray(mduInfoIntArray, cJSON_CreateNumber(data->mduInfoInt[i]));
    }
    cJSON_AddItemToObject(root, "mduInfoString", mduInfoStringArray = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_MDUSTRING_NUM; i++)
    {
        cJSON_AddItemToArray(mduInfoStringArray, cJSON_CreateString(data->mduInfoString[i]));
    }

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "devMduInfoEvt", strlen("devMduInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_charge_connect(evs_event_charge_connect_change *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;
    char time[16] = {0};

    root = cJSON_CreateObject();

    sprintf(time, "%d", data->yxOccurTime);

    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "cpStatus", data->cpStatus);
    cJSON_AddNumberToObject(root, "cpVolt", data->cpVolt);
    cJSON_AddNumberToObject(root, "s3Status", data->s3Status);
    cJSON_AddStringToObject(root, "yxOccurTime", time);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "acCarConChEvt", strlen("acCarConChEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_ble_plug_chg_info(evs_event_ble_plug_charge_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    cJSON *macList = NULL;
    char *payload = NULL;
    int i = 0;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "plugChgStatus", data->plugChgStatus);
    cJSON_AddNumberToObject(root, "macNum", data->macNum);
    cJSON_AddItemToObject(root, "macList", macList = cJSON_CreateArray());
    for (i = 0; i < data->macNum; i++)
    {
        cJSON_AddItemToArray(macList, cJSON_CreateString(data->macList[i]));
    }

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "blePlugChgInfoEvt", strlen("blePlugChgInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_ble_conn_change(evs_event_ble_conn_change *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;
    char time[16] = {0};

    root = cJSON_CreateObject();

    sprintf(time, "%d", data->yxOccurTime);

    cJSON_AddStringToObject(root, "yxOccurTime", time);
    cJSON_AddNumberToObject(root, "connCheckStatus", data->connCheckStatus);
    cJSON_AddStringToObject(root, "macInfo", data->macInfo);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "bleConnChEvt", strlen("bleConnChEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_pile_parts_ccu_info(evs_event_ccu_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "partsAddr", data->partsAddr);
    cJSON_AddNumberToObject(root, "residualLife", data->residualLife);
    cJSON_AddNumberToObject(root, "workTime", data->workTime);
    cJSON_AddNumberToObject(root, "alarmTimes", data->alarmTimes);
    cJSON_AddNumberToObject(root, "faultTimes", data->faultTimes);
    cJSON_AddNumberToObject(root, "k1Cnt", data->k1Cnt);
    cJSON_AddNumberToObject(root, "k2Cnt", data->k2Cnt);
    cJSON_AddNumberToObject(root, "reliefCircuitCnt", data->reliefCircuitCnt);
    cJSON_AddNumberToObject(root, "gunPlugCnt", data->gunPlugCnt);
    cJSON_AddNumberToObject(root, "gunLockCnt", data->gunLockCnt);
    cJSON_AddNumberToObject(root, "totalChargeCnt", data->totalChargeCnt);
    cJSON_AddNumberToObject(root, "totalChargeTime", data->totalChargeTime);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "ccuInfoEvt", strlen("ccuInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_pile_parts_pcu_info(evs_event_pcu_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "partsAddr", data->partsAddr);
    cJSON_AddNumberToObject(root, "residualLife", data->residualLife);
    cJSON_AddNumberToObject(root, "workTime", data->workTime);
    cJSON_AddNumberToObject(root, "alarmTimes", data->alarmTimes);
    cJSON_AddNumberToObject(root, "faultTimes", data->faultTimes);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "pcuInfoEvt", strlen("pcuInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_pile_parts_cu_info(evs_event_cu_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "partsAddr", data->partsAddr);
    cJSON_AddStringToObject(root, "devSN", data->devSN);
    cJSON_AddStringToObject(root, "devType", data->devType);
    cJSON_AddNumberToObject(root, "residualLife", data->residualLife);
    cJSON_AddNumberToObject(root, "workTime", data->workTime);
    cJSON_AddNumberToObject(root, "alarmTimes", data->alarmTimes);
    cJSON_AddNumberToObject(root, "faultTimes", data->faultTimes);
    cJSON_AddNumberToObject(root, "upTimes", data->upTimes);
    cJSON_AddNumberToObject(root, "pfcTemper", data->pfcTemper);
    cJSON_AddNumberToObject(root, "transformerTemper", data->transformerTemper);
    cJSON_AddNumberToObject(root, "frontFanTemper", data->frontFanTemper);
    cJSON_AddNumberToObject(root, "backFanTemper", data->backFanTemper);
    cJSON_AddNumberToObject(root, "inletTemer", data->inletTemer);
    cJSON_AddNumberToObject(root, "phaseVa", data->phaseVa);
    cJSON_AddNumberToObject(root, "phaseVb", data->phaseVb);
    cJSON_AddNumberToObject(root, "phaseVc", data->phaseVc);
    cJSON_AddNumberToObject(root, "status", data->status);

    cJSON_AddNumberToObject(root, "outputVol", data->outputVol);
    cJSON_AddNumberToObject(root, "outputCur", data->outputCur);
    cJSON_AddNumberToObject(root, "currentGID", data->currentGID);
    cJSON_AddNumberToObject(root, "alarm1", data->alarm1);
    cJSON_AddNumberToObject(root, "alarm2", data->alarm2);
    cJSON_AddNumberToObject(root, "alarm3", data->alarm3);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "cuInfoEvt", strlen("cuInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_pile_parts_switch_info(evs_event_switch_info *data)
{
    int res = 0;
    int i = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON *switchAnodeState = NULL;
    cJSON *switchAnodeInfo = NULL;
    cJSON *switchCathodeInfo = NULL;
    cJSON_AddNumberToObject(root, "partsAddr", data->partsAddr);
    cJSON_AddNumberToObject(root, "residualLife", data->residualLife);
    cJSON_AddNumberToObject(root, "workTime", data->workTime);
    cJSON_AddNumberToObject(root, "alarmTimes", data->alarmTimes);
    cJSON_AddNumberToObject(root, "faultTimes", data->faultTimes);
    cJSON_AddNumberToObject(root, "switchType", data->switchType);
    cJSON_AddNumberToObject(root, "commState", data->commState);

    cJSON_AddItemToObject(root, "switchAnodeState", switchAnodeState = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_PARTS_SWITCH_NUM; i++)
        cJSON_AddItemToArray(switchAnodeState, cJSON_CreateNumber(data->switchAnodeState[i]));

    cJSON_AddItemToObject(root, "switchAnodeActCnt", switchAnodeInfo = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_PARTS_SWITCH_NUM; i++)
        cJSON_AddItemToArray(switchAnodeInfo, cJSON_CreateNumber(data->switchAnodeActCnt[i]));

    cJSON_AddItemToObject(root, "switchCathodeActCnt", switchCathodeInfo = cJSON_CreateArray());
    for (i = 0; i < EVS_MAX_PARTS_SWITCH_NUM; i++)
        cJSON_AddItemToArray(switchCathodeInfo, cJSON_CreateNumber(data->switchCathodeActCnt[i]));

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "switchInfoEvt", strlen("switchInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_pile_parts_edas_info(evs_event_edas_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "partsAddr", data->partsAddr);
    cJSON_AddNumberToObject(root, "residualLife", data->residualLife);
    cJSON_AddNumberToObject(root, "workTime", data->workTime);
    cJSON_AddNumberToObject(root, "alarmTimes", data->alarmTimes);
    cJSON_AddNumberToObject(root, "faultTimes", data->faultTimes);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "evInfoEvt", strlen("evInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_pile_parts_cu_work_info(evs_event_cu_work_info *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "gunNo", data->gunNo);
    cJSON_AddNumberToObject(root, "sumModul", data->sumModul);
    cJSON_AddNumberToObject(root, "ratedPower", data->ratedPower);
    cJSON_AddNumberToObject(root, "workModulCnt", data->workModulCnt);
    cJSON_AddNumberToObject(root, "faultModulCnt", data->faultModulCnt);
    cJSON_AddNumberToObject(root, "portWorkModulCnt", data->portWorkModulCnt);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "cuWorkInfoEvt", strlen("cuWorkInfoEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_vin_list_result(evs_event_vinList_result *data)
{
    int res = 0, i = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "updateSN", data->updateSN);
    cJSON_AddNumberToObject(root, "packCnt", data->packCnt);
    cJSON_AddNumberToObject(root, "packTotalCnt", data->packTotalCnt);
    cJSON_AddNumberToObject(root, "totalVINCnt", data->totalVINCnt);
    cJSON_AddNumberToObject(root, "packVINCnt", data->packVINCnt);

    if (data->packVINCnt > 0)
    {
        cJSON *vinListArray = NULL;
        cJSON_AddItemToObject(root, "vinList", vinListArray = cJSON_CreateArray());
        for (i = 0; i < data->packVINCnt; i++)
        {
            cJSON_AddItemToArray(vinListArray, cJSON_CreateString(data->vinList[i]));
        }
    }

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "vinListCheckEvt", strlen("vinListCheckEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

static int send_event_time_sync_result(evs_event_time_sync_result *data)
{
    int res = 0;
    cJSON *root = NULL;
    char *payload = NULL;

    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "srvTime", data->srvTime);
    cJSON_AddStringToObject(root, "devTime", data->devTime);
    cJSON_AddNumberToObject(root, "resCode", data->resCode);

    payload = cJSON_PrintUnformatted(root);
    res = IOT_Linkkit_TriggerEvent(evs_g_user_ctx.master_devid, "timeSyncRetEvt", strlen("timeSyncRetEvt"), (char *)payload, strlen(payload));

    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (payload != NULL)
    {
        HAL_Free(payload);
    }

    PROTOCOL_TRACE("Post Event Message ID: %d", res);
    if (res < 0)
    {
        return -1;
    }
    return res;
}

int evs_linkkit_time_sync(void)
{
    int ret = 0;

    ret = IOT_Linkkit_Query(evs_g_user_ctx.master_devid, ITM_MSG_QUERY_TIMESTAMP, NULL, 0);
    if (ret)
    {
        PROTOCOL_TRACE("Linkkit Qurey Time Stamp Failed %d ", ret);
        return 1;
    }

    return 0;
}

int evs_linkkit_fota(unsigned char *buffer, int buffer_length)
{
    int ret = 0;

    ret = IOT_Linkkit_Query(evs_g_user_ctx.master_devid, ITM_MSG_QUERY_FOTA_DATA, buffer, buffer_length);
    if (ret)
    {
        PROTOCOL_TRACE("Linkkit Qurey Image Failed %d ", ret);
        return 1;
    }

    return 0;
}

#ifdef HTTP2_COMM_ENABLED
int evs_linkkit_file_upload(char *filePath)
{
    http2_upload_conn_info_t conn_info;
    http2_status_cb_t status_cb;
    http2_upload_result_cb_t result_cb;
    void *handle;
    unsigned int retry_time = 0;
    int ret;
    char url[250] = {0};

    memset(&conn_info, 0, sizeof(http2_upload_conn_info_t));

    sprintf(url, "%s.iot-as-http2.cn-shanghai.aliyuncs.com", master_meta_info.product_key); // mark， need to confirm is master_meta_info.product_key or master_meta_info.devicename
    conn_info.product_key = master_meta_info.product_key;
    conn_info.device_name = master_meta_info.device_name;
    conn_info.device_secret = master_meta_info.device_secret;
    conn_info.url = url;
    conn_info.port = HTTP2_ONLINE_SERVER_PORT;

    memset(&status_cb, 0, sizeof(http2_status_cb_t));
    status_cb.on_reconnect_cb = _on_http2_reconnect;
    status_cb.on_disconnect_cb = _on_http2_disconnect;

    memset(&result_cb, 0, sizeof(http2_upload_result_cb_t));
    result_cb.upload_completed_cb = upload_file_result;
    result_cb.upload_id_received_cb = upload_id_received_handle;

    handle = IOT_HTTP2_UploadFile_Connect(&conn_info, &status_cb);
    if (handle == NULL)
    {
        return -1;
    }
    is_connected = 1;

    http2_upload_params_t fs_params;
    memset(&fs_params, 0, sizeof(fs_params));
    fs_params.file_path = filePath;
    fs_params.opt_bit_map = UPLOAD_FILE_OPT_BIT_OVERWRITE;

    ret = IOT_HTTP2_UploadFile_Request(handle, &fs_params, &result_cb, NULL);
    if (ret < 0)
    {
        return -1;
    }

    /* wait until upload end */
    while (upload_result == 1)
    {
        HAL_SleepMs(200);
    }

    /* check the result */
    if (upload_result == UPLOAD_SUCCESS)
    {
        PROTOCOL_TRACE("upload succeed");
        ret = IOT_HTTP2_UploadFile_Disconnect(handle);
        PROTOCOL_TRACE("close connect %d\n", ret);
        return 0;
    }
    else
    {
        /* check if upload_id receivced */
        if (g_upload_id[0] == '\0')
        {
            PROTOCOL_TRACE("upload id is NULL, resume is impossible!");
            return -1;
        }
    }

    /* into resume routine */
    do
    {
        /* TODO: assume that http2 disconnected */
        if (upload_result < 0 && is_connected == 1)
        {
            HAL_SleepMs(30000);
        }

        /* wait until connected */
        while (is_connected == 0)
        {
            HAL_SleepMs(200);
        }

        /* reset upload result */
        upload_result = 1;

        /* use resume option to upload file */
        fs_params.file_path = filePath;
        fs_params.upload_len = 0;
        fs_params.upload_id = g_upload_id;
        fs_params.opt_bit_map = UPLOAD_FILE_OPT_BIT_RESUME; /* resume option used */
        ret = IOT_HTTP2_UploadFile_Request(handle, &fs_params, &result_cb, NULL);
        if (ret < 0)
        {
            PROTOCOL_TRACE("upload file request error");
            return -1;
        }

        while (upload_result == 1)
        {
            HAL_SleepMs(200);
        }

    } while (upload_result != UPLOAD_SUCCESS && (++retry_time < UPLOAD_RETRY_TIME));

    PROTOCOL_TRACE("upload succeed %d\n", ret);

    ret = IOT_HTTP2_UploadFile_Disconnect(handle);
    PROTOCOL_TRACE("close connect %d\n", ret);
    return 0;
}
#endif

extern void IOT_SetRootCA(int type);

int evs_linkkit_new(const int evs_access, const int is_device_uid)
{

#ifdef DYNAMIC_REGISTER
    int DeviceSecretLength = 0;
    int regCodeLenth = 0;
    iotx_http_region_types_t region = IOTX_HTTP_REGION_CUSTOM;
    int res = 0;
    iotx_dev_meta_info_t dev_reg_meta;
    char reg_code[IOTX_DEVICE_REG_CODE_LEN + 1] = "";
    char device_uid[IOTX_DEVICE_UID_LEN + 1] = "";

    IOT_Ioctl(IOTX_IOCTL_SET_REGISTER_REGION, (void *)&evs_access);
#endif

    int custom_port = 0;

    if (evs_access == ACCESS_IS_FORMAL)
    {
        custom_port = 18883;
        IOT_SetLogLevel(IOT_LOG_ERROR);
        IOT_SetRootCA(ACCESS_IS_FORMAL);
    }
    else
    {
        IOT_SetLogLevel(IOT_LOG_DEBUG);
        IOT_SetRootCA(ACCESS_IS_DEBUG);
    }

    main_loop_step = 0;
    void *callback;
    int dynamic_register = 0, post_reply_need = 0;
    evs_device_meta evs_dev_meta;
    memset(&evs_g_user_ctx, 0, sizeof(evs_g_user_ctx));
    int ret = 0;

#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0)
    {
        PROTOCOL_TRACE("IOT_ATM_Init failed!");
        return -1;
    }
#endif

#ifdef DYNAMIC_REGISTER
    memset(&dev_reg_meta, 0, sizeof(dev_reg_meta));
    memset(&evs_dev_meta, 0, sizeof(evs_dev_meta));
    callback = evs_service_callback(EVS_CERT_GET);
    if (callback)
    {
        ret = ((int (*)(evs_device_meta *meta))callback)(&evs_dev_meta);
    }

    DeviceSecretLength = strlen(evs_dev_meta.device_secret);
    rt_kprintf("1111111 aaaaaaaaaa(%d, %d)\n", DeviceSecretLength, ret);
    // 如果没有证书
    if ((DeviceSecretLength <= 5) || (DeviceSecretLength > IOTX_DEVICE_SECRET_LEN) || (ret < 0))
    {
        if (is_device_uid != 0)
        {
            callback = evs_service_callback(EVS_DEVICE_UID_GET);
            rt_kprintf("222222222 aaaaaaaaaa(%d, %d)\n", callback, ret);
            if (callback)
            {
                rt_kprintf("444444444444 aaaaaaaaaa(%d, %d)\n", callback, ret);
                regCodeLenth = ((int (*)(char *device_uid))callback)(device_uid);
                rt_kprintf("5555555555 aaaaaaaaaa(%d, %s)\n", regCodeLenth, device_uid);
                memcpy(dev_reg_meta.device_asset, device_uid, sizeof(device_uid));
            }
            rt_kprintf("333333333333 aaaaaaaaaa(%d, %d)\n", callback, ret);
            IOT_Get_Regcode(region, &dev_reg_meta);
            // PROTOCOL_TRACE("IOT_Get_Regcode dev_reg_meta.device_reg_code is %s\n",dev_reg_meta.device_reg_code);
            regCodeLenth = strlen(dev_reg_meta.device_reg_code);
            rt_kprintf("66666666666 aaaaaaaaaa(%d, %d)\n", regCodeLenth, ret);
        }
        else
        {
            callback = evs_service_callback(EVS_DEVICE_REG_CODE_GET);
            if (callback)
            {

                regCodeLenth = ((int (*)(char *device_reg_code))callback)(reg_code);
                memcpy(dev_reg_meta.device_reg_code, reg_code, sizeof(reg_code));
            }
        }
        // regCodeLenth = HAL_GetDeviceRegCode(g_device_reg_code);
        // 判断注册码，如果没有注册码，则在此循环
        // PROTOCOL_TRACE("dev_reg_meta.device_reg_code length is %d\n",regCodeLenth);
        // PROTOCOL_TRACE("dev_reg_meta.device_reg_code is %s\n",dev_reg_meta.device_reg_code);
        if ((regCodeLenth < 5) || (regCodeLenth > IOTX_DEVICE_REG_CODE_LEN))
        {
            PROTOCOL_TRACE("Get RegCode failed");
            rt_kprintf("00000000 aaaaaaaaaa\n");
            return -1;
        }
        res = IOT_Dynamic_Register(region, &dev_reg_meta);
        if (res < 0)
        {
            rt_kprintf("00000000 CCCCCCCCC(%d)\n", res);
            PROTOCOL_TRACE("IOT_Dynamic_Register failed");
            return -2;
        }
        // HAL_Printf("\nProduct Key: %s\n", dev_reg_meta.product_key);
        memcpy(evs_dev_meta.product_key, dev_reg_meta.product_key, strlen(dev_reg_meta.product_key));
        // HAL_Printf("\nDevice Name: %s\n", dev_reg_meta.device_name);
        memcpy(evs_dev_meta.device_name, dev_reg_meta.device_name, strlen(dev_reg_meta.device_name));
        // HAL_Printf("\nDevice Secret: %s\n", dev_reg_meta.device_secret);
        memcpy(evs_dev_meta.device_secret, dev_reg_meta.device_secret, strlen(dev_reg_meta.device_secret));

        memcpy(evs_dev_meta.device_reg_code, dev_reg_meta.device_reg_code, strlen(dev_reg_meta.device_reg_code));

        callback = evs_service_callback(EVS_CERT_SET);
        if (callback)
        {
            ((int (*)(const evs_device_meta meta))callback)(evs_dev_meta);
        }
    }
#endif

    callback = evs_service_callback(EVS_CERT_GET);
    if (callback)
    {
        if (((int (*)(evs_device_meta *))callback)(&evs_dev_meta) != 0)
        {
            rt_kprintf("00000000 dddddddddddd(%d)\n", res);
            PROTOCOL_TRACE("Get cert failed");
            return -3;
        }
    }

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, evs_dev_meta.product_key, strlen(evs_dev_meta.product_key));
    memcpy(master_meta_info.device_name, evs_dev_meta.device_name, strlen(evs_dev_meta.device_name));
    memcpy(master_meta_info.device_secret, evs_dev_meta.device_secret, strlen(evs_dev_meta.device_secret));

    /* 注册回调函数 */
    IOT_RegisterCallback(ITE_STATE_EVERYTHING, user_sdk_state_dump);
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);
    IOT_RegisterCallback(ITE_FOTA, user_fota_event_handler);
    IOT_RegisterCallback(ITE_CLOUD_ERROR, user_cloud_error_handler);
    IOT_RegisterCallback(ITE_DYNREG_DEVICE_SECRET, dynreg_device_secret);

    /* 选择上线方式（是否使用动态注册） */
    dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* 是否需用属性、事件上报(设置)应答 */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_reply_need);

    if (evs_access == ACCESS_IS_FORMAL)
    {
        IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)"10.111.186.1");
        IOT_Ioctl(IOTX_IOCTL_SET_MQTT_PORT, (void *)&custom_port);
    }

    return 0;
}

int evs_linkkit_free()
{
    IOT_Linkkit_Close(evs_g_user_ctx.master_devid);
    IOT_SetLogLevel(IOT_LOG_NONE);
    main_loop_step = -1;
    return 0;
}

void evs_set_firmware_version(const char *verion)
{
    HAL_SetFirmwareVersion(verion);
}

int evs_mainopen(void)
{
    int res = 0;

    PROTOCOL_TRACE("IOT_Linkkit_Open Start\n");
    evs_g_user_ctx.master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
    if (evs_g_user_ctx.master_devid >= 0)
    {
        PROTOCOL_TRACE("IOT_Linkkit_Open success\n");
    }
    else
    {
        PROTOCOL_TRACE("IOT_Linkkit_Open failed!\n");
        res = EVS_LINKKIT_OPEN_FAULT;
    }

    return res;
}

int evs_mainconnect(void)
{
    int res = 0;

    PROTOCOL_TRACE("IOT_Linkkit_Connect Start\n");
    res = IOT_Linkkit_Connect(evs_g_user_ctx.master_devid);
    if (res >= 0)
    {
        PROTOCOL_TRACE("IOT_Linkkit_Connect success\n");
    }
    else
    {
        PROTOCOL_TRACE("IOT_Linkkit_Connect failed!\n");
        res = EVS_LINKKIT_CONNECT_FAULT;
    }

    return res;
}

int evs_mainclose(void)
{
    PROTOCOL_TRACE("IOT_Linkkit_Close\n");
    return IOT_Linkkit_Close(evs_g_user_ctx.master_devid);
}

int evs_mainyield(void)
{
    int res = 0;

    PROTOCOL_TRACE("IOT_Linkkit_Yield Start\n");
    res = IOT_Linkkit_Yield(EVS_YIELD_TIMEOUT_MS);
    if (res < 0)
    {
        PROTOCOL_TRACE("IOT_Linkkit_Yield failed!\n");
        res = EVS_LINKKIT_YIELD_FAULT;
    }

    return res;
}

void evs_send_event(evs_cmd_event_enum event_type, void *param)
{
    switch (event_type)
    {
    case EVS_CMD_EVENT_FIRMWARE_INFO:
        send_event_firmware_info((evs_event_firmware_info *)param);
        break;
    case EVS_CMD_EVENT_ASK_FEEMODEL:
        send_event_ask_feeModel((evs_event_ask_feeModel *)param);
        break;
    case EVS_CMD_EVENT_STARTCHARGE:
        send_event_auth_start((evs_event_startCharge *)param);
        break;
    case EVS_CMD_EVENT_STARTRESULT:
        send_event_start_result((evs_event_startResult *)param);
        break;
    case EVS_CMD_EVENT_STOPCHARGE:
        send_event_stopCharge((evs_event_stopCharge *)param);
        break;
    case EVS_CMD_EVENT_TRADEINFO:
        send_event_tradeInfo((evs_event_tradeInfo *)param);
        break;
    case EVS_CMD_EVENT_ALARM:
        send_event_alarm((evs_event_alarm *)param);
        break;
    case EVS_CMD_EVENT_ACPILE_CHANGE:
        send_event_acPile_change((evs_event_pile_stutus_change *)param);
        break;
    case EVS_CMD_EVENT_DCPILE_CHANGE:
        send_event_dcPile_change((evs_event_pile_stutus_change *)param);
        break;
    case EVS_CMD_EVENT_GROUNDLOCK_CHANGE:
        send_event_groundLock_change((evs_event_groundLock_change *)param);
        break;
    case EVS_CMD_EVENT_GATELOCK_CHANGE:
        send_event_gateLock_change((evs_event_gateLock_change *)param);
        break;
    case EVS_CMD_EVENT_ASK_DEV_CONFIG:
        send_event_ask_dev_config();
        break;
    case EVS_CMD_EVENT_CAR_INFO:
        send_event_car_info((evs_event_car_info *)param);
        break;
    case EVS_CMD_EVENT_VER_INFO:
        send_event_ver_info((evs_event_ver_info *)param);
        break;
    case EVS_CMD_EVENT_LOGQUERY_RESULT:
        send_event_logquery_result((evs_event_logQuery_Result *)param);
        break;
    case EVS_CMD_EVENT_CARD_INFO:
        send_event_card_info((evs_event_card_info *)param);
        break;
    case EVS_CMD_EVENT_CARD_AUTH:
        send_event_card_auth((evs_event_card_auth *)param);
        break;
    case EVS_CMD_EVENT_CARD_AUTH_RESULT:
        send_event_card_auth_result((evs_event_card_auth_result *)param);
        break;
    case EVS_CMD_EVENT_CARD_CHECK_ERROR:
        send_event_card_check_error((evs_event_card_check_error *)param);
        break;
    case EVS_CMD_EVENT_DEVMDU_INFO:
        send_event_devmdu_info((evs_event_devmdu_info *)param);
        break;
    case EVS_CMD_EVENT_CHARGE_CONNECT:
        send_event_charge_connect((evs_event_charge_connect_change *)param);
        break;
    case EVS_CMD_EVENT_BLE_PLUG_CHARGE_INFO:
        send_event_ble_plug_chg_info((evs_event_ble_plug_charge_info *)param);
        break;
    case EVS_CMD_EVENT_BLE_CONN_CHANGE:
        send_event_ble_conn_change((evs_event_ble_conn_change *)param);
        break;
    case EVS_CMD_EVENT_VIN_CHECK_RESULT:
        send_event_vin_list_result((evs_event_vinList_result *)param);
        break;
    case EVS_CMD_EVENT_PILE_PARTS_CCU_INFO:
        send_event_pile_parts_ccu_info((evs_event_ccu_info *)param);
        break;
    case EVS_CMD_EVENT_PILE_PARTS_PCU_INFO:
        send_event_pile_parts_pcu_info((evs_event_pcu_info *)param);
        break;
    case EVS_CMD_EVENT_PILE_PARTS_CU_INFO:
        send_event_pile_parts_cu_info((evs_event_cu_info *)param);
        break;
    case EVS_CMD_EVENT_PILE_PARTS_SWITCH_INFO:
        send_event_pile_parts_switch_info((evs_event_switch_info *)param);
        break;
    case EVS_CMD_EVENT_PILE_PARTS_EDAS_INFO:
        send_event_pile_parts_edas_info((evs_event_edas_info *)param);
        break;
    case EVS_CMD_EVENT_PILE_PARTS_CU_WORK_INFO:
        send_event_pile_parts_cu_work_info((evs_event_cu_work_info *)param);
        break;
    case EVS_CMD_EVENT_RAW_DATA:
        send_event_raw((char *)param);
        break;
    case EVS_CMD_EVENT_DEV_MAINTAIN_RESULT:
        send_event_dev_maintain_result((evs_event_feedback_dev_maintain *)param);
        break;
    case EVS_CMD_EVENT_LOCK_CTRL_RESULT:
        send_event_lock_ctrl_result((evs_event_feedback_lockCtrl *)param);
        break;
    case EVS_CMD_EVENT_TIME_SYNC_RESULT:
        send_event_time_sync_result((evs_event_time_sync_result *)param);
        break;
    default:
        break;
    }
}

void evs_send_property(evs_cmd_property_enum property_type, void *param)
{
    switch (property_type)
    {
    case EVS_CMD_PROPERTY_DCPILE:
        send_property_dcPile((evs_property_dcPile *)param);
        break;
    case EVS_CMD_PROPERTY_ACPILE:
        send_property_acPile((evs_property_acPile *)param);
        break;
    case EVS_CMD_PROPERTY_AC_WORK:
        send_property_ac_work((evs_property_ac_work *)param);
        break;
    case EVS_CMD_PROPERTY_AC_NONWORK:
        send_property_ac_nonwork((evs_property_ac_nonWork *)param);
        break;
    case EVS_CMD_PROPERTY_BMS:
        send_property_bms((evs_property_BMS *)param);
        break;
    case EVS_CMD_PROPERTY_DC_INPUT_METER:
        send_property_dc_input_meter((evs_property_dc_input_meter *)param);
        break;
    case EVS_CMD_PROPERTY_AC_OUTMETER:
        send_property_ac_outmeter((evs_property_meter *)param);
        break;
    case EVS_CMD_PROPERTY_DC_OUTMETER:
        send_property_dc_outmeter((evs_property_meter *)param);
        break;
    case EVS_CMD_PROPERTY_DC_WORK:
        send_property_dc_work((evs_property_dc_work *)param);
        break;
    case EVS_CMD_PROPERTY_DC_NONWORK:
        send_property_dc_nonwork((evs_property_dc_nonWork *)param);
        break;
    default:
        break;
    }
}

/*--------------------------------------智能枪-----------------------------------------------*/

/*------------------------------------------------------------------------------------------------
* 向读卡器发送数据并接收数据返回
------------------------------------------------------------------------------------------------*/
static void evs_card_exchange_data(int fd, int send_count, unsigned char *send_buf, int *recv_count, unsigned char *reply_buf, int opCnt)
{
    int n = 0, nCount = 0, dataCount = 0;
    int reply_count = 128;
    int ret = 0, send_err_time = 0;
    char buf[128] = {0};

    for (n = 0; n < opCnt; n++)
    {
        nCount = 0;

        ret = HAL_Write(fd, send_buf, send_count);
        if (ret != send_count)
        {
            send_err_time++;
            if (send_err_time == 3)
            {
                PROTOCOL_TRACE("Send To Reader Failed");
                break;
            }
            continue;
        }

        memset(buf, 0, sizeof(buf));

        HAL_SleepMs(200);
        while ((nCount = HAL_Read(fd, buf, reply_count)) > 0)
        {
            if (nCount == (((buf[1] << 8) + buf[2]) + 5))
            {
                memcpy(reply_buf, buf, nCount);
                *recv_count = nCount;
                return;
            }
            else
            {
                dataCount = (((buf[1] << 8) + buf[2]) + 5);
                if (dataCount > 0)
                {
                    if ((buf[0] == 0x02) && (buf[dataCount - 1] == 0x03))
                    {
                        memcpy(reply_buf, buf, dataCount);
                        *recv_count = dataCount;
                        return;
                    }
                }
            }
        }
    }

    *recv_count = 0;
    return;
}

/*
static void evs_card_exchange_data(int fd, int send_count, unsigned char *send_buf, int *recv_count, unsigned char *reply_buf, int opCnt)
{
    int n = 0, nCount = 0, sleepCnt = 0;
    int ret = 0, send_err_time = 0, recv_err_time = 0;
    int reciev_data = 0, framLen = 0;
    int bufLen = *recv_count;

    for (n = 0; n < opCnt; n++)
    {
        sleepCnt = 100;
        nCount = 0;
        reciev_data = 0;
        memset(reply_buf, 0, sizeof(bufLen));

        ret = HAL_Write(fd, send_buf, send_count);
        if (ret != send_count)
        {
            send_err_time++;
            if (send_err_time == 3)
            {
                PROTOCOL_TRACE("Send To Reader Failed");
                break;
            }
            continue;
        }

        while (1)
        {
            HAL_SleepMs(sleepCnt);
            reciev_data = HAL_Read(fd, (reply_buf + nCount), bufLen);
            if (reciev_data > 0)
            {
                nCount += reciev_data;
                framLen = (reply_buf[1] << 8) + reply_buf[2] + 5;

                if (framLen > bufLen)
                {
                    nCount = 0;
                    memset(reply_buf, 0, sizeof(bufLen));
                }
                else
                {
                    if (nCount >= framLen)
                    {
                        if (reply_buf[0] == 0x2 && reply_buf[framLen - 1] == 0x3)
                        {
                            PROTOCOL_TRACE("Recv Reader %d Bytes", nCount);
                            *recv_count = nCount;
                            return;
                        }
                        else
                        {
                            nCount = 0;
                            memset(reply_buf, 0, sizeof(bufLen));
                        }
                    }
                }
            }
            else //等待数据
            {
                sleepCnt = 10;
                recv_err_time++;
                if (recv_err_time > 10)
                {
                    PROTOCOL_TRACE("Read From Reader Failed");
                    break;
                }
            }
        }
    }

    *recv_count = 0;
    return;
}
*/

/*------------------------------------------------------------------------------------------------
* 关闭射频
------------------------------------------------------------------------------------------------*/
static int evs_card_ctrl_rf_close(int fd)
{
    unsigned char order_buf[7];
    unsigned char reply_buf[128];
    int nCount = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x2;
    order_buf[3] = 0x31;
    order_buf[4] = 0x91;
    order_buf[5] = order_buf[3] ^ order_buf[4];
    order_buf[6] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if ((reply_buf[3] == 0) && (reply_buf[4] == 0))
        return EVS_OPT_READER_SUCCESS;
    else
        return EVS_CLOSE_RF_FAIL;
}

/*------------------------------------------------------------------------------------------------
* 打开射频
------------------------------------------------------------------------------------------------*/
static int evs_card_ctrl_rf_open(int fd)
{
    unsigned char order_buf[7];
    unsigned char reply_buf[128];
    int nCount = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x2;
    order_buf[3] = 0x31;
    order_buf[4] = 0x90;
    order_buf[5] = (order_buf[3] ^ order_buf[4]);
    order_buf[6] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if ((reply_buf[3] == 0) && (reply_buf[4] == 0))
        return EVS_OPT_READER_SUCCESS;
    else
        return EVS_OPEN_RF_FAIL;
}

/**-----------------------------------------------------------------------------------------------
* 寻卡
------------------------------------------------------------------------------------------------*/
static int evs_card_ctrl_found(int fd, int gunNo)
{
    unsigned char order_buf[9];
    unsigned char reply_buf[512];
    int i = 0;
    int nCount = 0, err = -1;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));
    memset(gun_auth_process_data[gunNo - 1].usrId, 0, sizeof(gun_auth_process_data[gunNo - 1].usrId));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x04;
    order_buf[3] = 0x32;
    order_buf[4] = 0x24;
    order_buf[5] = 0x00;
    order_buf[6] = 0xc3;
    order_buf[7] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6]);
    order_buf[8] = 0x3;

    nCount = sizeof(reply_buf);

    unsigned int startTime = HAL_UptimeMs();
    while (1)
    {
        if ((HAL_UptimeMs() - startTime) < 10 * 1000)
        {
            evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
            if (nCount == 0)
            {
                PROTOCOL_TRACE("Get Data Failed");
                return -1;
            }

            err = ((reply_buf[3] << 8) + reply_buf[4]);
            if (err == 0)
            {
                for (i = 0; i < 8; i++)
                {
                    gun_auth_process_data[gunNo - 1].usrId[i] = reply_buf[18 + i];
                }
            }
            else
            {
                PROTOCOL_TRACE("Activity Card Failed!");
            }
            break;
        }
        else
        {
            PROTOCOL_TRACE("Found Card Timeout!");
            break;
        }
    }
    return err;
}

/*------------------------------------------------------------------------------------------------
* 设置读卡器波特率
------------------------------------------------------------------------------------------------*/
static int evs_card_set_baudRate(int fd, int baud)
{
    unsigned char order_buf[8];
    unsigned char reply_buf[128];
    int nCount = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x02;
    order_buf[1] = 0x00;
    order_buf[2] = 0x03;
    order_buf[3] = 0x30;
    order_buf[4] = 0x01;
    order_buf[5] = baud;
    order_buf[6] = order_buf[3] ^ order_buf[4] ^ order_buf[5];
    order_buf[7] = 0x03;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if ((reply_buf[3] == 0) && (reply_buf[4] == 0))
        return EVS_OPT_READER_SUCCESS;
    else
        return EVS_SET_READER_BAUD_FAIL;
}

/**-----------------------------------------------------------------------------------------------
* 获取用户卡信息
------------------------------------------------------------------------------------------------*/
static int evs_card_get_cardSn(int fd)
{
    unsigned char order_buf[15];
    unsigned char reply_buf[128];
    unsigned char usr_id[10] = {0};
    unsigned char usr_version = 0;
    int i = 0;
    int nCount = 0;
    int nPos = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0xa;
    order_buf[3] = 0x32;
    order_buf[4] = 0x26;
    order_buf[5] = 0xff;
    order_buf[6] = 0x0;
    order_buf[7] = 0xA4;
    order_buf[8] = 0x00;
    order_buf[9] = 0x00;
    order_buf[10] = 0x02;
    order_buf[11] = 0xDF;
    order_buf[12] = 0x02;
    order_buf[13] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6] ^ order_buf[7] ^ order_buf[8] ^ order_buf[9] ^ order_buf[10] ^ order_buf[11] ^ order_buf[12]);
    order_buf[14] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if ((reply_buf[nCount - 4] == 0x90) && (reply_buf[nCount - 3] == 0x00))
    {
        for (i = 0; i < nCount - 1; i++)
        {
            if (reply_buf[i] == 0x9f && reply_buf[i + 1] == 0x0c)
            {
                usr_version = reply_buf[i + 12];
                nPos = i + 13;
                break;
            }
        }
    }

    if (nPos == 0)
    {
        return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
    }

    for (i = 0; i < 10; i++)
    {
        usr_id[i] = reply_buf[nPos + i];
    }

    return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
}

static int evs_card_check_pin(int fd)
{
    unsigned char order_buf[16];
    unsigned char reply_buf[128];
    int nCount = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0xb;
    order_buf[3] = 0x32;
    order_buf[4] = 0x26;
    order_buf[5] = 0xff;
    order_buf[6] = 0x0;
    order_buf[7] = 0x20;
    order_buf[8] = 0x00;
    order_buf[9] = 0x00;
    order_buf[10] = 0x03;
    // 默认用户卡密码
    order_buf[11] = 0x12;
    order_buf[12] = 0x34;
    order_buf[13] = 0x56;

    order_buf[14] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6] ^ order_buf[7] ^ order_buf[8] ^ order_buf[9] ^ order_buf[10] ^ order_buf[11] ^ order_buf[12] ^ order_buf[13]);
    order_buf[15] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if (((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]) != 0x9000)
    {
        PROTOCOL_TRACE("Check PIN Failed!");
    }
    return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
}

static int evs_card_get_keyInfo(int fd, int gunNo)
{
    unsigned char order_buf[13];
    unsigned char reply_buf[128];
    int nCount = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x8;
    order_buf[3] = 0x32;
    order_buf[4] = 0x26;
    order_buf[5] = 0xff;
    order_buf[6] = 0x0;
    order_buf[7] = 0xB0;
    order_buf[8] = 0x95;
    order_buf[9] = 0x09;
    order_buf[10] = 0x01;
    order_buf[11] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6] ^ order_buf[7] ^ order_buf[8] ^ order_buf[9] ^ order_buf[10]);
    order_buf[12] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if (((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]) == 0x9000)
    {
        gun_auth_process_data[gunNo - 1].PKid = reply_buf[5];
    }
    else
    {
        PROTOCOL_TRACE("Get Pkey Failed");
    }
    return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
}

/**-----------------------------------------------------------------------------------------------
* 验证平台下发的MAC2
------------------------------------------------------------------------------------------------*/
static int evs_card_trade_confirm(int fd, int gunNo)
{
    unsigned char order_buf[25] = {0};
    unsigned char reply_buf[128] = {0};
    int nCount = 0;

    memset(gun_auth_process_data[gunNo - 1].TAC, 0, sizeof(gun_auth_process_data[gunNo - 1].TAC));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x14;
    order_buf[3] = 0x32;
    order_buf[4] = 0x26;
    order_buf[5] = 0xff;
    order_buf[6] = 0x80;
    order_buf[7] = 0x52;
    order_buf[8] = 0x00;
    order_buf[9] = 0x00;
    order_buf[10] = 0x0B;

    /***************************trade date start***************************************/
    order_buf[11] = gun_auth_process_data[gunNo - 1].tradeTime[0];
    order_buf[12] = gun_auth_process_data[gunNo - 1].tradeTime[1];
    order_buf[13] = gun_auth_process_data[gunNo - 1].tradeTime[2];
    order_buf[14] = gun_auth_process_data[gunNo - 1].tradeTime[3];
    order_buf[15] = gun_auth_process_data[gunNo - 1].tradeTime[4];
    order_buf[16] = gun_auth_process_data[gunNo - 1].tradeTime[5];
    order_buf[17] = gun_auth_process_data[gunNo - 1].tradeTime[6];
    /*****************************trade date end***************************************/

    /*********************************MAC2 start***************************************/
    order_buf[18] = gun_auth_process_data[gunNo - 1].MAC2[0];
    order_buf[19] = gun_auth_process_data[gunNo - 1].MAC2[1];
    order_buf[20] = gun_auth_process_data[gunNo - 1].MAC2[2];
    order_buf[21] = gun_auth_process_data[gunNo - 1].MAC2[3];
    /*********************************MAC2 end*****************************************/
    order_buf[22] = 0x04;

    order_buf[23] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6] ^ order_buf[7] ^ order_buf[8] ^ order_buf[9] ^ order_buf[10] ^ order_buf[11] ^ order_buf[12] ^ order_buf[13] ^ order_buf[14] ^ order_buf[15] ^ order_buf[16] ^ order_buf[17] ^ order_buf[18] ^ order_buf[19] ^ order_buf[20] ^ order_buf[21] ^ order_buf[22]);
    order_buf[24] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if (((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]) == 0x9000)
    {
        gun_auth_process_data[gunNo - 1].TAC[0] = reply_buf[5];
        gun_auth_process_data[gunNo - 1].TAC[1] = reply_buf[6];
        gun_auth_process_data[gunNo - 1].TAC[2] = reply_buf[7];
        gun_auth_process_data[gunNo - 1].TAC[3] = reply_buf[8];
    }
    else
    {
        PROTOCOL_TRACE("Trade Confirm Failed");
    }

    return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
}

/**-----------------------------------------------------------------------------------------------
* 获取卡片随机数及MAC1等信息
------------------------------------------------------------------------------------------------*/
static int evs_card_get_authCode(int fd, int gunNo)
{
    unsigned char order_buf[25];
    unsigned char reply_buf[128];
    int nCount = 0;

    gun_auth_process_data[gunNo - 1].authVer = 0;
    gun_auth_process_data[gunNo - 1].authFlag = 0;
    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    memset(gun_auth_process_data[gunNo - 1].cardBalance, 0, sizeof(gun_auth_process_data[gunNo - 1].cardBalance));
    memset(gun_auth_process_data[gunNo - 1].tradeSn, 0, sizeof(gun_auth_process_data[gunNo - 1].tradeSn));

    memset(gun_auth_process_data[gunNo - 1].randomC, 0, sizeof(gun_auth_process_data[gunNo - 1].randomC));
    memset(gun_auth_process_data[gunNo - 1].MAC1, 0, sizeof(gun_auth_process_data[gunNo - 1].MAC1));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x14;
    order_buf[3] = 0x32;
    order_buf[4] = 0x26;
    order_buf[5] = 0xff;
    order_buf[6] = 0x80;
    order_buf[7] = 0x50;
    order_buf[8] = 0x00;
    order_buf[9] = 0x01;
    order_buf[10] = 0x0B;

    order_buf[11] = gun_auth_process_data[gunNo - 1].PKid;
    order_buf[12] = 0x00;
    order_buf[13] = 0x00;
    order_buf[14] = 0x00;
    order_buf[15] = 0x00;

    order_buf[16] = gun_auth_process_data[gunNo - 1].terminalSn[0];
    order_buf[17] = gun_auth_process_data[gunNo - 1].terminalSn[1];
    order_buf[18] = gun_auth_process_data[gunNo - 1].terminalSn[2];
    order_buf[19] = gun_auth_process_data[gunNo - 1].terminalSn[3];
    order_buf[20] = gun_auth_process_data[gunNo - 1].terminalSn[4];
    order_buf[21] = gun_auth_process_data[gunNo - 1].terminalSn[5];

    order_buf[22] = 0x10;

    order_buf[23] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6] ^ order_buf[7] ^ order_buf[8] ^ order_buf[9] ^ order_buf[10] ^ order_buf[11] ^ order_buf[12] ^ order_buf[13] ^ order_buf[14] ^ order_buf[15] ^ order_buf[16] ^ order_buf[17] ^ order_buf[18] ^ order_buf[19] ^ order_buf[20] ^ order_buf[21] ^ order_buf[22]);
    order_buf[24] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if (((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]) == 0x9000)
    {
        gun_auth_process_data[gunNo - 1].cardBalance[0] = reply_buf[5];
        gun_auth_process_data[gunNo - 1].cardBalance[1] = reply_buf[6];
        gun_auth_process_data[gunNo - 1].cardBalance[2] = reply_buf[7];
        gun_auth_process_data[gunNo - 1].cardBalance[3] = reply_buf[8];

        gun_auth_process_data[gunNo - 1].tradeSn[0] = reply_buf[9];
        gun_auth_process_data[gunNo - 1].tradeSn[1] = reply_buf[10];

        gun_auth_process_data[gunNo - 1].authVer = reply_buf[11];
        gun_auth_process_data[gunNo - 1].authFlag = reply_buf[12];

        gun_auth_process_data[gunNo - 1].randomC[0] = reply_buf[13];
        gun_auth_process_data[gunNo - 1].randomC[1] = reply_buf[14];
        gun_auth_process_data[gunNo - 1].randomC[2] = reply_buf[15];
        gun_auth_process_data[gunNo - 1].randomC[3] = reply_buf[16];

        gun_auth_process_data[gunNo - 1].MAC1[0] = reply_buf[17];
        gun_auth_process_data[gunNo - 1].MAC1[1] = reply_buf[18];
        gun_auth_process_data[gunNo - 1].MAC1[2] = reply_buf[19];
        gun_auth_process_data[gunNo - 1].MAC1[3] = reply_buf[20];
    }
    else
    {
        PROTOCOL_TRACE("Get Auth Info Failed");
    }

    return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
}

/*----------------------------------------------------------------
* 检查读卡器状态
---------------------------------------------------------------*/
static int evs_card_check_reader(int fd)
{
    unsigned char order_buf[7];
    unsigned char reply_buf[128];
    int nCount = 0;

    memset(reply_buf, 0, sizeof(reply_buf));
    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x2;

    order_buf[3] = 0x31;
    order_buf[4] = 0x11;

    order_buf[5] = order_buf[3] ^ order_buf[4];
    order_buf[6] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if ((reply_buf[3] == 0) && (reply_buf[4] == 0))
        return EVS_OPT_READER_SUCCESS;
    else
        return EVS_CHECK_READER_FAIL;
}

/**-----------------------------------------------------------------------------------------------
* 获取物理卡号及密码标识
------------------------------------------------------------------------------------------------*/
static int evs_card_get_psyInfo(int fd, int gunNo)
{
    unsigned char order_buf[13];
    unsigned char reply_buf[128];
    unsigned char Enabled = 0;
    unsigned char usr_type = 0;
    unsigned char card_type = 0;
    unsigned char pin_flage = 0;
    int i = 0;
    int nCount = 0;

    memset(order_buf, 0, sizeof(order_buf));
    memset(reply_buf, 0, sizeof(reply_buf));

    order_buf[0] = 0x2;
    order_buf[1] = 0x0;
    order_buf[2] = 0x8;
    order_buf[3] = 0x32;
    order_buf[4] = 0x26;
    order_buf[5] = 0xff;
    order_buf[6] = 0x0;
    order_buf[7] = 0xB0;
    order_buf[8] = 0x85;
    order_buf[9] = 0x00;
    order_buf[10] = 0x15;

    order_buf[11] = (order_buf[3] ^ order_buf[4] ^ order_buf[5] ^ order_buf[6] ^ order_buf[7] ^ order_buf[8] ^ order_buf[9] ^ order_buf[10]);
    order_buf[12] = 0x3;

    nCount = sizeof(reply_buf);

    evs_card_exchange_data(fd, sizeof(order_buf), order_buf, &nCount, reply_buf, 3);
    if (nCount == 0)
    {
        PROTOCOL_TRACE("Get Data Failed");
        return -1;
    }

    if (((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]) == 0x9000)
    {
        Enabled = reply_buf[5]; // 是否启用卡片

        for (i = 0; i < 8; i++)
        {
            gun_auth_process_data[gunNo - 1].psy_id[i] = reply_buf[6 + i]; // 物理卡号
        }

        usr_type = reply_buf[14];  // 用户类型
        card_type = reply_buf[15]; // 卡类型
        pin_flage = reply_buf[16]; // 密码标识
    }
    else
    {
        PROTOCOL_TRACE("Get Psy Info Failed");
    }

    return ((reply_buf[nCount - 4] << 8) + reply_buf[nCount - 3]);
}

int evs_card_ctrl_init(int fd)
{
    int err = 0;
    err = evs_card_set_baudRate(fd, 3);
    if (err != 0)
    {
        PROTOCOL_TRACE("Set Baud Ratio Failed!");
        return EVS_SET_READER_BAUD_FAIL;
    }

    HAL_SleepMs(100);

    err = evs_card_check_reader(fd);
    if (err != 0)
    {
        PROTOCOL_TRACE("Check Reader Failed!");
        return EVS_CHECK_READER_FAIL;
    }

    HAL_SleepMs(100);

    return err;
}

/*------------------------------------------------------------------------------------------------
* 读卡器操作
------------------------------------------------------------------------------------------------*/
static int evs_ctrl_reader(evs_cmd_card_enum ctrl_cmd, int fd, int gunNo)
{
    int ret = 0;
    switch (ctrl_cmd)
    {
    case EVS_CMD_READER_INIT:
        ret = evs_card_ctrl_init(fd);
        break;
    case EVS_CMD_READER_CHECK:
        ret = evs_card_check_reader(fd);
        break;
    case EVS_CMD_OPEN_RF:
        ret = evs_card_ctrl_rf_open(fd);
        break;
    case EVS_CMD_FOUND_CARD:
        ret = evs_card_ctrl_found(fd, gunNo);
        break;
    case EVS_CMD_CHECK_PIN:
        ret = evs_card_check_pin(fd);
        break;
    case EVS_CMD_GET_KEY_VER:
        ret = evs_card_get_keyInfo(fd, gunNo);
        break;
    case EVS_CMD_GET_USER_ID:
        ret = evs_card_get_cardSn(fd);
        break;
    case EVS_CMD_GET_PSY_ID:
        ret = evs_card_get_psyInfo(fd, gunNo);
        break;
    case EVS_CMD_GET_CARD_AUTH:
        ret = evs_card_get_authCode(fd, gunNo);
        break;
    case EVS_CMD_CONFIRM_AUTHCODE_S:
        ret = evs_card_trade_confirm(fd, gunNo);
        break;
    case EVS_CMD_CLOSE_RF:
        ret = evs_card_ctrl_rf_close(fd);
        break;
    default:
        break;
    }

    return ret;
}

static int evs_card_info_get(int fd, int gunNo)
{
    static int loop = 0;
    int err = 0;

    switch (loop)
    {
    case 0:
        err = evs_ctrl_reader(EVS_CMD_OPEN_RF, fd, gunNo);
        if (err != 0)
        {
            err = EVS_CARD_ERROR_OPEN_RF;
            break;
        }
        HAL_SleepMs(100);
        loop++;
    case 1:
        err = evs_ctrl_reader(EVS_CMD_FOUND_CARD, fd, gunNo);
        if (err != 0)
        {
            err = EVS_CARD_ERROR_FOUND_CARD;
            loop = 0;
            break;
        }
        HAL_SleepMs(100);
        loop++;
    case 2:
        err = evs_ctrl_reader(EVS_CMD_GET_USER_ID, fd, gunNo);
        if (err != 0x9000)
        {
            err = EVS_CARD_ERROR_GET_USER_ID;
            loop = 0;
            break;
        }
        HAL_SleepMs(100);
        loop++;
    case 3:
        err = evs_ctrl_reader(EVS_CMD_GET_PSY_ID, fd, gunNo);
        if (err != 0x9000)
        {
            err = EVS_CARD_ERROR_GET_PSY_ID;
            loop = 0;
            break;
        }
        HAL_SleepMs(100);
        loop++;
    case 4:
        err = evs_ctrl_reader(EVS_CMD_CHECK_PIN, fd, gunNo);
        if (err != 0x9000)
        {
            err = EVS_CARD_ERROR_CHECK_PIN;
            loop = 0;
            break;
        }
        HAL_SleepMs(100);
        loop++;
    case 5:
        err = evs_ctrl_reader(EVS_CMD_GET_KEY_VER, fd, gunNo);
        if (err != 0x9000)
            err = EVS_CARD_ERROR_GET_KEY_VER;
        loop = 0;
        break;
    default:
        break;
    }

    return err;
}

int evs_card_handle(int fd, evs_smart_gun_auth_param *param)
{
    static unsigned char step = 0;
    static unsigned int authTime;
    static unsigned char sendCnt = 0;
    int authResult = 0;

    if (param->gunNo < 1 || param->gunNo > EVS_MAX_PORT_NUM)
    {
        authResult = EVS_HANDLE_GUN_NUM_WRONG;
        HAL_Printf("param->gunNo is over range: %d", param->gunNo);
        return authResult;
    }

    evs_event_card_info card_info = {0};
    card_info.gunNo = param->gunNo;
    card_info.startMode = param->startMode;

    evs_event_card_auth card_auth = {0};
    card_auth.gunNo = param->gunNo;
    evs_event_card_auth_result card_auth_result = {0};
    card_auth_result.gunNo = param->gunNo;
    memcpy(card_auth_result.tradeNo, param->tradeNo, EVS_MAX_TRADE_LEN);

    evs_event_card_check_error card_error = {0};
    card_error.gunNo = param->gunNo;

    switch (step)
    {
    case 0:
        authResult = EVS_HANDLE_AUTHING;
        sendCnt = 0;
        memset(&gun_auth_process_data[param->gunNo - 1], 0, sizeof(evs_smart_gun_auth_process_data));
        gun_auth_process_data[param->gunNo - 1].gunNo = param->gunNo;

        int ret = evs_card_info_get(fd, param->gunNo);
        if (ret != 0x9000)
        {

            card_error.faultValue = ret;
            authResult = EVS_HANDLE_CARD_FAILED;
            evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
            break;
        }
        else
        {
            step++;
            authTime = HAL_UptimeMs();
            evs_send_event(EVS_CMD_EVENT_CARD_INFO, &card_info);
        }

    case 1:
        if (gun_auth_process_data[param->gunNo - 1].getTerminalFlag == 1)
        {
            gun_auth_process_data[param->gunNo - 1].getTerminalFlag = 0;
            step++;
            sendCnt = 0;
        }
        else
        {
            if ((HAL_UptimeMs() - authTime) > 10 * 1000)
            {
                if (sendCnt < 2)
                {
                    evs_send_event(EVS_CMD_EVENT_CARD_INFO, &card_info);
                    sendCnt++;
                    authTime = HAL_UptimeMs();
                }
                else
                {
                    card_error.faultValue = EVS_CARD_ERROR_TERMINAL_TIMEOUT; // get terminal number failed
                    authResult = EVS_HANDLE_TERMINAL_TIMEOUT;
                    evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
                }
            }
            break;
        }

    case 2:
        if (evs_ctrl_reader(EVS_CMD_GET_CARD_AUTH, fd, param->gunNo) != 0x9000)
        {
            card_error.faultValue = EVS_CARD_ERROR_GET_MAC1; // get mac1 failed
            authResult = EVS_HANDLE_GET_MAC1;
            evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
            break;
        }
        else
        {
            step++;
            authTime = HAL_UptimeMs();
            evs_send_event(EVS_CMD_EVENT_CARD_AUTH, &card_auth);
        }

    case 3:
        if (gun_auth_process_data[param->gunNo - 1].getServerAuthFlag == 1)
        {
            gun_auth_process_data[param->gunNo - 1].getServerAuthFlag = 0;
            step++;
            sendCnt = 0;
        }
        else
        {
            if ((HAL_UptimeMs() - authTime) > 10 * 1000)
            {
                if (sendCnt < 2)
                {
                    evs_send_event(EVS_CMD_EVENT_CARD_AUTH, &card_auth);
                    sendCnt++;
                    authTime = HAL_UptimeMs();
                }
                else
                {
                    card_error.faultValue = EVS_CARD_ERROR_CHECK_MAC1_TIMEOUT; // confirm card's mac1 or genarate mac2 failed!
                    authResult = EVS_HANDLE_AUTHCODE_TIMEOUT;
                    evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
                }
            }
            break;
        }
    case 4:
        if (evs_ctrl_reader(EVS_CMD_CONFIRM_AUTHCODE_S, fd, param->gunNo) != 0x9000)
        {
            card_error.faultValue = EVS_CARD_ERROR_CHECK_MAC2; // check mac2 failed
            authResult = EVS_HANDLE_CHECK_MAC2;
            evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
            break;
        }
        else
        {
            step++;
            authTime = HAL_UptimeMs();
            evs_ctrl_reader(EVS_CMD_CLOSE_RF, fd, param->gunNo);
            evs_send_event(EVS_CMD_EVENT_CARD_AUTH_RESULT, &card_auth_result);
        }

    case 5:
        if (gun_auth_process_data[param->gunNo - 1].getAuthChargeFlag == 1)
        {
            gun_auth_process_data[param->gunNo - 1].getAuthChargeFlag = 0;
            authResult = EVS_HANDLE_AUTH_SUCCESS;
        }
        else
        {
            if ((HAL_UptimeMs() - authTime) > 10 * 1000)
            {
                if (sendCnt < 2)
                {
                    evs_send_event(EVS_CMD_EVENT_CARD_AUTH_RESULT, &card_auth_result);
                    sendCnt++;
                    authTime = HAL_UptimeMs();
                }
                else
                {
                    card_error.faultValue = EVS_CARD_ERROR_AUTH_TIMEOUT; // authentication result timeout
                    authResult = EVS_HANDLE_AUTH_RESULT_TIMEOUT;
                    evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
                }
            }
            break;
        }
    default:
        break;
    }

    if (!param->gunIsReady && authResult == 0) // 已拔枪且鉴权未完成
    {
        card_error.faultValue = EVS_CARD_ERROR_GUN_PULL_OUT; // 鉴权过程中用户拔枪
        authResult = EVS_HANDLE_GUN_PULL_OUT;
        evs_send_event(EVS_CMD_EVENT_CARD_CHECK_ERROR, &card_error);
    }
    if (authResult != 0)
    {
        step = 0;
        evs_ctrl_reader(EVS_CMD_CLOSE_RF, fd, param->gunNo); // 关射频
    }

    return authResult;
}
