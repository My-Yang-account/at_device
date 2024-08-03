#include "interface.h"

/////////////////////////////////////////////////////////////////////////////////////////
#ifndef NULL
#define NULL (void *)0
#endif

typedef struct
{
    int service_id;
    void *callback;
} evs_service_map_t;

static evs_service_map_t g_evs_service_map[] = {

    /*服务回调*/
    {EVS_CONF_GET_SRV, NULL},               // 配置获取服务
    {EVS_CONF_UPDATE_SRV, NULL},            // 配置更新服务
    {EVS_QUE_DATA_SRV, NULL},               // 数据查询服务
    {EVS_DEV_MAINTAIN_SRV, NULL},           // 设备维护服务
    {EVS_CTRL_LOCK_SRV, NULL},              // 电子锁控制服务
    {EVS_FEE_MODEL_UPDATE_SRV, NULL},       // 计量计费模型更新服务
    {EVS_START_CHARGE_SRV, NULL},           // 平台远程启动服务
    {EVS_AUTH_RESULT_SRV, NULL},            // 启动充电鉴权结果
    {EVS_STOP_CHARGE_SRV, NULL},            // 平台停止充电
    {EVS_ORDER_CHECK_SRV, NULL},            // 交易记录确认服务
    {EVS_RSV_CHARGE_SRV, NULL},             // 预约充电服务
    {EVS_GROUND_LOCK_SRV, NULL},            // 地锁控制服务
    {EVS_GATE_LOCK_SRV, NULL},              // 智能门锁控制服务
    {EVS_ORDERLY_CHARGE_SRV, NULL},         // 有序充电策略下发
    {EVS_MAINTAIN_RESULT_SRV, NULL},        // 维护状态查询结果
    {EVS_FUN_CONF_UPDATE_SRV, NULL},        // 功能配置更新服务
    {EVS_FUN_CONF_GET_SRV, NULL},           // 功能配置获取服务
    {EVS_FEE_MODEL_QUERY_SRV, NULL},        // 计量计费模型查询服务
    {EVS_BLE_SECRET_UPDATE_SRV, NULL},      // 蓝牙秘钥更新服务
    {EVS_BLE_PLUG_CHG_INFO_GET_SRV, NULL},  // 蓝牙即插即充信息查询服务
    {EVS_BLE_PLUG_CHG_INFO_CONF_SRV, NULL}, // 蓝牙即插即充信息确认服务
    {EVS_BLE_AUTH_LIST_CLEAN_SRV, NULL},    // 蓝牙鉴权列表清除服务
    {EVS_BLE_RESET_SRV, NULL},              // 蓝牙即插即充重置服务
    {EVS_PILE_PARTS_CONF_SRV, NULL},        // 充电桩部件配置服务
    {EVS_PILE_PARTS_CONF_GET_SRV, NULL},    // 充电桩部件配置查询服务
    {EVS_VIN_LIST_UPDATE_SRV, NULL},        // VIN白名单更新服务
    {EVS_ORDER_ASK_SRV, NULL},              // 交易记录召测服务
    {EVS_Meter_ASK_SRV, NULL},              // 电表底值召测服务

    /*系统回调*/
    {EVS_STATE_EVERYTHING, NULL},    // SDK内部打印回调
    {EVS_CONNECT_SUCC, NULL},        // SDK连接平台成功回调
    {EVS_DISCONNECTED, NULL},        // SDK与平台断开连接回调
    {EVS_REPORT_REPLY, NULL},        // 属性上报回复回调
    {EVS_TRIGGER_EVENT_REPLY, NULL}, // 事件上报回复回调
    {EVS_CERT_GET, NULL},            // 证书获取回调
    {EVS_CERT_SET, NULL},            // 证书设置回调
    {EVS_DEVICE_REG_CODE_GET, NULL}, // 注册码获取回调
    {EVS_DEVICE_UID_GET, NULL},      // 设备唯一编码获取回调
    {EVS_TIME_SYNC, NULL},           // 时钟同步回调
    {EVS_OTA_UPDATE, NULL},          // 远程升级回调

};

void *evs_service_callback(int service)
{
    if (service < 0 || service >= sizeof(g_evs_service_map) / sizeof(evs_service_map_t))
    {
        return NULL;
    }
    return g_evs_service_map[service].callback;
}

