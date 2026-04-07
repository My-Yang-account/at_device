/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-02-06     31638       the first version
 */
#include "thaisenChargModuleLib.h"
#include "chargepile_config.h"
#include "app_data_info_interface.h"
#include "app_module.h"

/********************************* 屏幕上矩阵继电器控制布局  *********************************/
/**
 *      ---------      ---------      ---------
 *      |KPN_1_1|      |KPN_2_1|      |KPN_3_1|
 *      ---------      ---------      ---------
 *      ---------      ---------      ---------
 *      |KPN_1_2|      |KPN_2_2|      |KPN_3_2|
 *      ---------      ---------      ---------
 *      ---------      ---------      ---------
 *      |KPN_1_3|      |KPN_2_3|      |KPN_3_3|
 *      ---------      ---------      ---------
 *
 * 只有子母机(环矩)、子母机(半矩)需要继电器矩阵
 * 其中：
 * 1.子母机(环矩)排布以及使用的继电器：
 *      ---------        ---------       ---------
 *      |KPN_1_1|(母联1)      |KPN_2_1|(母联2)      |KPN_3_1|(母联3)
 *      ---------        ---------       ---------
 *                     ---------
 *                     |KPN_2_2|(母联4)
 *                     ---------
 * 2.子母机(半矩)排布以及使用的继电器：
 *      ---------
 *      |KPN_1_1|(母联1)
 *      ---------
 *      ---------       ---------
 *      |KPN_1_2|(母联2)      |KPN_2_2|(母联1)
 *      ---------       ---------
 *      ---------       ---------        ---------
 *      |KPN_1_3|(母联3)      |KPN_2_3|(母联1)      |KPN_3_3|(母联1)
 *      ---------       ---------        ---------
 */

/************************ 全矩/半矩继电器排布 SN ************************/
/**
 *       1          2          3
 *
 *
 *
 *       2          4
 *
 *
 *
 *       3          5          6
 */

#define MCTRL_CYCLE_MATRIX_MODULE_NUM              4               /** 半矩/环矩模块组数 */

#define MCTRL_RELEASE_RELAY_VOLTAGE_MAX            600             /** 满足断开继电器条件的最大电压(0.1V) */
#define MCTRL_RELEASE_RELAY_CURRENT_MAX            200             /** 满足断开继电器条件的最大电流(0.01A) */

typedef enum{
    MCTRL_RELAY_STATUS_RELEASE,                 /** 继电器状态：释放(断开) */                    //!< MCTRL_RELAY_STATUS_RELEASE
    MCTRL_RELAY_STATUS_ACTION,                  /** 继电器状态：动作(闭合) */                    //!< MCTRL_RELAY_STATUS_ACTION
    MCTRL_RELAY_STATUS_STICK,                   /** 继电器状态：粘连 */                        //!< MCTRL_RELAY_STATUS_STICK
    MCTRL_RELAY_STATUS_REJECT,                  /** 继电器状态：拒动 */                        //!< MCTRL_RELAY_STATUS_REJECT
    MCTRL_RELAY_STATUS_NONE,                    /** 继电器状态：半矩直连时返回(在逻辑判断中会认为该状态为正常状态) *///!< MCTRL_RELAY_STATUS_NONE
}mctrl_relay_status;

typedef enum{
    MCTRL_RELAY_TYPE_POSITIVE,                  /** 继电器类型：DC+ */
    MCTRL_RELAY_TYPE_NEGTIVE,                   /** 继电器类型：DC- */
    MCTRL_RELAY_TYPE_SIZE,                      /** 继电器类型 */
}mctrl_relay_type_t;

#ifdef CP_USING_CYCLE_MATRIX
#pragma pack(1)

typedef struct{
    unsigned char type;                         /** 模块排布类型 */
    unsigned char group_num;                    /** 模块组数 */
}module_ctrl_info_t;

/** 辅助信息 */
struct assistant{
    struct{
        unsigned char is_debug_started : 1;                        /** 已进行调试强制启动 */
        unsigned char is_debug_stoped : 1;                         /** 已进行调试强制停止 */
        unsigned char reserve : 5;                                 /** 预留 */
    }flag;
};

#pragma pack()

static powerctrl_init_t s_module_ctrl_base_info;
static module_ctrl_info_t s_module_ctrl_info;
static struct assistant s_module_ctrl_assistant_info[MCTRL_CYCLE_MATRIX_MODULE_NUM];
#endif /* CP_USING_CYCLE_MATRIX */

/***************************************************** 辅组函数 *****************************************************/
/**********************************************************************************************
 * 函数名              app_module_get_index_sn_to_mrelay
 * 功能                 根据继电器编号获取矩阵继电器控制下标
 * 参数                 sn         继电器编号
 *          dev_type    设备类型
 * 返回                矩阵继电器控制下标@thaisenMatrixRelay_t
 *********************************************************************************************/
static unsigned char app_module_get_index_sn_to_mrelay(unsigned char sn, unsigned char dev_type)
{
    unsigned char index = THAISEN_MATRIX_RELAY_SIZE;
#ifdef CP_USING_CYCLE_MATRIX
    switch(sn){
    case 1:
        index = THAISEN_MATRIX_RELAY_KPN1_1;
        break;
    case 2:
        index = THAISEN_MATRIX_RELAY_KPN1_2;
        break;
    case 3:
        index = THAISEN_MATRIX_RELAY_KPN1_3;
        break;
    case 4:
        index = THAISEN_MATRIX_RELAY_KPN2_1;
        break;
    case 5:
        if(dev_type == SYSTEM_FUNCTION_MS_MACHINE_HALF){
            index = THAISEN_MATRIX_RELAY_KPN2_2;
        }
        break;
    case 6:
        if(dev_type == SYSTEM_FUNCTION_MS_MACHINE_HALF){
            index = THAISEN_MATRIX_RELAY_KPN3_1;
        }
        break;
    default:
        break;
    }
#endif /* CP_USING_CYCLE_MATRIX */
    return index;
}

