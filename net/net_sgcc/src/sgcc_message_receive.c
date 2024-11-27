/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */

#include "net_operation.h"

#include "sgcc_message_send.h"
#include "sgcc_message_receive.h"
#include "sgcc_message_padding.h"
#include "sgcc_device_register.h"
#include "interface.h"

#define DBG_TAG "gw_callback"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

/**=======================================[服务器请求报文]=======================================*/
///设备配置信息
evs_data_dev_config evs_data_dev_configs;
/////设备功能配置信息
//evs_service_dev_fun_config evs_service_dev_fun_configs[NET_SYSTEM_GUN_NUMBER];
/////设备部件配置信息
//evs_service_config_parts evs_service_config_partss[NET_SYSTEM_GUN_NUMBER];
/////设备部件配置查询
//evs_service_parts_config_get evs_service_parts_config_gets[NET_SYSTEM_GUN_NUMBER];
///日志查询
evs_service_query_log evs_service_query_logs[NET_SYSTEM_GUN_NUMBER];
///维护
evs_service_dev_maintain evs_service_dev_maintains[NET_SYSTEM_GUN_NUMBER];
///电子锁
evs_service_lockCtrl evs_service_lockCtrls[NET_SYSTEM_GUN_NUMBER];
///计费模型
evs_service_issue_feeModel evs_service_issue_feeModels[NET_SYSTEM_GUN_NUMBER];
///开始充电
evs_service_startCharge evs_service_startCharges[NET_SYSTEM_GUN_NUMBER];
///鉴权
evs_service_authCharge evs_service_authCharges[NET_SYSTEM_GUN_NUMBER];
///停止充电
evs_service_stopCharge evs_service_stopCharges[NET_SYSTEM_GUN_NUMBER];
/////交易记录获取
//evs_service_trade_get evs_service_trade_gets[NET_SYSTEM_GUN_NUMBER];
/////电表底值获取
//evs_service_meter_get evs_service_meter_gets[NET_SYSTEM_GUN_NUMBER];
///交易记录确认
evs_service_confirmTrade evs_service_confirmTrades[NET_SYSTEM_GUN_NUMBER];
/////VIN码白名单更新
//evs_service_vinList_update evs_service_vinList_updates[NET_SYSTEM_GUN_NUMBER];
///预约
evs_service_rsvCharge evs_service_rsvCharges[NET_SYSTEM_GUN_NUMBER];
///地锁
evs_service_groundLock_ctrl evs_service_groundLock_ctrls[NET_SYSTEM_GUN_NUMBER];
///智能门锁
evs_service_gateLock_ctrl evs_service_gateLock_ctrls[NET_SYSTEM_GUN_NUMBER];
///有序充电
evs_service_orderCharge evs_service_orderCharges[NET_SYSTEM_GUN_NUMBER];
/////设备功能配置查询
//evs_service_get_dev_fun_config evs_service_get_dev_fun_configs[NET_SYSTEM_GUN_NUMBER];
/////计费模型查询
//evs_service_feeModel_query evs_service_feeModel_querys[NET_SYSTEM_GUN_NUMBER];
/////蓝牙密钥更新
//evs_service_blesecret_update evs_service_blesecret_updates[NET_SYSTEM_GUN_NUMBER];
/////蓝牙信息
//evs_service_ble_plug_charge_info_conf evs_service_ble_plug_charge_info_confs[NET_SYSTEM_GUN_NUMBER];
/////蓝牙清空
//evs_service_blelist_clean evs_service_blelist_cleans[NET_SYSTEM_GUN_NUMBER];

