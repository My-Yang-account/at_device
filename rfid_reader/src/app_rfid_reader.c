/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-04     我的杨yang       the first version
 */
#include "app_rfid_reader.h"
#include "rtthread.h"

#define DBG_TAG "app_rfidr"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define APP_RFIDR_DEBUG

#define RFIDR_CARD_KEY_NUM                            0x02                  /** 卡密钥个数 */

#define RFIDR_THREAD_PERIOD                           10                    /** 线程运行周期 */
#define RFIDR_DETECT_LEAVE_MAX                        20                    /** 检测卡离场次数(时基按10ms算) */
#define RFIDR_DETECT_EXIT_MAX                         10                    /** 检测卡存在次数(时基按10ms算) */
#define RFIDR_DETECT_OFFLINE_MAX                      10                    /** 检测读卡器离线次数 */

/** rfidr:rfid reader */
static rfid_reader s_rfidr_handle = {
        .is_forbid = APP_RFIDR_ENUM_FALSE,
};

static unsigned char s_rfidr_state = APP_RFIDR_STATE_DOWN;
static unsigned char s_rfidr_block = 0x09;     /** 卡号所在块 */
static unsigned char s_rfidr_sector = 0x02;     /** 卡号所在扇区 */
static unsigned char s_rfidr_card_key[RFIDR_CARD_KEY_NUM][0x06] = {{0x41, 0x31, 0x53, 0x4D, 0x31, 0x50}, {0x72, 0x28, 0x92, 0x63, 0x46, 0x23}};

static struct rt_mailbox s_rfidr_mailbox;
static unsigned long s_rfidr_mail, s_rfidr_mail_pool[RFIDR_MAIL_NUM_MAX];
static struct rt_thread s_rfidr_thread;
static unsigned char s_rfidr_thread_stack[RFIDR_THREAD_STACK_SIZE];


/*********************************************************************
 * 函数名        app_rfidr_read_block
 * 功能            射频读卡器读取块数据
 * 参数            bolck         块号
 *         buf           存放块数据的缓存
 *         blen          缓存长度
 * 返回            >=0：成功   <0：失败
 ********************************************************************/
static int app_rfidr_read_block(unsigned char bolck, unsigned char *buf, unsigned char blen)
{
    return rfid_dev_api_read_block_info(bolck, buf, blen);
}
/*********************************************************************
 * 函数名        app_rfidr_write_block
 * 功能            射频读卡器写块数据
 * 参数            bolck         块号
 *         buf           数据
 *         blen          数据长度
 * 返回            >=0：成功   <0：失败
 ********************************************************************/
static int app_rfidr_write_block(unsigned char bolck, unsigned char *data, unsigned char dlen)
{
    return rfid_dev_api_write_block_info(bolck, data, dlen);
}

/*********************************************************************
 * 函数名        rfidr_thread_entry
 * 功能            射频读卡器线程入口
 * 参数            parameter
 * 返回
 ********************************************************************/