/*服务回调函数定义*/
DEFINE_SEVICE_CALLBACK(EVS_DEV_MAINTAIN_SRV, int (*callback)(evs_service_dev_maintain *request))
DEFINE_SEVICE_CALLBACK(EVS_CTRL_LOCK_SRV, int (*callback)(evs_service_lockCtrl *request))
DEFINE_SEVICE_CALLBACK(EVS_FEE_MODEL_UPDATE_SRV, int (*callback)(evs_service_issue_feeModel *request, evs_service_feedback_feeModel *feedback))
DEFINE_SEVICE_CALLBACK(EVS_START_CHARGE_SRV, int (*callback)(evs_service_startCharge *request, evs_service_feedback_startCharge *feedback))
DEFINE_SEVICE_CALLBACK(EVS_AUTH_RESULT_SRV, int (*callback)(evs_service_authCharge *request, evs_service_feedback_authCharge *feedback))
DEFINE_SEVICE_CALLBACK(EVS_STOP_CHARGE_SRV, int (*callback)(evs_service_stopCharge *request, evs_service_feedback_stopCharge *feedback))
DEFINE_SEVICE_CALLBACK(EVS_ORDER_CHECK_SRV, int (*callback)(evs_service_confirmTrade *request, void *feedback))
DEFINE_SEVICE_CALLBACK(EVS_RSV_CHARGE_SRV, int (*callback)(evs_service_rsvCharge *request, evs_service_feedback_rsvCharge *feedback))
DEFINE_SEVICE_CALLBACK(EVS_GROUND_LOCK_SRV, int (*callback)(evs_service_groundLock_ctrl *request, evs_service_feedback_groundLock_ctrl *feedback))
DEFINE_SEVICE_CALLBACK(EVS_GATE_LOCK_SRV, int (*callback)(evs_service_gateLock_ctrl *request, evs_service_feedback_gateLock_ctrl *feedback))
DEFINE_SEVICE_CALLBACK(EVS_CONF_UPDATE_SRV, int (*callback)(evs_data_dev_config *request, int *feedback))
DEFINE_SEVICE_CALLBACK(EVS_CONF_GET_SRV, int (*callback)(evs_data_dev_config *feedback))
DEFINE_SEVICE_CALLBACK(EVS_MAINTAIN_RESULT_SRV, int (*callback)(evs_service_feedback_maintain_query *feedback))
DEFINE_SEVICE_CALLBACK(EVS_FUN_CONF_UPDATE_SRV, int (*callback)(evs_service_dev_fun_config *request, evs_service_feedback_dev_fun_config *feedback))
DEFINE_SEVICE_CALLBACK(EVS_FUN_CONF_GET_SRV, int (*callback)(evs_service_get_dev_fun_config *request, evs_service_dev_fun_config *feedback))
DEFINE_SEVICE_CALLBACK(EVS_FEE_MODEL_QUERY_SRV, int (*callback)(evs_service_feedback_feeModel_qurey *feedback))
DEFINE_SEVICE_CALLBACK(EVS_BLE_SECRET_UPDATE_SRV, int (*callback)(evs_service_blesecret_update *request, evs_service_feedback_blesecret_update *feedback))
DEFINE_SEVICE_CALLBACK(EVS_BLE_PLUG_CHG_INFO_GET_SRV, int (*callback)(evs_event_ble_plug_charge_info *feedback))
DEFINE_SEVICE_CALLBACK(EVS_BLE_PLUG_CHG_INFO_CONF_SRV, int (*callback)(evs_service_ble_plug_charge_info_conf *request))
DEFINE_SEVICE_CALLBACK(EVS_BLE_AUTH_LIST_CLEAN_SRV, int (*callback)(evs_service_blelist_clean *request, evs_service_feedback_blelist_clean *feedback))
DEFINE_SEVICE_CALLBACK(EVS_BLE_RESET_SRV, int (*callback)(evs_service_feedback_ble_reset *feedback))
DEFINE_SEVICE_CALLBACK(EVS_PILE_PARTS_CONF_SRV, int (*callback)(evs_service_config_parts *request, evs_service_feedback_config_parts *feedback))
DEFINE_SEVICE_CALLBACK(EVS_PILE_PARTS_CONF_GET_SRV, int (*callback)(evs_service_parts_config_get *request, evs_service_feedback_config_parts_get *feedback))
DEFINE_SEVICE_CALLBACK(EVS_VIN_LIST_UPDATE_SRV, int (*callback)(evs_service_vinList_update *request, evs_service_feedback_vinList_update *feedback))
DEFINE_SEVICE_CALLBACK(EVS_ORDER_ASK_SRV, int (*callback)(evs_service_trade_get *request, evs_service_feedback_trade_get *feedback))
DEFINE_SEVICE_CALLBACK(EVS_Meter_ASK_SRV, int (*callback)(evs_service_meter_get *request, evs_service_feedback_meter_get *feedback))
DEFINE_SEVICE_CALLBACK(EVS_QUE_DATA_SRV, int (*callback)(evs_service_query_log *request, evs_service_feedback_query_log *feedback))

/************直流功率调节控制与交流有序充电共用回调****************/
DEFINE_SEVICE_CALLBACK(EVS_ORDERLY_CHARGE_SRV, int (*callback)(evs_service_orderCharge *request, evs_service_feedback_orderCharge *feedback))

/*系统回调函数定义*/
DEFINE_SEVICE_CALLBACK(EVS_OTA_UPDATE, int (*callback)(const char *))
DEFINE_SEVICE_CALLBACK(EVS_TIME_SYNC, int (*callback)(const unsigned int))
DEFINE_SEVICE_CALLBACK(EVS_CONNECT_SUCC, int (*callback)(void))
DEFINE_SEVICE_CALLBACK(EVS_DISCONNECTED, int (*callback)(void))
DEFINE_SEVICE_CALLBACK(EVS_REPORT_REPLY, int (*callback)(const int, const int, const char *, const int))
DEFINE_SEVICE_CALLBACK(EVS_TRIGGER_EVENT_REPLY, int (*callback)(const int, const int, const char *, const int, const char *, const int))
DEFINE_SEVICE_CALLBACK(EVS_STATE_EVERYTHING, int (*callback)(int ev, const char *msg))
DEFINE_SEVICE_CALLBACK(EVS_CERT_GET, int (*callback)(evs_device_meta *meta))
DEFINE_SEVICE_CALLBACK(EVS_CERT_SET, int (*callback)(const evs_device_meta meta))
DEFINE_SEVICE_CALLBACK(EVS_DEVICE_REG_CODE_GET, int (*callback)(char *device_reg_code))
DEFINE_SEVICE_CALLBACK(EVS_DEVICE_UID_GET, int (*callback)(char *device_uid))