/**=======================================[服务器响应报文]=======================================*/
/////设备维护指令结果
//evs_event_feedback_dev_maintain evs_event_feedback_dev_maintains[NET_SYSTEM_GUN_NUMBER];
/////电子锁控制结果
//evs_event_feedback_lockCtrl evs_event_feedback_lockCtrls[NET_SYSTEM_GUN_NUMBER];
///日志查询
evs_service_feedback_query_log evs_service_feedback_query_logs[NET_SYSTEM_GUN_NUMBER];
///设备维护状态
evs_service_feedback_maintain_query evs_service_feedback_maintain_querys[NET_SYSTEM_GUN_NUMBER];
///计费模型更新
evs_service_feedback_feeModel evs_service_feedback_feeModels[NET_SYSTEM_GUN_NUMBER];
///启动充电
evs_service_feedback_startCharge evs_service_feedback_startCharges[NET_SYSTEM_GUN_NUMBER];
///鉴权充电
evs_service_feedback_authCharge evs_service_feedback_authCharges[NET_SYSTEM_GUN_NUMBER];
///停止充电
evs_service_feedback_stopCharge evs_service_feedback_stopCharges[NET_SYSTEM_GUN_NUMBER];
/////设备部件配置参数
//evs_service_feedback_config_parts evs_service_feedback_config_partss[NET_SYSTEM_GUN_NUMBER];
/////设备部件配置获取
//evs_service_feedback_config_parts_get evs_service_feedback_config_parts_gets[NET_SYSTEM_GUN_NUMBER];
/////交易记录获取
//evs_service_feedback_trade_get evs_service_feedback_trade_gets[NET_SYSTEM_GUN_NUMBER];
/////电表底值
//evs_service_feedback_meter_get evs_service_feedback_meter_gets[NET_SYSTEM_GUN_NUMBER];
/////VIN码列表更新
//evs_service_feedback_vinList_update evs_service_feedback_vinList_updates[NET_SYSTEM_GUN_NUMBER];
///预约结果
evs_service_feedback_rsvCharge evs_service_feedback_rsvCharges[NET_SYSTEM_GUN_NUMBER];
///地锁
evs_service_feedback_groundLock_ctrl evs_service_feedback_groundLock_ctrls[NET_SYSTEM_GUN_NUMBER];
///智能门锁
evs_service_feedback_gateLock_ctrl evs_service_feedback_gateLock_ctrls[NET_SYSTEM_GUN_NUMBER];
///有序充电
evs_service_feedback_orderCharge evs_service_feedback_orderCharges[NET_SYSTEM_GUN_NUMBER];
/////设备功能配置查询
//evs_service_feedback_dev_fun_config evs_service_feedback_dev_fun_configs[NET_SYSTEM_GUN_NUMBER];
/////设备计费模型查询
//evs_service_feedback_feeModel_qurey evs_service_feedback_feeModel_qureys[NET_SYSTEM_GUN_NUMBER];
/////蓝牙密钥更新
//evs_service_feedback_blesecret_update evs_service_feedback_blesecret_updates[NET_SYSTEM_GUN_NUMBER];
/////蓝牙列表清除
//evs_service_feedback_blelist_clean evs_service_feedback_blelist_cleans[NET_SYSTEM_GUN_NUMBER];
/////蓝牙重置
//evs_service_feedback_ble_reset evs_service_feedback_ble_resets[NET_SYSTEM_GUN_NUMBER];

static uint8_t sgcc_firmware_version[NET_SGCC_FIRMWARE_VERSION_LEN];
static sgcc_recv_item s_sgcc_recv_item[NET_SGCC_RECV_MESSAGE_ITEM_MAX];

/***********************************************
 * 函数名      sgcc_get_firmware_pack_version
* 功能           获取固件包版本
 **********************************************/
uint8_t *sgcc_get_firmware_pack_version(void)
{
    return sgcc_firmware_version;
}

/***********************************************
 * 函数名      sgcc_clear_recv_message_item
* 功能           根据信息项的事件码清除指定的接收信息项
 **********************************************/
void sgcc_clear_recv_message_item(uint8_t event, uint8_t gunno, uint8_t res)
{
    uint8_t item = 0x00, index = 0x00;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    for(index = 0x00; index < NET_SGCC_RECV_MESSAGE_ITEM_MAX; index++){
        if((s_sgcc_recv_item[index].event == event) && (s_sgcc_recv_item[index].gunno == gunno) && (s_sgcc_recv_item[index].result == res)){
            item = index;
            break;
        }
    }
    if(index < NET_SGCC_RECV_MESSAGE_ITEM_MAX){
        for(item = index; item < NET_SGCC_RECV_MESSAGE_ITEM_MAX; item++){
            if(item < (NET_SGCC_RECV_MESSAGE_ITEM_MAX - 1)){
                s_sgcc_recv_item[item].event = s_sgcc_recv_item[item + 1].event;
                s_sgcc_recv_item[item].result = s_sgcc_recv_item[item + 1].result;
                s_sgcc_recv_item[item].gunno = s_sgcc_recv_item[item + 1].gunno;
            }
        }
        memset(&s_sgcc_recv_item[NET_SGCC_RECV_MESSAGE_ITEM_MAX - 1], 0x00, sizeof(s_sgcc_recv_item[NET_SGCC_RECV_MESSAGE_ITEM_MAX - 1]));
    }
}

/***********************************************
 * 函数名      sgcc_get_recv_message_item_result
* 功能           根据信息项的指令码获取信息项的处理结果
 **********************************************/