/**********************************************************************************************
 * 函数名              app_module_get_index_mrelay_to_sn
 * 功能                 根据矩阵继电器控制下标获取继电器编号
 * 参数                 relay_port  矩阵继电器控制下标
 *          dev_type    设备类型
 * 返回                继电器编号
 *********************************************************************************************/
static unsigned char app_module_get_index_mrelay_to_sn(unsigned char relay_port, unsigned char dev_type)
{
    unsigned char index = 0xFF;
#ifdef CP_USING_CYCLE_MATRIX
    switch(relay_port){
    case THAISEN_MATRIX_RELAY_KPN1_1:
        index = 1;
        break;
    case THAISEN_MATRIX_RELAY_KPN1_2:
        index = 2;
        break;
    case THAISEN_MATRIX_RELAY_KPN1_3:
        index = 3;
        break;
    case THAISEN_MATRIX_RELAY_KPN2_1:
        index = 4;
        break;
    case THAISEN_MATRIX_RELAY_KPN2_2:
        if(dev_type == SYSTEM_FUNCTION_MS_MACHINE_HALF){
            index = 5;
        }
        break;
    case THAISEN_MATRIX_RELAY_KPN3_1:
        if(dev_type == SYSTEM_FUNCTION_MS_MACHINE_HALF){
            index = 6;
        }
        break;
    default:
        break;
    }
#endif /* CP_USING_CYCLE_MATRIX */
    return index;
}

/***************************************************** 功能函数 *****************************************************/
/*********************************************************************************************
 * 函数名      app_module_relay_check_enable
 * 功能          模块矩阵继电器故障检测使能
 * 参数          relay_port      继电器口@enum udrv_relay_port
 *         state           状态(1:使能     0:不使能)
 * 返回
 ********************************************************************************************/
void app_module_relay_check_enable(unsigned char relay_port, unsigned char state)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
//    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
//        return;
//    }
    /** 无此继电器 */
    if(relay_port >= THAISEN_MATRIX_RELAY_SIZE){
        return;
    }
    unsigned char sn = 0xFF;

    state = state == 0x00 ? 0x00 : 0x01;
    /** 获取继电器编号 */
    sn = app_module_get_index_mrelay_to_sn(relay_port, s_module_ctrl_info.type);
    /** 继电器编号有效 */
    if(sn != 0xFF){
        /** 使能故障检测 */
        if(state){
            for(unsigned char i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                thaisenSetSysFaultCheckBit((thaisenFaultMatrixRelay_KPN1_1 + relay_port), i);
            }
        }
        /** 禁止故障检测 */
        else{
            for(unsigned char i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                thaisenClearSysFaultCheckBit((thaisenFaultMatrixRelay_KPN1_1 + relay_port), i);
            }
        }
        thaisen_relay_set_feedbackenbale(sn, MCTRL_RELAY_TYPE_POSITIVE, state);
        thaisen_relay_set_feedbackenbale(sn, MCTRL_RELAY_TYPE_NEGTIVE, state);
    }
#endif /* CP_USING_CYCLE_MATRIX */
}


/*********************************************************************************************
 * 函数名      app_module_relay_fb_reversal
 * 功能          模块矩阵继电器反馈取反
 * 参数          relay_port      继电器口@enum udrv_relay_port
 *         state           状态(1:取反     0:不取反)
 * 返回
 ********************************************************************************************/
void app_module_relay_fb_reversal(unsigned char relay_port, unsigned char state)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        return;
    }
    /** 无此继电器 */
    if(relay_port >= THAISEN_MATRIX_RELAY_SIZE){
        return;
    }
    unsigned char sn = 0xFF;

    state = state == 0x00 ? 0x00 : 0x01;
    /** 获取继电器编号 */
    sn = app_module_get_index_mrelay_to_sn(relay_port, s_module_ctrl_info.type);
    /** 继电器编号有效 */
    if(sn != 0xFF){
        /** 反馈状态取反 */
        thaisenSetMatrixRelay_Status(relay_port, state);

        thaisen_relay_set_feedbackinvert(sn, MCTRL_RELAY_TYPE_POSITIVE, state);
        thaisen_relay_set_feedbackinvert(sn, MCTRL_RELAY_TYPE_NEGTIVE, state);
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_set_module_current_min
 * 功能                设置模块最小输出电流(0.01A)
 * 参数                current     电流值
 * 返回
 ****************************************/
void app_module_set_module_current_min(unsigned short current)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        return;
    }
    /** 单个模块最小电流 */
    s_module_ctrl_base_info.module_mincurr = current /100;
    thaisen_set_module_mincurr(current /100);

    MCTRL_DEBUG("module control module_mincurr:%d\n", s_module_ctrl_base_info.module_mincurr);
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_set_module_current_max
 * 功能                设置模块最大输出电流(0.01A)
 * 参数                current     电流值
 * 返回
 ****************************************/
void app_module_set_module_current_max(unsigned int current)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        return;
    }
    /** 单个模块最大电流 */
    s_module_ctrl_base_info.module_maxcurr = current;
    thaisen_set_module_maxcurr(current);
    MCTRL_DEBUG("module control module_maxcurr:%d\n", s_module_ctrl_base_info.module_maxcurr);
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_set_power_allocate_way
 * 功能                设置功率分配方式
 * 参数                way     功率分配方式
 * 返回
 ****************************************/
void app_module_set_power_allocate_way(unsigned char way)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        return;
    }
    /** 矩阵内有枪在充电, 不允许修改 */
    if(thaisen_get_pileCharging()){
        return;
    }
    /** 分配方式 */
    switch(way){
    case POWER_ALLOCATION_WAY_AVERAGE:
        s_module_ctrl_base_info.allomethod = ModuleAllo_share;
        break;
    case POWER_ALLOCATION_WAY_SEQ_PRIORITY:
        s_module_ctrl_base_info.allomethod = ModuleAllo_fcfs;
        break;
    case POWER_ALLOCATION_WAY_POWER_PRIORITY:
        s_module_ctrl_base_info.allomethod = ModuleAllo_hpf;
        break;
    default:
        return;         /** 这是运行中修改的，如果不对，直接退出不修改 */
    }
