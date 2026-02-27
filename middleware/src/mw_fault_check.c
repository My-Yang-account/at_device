/**
 ******************************************************************************
 * @file mw_fault_check.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_fault_check.h"

/**********************************************
 * 函数名         mw_convert_to_system_stopway
 * 功能            系统停充原因转换
 * 参数             gunno    枪号
 * 返回            指定枪库停充原因
 *********************************************/
static uint16_t mw_convert_to_system_stopway(uint8_t gunno, thaisenChargeCtlStopWayEn way)
{
    switch(way){
    case thaisen_chargeCtl_stopWay_Common:
    {
        if(gunno >= APP_SYSTEM_GUNNO_SIZE){
            return way;
        }
        thaisenMsgSended_t sended = thaisenGetMsgSended(gunno);
        thaisenCommuTimeoutEnum reason = thaisenGetCommuTimeoutDetailed(gunno);
        thaisenWaitingMsgEnum waiting_msg = thaisenGetWaitingMsgDetailed(gunno);
        switch(reason){
        case THAISEN_COMMUTIMEOUT_BRM:
            return APP_SYSTEM_STOP_WAY_BRM_TIMEOUT;
        case THAISEN_COMMUTIMEOUT_BCP:
            return APP_SYSTEM_STOP_WAY_BCP_TIMEOUT;
        case THAISEN_COMMUTIMEOUT_BRO:
            return APP_SYSTEM_STOP_WAY_BRO_TIMEOUT;
        case THAISEN_COMMUTIMEOUT_BRO_AA:
            return APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT;
        case THAISEN_COMMUTIMEOUT_BCL:
            /** CCS 报文已发送，说明已经进入了充电状态 */
            if(sended.CCS){
                return APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT;
            }else{
                return APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT;
            }
        case THAISEN_COMMUTIMEOUT_BCS:
            /** CCS 报文已发送，说明已经进入了充电状态 */
            if(sended.CCS){
                return APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT;
            }else{
                return APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT;
            }
#ifdef APP_INCLUDE_V2G
        case THAISEN_COMMUTIMEOUT_BCPP:
            return APP_SYSTEM_STOP_WAY_BCPP_TIMEOUT;
        case THAISEN_COMMUTIMEOUT_BCSP:
            /** CCS 报文已发送，说明已经进入了充电状态 */
            if(sended.CCS){
                return APP_SYSTEM_STOP_WAY_CHARGEING_BCSP_TIMEOUT;
            }else{
                return APP_SYSTEM_STOP_WAY_STARTING_BCSP_TIMEOUT;
            }
#endif /* APP_INCLUDE_V2G */
        default:
            break;
        }

        switch(waiting_msg){
        case THAISEN_WAITING_MSG_BHM:
            break;
        case THAISEN_WAITING_MSG_BRM:
            return APP_SYSTEM_STOP_WAY_BRM_TIMEOUT;
        case THAISEN_WAITING_MSG_BCP:
            return APP_SYSTEM_STOP_WAY_BCP_TIMEOUT;
        case THAISEN_WAITING_MSG_BRO:
            return APP_SYSTEM_STOP_WAY_BRO_TIMEOUT;
        case THAISEN_WAITING_MSG_BRO_AA:
            return APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT;
        case THAISEN_WAITING_MSG_BCL:
            /** CCS 报文已发送，说明已经进入了充电状态 */
            if(sended.CCS){
                return APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT;
            }else{
                return APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT;
            }
        case THAISEN_WAITING_MSG_BCS:
            /** CCS 报文已发送，说明已经进入了充电状态 */
            if(sended.CCS){
                return APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT;
            }else{
                return APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT;
            }
#ifdef APP_INCLUDE_V2G
        case THAISEN_WAITING_MSG_BCPP:
            return APP_SYSTEM_STOP_WAY_BCPP_TIMEOUT;
        case THAISEN_WAITING_MSG_BCSP:
            /** CCS 报文已发送，说明已经进入了充电状态 */
            if(sended.CCS){
                return APP_SYSTEM_STOP_WAY_CHARGEING_BCSP_TIMEOUT;
            }else{
                return APP_SYSTEM_STOP_WAY_STARTING_BCSP_TIMEOUT;
            }
#endif /* APP_INCLUDE_V2G */
        default:
            break;
        }
        return thaisen_chargeCtl_stopWay_Common;
    }
    case thaisen_chargeCtl_stopWay_BST:
    {
        if(gunno >= APP_SYSTEM_GUNNO_SIZE){
            return way;
        }
        thaisenBSTDetailed_t reason = thaisenGetBSTDetailed(gunno);
        if(reason.data.SOCGetObj){
            return thaisen_chargeCtl_stopWay_BST_TargetSOC;
        }else if(reason.data.VoltGetObj){
            return thaisen_chargeCtl_stopWay_BST_TargetTotalVolt;
        }else if(reason.data.CeliVoltGetObj){
            return thaisen_chargeCtl_stopWay_BST_TargetSingleVolt;
        }else if(reason.data.ChargInitiStop){
            return thaisen_chargeCtl_stopWay_BST_ChargerEnd;
        }else if(reason.data.InsltFault){
            return thaisen_chargeCtl_stopWay_BST_InsultionFault;
        }else if(reason.data.OutConectOVtemp){
            return thaisen_chargeCtl_stopWay_BST_OutLinkerFault;
        }else if(reason.data.BMSCompOVtemp){
            return thaisen_chargeCtl_stopWay_BST_BMSElement;
        }else if(reason.data.Conectfault){
            return thaisen_chargeCtl_stopWay_BST_ChargeLinkerFault;
        }else if(reason.data.BatOVtemp){
            return thaisen_chargeCtl_stopWay_BST_BatGroupOT;
        }else if(reason.data.HVRelaysFault){
            return thaisen_chargeCtl_stopWay_BST_HV_Relay;
        }else if(reason.data.Check2Ft){
            return thaisen_chargeCtl_stopWay_BST_DetectPiont_2;
        }else if(reason.data.OverCurlt){
            return thaisen_chargeCtl_stopWay_BST_OverCurrent;
        }else if(reason.data.Voltfault){
            return thaisen_chargeCtl_stopWay_BST_AbnormalVoltage;
        }else{
            return thaisen_chargeCtl_stopWay_BST;
        }
    }
    case thaisen_chargeCtl_stopWay_BSM:
    {
        if(gunno >= APP_SYSTEM_GUNNO_SIZE){
            return way;
        }
        thaisenBSMDetailed_t reason = thaisenGetBSMDetailed(gunno);
        if(reason.data.CellOverVolt){
            return thaisen_chargeCtl_stopWay_BSM_SingleBat_OV;
        }else if(reason.data.SOCState){
            return thaisen_chargeCtl_stopWay_BSM_AbnormalSOC;
        }else if(reason.data.BatOverCurlt){
            return thaisen_chargeCtl_stopWay_BSM_OverCurrent;
        }else if(reason.data.BatOverTemp){
            return thaisen_chargeCtl_stopWay_BSM_BatteryOT;
        }else if(reason.data.Insulat){
            return thaisen_chargeCtl_stopWay_BSM_BatInsultionAbnormal;
        }else if(reason.data.OutConect){
            return thaisen_chargeCtl_stopWay_BSM_OutLinkerAbnormal;
        }else if(reason.data.AllowChg){
            return thaisen_chargeCtl_stopWay_BSM_Forbid;
        }else{
            return APP_SYSTEM_STOP_WAY_BSM;
        }
    }
    default:
        break;
    }
    return way;
}