int16_t sgcc_get_recv_message_item_result(uint8_t event, uint8_t gunno, uint8_t *res)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    for(uint8_t index = 0; index < NET_SGCC_RECV_MESSAGE_ITEM_MAX; index++){
        if((s_sgcc_recv_item[index].event == event) && (s_sgcc_recv_item[index].gunno == gunno)){
            if(res){
                *res = s_sgcc_recv_item[index].result;
            }
            return 0x00;
        }
    }
    return -0x01;
}

/***********************************************
 * 函数名      sgcc_input_recv_message_item
* 功能           向接收信息队列中插入一个信息项
 **********************************************/
static int8_t sgcc_input_recv_message_item(uint8_t event, uint8_t res, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    for(uint8_t index = 0x00; index < NET_SGCC_RECV_MESSAGE_ITEM_MAX; index++){
        if(s_sgcc_recv_item[index].event == 0x00){
            s_sgcc_recv_item[index].event = event;
            s_sgcc_recv_item[index].result = res;
            s_sgcc_recv_item[index].gunno = gunno;
            return 0x00;
        }
    }
    return -0x01;
}

static int callback_evs_service_device_maintain(evs_service_dev_maintain *request, evs_service_feedback_dev_maintain *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }

    LOG_D("sgcc device_maintain-> ctrlType: %d", request->ctrlType);

    int result = 0x00;

    feedback->ctrlType = request->ctrlType;
    feedback->reason = sgcc_message_pro_dev_maintain_request(request, sizeof(evs_service_dev_maintain));

    return 0;
}

static int callback_evs_service_ctrl_elock(evs_service_lockCtrl *request, evs_service_feedback_lockCtrl *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }

    int8_t result = 0x00;
    uint8_t gunno = request->gunNo;

    feedback->gunNo = gunno;
    feedback->lockStatus = SGCC_OPSCTL_ACTION;
    feedback->resCode = SGCC_ELOCK_CTRL_SUCCESS;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        feedback->resCode = SGCC_ELOCK_CTRL_NO_ELOCK;
        feedback->lockStatus = SGCC_OPSCTL_SILENT;
        LOG_E("gunno error when call callback_evs_service_ctrl_elock(%d)", gunno);
    }

    if(feedback->resCode == SGCC_ELOCK_CTRL_SUCCESS){
        result = sgcc_message_pro_ctrl_elock_request((gunno - 0x01), request, sizeof(evs_service_lockCtrl));
        if(result >= 0x00){
            if(result == 0x01){
                feedback->resCode = SGCC_ELOCK_CTRL_FAIL_FORBID_UNLOCK;
                if(request->lockParam == SGCC_OPSCTL_ACTION){
                    feedback->resCode = SGCC_ELOCK_CTRL_FAIL_FORBID_LOCK;
                }
            }else if(result == 0x02){
                feedback->lockStatus = SGCC_OPSCTL_SILENT;
            }else if(result == 0x03){
                feedback->lockStatus = SGCC_OPSCTL_ACTION;
            }else{
                feedback->lockStatus = SGCC_OPSCTL_SILENT;
                if(request->lockParam == SGCC_OPSCTL_ACTION){
                    feedback->lockStatus = SGCC_OPSCTL_ACTION;
                }
            }
        }
    }

    return 0;
}

static int callback_service_EVS_FEE_MODEL_UPDATE_SRV(evs_service_issue_feeModel *request, evs_service_feedback_feeModel *feedback)
{
    if(request == NULL){
        return -0x01;
    }

    LOG_D("sgcc remote eleModelId-> no: %s", request->eleModelId);
    LOG_D("sgcc remote serModelId-> no: %s", request->serModelId);

    feedback->result = SGCC_OPSCTL_NONE;
    memset(feedback->eleModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
    memcpy(feedback->eleModelId, request->eleModelId, EVS_MAX_MODEL_ID_LEN);

    memset(feedback->serModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
    memcpy(feedback->serModelId, request->serModelId, EVS_MAX_MODEL_ID_LEN);

    if(sgcc_message_pro_billing_model_request_response(request, sizeof(evs_service_issue_feeModel), 0x00) >= 0x00){
        feedback->result = SGCC_OPSCTL_ACTION;
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_SREQ_EVENT_BILLING_MODEL_SET);
        }
    }

    return 0;
}

/*****************************************************************
 * 函数名                   callback_service_EVS_START_CHARGE_SRV
 * 功能                       处理远程启动充电请求
 *           request       请求数据
 *           feedback      响应数据
 * 返回                        无
 ****************************************************************/