static void rfidr_thread_entry(void *parameter)
{
    unsigned char uuid[RFIDR_UUID_LEN_MAX], uuid_len = 0x00;
    unsigned char leave_count = 0x00, exit_count = 0x00, offline_count = 0x00, key;
    int ret = APP_RFIDR_ENUM_FALSE;

    rt_thread_mdelay(1000);

    while(1){
        if(s_rfidr_handle.data_update){
            s_rfidr_handle.data_update(&s_rfidr_handle);
        }
        if(s_rfidr_handle.is_forbid){
            rt_thread_mdelay(3000);
            continue;
        }

        if(rt_mb_recv(&s_rfidr_mailbox, &s_rfidr_mail, 0x00) >= 0x00){
            rfid_dev_api_buzzer(s_rfidr_mail);
        }

        switch(s_rfidr_state){
        case APP_RFIDR_STATE_DOWN:
            ret = rfid_dev_api_active_card(uuid, RFIDR_UUID_LEN_MAX, &uuid_len);
            if(ret < APP_RFIDR_ENUM_FALSE){
                offline_count++;
                if(offline_count > RFIDR_DETECT_OFFLINE_MAX){
                    offline_count = 0x00;
                    s_rfidr_state = APP_RFIDR_STATE_OFFLINE;
                    if(s_rfidr_handle.fault){
                        s_rfidr_handle.fault(APP_RFIDR_OFFLINE);
                    }
                }
            }else if(ret == APP_RFIDR_ENUM_TRUE){
                offline_count = 0x00;
                s_rfidr_state = APP_RFIDR_STATE_IDLE;

                s_rfidr_handle.uuid_len = uuid_len;
                memcpy(s_rfidr_handle.uuid, uuid, RFIDR_UUID_LEN_MAX);
#ifdef APP_RFIDR_DEBUG
                LOG_D("rfid_uuid:");
                for(unsigned char i = 0; i < RFIDR_UUID_LEN_MAX; i++){
                    rt_kprintf("%02X ", s_rfidr_handle.uuid[i]);
                }
                rt_kprintf("\n");
#endif /*APP_RFIDR_DEBUG */
                if(s_rfidr_handle.fault){
                    s_rfidr_handle.fault(APP_RFIDR_ONLINE);
                }
            }else{
                offline_count = 0x00;
                if(s_rfidr_handle.fault){
                    s_rfidr_handle.fault(APP_RFIDR_ONLINE);
                }
            }
            break;
        case APP_RFIDR_STATE_IDLE:
            for(key = 0x00; key < APP_RFIDR_KEY_TYPE_SIZE; key++){
                ret = rfid_dev_api_key_authentication(s_rfidr_sector, s_rfidr_card_key[key], sizeof(s_rfidr_card_key[key]));
                if(ret >= 0x00){
                    s_rfidr_state = APP_RFIDR_STATE_ACTIVATION;
                    s_rfidr_handle.key_type = key;
                    LOG_D("rfidr card key authen success");
                    break;
                }else{
                    rt_thread_mdelay(100);
                    rfid_dev_api_active_card(uuid, RFIDR_UUID_LEN_MAX, &uuid_len);
                }
            }

            s_rfidr_handle.info_type = APP_RFIDR_INFO_TYPE_UUID;

            if(key >= APP_RFIDR_KEY_TYPE_SIZE){
                s_rfidr_state = APP_RFIDR_STATE_OFFFIELD;

                if(s_rfidr_handle.info_process){
                    ret = s_rfidr_handle.info_process(&s_rfidr_handle);
                }
                s_rfidr_handle.swip_state |= (1 <<s_rfidr_handle.current_port);

                LOG_D("rfidr card key authen fail\n");
            }
            break;
        case APP_RFIDR_STATE_ACTIVATION:
            ret = rfid_dev_api_read_block_info(s_rfidr_block, s_rfidr_handle.card_number, RFIDR_CARD_NUMBER_LEN_MAX);
            if(ret >= 0x00){
                s_rfidr_handle.info_type = APP_RFIDR_INFO_TYPE_CARD_NUMBER;
                s_rfidr_handle.card_number_len = RFIDR_CARD_NUMBER_LEN_MAX;

                s_rfidr_state = APP_RFIDR_STATE_RW_INFO;
#ifdef APP_RFIDR_DEBUG
                LOG_D("rfid_card number:");
                for(unsigned char i = 0; i < RFIDR_CARD_NUMBER_LEN_MAX; i++){
                    rt_kprintf("%02X ", s_rfidr_handle.card_number[i]);
                }
                rt_kprintf("\n");
#endif /* APP_RFIDR_DEBUG */
            }else{
                s_rfidr_handle.info_type |= APP_RFIDR_INFO_TYPE_ERROR;
                s_rfidr_handle.swip_state |= (1 <<s_rfidr_handle.current_port);
                s_rfidr_state = APP_RFIDR_STATE_OFFFIELD;

                LOG_D("rfidr read card number fail");
            }
            break;
        case APP_RFIDR_STATE_RW_INFO:
            if(s_rfidr_handle.info_process){
                ret = s_rfidr_handle.info_process(&s_rfidr_handle);
                if(ret < 0x00){
                    s_rfidr_handle.info_type = APP_RFIDR_INFO_TYPE_RW_FAIL;
                }
#ifdef APP_RFIDR_DEBUG
                LOG_D("rfidr rw info type(%d, %d)", ret, s_rfidr_handle.info_type);
#endif /* APP_RFIDR_DEBUG */
            }
            s_rfidr_handle.swip_state |= (1 <<s_rfidr_handle.current_port);

            s_rfidr_state = APP_RFIDR_STATE_OFFFIELD;
            break;
        case APP_RFIDR_STATE_OFFFIELD:
            ret = rfid_dev_api_active_card(uuid, RFIDR_UUID_LEN_MAX, &uuid_len);
            if(ret != APP_RFIDR_ENUM_TRUE){
                leave_count++;
            }else{
                leave_count = 0x00;
            }

            if(leave_count >= RFIDR_DETECT_LEAVE_MAX){
                s_rfidr_state = APP_RFIDR_STATE_DOWN;
                leave_count = 0x00;
                LOG_D("rfid card have leaved reader");
            }
            break;
        case APP_RFIDR_STATE_OFFLINE:
            ret = rfid_dev_api_active_card(uuid, RFIDR_UUID_LEN_MAX, &uuid_len);
            if(ret < APP_RFIDR_ENUM_FALSE){
                exit_count = 0x00;
                break;
            }

            exit_count++;
            if(exit_count > RFIDR_DETECT_EXIT_MAX){
                exit_count = 0;
                s_rfidr_state = APP_RFIDR_STATE_DOWN;
                if(s_rfidr_handle.fault){
                    s_rfidr_handle.fault(APP_RFIDR_ONLINE);
                }
                LOG_D("rfid reader is exit");
            }else{
                if(s_rfidr_handle.fault){
                    s_rfidr_handle.fault(APP_RFIDR_OFFLINE);
                }
            }
            break;
        default:
            break;
        }

        rt_thread_mdelay(RFIDR_THREAD_PERIOD);
    }
}

