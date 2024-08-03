#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "protocol_data_def.h"

    /*服务类型定义*/
    typedef enum
    {
        /*服务回调*/
        EVS_CONF_GET_SRV,               // 配置获取服务
        EVS_CONF_UPDATE_SRV,            // 配置更新服务
        EVS_QUE_DATA_SRV,               // 设备日志查询
        EVS_DEV_MAINTAIN_SRV,           // 设备维护服务
        EVS_CTRL_LOCK_SRV,              // 电子锁控制服务
        EVS_FEE_MODEL_UPDATE_SRV,       // 计量计费模型更新服务
        EVS_START_CHARGE_SRV,           // 平台远程启动服务
        EVS_AUTH_RESULT_SRV,            // 启动充电鉴权结果
        EVS_STOP_CHARGE_SRV,            // 平台停止充电
        EVS_ORDER_CHECK_SRV,            // 交易记录确认服务
        EVS_RSV_CHARGE_SRV,             // 预约充电服务
        EVS_GROUND_LOCK_SRV,            // 地锁控制服务
        EVS_GATE_LOCK_SRV,              // 智能门锁控制服务
        EVS_ORDERLY_CHARGE_SRV,         // 有序充电策略下发
        EVS_MAINTAIN_RESULT_SRV,        // 维护状态查询结果
        EVS_FUN_CONF_UPDATE_SRV,        // 功能配置更新服务
        EVS_FUN_CONF_GET_SRV,           // 功能配置获取服务
        EVS_FEE_MODEL_QUERY_SRV,        // 计量计费模型查询服务
        EVS_BLE_SECRET_UPDATE_SRV,      // 蓝牙秘钥更新服务
        EVS_BLE_PLUG_CHG_INFO_GET_SRV,  // 蓝牙即插即充信息查询服务
        EVS_BLE_PLUG_CHG_INFO_CONF_SRV, // 蓝牙即插即充信息确认服务
        EVS_BLE_AUTH_LIST_CLEAN_SRV,    // 蓝牙鉴权列表清除服务
        EVS_BLE_RESET_SRV,              // 蓝牙即插即充重置服务
        EVS_PILE_PARTS_CONF_SRV,        // 充电桩部件配置服务
        EVS_PILE_PARTS_CONF_GET_SRV,    // 充电桩部件配置查询服务
        EVS_VIN_LIST_UPDATE_SRV,        // VIN白名单更新服务
        EVS_ORDER_ASK_SRV,              // 交易记录召测服务
        EVS_Meter_ASK_SRV,              // 电表底值召测服务

        /*系统回调*/
        EVS_STATE_EVERYTHING,    // SDK状态变化回调
        EVS_CONNECT_SUCC,        // SDK连接平台成功回调
        EVS_DISCONNECTED,        // SDK与平台断开连接回调
        EVS_REPORT_REPLY,        // 属性上报回复回调
        EVS_TRIGGER_EVENT_REPLY, // 事件上报回复回调
        EVS_CERT_GET,            // 证书获取回调
        EVS_CERT_SET,            // 证书设置回调
        EVS_DEVICE_REG_CODE_GET, // 注册码获取回调
        EVS_DEVICE_UID_GET,      // 设备唯一编码获取回调
        EVS_TIME_SYNC,           // 时钟同步
        EVS_OTA_UPDATE           // OTA升级

    } evs_service_type_t;

void *evs_service_callback(int service);