static int callback_service_EVS_START_CHARGE_SRV(evs_service_startCharge *request, evs_service_feedback_startCharge *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc remote start charge-> no: %d", request->gunNo);
    LOG_D("sgcc remote startType-> no: %d", request->startType);
    LOG_D("sgcc remote chargeMode-> no: %d", request->chargeMode);
    LOG_D("sgcc remote limitData-> no: %d", request->limitData);
    LOG_D("sgcc remote stopCode-> no: %d", request->stopCode);
    LOG_D("sgcc remote startMode-> no: %d", request->startMode);
    LOG_D("sgcc remote insertGunTime-> no: %d", request->insertGunTime);
    for(uint8_t count = 0x00; count < sizeof(evs_service_startCharge); count++){
        rt_kprintf("%02X ", *((uint8_t*)(request) + count));
        if((count %16 == 0x00) && count != 0x00){
            rt_kprintf("\n");
        }
    }
    rt_kprintf("\n");

    uint8_t gunno = request->gunNo;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        sgcc_chargepile_create_local_transaction_number(0x00, request->tradeNo, EVS_MAX_TRADE_LEN);

        sgcc_input_recv_message_item(EVS_START_CHARGE_SRV, 0x01, 0x00);
        memcpy(&(evs_service_startCharges[0x00]), request, sizeof(evs_service_startCharge));

        memset(feedback, 0x00, sizeof(evs_service_feedback_startCharge));
        feedback->gunNo = request->gunNo;
        memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
        memcpy(feedback->tradeNo, request->tradeNo, EVS_MAX_TRADE_LEN);

        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_SREQ_EVENT_START_CHARGE);

        LOG_E("gunno error when call callback_service_EVS_START_CHARGE_SRV(%d)", gunno);
    }

    sgcc_chargepile_create_local_transaction_number((gunno - 0x01), request->tradeNo, EVS_MAX_TRADE_LEN);

    sgcc_input_recv_message_item(EVS_START_CHARGE_SRV, 0x00, (gunno - 0x01));
    memcpy(&(evs_service_startCharges[(gunno - 0x01)]), request, sizeof(evs_service_startCharge));

    memset(feedback, 0x00, sizeof(evs_service_feedback_startCharge));
    feedback->gunNo = request->gunNo;
    memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
    memcpy(feedback->tradeNo, request->tradeNo, EVS_MAX_TRADE_LEN);

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_SGCC_SREQ_EVENT_START_CHARGE);

    return 0x00;
}

static int callback_service_EVS_AUTH_RESULT_SRV(evs_service_authCharge *request, evs_service_feedback_authCharge *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc authenty start charge-> no: %d", request->gunNo);
    LOG_D("sgcc authenty chargeMode-> no: %d", request->chargeMode);
    LOG_D("sgcc authenty result-> no: %d", request->result);
    LOG_D("sgcc authenty limitData-> no: %d", request->limitData);
    LOG_D("sgcc authenty stopCode-> no: %d", request->stopCode);
    LOG_D("sgcc authenty startMode-> no: %d", request->startMode);
    LOG_D("sgcc authenty insertGunTime-> no: %d", request->insertGunTime);

    uint8_t gunno = request->gunNo;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        sgcc_input_recv_message_item(EVS_AUTH_RESULT_SRV, 0x01, 0x00);
        memcpy(&(evs_service_authCharges[0x00]), request, sizeof(evs_service_authCharge));

        memset(feedback, 0x00, sizeof(evs_service_feedback_authCharge));
        feedback->gunNo = request->gunNo;
        memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
        memcpy(feedback->tradeNo, evs_event_startCharges[0x00].tradeNo, EVS_MAX_TRADE_LEN);

        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, 0x00, NET_SGCC_SRES_EVENT_APPLY_CHARGE_ACTIVE);

        LOG_E("gunno error when call callback_service_EVS_AUTH_RESULT_SRV(%d)", gunno);
    }

    sgcc_input_recv_message_item(EVS_AUTH_RESULT_SRV, 0x00, (gunno - 0x01));
    memcpy(&(evs_service_authCharges[(gunno - 0x01)]), request, sizeof(evs_service_authCharge));
    memcpy(evs_service_authCharges[(gunno - 0x01)].tradeNo, evs_event_startCharges[gunno - 0x01].tradeNo, EVS_MAX_TRADE_LEN);

    memset(feedback, 0x00, sizeof(evs_service_feedback_authCharge));
    feedback->gunNo = request->gunNo;
    memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
    memcpy(feedback->tradeNo, evs_event_startCharges[gunno - 0x01].tradeNo, EVS_MAX_TRADE_LEN);

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_SGCC_SRES_EVENT_APPLY_CHARGE_ACTIVE);

    return 0;
}

