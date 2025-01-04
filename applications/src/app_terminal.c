/**
  ******************************************************************************
  * @file
  * @brief 终端实现，用于与上位机的交互
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#include <rtthread.h>

#include <stdio.h>
#include <stdlib.h>

#include "chargepile_config.h"

#include "bsp_serial.h"

#include "app_terminal.h"
#include "app_ofsm.h"

#include "mw_fault_check.h"
#include "mw_cc1.h"
#include "mw_fault_check.h"
#include "app_data_info_interface.h"


#define DBG_TAG "app.terminal"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define THAISEN_EMS_FANCTION_CODE_REQ         0x03               //发送数据功能码
#define THAISEN_EMS_LEGTH_REQ                 0x20               //发送数据长度

#define THAISEN_EMS_FANCTION_CODE_RES         0x04               //接收数据功能码
#define THAISEN_EMS_LEGTH_RES                 0x06               //接收数据长度
#define THAISEN_EMS_RECV_HEAD                 0x01               //接收数据头
#define THAISEN_EMS_RECV_FRAME_CMD            0x04               //接收数据指令码(目前只有一帧，这样做能减少错误率)
#define THAISEN_EMS_RECV_FRAME_LENGTH         0x06               //接收数据长度值(目前只有一帧，这样做能减少错误率)
#define THAISEN_EMS_ALL_LEGTH_RES             0x0B               //接收数据长度

#define TERMINAL_EMS_SEND_PERIOD              250                //ems数据发送间隔(ms)
#define EMS_WITE_PERIOD                       1000                //EMS主动上报的状态500ms周期(单位 ms)

#define HTONS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8)) //宏把低字节在前转为高字节在前

#pragma pack(1)                                                  // 1字节对齐，成员紧密排列，没有填充

struct terminal_data
{
    uint8_t buffer[256];
    uint32_t length;
};

// 定义帧结构
struct chargegun_data{
    uint16_t volt;       //电压
    uint16_t curr;       //电流
    uint16_t soc;        //soc
    uint16_t power;      //功率
    uint16_t state;      //状态
};                                                              //发送数据电压电流结构体

typedef struct{
    uint8_t header;
    uint8_t function_code;
    uint8_t length;

    struct chargegun_data gun_data[2];                          //目前协议按照双枪发送

    uint16_t year;        //年
    uint16_t month;       //月
    uint16_t day;         //日
    uint16_t hour;        //时
    uint16_t minute;      //分
    uint16_t second;      //秒

    uint16_t crc;
}ems_frame_req;                                                 //发送数据结构体

typedef struct{
    uint8_t header;
    uint8_t function_code;
    uint8_t length;
    uint16_t power;      //功率
    uint16_t volt;       //电压
    uint16_t curr;       //电流
    uint16_t soc;
    uint16_t crc;
}ems_frame_res;                                                 //接收数据结构体

#pragma pack() /* #pragma pack(1) */                            //回到默认对齐方式

APP_DEF_SRAM1 static ems_frame_req s_ems_frame_request;                       //数据发送
APP_DEF_SRAM1 static struct terminal_data s_terminal_data;
APP_DEF_SRAM1 static ems_frame_res s_ems_frame_res;                           //数据接收
APP_DEF_SRAM1 static uint8_t s_power_data_received_flag = 0;                  //是否收到数据标志位
APP_DEF_SRAM1 static uint16_t s_last_power;                                   //上次调整功率数据

#ifdef APP_DESIGNATE_REGION
/*************************************
 * 函数名       app_terminal_info_init
 * 功能           中断串口信息、变量初始化
 * 参数
 * 返回
 ************************************/
void app_terminal_info_init(void)
{
    memset(&s_ems_frame_request, 0x00, sizeof(s_ems_frame_request));
    memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
    memset(&s_ems_frame_res, 0x00, sizeof(s_ems_frame_res));

    s_power_data_received_flag = 0;
    s_last_power = 0;
}
#endif /* APP_DESIGNATE_REGION */

/*******************************************************
 * 函数名               terminal_enter_critical
 * 功能                  进入临界区
 * 参数
 * 返回
 ******************************************************/
static void terminal_enter_critical(void)
{
    rt_enter_critical();
}

/*******************************************************
 * 函数名               terminal_exit_critical
 * 功能                  退出临界区
 * 参数
 * 返回
 ******************************************************/