uint32_t* mw_get_system_fault_set(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return thaisenGetSysFault(gunno);
    }
    return NULL;
}

uint8_t mw_get_system_fset_num(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return thaisenGetSysFaultSetNum(gunno);
    }
    return 0x00;
}

uint32_t* mw_get_charge_fault_set(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return thaisenGetChargFault(gunno);
    }
    return NULL;
}

uint16_t mw_get_system_stop_way(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        uint16_t stop_way;
        stop_way = thaisenGetChargCtlStopWay(gunno);
        stop_way = mw_convert_to_system_stopway(gunno, stop_way);

        rt_kprintf("gunno(%d) stop charge, reason is(%d)\n", gunno, stop_way);
        return stop_way;
    }
    return APP_SYSTEM_STOP_WAY_SIZE;
}


uint16_t mw_system_fault_convert(uint16_t code, uint8_t opt)
{
    if(opt == APP_FCONVERT_LIB_TO_APP){
        if(code >= APP_SYS_FAULT_MAX){
            return APP_SYS_FAULT_MAX;
        }

        if(code < APP_ORIGIN_SYSFAULT_MAX){
            return code;
        }else if(code < APP_SYS_FAULT_NO_ERROR){
            return (code + APP_SYSFAULT_OFFSET_MIN);
        }
        return APP_SYS_FAULT_MAX;
    }else{
        if(code < APP_ORIGIN_SYSFAULT_MAX){
            return code;
        }else if(code >= APP_USER_SYSFAULT_MIN_NEW_DEF){
            return (code - APP_SYSFAULT_OFFSET_MIN);
        }
    }

    return code;
}