/*****************************************************************
 * 函数名                   callback_service_EVS_STOP_CHARGE_SRV
 * 功能                       处理远程停止充电请求
 *           request       请求数据
 *           feedback      响应数据
 * 返回                        无
 ****************************************************************/
static int callback_service_EVS_STOP_CHARGE_SRV(evs_service_stopCharge *request, evs_service_feedback_stopCharge *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc remote stop charge-> no: %d", request->gunNo);

    uint8_t gunno = request->gunNo;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        memcpy(request->tradeNo, evs_service_startCharges[0x00].tradeNo, EVS_MAX_TRADE_LEN);
        sgcc_input_recv_message_item(EVS_STOP_CHARGE_SRV, 0x01, 0x00);
        memcpy(&(evs_service_stopCharges[0x00]), request, sizeof(evs_service_stopCharge));

        memset(feedback, 0x00, sizeof(evs_service_feedback_stopCharge));
        feedback->gunNo = request->gunNo;
        memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
        memcpy(feedback->tradeNo, request->tradeNo, EVS_MAX_TRADE_LEN);

        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_SREQ_EVENT_STOP_CHARGE);

        LOG_E("gunno error when call callback_service_EVS_STOP_CHARGE_SRV(%d)", gunno);
    }

    memcpy(request->tradeNo, evs_service_startCharges[(gunno - 0x01)].tradeNo, EVS_MAX_TRADE_LEN);

    sgcc_input_recv_message_item(EVS_STOP_CHARGE_SRV, 0x00, (gunno - 0x01));
    memcpy(&(evs_service_stopCharges[(gunno - 0x01)]), request, sizeof(evs_service_stopCharge));

    memset(feedback, 0x00, sizeof(evs_service_feedback_stopCharge));
    feedback->gunNo = request->gunNo;
    memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
    memcpy(feedback->tradeNo, request->tradeNo, EVS_MAX_TRADE_LEN);

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_SGCC_SREQ_EVENT_STOP_CHARGE);

    return 0x00;
}

static int callback_service_EVS_ORDER_CHECK_SRV(evs_service_confirmTrade *response, void *feedback)
{
    if(response == NULL){
        return -0x01;
    }

    uint8_t gunno = response->gunNo;
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("gunno error(%d) when call callback_service_EVS_ORDER_CHECK_SRV\n", gunno);
        return -0x01;
    }
    if(response->errcode != NET_SGCC_ORDER_VERIFY_RESULT_SUCCESS){
        LOG_E("gunno(%d) order verify fail(%d)\n", gunno, response->errcode);
        return -0x01;
    }

    LOG_D("gunno(%d) verify order sn[%s][%s]\n", (gunno - 0x01), response->preTradeNo, response->tradeNo);

    sgcc_clear_message_wait_response_state((gunno - 0x01), NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_SGCC_SRES_EVENT_BILL_VERIFY);

    return 0x00;
}

static int callback_evs_service_reservation_charge(evs_service_rsvCharge *request, evs_service_feedback_rsvCharge *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc reservation_charge-> gunno: %d", request->gunNo);

    uint8_t gunno = request->gunNo;

    feedback->gunNo = gunno;
    feedback->appomathod = request->appomathod;
    feedback->ret = 10;
    feedback->reason = 10;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        feedback->ret = 12;
        feedback->reason = 12;
    }else{
        if(sgcc_message_pro_reservation_request((gunno - 0x01), request, sizeof(evs_service_rsvCharge)) < 0x00){
            feedback->ret = 12;
            feedback->reason = 12;
        }
    }

    return 0;
}

static int callback_service_EVS_GROUND_LOCK_SRV(evs_service_groundLock_ctrl *request, evs_service_feedback_groundLock_ctrl *feedback)
{
    return 0;
}

static int callback_service_EVS_GATE_LOCK_SRV(evs_service_gateLock_ctrl *request, evs_service_feedback_gateLock_ctrl *feedback)
{
    return 0;
}

static int callback_service_EVS_CONF_UPDATE_SRV(evs_data_dev_config *request, int *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc system config update");

    *feedback = SGCC_OPSCTL_ACTION;
    if(sgcc_message_pro_config_update_request((uint8_t*)request, sizeof(evs_data_dev_config)) < 0x00){
        *feedback = SGCC_OPSCTL_SILENT;
    }
    return 0;
}