//    thaisen_base_init(s_module_ctrl_base_info);
    MCTRL_DEBUG("module control allocate way:%d\n", s_module_ctrl_base_info.allomethod);
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_set_single_module_power
 * 功能                设置单个模块功率(1W)
 * 参数                power     单个模块功率值
 * 返回
 ****************************************/
void app_module_set_single_module_power(unsigned int power)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 半矩和环矩类型 */
    if((s_module_ctrl_info.type == SYSTEM_FUNCTION_MS_MACHINE_CYCLE) || (s_module_ctrl_info.type == SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        /** 模块额定功率 */
        s_module_ctrl_base_info.module_preserpower = power;
        thaisen_chargemain_set_ModulePreserPower(power);
        MCTRL_DEBUG("set single module power:%dW\n", power);
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_get_setup_voltage
 * 功能                按组获取给模块设置的电压(0.1V)
 * 参数                group     组号
 * 返回                给模块设置的电压(0.1V)
 ****************************************/
unsigned int app_module_get_setup_voltage(unsigned char group)
{
#ifdef CP_USING_CYCLE_MATRIX
    extern uint16_t thaisen_guowang_get_groupctrlvolt(uint8_t groupnum);
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(group >= APP_SYSTEM_GUNNO_SIZE){
            return 0x00;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(group >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return 0x00;
        }
    }
    return thaisen_guowang_get_groupctrlvolt((group + 0x01));
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_get_setup_current
 * 功能                按组获取给模块设置的电流(0.01A)
 * 参数                group     组号
 * 返回                给模块设置的电流(0.01A)
 ****************************************/
unsigned int app_module_get_setup_current(unsigned char group)
{
#ifdef CP_USING_CYCLE_MATRIX
    extern uint16_t thaisen_guowang_get_groupctrlcurr(uint8_t groupnum);
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(group >= APP_SYSTEM_GUNNO_SIZE){
            return 0x00;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(group >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return 0x00;
        }
    }
    return thaisen_guowang_get_groupctrlcurr((group + 0x01));
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_get_setup_current
 * 功能                按组获取给模块设置的电流(0.01A)
 * 参数                group     组号
 * 返回                给模块设置的电流(0.01A)
 ****************************************/
unsigned char app_module_is_open(unsigned char group)
{
#ifdef CP_USING_CYCLE_MATRIX
    extern uint8_t thaisen_guowang_get_groupctrlcmd(uint8_t groupnum);
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(group >= APP_SYSTEM_GUNNO_SIZE){
            return 0x00;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(group >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return 0x00;
        }
    }
    if(thaisen_guowang_get_groupctrlcmd(group + 0x01)){
        return 0x01;
    }
    return 0x00;
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_belong_gun
 * 功能                获取模块组归属枪
 * 参数               group      模块组组号(从0开始)
 * 返回                归属枪号(从1开始)
 ****************************************/
unsigned char app_module_belong_gun(unsigned char group)
{
    unsigned char belong_gun = 0xFF;            /** 归属枪(从1开始) */
#ifdef CP_USING_CYCLE_MATRIX
    extern uint8_t th_moduleallo_GetMgroupAimGun(uint8_t Mgroupnum);
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(group >= APP_SYSTEM_GUNNO_SIZE){
            return belong_gun;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(group >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return belong_gun;
        }
    }
    belong_gun = th_moduleallo_GetMgroupAimGun((group + 0x01));
    if(belong_gun == 0x00){
        belong_gun = 0xFF;
    }
#endif /* CP_USING_CYCLE_MATRIX */
    return belong_gun;
}


/*****************************************
 * 函数名             app_module_schedule_judge
 * 功能                模块调度启用判断
 * 参数                current     电流值
 * 返回
 ****************************************/
void app_module_schedule_judge(void)
{
#ifdef CP_USING_CYCLE_MATRIX
    extern uint8_t thaisenModule_IsInPowerConnected(void);
    static uint8_t is_connected = 0x00, delay_time = 0x00;

    /** 模块电源已连接 */
    if(is_connected){
        /** 模块电源已断开 */
        if(thaisenModule_IsInPowerConnected() == 0){
            delay_time = 0x00;
            is_connected = 0x00;
            thaisen_module_set_ModuleSchedulingDisable();  /** 禁止模块调度 */
        }else{
            delay_time = 0x00;
        }
    }else{
        /** 模块电源已连接 */
        if(thaisenModule_IsInPowerConnected()){
            /** 外部调用时基100ms，大概5s */
            if(delay_time < 0xFF){
                delay_time++;
            }
            if(delay_time > (4900 /100)){
                delay_time = 0x00;
                is_connected = 0x01;
                thaisen_module_set_ModuleSchedulingEnable(); /** 开启模块调度 */
            }
        }else{
            delay_time = 0x00;
        }
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************************
 * 函数名              app_module_input_power_control
 * 功能                 模块输入电源控制
 * 参数
 * 返回
 ****************************************************/
void app_module_input_power_control(void)
{
#ifdef CP_USING_CYCLE_MATRIX

#define MCTRL_THREAD_RUNNING_TICK     100           /* 外部调用时基(ms) */
#define APP_MCTRL_STEP_IDLE           0x00          /** 空闲 */
#define APP_MCTRL_STEP_INSERT_GUN     0x01          /** 插枪 */
#define APP_MCTRL_STEP_CHARGING       0x02          /** 充电 */
#define APP_MCTRL_STEP_INSERT_GUN_NOT_CHARGE     0x03          /** 插着枪但未充电 */

#define APP_MCTRL_RELEASE_IN_POWER_TIME     (5 *60 *1000 /MCTRL_THREAD_RUNNING_TICK)    /** 断开模块输入电源延时时间(ms) */

    extern unsigned char thaisen_get_needAcPowerOn(void);

    static unsigned char step = APP_MCTRL_STEP_IDLE, connect_state_last[APP_SYSTEM_GUNNO_SIZE], count = 0x00;
    static unsigned int timing = 0x00;
    static struct ofsm_info *ofsm = NULL;
    unsigned char need_inpower = 0x00;

    need_inpower = thaisen_get_needAcPowerOn();
    if(thaisen_is_debug()){
        step = APP_MCTRL_STEP_IDLE;
        timing = 0x00;
        return;
    }
    if(*(sys_read_config_item_content(CONFIG_ITEM_LP_MODULE, 0x00)) == CONFIG_LP_CONSUMPTION_MODULE_YN){
        return;   /** 是磁保持的交流接触器 */
    }

    count = 0x00;
    for(unsigned char i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
        ofsm = get_ofsm_info(i);
        if(ofsm->base.flag.connect_state != connect_state_last[i]){
            connect_state_last[i] = ofsm->base.flag.connect_state;
            if(ofsm->base.flag.connect_state == APP_CONNECT_STATE_CONNECT){
                count++;
            }
        }
    }
    if(count || need_inpower){
        if((step == APP_MCTRL_STEP_IDLE) || (step == APP_MCTRL_STEP_INSERT_GUN_NOT_CHARGE)){
            step = APP_MCTRL_STEP_INSERT_GUN;
        }
        thaisen_relay_ac_on();
        timing = 0x00;
    }

    switch(step){
    case APP_MCTRL_STEP_IDLE:
        /** 空闲时插枪即闭合输入电源 */
        timing = 0x00;

        for(unsigned char i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
            ofsm = get_ofsm_info(i);
            if(ofsm->base.flag.connect_state == APP_CONNECT_STATE_CONNECT){
                step = APP_MCTRL_STEP_INSERT_GUN;
                thaisen_relay_ac_on();
                timing = 0x00;
                break;
            }
        }
        break;
    case APP_MCTRL_STEP_INSERT_GUN:
    {
        unsigned char i = 0x00;
        for(i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
            ofsm = get_ofsm_info(i);
            if((ofsm->base.state.current == APP_OFSM_STATE_STARTING) || ofsm->base.state.current == APP_OFSM_STATE_CHARGING){
                break;
            }
        }
        /** 插枪时超过 APP_MCTRL_RELEASE_IN_POWER_TIME 时间所有枪未充电则断开输入电源 */
        if(i >= APP_SYSTEM_GUNNO_SIZE){
            if(timing < (0xFFFFFFFF - 0x01)){
                timing++;
            }
            if(timing > APP_MCTRL_RELEASE_IN_POWER_TIME){
                step = APP_MCTRL_STEP_INSERT_GUN_NOT_CHARGE;
                timing = 0x00;
                thaisen_relay_ac_off();
                break;
            }
        }
        /** 插枪时有枪充电则闭合输入电源并进入充电状态 */
        else if(i < APP_SYSTEM_GUNNO_SIZE){
            step = APP_MCTRL_STEP_CHARGING;
            timing = 0x00;
        }
    }
        break;
    case APP_MCTRL_STEP_INSERT_GUN_NOT_CHARGE:
    {
        unsigned char i = 0x00, idle_count = 0x00;
        for(i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
            ofsm = get_ofsm_info(i);
            if((ofsm->base.state.current == APP_OFSM_STATE_STARTING) || ofsm->base.state.current == APP_OFSM_STATE_CHARGING){
                break;
            }
            if(ofsm->base.flag.connect_state != APP_CONNECT_STATE_CONNECT){
                idle_count++;
            }
        }
        /** 插枪未充电期间检测到所有枪空闲则回到空闲状态 */
        if(idle_count >= APP_SYSTEM_GUNNO_SIZE){
            step = APP_MCTRL_STEP_IDLE;
            timing = 0x00;
            break;
        }
        /** 插枪未充电期间检测到有枪充电则进入充电状态 */
        if(i < APP_SYSTEM_GUNNO_SIZE){
            step = APP_MCTRL_STEP_CHARGING;
            thaisen_relay_ac_on();
            timing = 0x00;
            break;
        }
        timing = 0x00;
    }
        break;
    case APP_MCTRL_STEP_CHARGING:
    {
        /** 充电期间检测到所有枪都不在充电状态则进入插枪状态 */
        unsigned char i = 0x00;
        for(i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
            ofsm = get_ofsm_info(i);
            if((ofsm->base.state.current == APP_OFSM_STATE_STARTING) || ofsm->base.state.current == APP_OFSM_STATE_CHARGING){
                break;
            }
        }
        if(i >= APP_SYSTEM_GUNNO_SIZE){
            step = APP_MCTRL_STEP_INSERT_GUN;
            timing = 0x00;
            break;
        }
        timing = 0x00;
    }
        break;
    default:
        step = APP_MCTRL_STEP_IDLE;
        timing = 0x00;
        break;
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************************
 * 函数名              app_mctrl_fan_control
 * 功能                 风机控制
 * 参数
 * 返回
 ****************************************************/
void app_module_fan_control(void)
{
#ifdef CP_USING_CYCLE_MATRIX
#define MCTRL_THREAD_RUNNING_TICK          100                                               /* 外部调用时基(ms) */
#define APP_MCTRL_CLOSE_FAN_DELAY_TIME     (2 *60 *1000 /MCTRL_THREAD_RUNNING_TICK)          /** 关闭风机延时时间(ms) */

    static unsigned int tick = 0x00, work_time = APP_MCTRL_CLOSE_FAN_DELAY_TIME;

    if(tick < (0xFFFFFFFF - 0x01))
        tick++;
    /** 矩阵内有枪在充电 */
    if(thaisen_get_pileCharging()){
        /** 控制风机启动 */
        if(thaisen_is_debug() == 0x00){
            thaisen_fan_B_on();
        }
        tick = 0x00;
    }
    /** 所有枪都空闲 */
    else{
        work_time = *((unsigned short*)sys_read_config_item_content(CONFIG_ITEM_FAN_WORK_TIME, 0x00));
        if((work_time < CHARGEPILE_FAN_WORK_TIME_MIN) || (work_time > CHARGEPILE_FAN_WORK_TIME_MAX)){
            work_time = CHARGEPILE_FAN_WORK_TIME_DEF;
        }
        if(tick > (work_time *1000 /MCTRL_THREAD_RUNNING_TICK)){
            if(thaisen_is_debug() == 0x00){
                thaisen_fan_B_off();
            }
        }
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_loop
 * 功能                模块控制部分实时运行
 * 参数
 * 返回
 ****************************************/
void app_module_loop(void)
{
#ifdef CP_USING_CYCLE_MATRIX
    unsigned char thaisen_get_needAcPowerOn(void);
    extern uint16_t thaisen_guowang_get_groupvolt(uint8_t groupnum);
    extern uint16_t thaisen_guowang_get_groupcurr(uint8_t groupnum);

    for(unsigned gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        if(s_module_ctrl_assistant_info[gunno].flag.is_debug_stoped){
            /** 当模块电压、电流小于设定值时即可认为模块已关机 */
            if((thaisen_guowang_get_groupvolt(gunno + 0x01) < MCTRL_RELEASE_RELAY_VOLTAGE_MAX) && \
                    (thaisen_guowang_get_groupcurr(gunno + 0x01) < MCTRL_RELEASE_RELAY_CURRENT_MAX)){
                s_module_ctrl_assistant_info[gunno].flag.is_debug_stoped = 0x00;
                if(gunno == APP_SYSTEM_GUNNOA){
                    thaisenDcRelay_A_Disable_Debug();
                }
#ifdef APP_USING_DOUBLEGUN
                else if(gunno == APP_SYSTEM_GUNNOB){
                    thaisenDcRelay_B_Disable_Debug();
                }
#endif /* APP_USING_DOUBLEGUN */
            }
        }
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*********************************************************************************************
 * 函数名         app_module_debug_start
 * 功能             模块调试强制启动
 * 参数             gunno      枪号
 *       voltage    设置电压(0.1V)
 *       current    设置电流(0.1A)
 * 返回            >=0：成功     <0：失败
 ********************************************************************************************/
int app_module_debug_start(unsigned char gunno, unsigned short voltage, unsigned short current)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(gunno >= APP_SYSTEM_GUNNO_SIZE){
            return -0x01;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(gunno >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return -0x01;
        }
    }

    unsigned short battery_volt = 0x00;
    unsigned short voltage_min = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0x00));
    unsigned short voltage_max = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, 0x00));
    unsigned short current_min = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_MIN_LIMIT_CURRENT, 0x00));
    unsigned short gun_current_max = (*(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0x00))) /APP_SYSTEM_GUNNO_SIZE;
    int _curr_offset = *(unsigned short*)(sys_read_config_item_content((CONFIG_ITEM_GUN1_CURR_OFFSET + gunno), 0x00));

    thaisenAcRelay_Enable_Debug();

    s_module_ctrl_assistant_info[gunno].flag.is_debug_started = 0x01;
    s_module_ctrl_assistant_info[gunno].flag.is_debug_stoped = 0x00;

    voltage = voltage < (voltage_min *10) ? (voltage_min *10) : voltage;
    voltage = voltage > (voltage_max *10) ? (voltage_max *10) : voltage;

    current *= 10;
    /** 电流偏移有效性判断 */
    /** 负偏移 */
    if(_curr_offset < CP_CURRENT_OFFSET_SEPARATE){
        if((_curr_offset < CP_CURRENT_OFFSET_MIN) || (_curr_offset > CP_CURRENT_OFFSET_MAX)){
            _curr_offset = CP_CURRENT_OFFSET_DEF;
        }
        _curr_offset = 0x00 - _curr_offset;
    }
    /** 正偏移 */
    else{
        _curr_offset -= CP_CURRENT_OFFSET_SEPARATE;
        if((_curr_offset < CP_CURRENT_OFFSET_MIN) || (_curr_offset > CP_CURRENT_OFFSET_MAX)){
            _curr_offset = CP_CURRENT_OFFSET_DEF;
        }
    }

    /** 电流偏移计算 */
    if(_curr_offset < 0x00){
        if(((0 - _curr_offset) <= current)){
            current += _curr_offset;
        }else{
            current = 0;
        }
    }else{
        current += _curr_offset;
    }

    current_min *= 10;
    /** 最小输出电流(1A)：为0表示 0.5A */
    if(current_min < 0x05){
        current_min = 0x05;
    }
    current = current < (current_min *10) ? (current_min *10) : current;
    current = current > (gun_current_max *100) ? (gun_current_max *100) : current;

    battery_volt = voltage;
    battery_volt = battery_volt > 200 ? (battery_volt - 200) : battery_volt;
    /** 按快速开机来 */
    thaisen_guowang_set_controlparam((gunno + 0x01), 0x01, voltage, current, battery_volt);
    thaisen_guowang_moduledebug_enable((gunno + 0x01));

    if(gunno == APP_SYSTEM_GUNNOA){
        thaisenDcRelay_A_Enable_Debug();
    }
#ifdef APP_USING_DOUBLEGUN
    else if(gunno == APP_SYSTEM_GUNNOB){
        thaisenDcRelay_B_Enable_Debug();
    }
#endif /* APP_USING_DOUBLEGUN */
#endif /* CP_USING_CYCLE_MATRIX */
    return 0x00;
}

/*********************************************************************************************
 * 函数名         app_module_debug_stop
 * 功能             模块调试强制停止
 * 参数             gunno      枪号
 * 返回            >=0：成功     <0：失败
 ********************************************************************************************/
int app_module_debug_stop(unsigned char gunno)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(gunno >= APP_SYSTEM_GUNNO_SIZE){
            return -0x01;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(gunno >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return -0x01;
        }
    }
    thaisen_guowang_moduledebug_disable((gunno + 0x01));
    s_module_ctrl_assistant_info[gunno].flag.is_debug_stoped = 0x01;
    s_module_ctrl_assistant_info[gunno].flag.is_debug_started = 0x00;
#endif /* CP_USING_CYCLE_MATRIX */
    return 0x00;
}

/*********************************************************************************************
 * 函数名         app_module_is_debug_started
 * 功能             判断模块是否已进行调试强制启动
 * 参数             gunno      枪号
 * 返回            1：是        0：否
 ********************************************************************************************/
unsigned char app_module_is_debug_started(unsigned char gunno)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(gunno >= APP_SYSTEM_GUNNO_SIZE){
            return 0x00;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(gunno >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return 0x00;
        }
    }
    if(s_module_ctrl_assistant_info[gunno].flag.is_debug_started){
        return 0x01;
    }
#endif /* CP_USING_CYCLE_MATRIX */
    return 0x00;
}

/*****************************************
 * 函数名             app_module_get_module_group_voltage
 * 功能                按组获取模块输出电压(0.1V, 最高电压)
 * 参数                group     组号(从0开始)
 * 返回                组模块输出电压(0.1V, 最高电压)
 ****************************************/
unsigned int app_module_get_module_group_voltage(unsigned char group)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(group >= APP_SYSTEM_GUNNO_SIZE){
            return -0x01;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(group >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return -0x01;
        }
    }
    extern uint16_t thaisen_guowang_get_groupvolt(uint8_t groupnum);

    return thaisen_guowang_get_groupvolt((group + 0x01));
#else
    return 0x00;
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************
 * 函数名             app_module_get_module_group_current
 * 功能                按组获取模块输出电流(0.01A)
 * 参数                group     组号(从0开始)
 * 返回                组模块输出电流(0.01A)
 ****************************************/
unsigned int app_module_get_module_group_current(unsigned char group)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        if(group >= APP_SYSTEM_GUNNO_SIZE){
            return -0x01;
        }
    }else{
        /** 环矩/半矩设备：模块都在母机，最多MCTRL_CYCLE_MATRIX_MODULE_NUM把枪(需要考虑双枪改造时的情况) */
        if(group >= MCTRL_CYCLE_MATRIX_MODULE_NUM){
            return -0x01;
        }
    }
    extern uint16_t thaisen_guowang_get_groupcurr(uint8_t groupnum);

    return thaisen_guowang_get_groupcurr((group + 0x01));
#else
    return 0x00;
#endif /* CP_USING_CYCLE_MATRIX */
}


/***************************************************** 回调函数 *****************************************************/

/*****************************************************
 * 函数名              app_module_allocate_log
 * 功能                 模块分配日志回调
 * 参数                 log      日志数据
 *           log_len  日志数据长度
 * 返回
 ****************************************************/
static void app_module_allocate_log(unsigned char const* const log, unsigned char log_len)
{
    extern void ykc_monitor_module_allocate_log_callback(uint8_t *log, uint8_t log_len);
    ykc_monitor_module_allocate_log_callback((unsigned char*)log, log_len);
}

/*****************************************************
 * 函数名              app_module_relay_state_changed
 * 功能                 继电器状态变化
 * 参数                 relay_sn      继电器编号
 *         type          继电器类型@mctrl_relay_status
 *         state         状态
 * 返回
 ****************************************************/
static void app_module_relay_state_changed(unsigned char relay_sn, unsigned char type, unsigned char state)
{

}

/*****************************************************
 * 函数名              app_module_relay_state_faulting
 * 功能                 设置枪继电器故障
 * 参数                 relay_sn      继电器编号
 *         type          继电器类型@mctrl_relay_type_t
 *         state         控制状态
 * 返回
 ****************************************************/
static void app_module_relay_state_faulting(unsigned char gunno, unsigned char relay_sn)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 调试模式下不报故障 */
    if(thaisen_is_debug()){
        return;
    }
    if((gunno <= 0x00) || (gunno > APP_SYSTEM_GUNNO_SIZE)){
        return;
    }
    /** 非半矩和环矩类型不报矩阵继电器故障 */
//    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
//        return;
//    }
    unsigned char index = 0x00;
    /** 获取控制下标 */
    index = app_module_get_index_sn_to_mrelay(relay_sn, s_module_ctrl_info.type);
    /** 下标有效 */
    if(index < THAISEN_MATRIX_RELAY_SIZE){
        /** 设置故障 */
        thaisenSetSysFaultLib((thaisenFaultMatrixRelay_KPN1_1 + index), (gunno - 0x01));
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************************
 * 函数名              app_module_relay_state_resum
 * 功能                 清除枪继电器故障
 * 参数                 relay_sn      继电器编号
 *         type          继电器类型@mctrl_relay_type_t
 *         state         控制状态
 * 返回
 ****************************************************/
static void app_module_relay_state_resum(unsigned char gunno, unsigned char relay_sn)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((gunno <= 0x00) || (gunno > APP_SYSTEM_GUNNO_SIZE)){
        return;
    }
    /** 非半矩和环矩类型不报矩阵继电器故障 */
//    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
//        return;
//    }
    unsigned char index = 0x00;
    /** 获取控制下标 */
    index = app_module_get_index_sn_to_mrelay(relay_sn, s_module_ctrl_info.type);
    /** 下标有效 */
    if(index < THAISEN_MATRIX_RELAY_SIZE){
        /** 清除故障 */
        thaisenClearSysFaultLib((thaisenFaultMatrixRelay_KPN1_1 + index), (gunno - 0x01));
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************************
 * 函数名              app_module_relay_contrl
 * 功能                 继电器控制
 * 参数                 relay_sn      继电器编号
 *         type          继电器类型@mctrl_relay_status
 *         state         控制状态
 * 返回
 ****************************************************/
static void app_module_relay_contrl(unsigned char relay_sn, unsigned char type, unsigned char state)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        return;
    }
    unsigned char index = 0x00;
    /** 获取控制下标 */
    index = app_module_get_index_sn_to_mrelay(relay_sn, s_module_ctrl_info.type);
    /** 下标有效 */
    if(index < THAISEN_MATRIX_RELAY_SIZE){
        /** 控制断开 */
        if(state == MCTRL_RELAY_STATUS_RELEASE){
            thaisenMatrixRelay_off_Only(index);
        }
        /** 控制闭合 */
        else if(state == MCTRL_RELAY_STATUS_ACTION){
            thaisenMatrixRelay_on_Only(index);
        }
    }
#endif /* CP_USING_CYCLE_MATRIX */
}

/*****************************************************
 * 函数名              app_module_query_relay_state
 * 功能                 继电器状态查询
 * 参数                 relay_sn      继电器编号
 *         type          继电器类型@mctrl_relay_type_t
 * 返回                 继电器状态@mctrl_relay_status
 ****************************************************/
static unsigned char app_module_query_relay_state(unsigned char relay_sn, unsigned char type)
{
#ifdef CP_USING_CYCLE_MATRIX
    /** 非半矩和环矩类型不报矩阵继电器故障 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        return MCTRL_RELAY_STATUS_RELEASE;
    }
    unsigned char index = 0x00, state = 0x00;
    /** 获取控制下标 */
    index = app_module_get_index_sn_to_mrelay(relay_sn, s_module_ctrl_info.type);
    /** 下标有效 */
    if(index < THAISEN_MATRIX_RELAY_SIZE){
        /** 根据下标获取反馈状态 */
        state = thaisenMatrixRelay_FB(index);
        /** 正极继电器 */
        if(type == MCTRL_RELAY_TYPE_POSITIVE){
            /** 反馈状态为闭合 */
            if(state == thaisenRelayClose){
                return MCTRL_RELAY_STATUS_ACTION;
            }else{
                return MCTRL_RELAY_STATUS_RELEASE;
            }
        }else if(type == MCTRL_RELAY_TYPE_NEGTIVE){
            /** 反馈状态为闭合 */
            if(state == thaisenRelayClose){
                return MCTRL_RELAY_STATUS_ACTION;
            }else{
                return MCTRL_RELAY_STATUS_RELEASE;
            }
        }
    }
#endif /* CP_USING_CYCLE_MATRIX */
    /** 默认返回断开 */
    return MCTRL_RELAY_STATUS_RELEASE;
}

/*****************************************************
 * 函数名              app_module_query_dcrelay_state
 * 功能                 查询直流继电器状态
 * 参数                 gunno       继电器归属枪号
 *           type        继电器类型@mctrl_relay_type_t
 * 返回                 继电器状态@mctrl_relay_status
 ****************************************************/
static unsigned char app_module_query_dcrelay_state(unsigned char gunno, unsigned char type)
{
#ifdef CP_USING_CYCLE_MATRIX
    if((gunno <= 0x00) || (gunno > APP_SYSTEM_GUNNO_SIZE)){
        return MCTRL_RELAY_STATUS_RELEASE;
    }
    gunno--;

    switch(type){
    case MCTRL_RELAY_TYPE_POSITIVE:
        if(thaisenDcRelay_StateQuery(gunno, thaisenRelayClose)){
            return MCTRL_RELAY_STATUS_ACTION;
        }
        break;
    case MCTRL_RELAY_TYPE_NEGTIVE:
        if(thaisenDcRelay_StateQuery(gunno, thaisenRelayClose)){
            return MCTRL_RELAY_STATUS_ACTION;
        }
        break;
    default:
        break;
    }
#endif /* CP_USING_CYCLE_MATRIX */
    return MCTRL_RELAY_STATUS_RELEASE;
}

/*****************************************
 * 函数名             app_module_ctrl_init
 * 功能                模块控制部分初始化
 * 参数
 * 返回                 >=0：成功     <0：失败
 ****************************************/
int app_module_ctrl_init(void)
{
#ifdef CP_USING_CYCLE_MATRIX
    unsigned int config_data = 0x00;
    thaisen_masterSlaveCom_init_t ms_init;

    memset(s_module_ctrl_assistant_info, 0x00, sizeof(s_module_ctrl_assistant_info));
    memset(&s_module_ctrl_info, 0x00, sizeof(s_module_ctrl_info));
    /** 设备地址 */
    config_data = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_TEMINAL_ADDRA, 0x00));
    ms_init.gunAddr[0x00] = (unsigned char)config_data;
    config_data = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_TEMINAL_ADDRB, 0x00));
    ms_init.gunAddr[0x01] = (unsigned char)config_data;
    MCTRL_DEBUG("module control  device address:%X, %X\n", ms_init.gunAddr[0x00], ms_init.gunAddr[0x01]);

    /** 矩阵类型/子母机配置 */
    config_data = *(unsigned char*)(sys_read_config_item_content(CONFIG_ITEM_DEVICE_TYPE, 0x00));
    switch(config_data){
    case SYSTEM_FUNCTION_MS_MACHINE_CYCLE:
        s_module_ctrl_info.type = SYSTEM_FUNCTION_MS_MACHINE_CYCLE;
        s_module_ctrl_base_info.matrix_type = thaisen_moduleallo_matrixtype_ringmatrix;
        break;
    case SYSTEM_FUNCTION_MS_MACHINE_HALF:
        s_module_ctrl_info.type = SYSTEM_FUNCTION_MS_MACHINE_HALF;
        s_module_ctrl_base_info.matrix_type = thaisen_moduleallo_matrixtype_halfmatrix;
        break;
    case SYSTEM_FUNCTION_DOUBLE_WHOLE:
        s_module_ctrl_info.type = SYSTEM_FUNCTION_DOUBLE_WHOLE;
        s_module_ctrl_base_info.matrix_type = thaisen_moduleallo_matrixtype_none;
        break;
    default:
        s_module_ctrl_info.type = SYSTEM_FUNCTION_MS_MACHINE_CYCLE;
        s_module_ctrl_base_info.matrix_type = thaisen_moduleallo_matrixtype_ringmatrix;         /** 默认矩阵排布 */
        break;
    }
    MCTRL_DEBUG("module control  matrix_type:%d\n", s_module_ctrl_base_info.matrix_type);