static void terminal_exit_critical(void)
{
    rt_exit_critical();
}

static int16_t terminal_get_char(void)
{
    uint8_t ch = 0x00;
    uint8_t result = 0;
    while (rt_device_read(g_terminal_serial, -1, &ch, 1) != 1){
        result = rt_sem_take(&g_terminal_rx_sem, EMS_WITE_PERIOD);
        if(result != RT_EOK){
            return -1;
        }
    }

    return ch;
}

// CRC校验函数
static unsigned int xfmbmcrcsum(unsigned char *data,int count)
{
    int i,j;
    unsigned int temp=0xffff,temp1;
    for(i=0;i<count;i++)
    {
        temp=*(data+i)^temp;
        for(j=0;j<8;j++)
        {
            temp1=temp&1;
            temp>>=1;
            if(temp1) temp^=0xa001;
        }
    }
    return temp;
}

/*****************************************
 * 函数名         terminal_is_ems_adjust_power
 * 功能             查询 ems 是否设置了新的功率
 * 参数
 * 返回             1: 是     0: 否
 ****************************************/
uint8_t terminal_is_ems_adjust_power(void)
{
    if(s_power_data_received_flag){
        return 1;
    }
    return 0;
}

/*****************************************
 * 函数名         terminal_is_ems_adjust_power
 * 功能             查询 ems 是否设置了新的功率
 * 参数
 * 返回             1: 是     0: 否
 ****************************************/
void terminal_clear_is_ems_adjust_power(void)
{
    s_power_data_received_flag = 0;
}

/*****************************************
 * 函数名         terminal_get_ems_set_power
 * 功能             获取 ems 设置的功率
 * 参数
 * 返回             ems 设置的功率
 ****************************************/
uint16_t terminal_get_ems_set_power()
{
    return s_ems_frame_res.power;
}

/*****************************************
 * 函数名         terminal_get_ems_soc
 * 功能             获取 ems SOC (1位小数)
 * 参数
 * 返回             ems SOC
 ****************************************/
uint16_t terminal_get_ems_soc()
{
    return 0;
//    return s_ems_frame_res.soc;
}

/*****************************************
 * 函数名         terminal_get_ems_voltage
 * 功能             获取 ems 电压 (1位小数)
 * 参数
 * 返回             ems 电压
 ****************************************/
uint16_t terminal_get_ems_voltage()
{
    return s_ems_frame_res.volt;
}

/*****************************************
 * 函数名         terminal_get_ems_current
 * 功能             获取 ems 电流  (1位小数)
 * 参数
 * 返回             ems 电流
 ****************************************/
uint16_t terminal_get_ems_current()
{
    return s_ems_frame_res.curr;
}


/*****************************************
 * 函数名         terminal_ems_send_charger_status
 * 功能             向 ems 发送桩状态
 * 参数
 * 返回
 ****************************************/
static void terminal_ems_send_charger_status(void)                                   //发送数据
{
    struct tm *ems_tm;
    s_ems_frame_request.header = THAISEN_EMS_RECV_HEAD;
    s_ems_frame_request.function_code = THAISEN_EMS_FANCTION_CODE_REQ;
    s_ems_frame_request.length = THAISEN_EMS_LEGTH_REQ;
    uint32_t tick = mw_get_current_timestamp();

#ifdef APP_USING_DOUBLEGUN                                           //双枪模式
    for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        struct ofsm_info *ofsm = get_ofsm_info(gunno);
        if(ofsm->base.state.current == APP_OFSM_STATE_CHARGING){                                            //充电状态赋值充电数据
            s_ems_frame_request.gun_data[gunno].curr = HTONS(ofsm->base.current_a /10);
            s_ems_frame_request.gun_data[gunno].power = HTONS(ofsm->base.power_a /1000);

            rt_kprintf("ofsm->base.current_a /10(%d)\r\n",ofsm->base.current_a /10);
            rt_kprintf("ofsm->base.voltage_a /10(%d)\r\n",ofsm->base.voltage_a /10);
            s_ems_frame_request.gun_data[gunno].soc = HTONS(ofsm->base.current_soc);
            s_ems_frame_request.gun_data[gunno].volt = HTONS(ofsm->base.voltage_a /10);
        }else{
            memset(&s_ems_frame_request.gun_data[gunno], 0x00, sizeof(s_ems_frame_request.gun_data[gunno]));//非充电状态数据置0
        }
        switch(ofsm->base.state.current){                        //赋值充电状态给要发送的数据
        case APP_OFSM_STATE_WAIT_NET:
        case APP_OFSM_STATE_IDLEING:
        case APP_OFSM_STATE_READYING:
            s_ems_frame_request.gun_data[gunno].state = HTONS(0);
            break;
        case APP_OFSM_STATE_STARTING:
            s_ems_frame_request.gun_data[gunno].state = HTONS(2);
            break;
        case APP_OFSM_STATE_CHARGING:
            s_ems_frame_request.gun_data[gunno].state = HTONS(1);
            break;
        case APP_OFSM_STATE_STOPING:
        case APP_OFSM_STATE_FINISHING:
            s_ems_frame_request.gun_data[gunno].state = HTONS(3);
            break;
        case APP_OFSM_STATE_FAULTING:
            s_ems_frame_request.gun_data[gunno].state = HTONS(4);
            break;
        default:
            break;
        }
    }