static int callback_service_EVS_CONF_GET_SRV(evs_data_dev_config *feedback)
{
    if(feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc system config query");

    sgcc_message_pro_config_query_request((uint8_t*)feedback, sizeof(evs_data_dev_config), NULL);
    return 0;
}

static int callback_evs_service_query_dev_maintain_info(evs_service_feedback_maintain_query *feedback)
{
    if(feedback == NULL){
        return -0x01;
    }

    LOG_D("sgcc query_dev_maintain_info");

    if(sgcc_message_pro_query_maintain_info_request(feedback, sizeof(evs_service_feedback_maintain_query), NULL) < 0x00){
        feedback->ctrlType = SGCC_DEV_STATE_COMMISSIONING;
        feedback->result = 11;
    }

    return 0;
}
#if 0
static int callback_service_EVS_FUN_CONF_UPDATE_SRV(evs_service_dev_fun_config *request, evs_service_feedback_dev_fun_config *feedback)
{
    return 0;
}

static int callback_service_EVS_FUN_CONF_GET_SRV(evs_service_get_dev_fun_config *request, evs_service_dev_fun_config *feedback)
{
    return 0;
}

static int callback_service_EVS_FEE_MODEL_QUERY_SRV(evs_service_feedback_feeModel_qurey *feedback)
{
    return 0;
}

static int callback_service_EVS_BLE_SECRET_UPDATE_SRV(evs_service_blesecret_update *request, evs_service_feedback_blesecret_update *feedback)
{
    return 0;
}

static int callback_service_EVS_BLE_PLUG_CHG_INFO_GET_SRV(evs_event_ble_plug_charge_info *feedback)
{
    return 0;
}

static int callback_service_EVS_BLE_PLUG_CHG_INFO_CONF_SRV(evs_service_ble_plug_charge_info_conf *request)
{
    return 0;
}

static int callback_service_EVS_BLE_AUTH_LIST_CLEAN_SRV(evs_service_blelist_clean *request, evs_service_feedback_blelist_clean *feedback)
{
    return 0;
}

static int callback_service_EVS_BLE_RESET_SRV(evs_service_feedback_ble_reset *feedback)
{
    return 0;
}

static int callback_service_EVS_PILE_PARTS_CONF_SRV(evs_service_config_parts *request, evs_service_feedback_config_parts *feedback)
{
    return 0;
}

static int callback_service_EVS_PILE_PARTS_CONF_GET_SRV(evs_service_parts_config_get *request, evs_service_feedback_config_parts_get *feedback)
{
    return 0;
}

static int callback_service_EVS_VIN_LIST_UPDATE_SRV(evs_service_vinList_update *request, evs_service_feedback_vinList_update *feedback)
{
    return 0;
}

static int callback_service_EVS_ORDER_ASK_SRV(evs_service_trade_get *request, evs_service_feedback_trade_get *feedback)
{
    return 0;
}

static int callback_service_EVS_Meter_ASK_SRV(evs_service_meter_get *request, evs_service_feedback_meter_get *feedback)
{
    return 0;
}
#endif

static int callback_evs_service_query_record(evs_service_query_log *request, evs_service_feedback_query_log *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc query_record-> gunNo: %d", request->gunNo);
    LOG_D("sgcc query_record-> startDate: %d", request->startDate);
    LOG_D("sgcc query_record-> stopDate: %d", request->stopDate);
    LOG_D("sgcc query_record-> askType: %d", request->askType);
    LOG_D("sgcc query_record-> logQueryNo: %s", request->logQueryNo);

    int result = 0x00;
    uint8_t gunno = request->gunNo;

    feedback->gunNo = request->gunNo;
    feedback->startDate = request->startDate;
    feedback->stopDate = request->stopDate;
    feedback->askType = request->askType;
    feedback->result = 11;
    memcpy(feedback->logQueryNo, request->logQueryNo, EVS_MAX_LOGQUERY_LEN);

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        feedback->result = 10;
        LOG_E("gunno error when call callback_evs_service_query_record(%d)", gunno);
        return -0x01;
    }
    if((result = sgcc_message_pro_query_dev_record_request((gunno - 0x01), request, sizeof(evs_service_query_log))) < 0x00){
        feedback->result = 10;
        if(result == -0x03){
            feedback->result = 12;
        }
    }

    memcpy(&evs_service_query_logs[gunno - 0x01], request, sizeof(evs_service_query_log));
    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_SGCC_SREQ_EVENT_QUERY_DEV_RECORD);

    return 0;
}