/*********************************************************************
 * 函数名        rfidr_thread_init
 * 功能            初始化射频读卡器线程
 * 参数
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
static int rfidr_thread_init(void)
{
    s_rfidr_handle.bolck_read = app_rfidr_read_block;
    s_rfidr_handle.bolck_write = app_rfidr_write_block;

    if(rt_mb_init(&s_rfidr_mailbox, "rfidrmb", s_rfidr_mail_pool, RFIDR_MAIL_NUM_MAX, RT_IPC_FLAG_PRIO) < 0x00){
        LOG_E("rfid reader mailbox create fail, please check");
        return -0x01;
    }

    if(rt_thread_init(&s_rfidr_thread, "rfidrtask", rfidr_thread_entry, NULL,
            s_rfidr_thread_stack, RFIDR_THREAD_STACK_SIZE, 16, 10) < 0x00){
        LOG_E("rfid reader thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_rfidr_thread) < 0x00){
        LOG_E("rfid reader thread startup fail, please check");
        return -0x01;
    }

    return 0x00;
}

/*********************************************************************
 * 函数名        app_rfidr_set_current_port
 * 功能            设置当前端口
 * 参数            port   端口号
 * 返回
 ********************************************************************/
void app_rfidr_set_current_port(unsigned char port)
{
    s_rfidr_handle.current_port = port;
}

/*********************************************************************
 * 函数名        app_rfidr_send_mail
 * 功能            给射频读卡器发送邮件
 * 参数            value    邮件值
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_send_mail(unsigned int value)
{
    return rt_mb_send(&s_rfidr_mailbox, value);
}

/*********************************************************************
 * 函数名        app_rfidr_config_handle_fault
 * 功能            配置故障设置、清除句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_fault(void *handle)
{
    s_rfidr_handle.fault = (void (*)(unsigned char))handle;
    return 0x00;
}

/*********************************************************************
 * 函数名        app_rfidr_config_handle_data_update
 * 功能            配置数据更新句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_data_update(void *handle)
{
    s_rfidr_handle.data_update = (void (*)(void*))handle;
    return 0x00;
}

/*********************************************************************
 * 函数名        app_rfidr_config_handle_info_process
 * 功能            配置信息处理句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_info_process(void *handle)
{
    s_rfidr_handle.info_process = (int (*)(void *))handle;
    return 0x00;
}

/*********************************************************************
 * 函数名        app_rfidr_init
 * 功能            初始化
 * 参数
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_init(void)
{
    int ret = 0x00;

    ret = rfid_dev_api_init();
    if(ret < 0x00){
        return ret;
    }
    ret = rfidr_thread_init();
    if(ret < 0x00){
        return ret;
    }

    return ret;
}





/*********************************************************************
 * 函数名        rfidr_query_card_number
 * 功能            查询卡号
 * 参数
 * 返回           卡号
 ********************************************************************/
unsigned char* rfidr_query_card_number(void)
{
    return s_rfidr_handle.card_number;
}

/*********************************************************************
 * 函数名        rfidr_query_card_number_len
 * 功能            查询卡号长度
 * 参数
 * 返回           卡号长度
 ********************************************************************/
unsigned char rfidr_query_card_number_len(void)
{
    return s_rfidr_handle.card_number_len;
}

/*********************************************************************
 * 函数名        rfidr_query_uuid
 * 功能            查询卡UUID
 * 参数
 * 返回           卡UUID
 ********************************************************************/
unsigned char* rfidr_query_uuid(void)
{
    return s_rfidr_handle.uuid;
}

/*********************************************************************
 * 函数名        rfidr_query_uuid_len
 * 功能            查询卡UUID长度
 * 参数
 * 返回           卡UUID长度
 ********************************************************************/
unsigned char rfidr_query_uuid_len(void)
{
    return s_rfidr_handle.uuid_len;
}

/*********************************************************************
 * 函数名        rfidr_query_info_type
 * 功能            查询刷卡信息类型
 * 参数
 * 返回           信息类型
 ********************************************************************/
unsigned char rfidr_query_info_type(void)
{
    return s_rfidr_handle.info_type;
}

/*********************************************************************
 * 函数名        rfidr_query_swipe_state
 * 功能            查询刷卡状态
 * 参数            port   端口(枪号)
 * 返回           非0：刷卡了    0：未刷卡
 ********************************************************************/
uint8_t rfidr_query_swipe_state(unsigned char port)
{
    return (s_rfidr_handle.swip_state &(0x01 <<port));
}

/*********************************************************************
 * 函数名        rfidr_clear_swipe_state
 * 功能            清除刷卡状态
 * 参数            port   端口(枪号)
 * 返回
 ********************************************************************/
void rfidr_clear_swipe_state(unsigned char port)
{
    s_rfidr_handle.swip_state &= (~(0x01 <<port));
}