#else                                                          //单枪模式
    struct ofsm_info *ofsm = get_ofsm_info(0x00);
    s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].curr = HTONS(ofsm->base.current_a /10);
    s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].power = HTONS(ofsm->base.power_a /1000);
    s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].soc = HTONS(ofsm->base.current_soc);
    s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].volt =HTONS(ofsm->base.voltage_a /10);
    switch(ofsm->base.state.current){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
        s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].state = HTONS(0);
        break;
    case APP_OFSM_STATE_STARTING:
        s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].state = HTONS(2);
        break;
    case APP_OFSM_STATE_CHARGING:
        s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].state = HTONS(1);
        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].state = HTONS(3);
        break;
    case APP_OFSM_STATE_FAULTING:
        s_ems_frame_request.gun_data[APP_SYSTEM_GUNNOA].state = HTONS(4);
        break;
    default:
        break;
    }
#endif /* APP_USING_DOUBLEGUN */

    terminal_enter_critical();

    ems_tm = localtime((time_t*)&tick);
    s_ems_frame_request.year= HTONS(ems_tm->tm_year + 1900);             //年加上1900
    s_ems_frame_request.month = HTONS(ems_tm->tm_mon + 1);               //月是从0开始，0代表1月
    s_ems_frame_request.day = HTONS(ems_tm->tm_mday);
    s_ems_frame_request.hour = HTONS(ems_tm->tm_hour);
    s_ems_frame_request.minute = HTONS(ems_tm->tm_min);
    s_ems_frame_request.second = HTONS(ems_tm->tm_sec);

    terminal_exit_critical();

    s_ems_frame_request.crc = xfmbmcrcsum((uint8_t *)&s_ems_frame_request, sizeof(s_ems_frame_request) - 2);//crc校验校验数据高字节在前

    terminal_send_data((uint8_t *)&s_ems_frame_request, sizeof(ems_frame_req));//250ms发送一次数据
}

void terminal_req_thread_entry(void *parameter)
{
    uint32_t tick = rt_tick_get();
    while(1)
    {
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > TERMINAL_EMS_SEND_PERIOD){
            tick = rt_tick_get();
            terminal_ems_send_charger_status();
        }

        rt_thread_mdelay(5);
    }
}