static int callback_evs_service_orderly_charge(evs_service_orderCharge *request, evs_service_feedback_orderCharge *feedback)
{
    if(request == NULL || feedback == NULL){
        return -0x01;
    }
    LOG_D("sgcc orderly_charge-> preTradeNo: %s", request->preTradeNo);
    LOG_D("sgcc orderly_charge-> num: %s", request->num);
    LOG_D("sgcc orderly_charge-> validTime0: %s", request->validTime[0]);
    LOG_D("sgcc orderly_charge-> validTime1: %s", request->validTime[1]);
    LOG_D("sgcc orderly_charge-> validTime2: %s", request->validTime[2]);
    LOG_D("sgcc orderly_charge-> validTime3: %s", request->validTime[3]);
    LOG_D("sgcc orderly_charge-> validTime4: %s", request->validTime[4]);
    LOG_D("sgcc orderly_charge-> kw: %d, %d, %d, %d, %d", request->kw[0], request->kw[1], request->kw[2], request->kw[3], request->kw[4]);

    int result = 0x00;
    memcpy(feedback->preTradeNo, request->preTradeNo, EVS_MAX_TRADE_LEN);
    feedback->result = 10;
    feedback->reason = 10;
    if((result = sgcc_message_pro_orderly_charge_request(request, sizeof(evs_service_orderCharge))) < 0x00){
        feedback->result = 11;
        feedback->reason = 11;
        if(result == -0x04){
            feedback->reason = 12;
        }
    }

    return 0;
}

static int callback_service_EVS_OTA_UPDATE(const char *request)
{
    LOG_D("sgcc firmware pack version(%s)\n", request);

    uint8_t valid_len = strlen(request);
    valid_len = valid_len > NET_SGCC_FIRMWARE_VERSION_LEN ? NET_SGCC_FIRMWARE_VERSION_LEN : valid_len;

    memset(sgcc_firmware_version, 0x00, NET_SGCC_FIRMWARE_VERSION_LEN);
    memcpy(sgcc_firmware_version, request, valid_len);

    if(net_get_ota_info()->state != NET_OTA_STATE_NULL){
        LOG_E("sgcc system is updating, quit");
        return -0x01;
    }

    net_get_ota_info()->state = NET_OTA_STATE_UPDATING;

//    sgcc_set_ota_was_requested_flag();

    extern int evs_linkkit_fota(unsigned char *buffer, int buffer_length);
    evs_linkkit_fota(sgcc_get_ota_buff(), sgcc_get_ota_blen());

    net_get_ota_info()->state = NET_OTA_STATE_SUCCESS;
//    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_SREQ_EVENT_FIRMWARE_UPDATE);

    return 0;
}

static int callback_evs_service_time_sync(const unsigned int request)
{
    if(sgcc_message_pro_time_sync_request((void*)&request, sizeof(request)) >= 0x00){
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_SREQ_EVENT_TIME_SYNC);
    }
    return 0;
}

static int callback_service_EVS_CONNECT_SUCC(void)
{
    extern int _get_mqtt_socketfd(void);
    sgcc_get_socket_info()->fd = _get_mqtt_socketfd();

    LOG_D("sgcc server connected, socket fd(%d)", sgcc_get_socket_info()->fd);
    return 0;
}

static int callback_service_EVS_DISCONNECTED(void)
{
    return 0;
}

static int callback_service_EVS_REPORT_REPLY(const int msgid, const int code, const char *reply, const int reply_len)
{
    return 0;
}

static int callback_service_EVS_TRIGGER_EVENT_REPLY(const int msgid, const int code, const char *eventid, const int eventid_len, const char *message, const int message_len)
{
    rt_kprintf(">>>>>>>>>>>>>nnn\n");
    return 0;
}

static int callback_service_EVS_STATE_EVERYTHING(int ev, const char *msg)
{
    return 0;
}