/** stopway 需是直接从 mw_get_system_stop_way 中获取的 */
uint16_t mw_system_stop_way_convert(uint16_t stopway)
{
    uint16_t diff = 0x00;

    if(stopway >= APP_SYSTEM_STOP_WAY_SIZE){
        return APP_SYSTEM_STOP_WAY_SIZE;
    }

    /** 和系统故障码一样的停充原因码 */
    if(stopway < APP_ORIGIN_SYSFAULT_STOPWAY_MAX){
        return stopway;
    }else if(stopway < APP_SYS_FAULT_NO_ERROR){  /** 这里的 APP_SYS_FAULT_NO_ERROR相当于当前的因系统故障停充的停充原因最大值 */
        return (stopway + APP_SYSFAULT_STOPWAY_OFFSET);
    }

    /** 和系统故障码不一样的停充原因码 */
    if((stopway >= APP_SYS_FAULT_NO_ERROR)){  /** 这里的 APP_SYS_FAULT_NO_ERROR相当于当前的因系统故障停充的停充原因最大值 */
        if((stopway - APP_SYS_FAULT_NO_ERROR) <= (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN)){
            diff = (stopway - APP_SYS_FAULT_NO_ERROR);
            return (diff + APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN);
        }else if((stopway < APP_SYSTEM_STOP_WAY_NULL)){ /** 若 stopway == APP_SYSTEM_STOP_WAY_NULL，则返回APP_SYSTEM_STOP_WAY_SIZE */
            diff = ((stopway - APP_SYS_FAULT_NO_ERROR) - (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX + 0x01 - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN));
            return (diff + APP_NONE_SYSFAULT_STOPWAY_OFFSET + APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX);
        }else if(stopway == APP_SYSTEM_STOP_WAY_NULL){
            return (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX + 0x01);
        }
    }

    /** 自定义故障码 */
    if((stopway > APP_SYSTEM_STOP_WAY_NULL)){
        if((stopway - (APP_SYSTEM_STOP_WAY_NULL + 0x01)) <= APP_USER_STOPWAY_NUM_MAX){
            diff = (stopway - (APP_SYSTEM_STOP_WAY_NULL + 0x01));
            return (diff + APP_ORIGIN_USER_STOPWAY_MIN);
        }
    }

    return APP_SYSTEM_STOP_WAY_SIZE;
}

/**********************************************************************
 * 函数名         mw_query_bms_communicate_fault
 * 功能             查询BMS具体通讯故障
 * 参数             gunno      枪号
 * 返回             @enum communication_fault_t
 *********************************************************************/
uint16_t mw_query_bms_communicate_fault(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_SYSTEM_STOP_WAY_COMMINICATION;
    }

    thaisenMsgSended_t sended = thaisenGetMsgSended(gunno);
    thaisenCommuTimeoutEnum reason = thaisenGetCommuTimeoutDetailed(gunno);
    switch(reason){
    case THAISEN_COMMUTIMEOUT_BRM:
        return APP_SYSTEM_STOP_WAY_BRM_TIMEOUT;
    case THAISEN_COMMUTIMEOUT_BCP:
        return APP_SYSTEM_STOP_WAY_BCP_TIMEOUT;
    case THAISEN_COMMUTIMEOUT_BRO:
        return APP_SYSTEM_STOP_WAY_BRO_TIMEOUT;
    case THAISEN_COMMUTIMEOUT_BRO_AA:
        return APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT;
    case THAISEN_COMMUTIMEOUT_BCL:
        /** CCS 报文已发送，说明已经进入了充电状态 */
        if(sended.CCS){
            return APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT;
        }else{
            return APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT;
        }
    case THAISEN_COMMUTIMEOUT_BCS:
        /** CCS 报文已发送，说明已经进入了充电状态 */
        if(sended.CCS){
            return APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT;
        }else{
            return APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT;
        }
#ifdef APP_INCLUDE_V2G
    case THAISEN_COMMUTIMEOUT_BCPP:
        return APP_SYSTEM_STOP_WAY_BCPP_TIMEOUT;
    case THAISEN_COMMUTIMEOUT_BCSP:
        /** CCS 报文已发送，说明已经进入了充电状态 */
        if(sended.CCS){
            return APP_SYSTEM_STOP_WAY_CHARGEING_BCSP_TIMEOUT;
        }else{
            return APP_SYSTEM_STOP_WAY_STARTING_BCSP_TIMEOUT;
        }
#endif /* APP_INCLUDE_V2G */
    default:
        break;
    }
    return APP_SYSTEM_STOP_WAY_COMMINICATION;
}

/**********************************************************************
 * 函数名         mw_is_bms_communicate_repeat
 * 功能             判断是否与BMS进行了通讯重连
 * 参数             gunno      枪号
 * 返回             1：是     0：否
 *********************************************************************/
uint8_t mw_is_bms_communicate_repeat(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(thaisen_get_ChargeWarnningInfo(gunno) == thaisenChargeWarnCommu){
        return 0x01;
    }
    return 0x00;
}