void terminal_thread_entry(void *parameter)
{
    (void)parameter;
    uint8_t ch = 0;                                            //若terminal_get_char()没有收到数据返回值-1，这里用到有符号数
    memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
    memset(&s_ems_frame_res, 0x00, sizeof(s_ems_frame_res));
    uint8_t rlen = 0;                                         //接收数据长度
    uint8_t data_buffer[THAISEN_EMS_ALL_LEGTH_RES];           //用来存放接收数据

    while(1)
    {
        ch = terminal_get_char();

        if(ch < 0){
            rlen = 0;
            continue;                                      //没有收到数据返回while重新接收
        }

        if((ch != THAISEN_EMS_RECV_HEAD) && (rlen == 0)){                           //收到帧头
            rlen = 0;
            continue;
        }
        if(rlen == 0){
            rlen++;
            data_buffer[0] = THAISEN_EMS_RECV_HEAD;
            continue;
        }

        if((ch != THAISEN_EMS_RECV_FRAME_CMD) && (rlen == 1)){                           //收到帧头
            rlen = 0;
            continue;
        }
        if(rlen == 1){
            rlen++;
            data_buffer[1] = THAISEN_EMS_RECV_FRAME_CMD;
            continue;
        }

        if((ch != THAISEN_EMS_RECV_FRAME_LENGTH) && (rlen == 2)){                           //收到帧头            rlen = 0;
            continue;
        }
        if(rlen == 2){
            rlen++;
            data_buffer[2] = THAISEN_EMS_RECV_FRAME_LENGTH;
            continue;
        }

        //已接收到头
        if(rlen > 0){
            data_buffer[rlen++] = ch;
            if(rlen >= THAISEN_EMS_ALL_LEGTH_RES){
                if(xfmbmcrcsum((uint8_t *)&data_buffer, sizeof(data_buffer) - 2) != (uint16_t)((data_buffer[10] << 8) | data_buffer[9])){
                    rlen = 0;
                    continue;                                      //校验失败返回while重新接收
                }else{
                    s_ems_frame_res.header = data_buffer[0];
                    s_ems_frame_res.function_code = data_buffer[1];
                    s_ems_frame_res.length = data_buffer[2];
                    s_ems_frame_res.power = ((data_buffer[3] << 8) | data_buffer[4]);//这里不能*1000数据太大
                    s_ems_frame_res.volt = ((data_buffer[5] << 8) | data_buffer[6]) * 10;
                    s_ems_frame_res.curr = ((data_buffer[7] << 8) | data_buffer[8]) * 10;
                    s_ems_frame_res.soc = ((data_buffer[9] << 8) | data_buffer[10]);
                    s_ems_frame_res.crc = (data_buffer[11] << 11) | data_buffer[12];

                    if(s_ems_frame_res.power != s_last_power){
                        uint32_t issue_power = s_ems_frame_res.power *100;
                        s_power_data_received_flag = 1;                             //调节功率发生改变，收到数据收到数据标志位置1，进行功率调节

                        if(issue_power > sys_query_system_max_power()){
                            issue_power = sys_query_system_max_power();
                        }
                        sys_sync_config_item_content(CONFIG_ITEM_SYSTEM_POWER_TOTAL, &issue_power, sizeof(issue_power));
                        s_last_power = s_ems_frame_res.power;
                    }
                }
                rlen = 0;
            }
        }else{
            rlen = 0;
            continue;
        }
     }
#if 0
        if (s_terminal_data.length >= sizeof(s_terminal_data.buffer)) {
            s_terminal_data.length = 0;
            LOG_E("terminal receive data out of buff size|%d", sizeof(s_terminal_data.buffer));
        }
        s_terminal_data.buffer[s_terminal_data.length++] = ch;

        if (s_terminal_data.buffer[0] != 'U') {
            memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
            continue;
        }
        else
        {
            if (4 > s_terminal_data.length){
                continue;
            }
            else
            {
                if ((s_terminal_data.buffer[1] != 'C') || (s_terminal_data.buffer[2] != 'A') || (s_terminal_data.buffer[3] != 'C'))
                {
                    memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
                    continue;
                }
            }
        }

        if ((s_terminal_data.buffer[s_terminal_data.length - 1] == '=') && (0 == count)) {
            count = s_terminal_data.length - 1;
            LOG_D("CHAR '=' INDEX IS -> %d", count);
        }

        if (!((s_terminal_data.buffer[s_terminal_data.length - 1] == 0x0A) && (s_terminal_data.buffer[s_terminal_data.length - 2] == 0x0D))) {
            continue;
        }

        if (0 == count) {
            LOG_D("terminal read data");
            if (0 == memcmp(s_terminal_data.buffer, "UCACRDpileInfo", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDpileInfo OK\r\n");
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%s%s", "pileNum=", (char *)(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0)), "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d.%d.%d%s", "version=", SOFTWARE_VERSION, SOFTWARE_SUBVERSION, SOFTWARE_REVISION, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%s%s", "ipv4=", (char *)(sys_read_config_item_content(CONFIG_ITEM_IP_DOMAIN, 0)), "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%u%s", "port=", *((unsigned int*)(sys_read_config_item_content(CONFIG_ITEM_PORT, 0))), "\r\n");
                terminal_send_data((char *)&data);
                LOG_D("terminal read is UCACRDpileInfo");
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACRDpileStatus", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDpileStatus OK\r\n");
                memset(data, 0x00, sizeof(data));

                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    switch (get_ofsm_info(gunno)->state) {
                    case APP_OFSM_STATE_WAIT_NET:
                        state = 7;
                        break;
                    case APP_OFSM_STATE_IDLEING:
                        state = 0;
                        break;
                    case APP_OFSM_STATE_READYING:
                        state = 1;
                        break;
                    case APP_OFSM_STATE_STARTING:
                        state = 5;
                        break;
                    case APP_OFSM_STATE_CHARGING:
                        state = 3;
                        break;
                    case APP_OFSM_STATE_STOPING:
                        state = 6;
                        break;
                    case APP_OFSM_STATE_FINISHING:
                        state = 3;
                        break;
                    case APP_OFSM_STATE_FAULTING:
                        state = 4;
                        break;
                    default:
                        state = 0xFF;
                        break;
                    }
                    sprintf((char *)&data, "%s%c%c%d%s", "pileStatus Gun", ('A' + gunno), '=', state, "\r\n");
                    terminal_send_data((char *)&data);
                }
                LOG_D("terminal read is UCACRDpileStatus");
            }else if (0 == memcmp(s_terminal_data.buffer, "UCACRDchargeInfo", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDchargeInfo OK\r\n");
                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%d%s", "cc1Volt Gun", ('A' + gunno), '=', mw_get_cc1_value(mw_get_cc1(gunno)), "\r\n");
                    terminal_send_data((char *)&data);
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%.2f%s", "chargeVolt Gun", ('A' + gunno), '=', get_ofsm_info(gunno)->base.voltage_a / 100.0f, "\r\n");
                    terminal_send_data((char *)&data);
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%.2f%s", "chargeCurr Gun", ('A' + gunno), '=', get_ofsm_info(gunno)->base.current_a / 100.0f, "\r\n");
                    terminal_send_data((char *)&data);
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%.3f%s", "chargePower Gun", ('A' + gunno), '=', get_ofsm_info(gunno)->base.power_a / 10.0f, "\r\n");
                    terminal_send_data((char *)&data);
                }
                LOG_D("terminal read is UCACRDchargeInfo");
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACRDpileConfig", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDpileConfig OK\r\n");
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "Leakage=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "adhesionSingle=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "adhesionDouble=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "gndCheck=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "shortCheck=", 0, "\r\n");
                terminal_send_data((char *)&data);
                LOG_D("terminal read is UCACRDpileConfig");
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACRDfaultInfo", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDfaultInfo OK\r\n");
                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%d%s", "faultCode Gun", ('A' + gunno), '=', (uint8_t)app_get_highest_priority_system_fault(gunno), "\r\n");
                    terminal_send_data((char *)&data);
                }
                LOG_D("terminal read is UCACRDfaultInfo");
            }
        } else {
            LOG_D("terminal write data (%d)", count);
            if (0 == memcmp(s_terminal_data.buffer, "UCACWDpileNum", count)) {
                LOG_D("terminal write is UCACWDpileNum");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';

                if (0 > sys_sync_config_item_content(CONFIG_ITEM_PILE_NUMBER, data, j)) {
                    terminal_send_data("ACUCWDpileNum ERROR\r\n");
                    LOG_D("terminal configure charging-pile number failed");
                } else {
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDpileNum ERROR\r\n");
                        LOG_D("terminal configure charging-pile number failed");
                    }else{
                        terminal_send_data("ACUCWDpileNum OK\r\n");
                        LOG_D("terminal configure charging-pile number success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDqrCode", count)) {
                LOG_D("terminal write is UCACWDqrCode");
                data[0] = CP_SET_QRCODE_FORMAT_PREFIX;
                data[1] = CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[2 + j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                if(sys_sync_config_item_content(CONFIG_ITEM_QRCODE_PRE, data, j) < 0){
                    terminal_send_data("ACUCWDqrCode ERROR\r\n");
                    LOG_D("terminal configure qrcode failed");
                }else{
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDqrCode ERROR\r\n");
                        LOG_D("terminal configure qrcode failed");
                    }else{
                        terminal_send_data("ACUCWDqrCode OK\r\n");
                        LOG_D("terminal configure qrcode success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDip", count)) {
                LOG_D("terminal write is UCACWDip");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i]; /* 获取数据 */
                }
                data[j] = '\0';
                if (0 > sys_sync_config_item_content(CONFIG_ITEM_IP_DOMAIN, data, j)) {
                    terminal_send_data("ACUCWDip ERROR\r\n");
                    LOG_D("terminal configure ip addr failed");
                } else {
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDip ERROR\r\n");
                        LOG_D("terminal configure ip addr failed");
                    }else{
                        terminal_send_data("ACUCWDip OK\r\n");
                        LOG_D("terminal configure ip addr success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDport", count)) {
                uint32_t port = 0;

                LOG_D("terminal write is UCACWDport");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                port = (uint32_t)(atoi((const char *)data));

                if (0 > sys_sync_config_item_content(CONFIG_ITEM_PORT, &port, j)) {
                    terminal_send_data("ACUCWDport ERROR\r\n");
                    LOG_D("terminal configure port addr failed");
                } else {
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDport ERROR\r\n");
                        LOG_D("terminal configure port addr failed");
                    }else{
                        terminal_send_data("ACUCWDport OK\r\n");
                        LOG_D("terminal configure port addr success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDleakage", count)) {
                LOG_D("terminal write is UCACWDleakage");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDleakage ERROR\r\n");
                    LOG_D("terminal configure leakage failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDleakage ERROR\r\n");
                        LOG_D("terminal configure leakage failed");
                    } else {
                        terminal_send_data("ACUCWDleakage OK\r\n");
                        LOG_D("terminal configure leakage success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDadhesionSingle", count)) {
                LOG_D("terminal write is UCACWDadhesionSingle");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDadhesionSingle ERROR\r\n");
                    LOG_D("terminal configure single-adhesion failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDadhesionSingle ERROR\r\n");
                        LOG_D("terminal configure single-adhesion failed");
                    } else {
                        terminal_send_data("ACUCWDadhesionSingle OK\r\n");
                        LOG_D("terminal configure single-adhesion success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDadhesionDouble", count)) {
                LOG_D("terminal write is UCACWDadhesionDouble");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDadhesionDouble ERROR\r\n");
                    LOG_D("terminal configure double-adhesion failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDadhesionDouble ERROR\r\n");
                        LOG_D("terminal configure double-adhesion failed");
                    } else {
                        terminal_send_data("ACUCWDadhesionDouble OK\r\n");
                        LOG_D("terminal configure double-adhesion success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDgndCheck", count)) {
                LOG_D("terminal write is UCACWDgndCheck");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDgndCheck ERROR\r\n");
                    LOG_D("terminal configure ground failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDgndCheck ERROR\r\n");
                        LOG_D("terminal configure ground failed");
                    } else {
                        terminal_send_data("ACUCWDgndCheck OK\r\n");
                        LOG_D("terminal configure ground success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDshort", count)) {
                LOG_D("terminal write is UCACWDshort");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDshort ERROR\r\n");
                    LOG_D("terminal configure short-circuit failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDshort ERROR\r\n");
                        LOG_D("terminal configure short-circuit failed");
                    } else {
                        terminal_send_data("ACUCWDshort OK\r\n");
                        LOG_D("terminal configure short-circuit success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDADDWhiteList", count)) {
                LOG_D("terminal write is UCACWDADDWhiteList");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                if (1) {
                    terminal_send_data("ACUCWDADDWhiteList ERROR\r\n");
                    LOG_D("terminal append white-list failed");
                } else {
                    terminal_send_data("ACUCWDADDWhiteList OK\r\n");
                    LOG_D("terminal append white-list success");
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDDELWhiteList", count)) {
                LOG_D("terminal write is UCACWDDELWhiteList");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                if (1) {
                    terminal_send_data("ACUCWDDELWhiteList ERROR\r\n");
                    LOG_D("terminal delete white-list failed");
                } else {
                    terminal_send_data("ACUCWDDELWhiteList OK\r\n");
                    LOG_D("terminal delete white-list success");
                }
            } else {
                LOG_D("terminal write data error");
            }
        }

        count = 0;
        memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
#endif
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/