int sgcc_message_recvive_init(void)
{
    EVS_RegisterCallback(EVS_DEV_MAINTAIN_SRV, callback_evs_service_device_maintain);
    EVS_RegisterCallback(EVS_CTRL_LOCK_SRV, callback_evs_service_ctrl_elock);
    EVS_RegisterCallback(EVS_FEE_MODEL_UPDATA_SRV, callback_service_EVS_FEE_MODEL_UPDATE_SRV);
    EVS_RegisterCallback(EVS_START_CHARGE_SRV, callback_service_EVS_START_CHARGE_SRV);
    EVS_RegisterCallback(EVS_AUTH_RESULT_SRV, callback_service_EVS_AUTH_RESULT_SRV);
    EVS_RegisterCallback(EVS_STOP_CHARGE_SRV, callback_service_EVS_STOP_CHARGE_SRV);
    EVS_RegisterCallback(EVS_ORDER_CHECK_SRV, callback_service_EVS_ORDER_CHECK_SRV);
    EVS_RegisterCallback(EVS_RSV_CHARGE_SRV, callback_evs_service_reservation_charge);
    EVS_RegisterCallback(EVS_GROUND_LOCK_SRV, callback_service_EVS_GROUND_LOCK_SRV);
    EVS_RegisterCallback(EVS_GATE_LOCK_SRV, callback_service_EVS_GATE_LOCK_SRV);
    EVS_RegisterCallback(EVS_CONF_UPDATE_SRV, callback_service_EVS_CONF_UPDATE_SRV);
    EVS_RegisterCallback(EVS_CONF_GET_SRV, callback_service_EVS_CONF_GET_SRV);
    EVS_RegisterCallback(EVS_MAINTAIN_RESULT_SRV, callback_evs_service_query_dev_maintain_info);
//    EVS_RegisterCallback(EVS_FUN_CONF_UPDATE_SRV, callback_service_EVS_FUN_CONF_UPDATE_SRV);
//    EVS_RegisterCallback(EVS_FUN_CONF_GET_SRV, callback_service_EVS_FUN_CONF_GET_SRV);
//    EVS_RegisterCallback(EVS_FEE_MODEL_QUERY_SRV, callback_service_EVS_FEE_MODEL_QUERY_SRV);
//    EVS_RegisterCallback(EVS_BLE_SECRET_UPDATE_SRV, callback_service_EVS_BLE_SECRET_UPDATE_SRV);
//    EVS_RegisterCallback(EVS_BLE_PLUG_CHG_INFO_GET_SRV, callback_service_EVS_BLE_PLUG_CHG_INFO_GET_SRV);
//    EVS_RegisterCallback(EVS_BLE_PLUG_CHG_INFO_CONF_SRV, callback_service_EVS_BLE_PLUG_CHG_INFO_CONF_SRV);
//    EVS_RegisterCallback(EVS_BLE_AUTH_LIST_CLEAN_SRV, callback_service_EVS_BLE_AUTH_LIST_CLEAN_SRV);
//    EVS_RegisterCallback(EVS_BLE_RESET_SRV, callback_service_EVS_BLE_RESET_SRV);
//    EVS_RegisterCallback(EVS_PILE_PARTS_CONF_SRV, callback_service_EVS_PILE_PARTS_CONF_SRV);
//    EVS_RegisterCallback(EVS_PILE_PARTS_CONF_GET_SRV, callback_service_EVS_PILE_PARTS_CONF_GET_SRV);
//    EVS_RegisterCallback(EVS_VIN_LIST_UPDATE_SRV, callback_service_EVS_VIN_LIST_UPDATE_SRV);
//    EVS_RegisterCallback(EVS_ORDER_ASK_SRV, callback_service_EVS_ORDER_ASK_SRV);
//    EVS_RegisterCallback(EVS_Meter_ASK_SRV, callback_service_EVS_Meter_ASK_SRV);
    EVS_RegisterCallback(EVS_QUE_DATA_SRV, callback_evs_service_query_record);
    EVS_RegisterCallback(EVS_ORDERLY_CHARGE_SRV, callback_evs_service_orderly_charge);
    EVS_RegisterCallback(EVS_OTA_UPDATE, callback_service_EVS_OTA_UPDATE);
    EVS_RegisterCallback(EVS_TIME_SYNC, callback_evs_service_time_sync);
    EVS_RegisterCallback(EVS_CONNECT_SUCC, callback_service_EVS_CONNECT_SUCC);
    EVS_RegisterCallback(EVS_DISCONNECTED, callback_service_EVS_DISCONNECTED);
    EVS_RegisterCallback(EVS_REPORT_REPLY, callback_service_EVS_REPORT_REPLY);
    EVS_RegisterCallback(EVS_TRIGGER_EVENT_REPLY, callback_service_EVS_TRIGGER_EVENT_REPLY);
    EVS_RegisterCallback(EVS_STATE_EVERYTHING, callback_service_EVS_STATE_EVERYTHING);

    return 0x00;
}

#endif /* NET_PACK_USING_SGCC */