#define EVS_RegisterCallback(service, cb) evs_register_for_##service(cb)
#define DECLARE_SEVICE_CALLBACK(service, cb) int evs_register_for_##service(cb);
#define DEFINE_SEVICE_CALLBACK(service, cb)                                                  \
    int evs_register_for_##service(cb)                                                       \
    {                                                                                        \
        if (service < 0 || service >= sizeof(g_evs_service_map) / sizeof(evs_service_map_t)) \
        {                                                                                    \
            return -1;                                                                       \
        }                                                                                    \
        g_evs_service_map[service].callback = (void *)callback;                              \
        return 0;                                                                            \
    }

    /*服务回调函数声明*/
    DECLARE_SEVICE_CALLBACK(EVS_DEV_MAINTAIN_SRV, int (*cb)(evs_service_dev_maintain *request))
    DECLARE_SEVICE_CALLBACK(EVS_CTRL_LOCK_SRV, int (*cb)(evs_service_lockCtrl *request))
    DECLARE_SEVICE_CALLBACK(EVS_FEE_MODEL_UPDATE_SRV, int (*cb)(evs_service_issue_feeModel *request, evs_service_feedback_feeModel *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_START_CHARGE_SRV, int (*cb)(evs_service_startCharge *request, evs_service_feedback_startCharge *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_AUTH_RESULT_SRV, int (*cb)(evs_service_authCharge *request, evs_service_feedback_authCharge *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_STOP_CHARGE_SRV, int (*cb)(evs_service_stopCharge *request, evs_service_feedback_stopCharge *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_ORDER_CHECK_SRV, int (*cb)(evs_service_confirmTrade *request, void *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_RSV_CHARGE_SRV, int (*cb)(evs_service_rsvCharge *request, evs_service_feedback_rsvCharge *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_GROUND_LOCK_SRV, int (*cb)(evs_service_groundLock_ctrl *request, evs_service_feedback_groundLock_ctrl *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_GATE_LOCK_SRV, int (*cb)(evs_service_gateLock_ctrl *request, evs_service_feedback_gateLock_ctrl *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_CONF_UPDATE_SRV, int (*cb)(evs_data_dev_config *request, int *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_CONF_GET_SRV, int (*cb)(evs_data_dev_config *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_MAINTAIN_RESULT_SRV, int (*cb)(evs_service_feedback_maintain_query *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_FUN_CONF_UPDATE_SRV, int (*cb)(evs_service_dev_fun_config *request, evs_service_feedback_dev_fun_config *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_FUN_CONF_GET_SRV, int (*cb)(evs_service_get_dev_fun_config *request, evs_service_dev_fun_config *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_FEE_MODEL_QUERY_SRV, int (*cb)(evs_service_feedback_feeModel_qurey *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_BLE_SECRET_UPDATE_SRV, int (*cb)(evs_service_blesecret_update *request, evs_service_feedback_blesecret_update *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_BLE_PLUG_CHG_INFO_GET_SRV, int (*cb)(evs_event_ble_plug_charge_info *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_BLE_PLUG_CHG_INFO_CONF_SRV, int (*cb)(evs_service_ble_plug_charge_info_conf *request))
    DECLARE_SEVICE_CALLBACK(EVS_BLE_AUTH_LIST_CLEAN_SRV, int (*cb)(evs_service_blelist_clean *request, evs_service_feedback_blelist_clean *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_BLE_RESET_SRV, int (*cb)(evs_service_feedback_ble_reset *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_PILE_PARTS_CONF_SRV, int (*cb)(evs_service_config_parts *request, evs_service_feedback_config_parts *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_PILE_PARTS_CONF_GET_SRV, int (*cb)(evs_service_parts_config_get *request, evs_service_feedback_config_parts_get *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_VIN_LIST_UPDATE_SRV, int (*cb)(evs_service_vinList_update *request, evs_service_feedback_vinList_update *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_ORDER_ASK_SRV, int (*cb)(evs_service_trade_get *request, evs_service_feedback_trade_get *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_Meter_ASK_SRV, int (*cb)(evs_service_meter_get *request, evs_service_feedback_meter_get *feedback))
    DECLARE_SEVICE_CALLBACK(EVS_QUE_DATA_SRV, int (*cb)(evs_service_query_log *request, evs_service_feedback_query_log *feedback))

    /************直流功率调节控制与交流有序充电共用回调****************/
    DECLARE_SEVICE_CALLBACK(EVS_ORDERLY_CHARGE_SRV, int (*cb)(evs_service_orderCharge *request, evs_service_feedback_orderCharge *feedback))

    /*系统回调函数声明*/
    DECLARE_SEVICE_CALLBACK(EVS_OTA_UPDATE, int (*cb)(const char *))
    DECLARE_SEVICE_CALLBACK(EVS_TIME_SYNC, int (*cb)(const unsigned int))
    DECLARE_SEVICE_CALLBACK(EVS_CONNECT_SUCC, int (*cb)(void))
    DECLARE_SEVICE_CALLBACK(EVS_DISCONNECTED, int (*cb)(void))
    DECLARE_SEVICE_CALLBACK(EVS_REPORT_REPLY, int (*cb)(const int, const int, const char *, const int))
    DECLARE_SEVICE_CALLBACK(EVS_TRIGGER_EVENT_REPLY, int (*cb)(const int, const int, const char *, const int, const char *, const int))
    DECLARE_SEVICE_CALLBACK(EVS_STATE_EVERYTHING, int (*cb)(int ev, const char *msg))
    DECLARE_SEVICE_CALLBACK(EVS_CERT_GET, int (*cb)(evs_device_meta *meta))
    DECLARE_SEVICE_CALLBACK(EVS_CERT_SET, int (*cb)(const evs_device_meta meta))
    DECLARE_SEVICE_CALLBACK(EVS_DEVICE_REG_CODE_GET, int (*cb)(char *device_reg_code))
    DECLARE_SEVICE_CALLBACK(EVS_DEVICE_UID_GET, int (*cb)(char *device_uid))

#ifdef __cplusplus
}
#endif

#endif /* protocol.h */