#if 0
    /** 设备类型非半矩和环矩是不需要进行以下初始化 */
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        ms_init.devType = thaisen_masterSlaveCom_devType_Slave;
        thaisenMasterSlave_Init(ms_init);
        thaisen_base_deInit();

        MCTRL_DEBUG("module control device type is slave\n");
        return 0x00;
    }
#endif
    /** 分配方式 */
    config_data = *(unsigned char*)(sys_read_config_item_content(CONFIG_ITEM_ALLOCATION_WAY, 0x00));
    switch(config_data){
    case POWER_ALLOCATION_WAY_AVERAGE:
        s_module_ctrl_base_info.allomethod = ModuleAllo_share;
        break;
    case POWER_ALLOCATION_WAY_SEQ_PRIORITY:
        s_module_ctrl_base_info.allomethod = ModuleAllo_fcfs;
        break;
    case POWER_ALLOCATION_WAY_POWER_PRIORITY:
        s_module_ctrl_base_info.allomethod = ModuleAllo_hpf;
        break;
    default:
        s_module_ctrl_base_info.allomethod = ModuleAllo_fcfs;         /** 默认先到先得 */
        break;
    }
    MCTRL_DEBUG("module control allocate way:%d\n", s_module_ctrl_base_info.allomethod);

    /** 模块额定功率 */
    s_module_ctrl_base_info.module_preserpower = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0x00));
    config_data = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0x00));
    s_module_ctrl_base_info.module_preserpower *= config_data;
    s_module_ctrl_base_info.module_preserpower = s_module_ctrl_base_info.module_preserpower *sys_get_power_percent() /1000;
    MCTRL_DEBUG("module control rated power:%d\n", s_module_ctrl_base_info.module_preserpower);

    /** 模块组数，组内模块数 */
    memset(s_module_ctrl_base_info.module_cntforgroup, 0x00, sizeof(s_module_ctrl_base_info.module_cntforgroup));
    s_module_ctrl_base_info.module_groupcnt = *(unsigned char*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));
    if(s_module_ctrl_base_info.module_groupcnt > MODULE_GROUP_NUMBER_MAX){
        s_module_ctrl_base_info.module_groupcnt = MODULE_GROUP_NUMBER_MAX;
    }
    MCTRL_DEBUG("module control module_group number:%d\n", s_module_ctrl_base_info.module_groupcnt);

    for(unsigned char i = 0x00; i < s_module_ctrl_base_info.module_groupcnt; i++){
        s_module_ctrl_base_info.module_cntforgroup[i] = *(unsigned char*)(sys_read_config_item_content((CONFIG_ITEM_MODULE_NUM_GROUP_1 + i), 0x00));
        if(s_module_ctrl_base_info.module_cntforgroup[i] > MODULE_NUMBER_SINGLE_MAX){
            s_module_ctrl_base_info.module_cntforgroup[i] = MODULE_NUMBER_SINGLE_MAX;
        }
        MCTRL_DEBUG("module control matrix num single grp:%d, %d\n", i, s_module_ctrl_base_info.module_cntforgroup[i]);
    }

    /** 单个模块最大电流 */
    s_module_ctrl_base_info.module_maxcurr = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_SMODULE_OUTCURR_MAX, 0x00)) *100;
    MCTRL_DEBUG("module control module_maxcurr:%d\n", s_module_ctrl_base_info.module_maxcurr);
    /** 单个模块最小电流 */
    s_module_ctrl_base_info.module_mincurr = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_MIN_LIMIT_CURRENT, 0x00));
    MCTRL_DEBUG("module control module_mincurr:%d\n", s_module_ctrl_base_info.module_mincurr);
    /** 单个模块最小电压 */
    s_module_ctrl_base_info.module_minvolt = *(unsigned short*)(sys_read_config_item_content(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0x00)) *10;
    MCTRL_DEBUG("module control module_minvolt:%d\n", s_module_ctrl_base_info.module_minvolt);

    /** 动态分组地址偏移 */
    config_data = *(unsigned char*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_MODEL, 0x00));
    switch(config_data){
    case MODULE_MODEL_YFY:
        s_module_ctrl_base_info.module_addroffset = 0x60;   /** 英飞源动态分组默认地址偏移 */
        break;
    default:
        s_module_ctrl_base_info.module_addroffset = 0x20;   /** 动态分组默认地址偏移 */
        break;
    }
    MCTRL_DEBUG("module control module address offset:0x%02X\n", s_module_ctrl_base_info.module_addroffset);
    /** 枪数量 */
    s_module_ctrl_base_info.guncnt = s_module_ctrl_base_info.module_groupcnt;

    /** 回调函数注册 */
    s_module_ctrl_base_info.setrelaysta = app_module_relay_contrl;             /** 继电器控制 */
    s_module_ctrl_base_info.getrelaysta = app_module_query_relay_state;        /** 继电器状态查询 */
    s_module_ctrl_base_info.stachangefb = app_module_relay_state_changed;      /** 继电器状态变化回调 */
    s_module_ctrl_base_info.getdcrelaysta = app_module_query_dcrelay_state;    /** 查询直流继电器状态 */
    s_module_ctrl_base_info.gunfaultset = app_module_relay_state_faulting;     /** 设置枪继电器故障 */
    s_module_ctrl_base_info.gunfaultclean = app_module_relay_state_resum;      /** 清除枪继电器故障 */
    s_module_ctrl_base_info.NetLogSend = app_module_allocate_log;              /** 模块分配日志回调 */

    /** 底层初始化 */
    ms_init.devType = thaisen_masterSlaveCom_devType_Master;
    s_module_ctrl_base_info.devType = thaisen_masterSlaveCom_devType_Master;
    if((s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_CYCLE) && (s_module_ctrl_info.type != SYSTEM_FUNCTION_MS_MACHINE_HALF)){
        ms_init.devType = thaisen_masterSlaveCom_devType_Slave;
        s_module_ctrl_base_info.devType = thaisen_masterSlaveCom_devType_Slave;
    }

    thaisen_base_init(s_module_ctrl_base_info);
    thaisenMasterSlave_Init(ms_init);

    MCTRL_DEBUG("module control device type is master\n");
#endif /* CP_USING_CYCLE_MATRIX */
    return 0x00;
}
