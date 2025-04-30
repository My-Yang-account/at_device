/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-04     我的杨yang       the first version
 */
#ifndef RFID_READER_INC_APP_RFID_READER_H_
#define RFID_READER_INC_APP_RFID_READER_H_

#include "rfid_dev_api.h"

//#define RFIDR_USING_XJ_CARD                                                            /** 使用小桔的卡 */

#define RFIDR_NODE_RUNNING_OPTION_ENTRY_MAX             (1 <<0x01)                     /** 节点运行选项字：最大容忍次数 */
#define RFIDR_NODE_RUNNING_OPTION_URGENT                (1 <<0x02)                     /** 节点运行选项字：紧急(无需判断，直接处理) */
#define RFIDR_NODE_RUNNING_OPTION_NAME                  (1 <<0x03)                     /** 节点运行选项字：线程名字 */

#define RFIDR_MAIL_NUM_MAX                               0x05                          /** 射频读卡器邮箱邮件数量 */
#define RFIDR_THREAD_STACK_SIZE                          2048                          /** 射频读卡器线程栈大小(B) */

#define RFIDR_UUID_LEN_MAX                               0x08                          /** 卡UUID长度 */
#define RFIDR_CARD_NUMBER_LEN_MAX                        0x10                          /** 卡号长度 */

enum{
    APP_RFIDR_STATE_DOWN = 0,                                                          /** 掉电状态（属于正常状态，RST引脚为低电平） */
    APP_RFIDR_STATE_IDLE,                                                              /** 闲置状态 */
    APP_RFIDR_STATE_ACTIVATION,                                                        /** 激活状态 */
    APP_RFIDR_STATE_RW_INFO,                                                           /** 读写信息 */
    APP_RFIDR_STATE_OFFFIELD,                                                          /** 离场状态 */
    APP_RFIDR_STATE_OFFLINE,                                                           /** 离线状态（属于异常状态） */
    APP_RFIDR_STATE_SIZE,
};

enum{
    APP_RFIDR_INFO_TYPE_UUID = 0x01,                                                   /** 卡信息类型：UUID */
    APP_RFIDR_INFO_TYPE_CARD_NUMBER = 0x02,                                            /** 卡信息类型：卡号 */
    APP_RFIDR_INFO_TYPE_ERROR = 0x04,                                                  /** 卡信息类型：读取卡号错误 */
    APP_RFIDR_INFO_TYPE_RW_FAIL = 0x08,                                                /** 卡信息类型：卡信息读写失败 */
};

enum{
    APP_RFIDR_OFFLINE,                                                                 /** 射频读卡器离线 */
    APP_RFIDR_ONLINE,                                                                  /** 射频读卡器在线 */
};

enum{
    APP_RFIDR_ENUM_FALSE,
    APP_RFIDR_ENUM_TRUE,
};

enum{
#ifdef RFIDR_USING_XJ_CARD
    APP_RFIDR_KEY_TYPE_XJ,                                                            /** 卡密钥类型：小桔 */
#else
    APP_RFIDR_KEY_TYPE_THA,                                                            /** 卡密钥类型：钛昕 */
    APP_RFIDR_KEY_TYPE_YKC,                                                            /** 卡密钥类型：云快充 */
#endif /* RFIDR_USING_XJ_CARD */
    APP_RFIDR_KEY_TYPE_SIZE,
};

typedef struct{
    struct{
        unsigned char is_forbid : 1;                                                   /** 禁止运行 */
        unsigned char enable_fdetect : 1;                                              /** 使能故障检测 */
    }flag;
    unsigned char current_port;                                                        /** 当前端口(枪号) */
    unsigned char key_type;                                                            /** 卡密钥类型 */
    unsigned char info_type;                                                           /** 刷卡信息类型 */
    unsigned char swip_state;                                                          /** 刷卡状态 */
    unsigned char uuid_len;                                                            /** 卡UUID长度 */
    unsigned char uuid[RFIDR_UUID_LEN_MAX];                                            /** 卡UUID */
    unsigned char card_number_len;                                                     /** 卡号长度 */
    unsigned char card_number[RFIDR_CARD_NUMBER_LEN_MAX];                              /** 卡号 */
    void (*fault)(unsigned char status);                                               /** 故障设置与恢复 */
    int (*node_init)(void *node, void *para, unsigned int plen, unsigned int option);  /** 节点初始化 */
    int (*node_running)(void *node, void *para, unsigned int plen, unsigned int option); /** 节点运行(外部调用) */
    void (*data_update)(void *handle);                                                 /** 数据更新(外部调用) */
    int (*active_card)(void);                                                          /** 寻卡(外部调用) */
    int (*bolck_read)(unsigned char sector, unsigned char bolck, unsigned char *buf, unsigned char blen);    /** 读块数据(外部调用) */
    int (*bolck_write)(unsigned char sector, unsigned char bolck, unsigned char *data, unsigned char dlen);  /** 写块数据(外部调用) */
    int (*key_authenticate)(unsigned char sector, unsigned char block, unsigned char *key, unsigned char klen);  /** 扇区密钥验证(外部调用) */
    int (*info_process)(void *handle);                                                 /** 信息处理(卡鉴权通过、进入读、写块状态后自动调用) */

}rfid_reader;

/*********************************************************************
 * 函数名        app_rfidr_set_current_port
 * 功能            设置当前端口
 * 参数            port   端口号
 * 返回
 ********************************************************************/
void app_rfidr_set_current_port(unsigned char port);

/*********************************************************************
 * 函数名        app_rfidr_send_mail
 * 功能            给射频读卡器发送邮件
 * 参数            value    邮件值
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_send_mail(unsigned int value);

/*********************************************************************
 * 函数名        app_rfidr_config_handle_fault
 * 功能            配置故障设置、清除句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_fault(void *handle);

/*********************************************************************
 * 函数名        app_rfidr_config_handle_data_update
 * 功能            配置数据更新句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_data_update(void *handle);

/*********************************************************************
 * 函数名        app_rfidr_config_handle_info_process
 * 功能            配置信息处理句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_info_process(void *handle);

/*********************************************************************
 * 函数名        app_rfidr_config_handle_node_init
 * 功能            配置节点(线程)初始化回调句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_node_init(void *handle);

/*********************************************************************
 * 函数名        app_rfidr_config_handle_node_running
 * 功能            配置节点运行句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_config_handle_node_running(void *handle);

/*********************************************************************
 * 函数名        app_rfidr_init
 * 功能            初始化
 * 参数
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int app_rfidr_init(void);




/*********************************************************************
 * 函数名        rfidr_query_card_number
 * 功能            查询卡号
 * 参数
 * 返回           卡号
 ********************************************************************/
unsigned char* rfidr_query_card_number(void);

/*********************************************************************
 * 函数名        rfidr_query_card_number_len
 * 功能            查询卡号长度
 * 参数
 * 返回           卡号长度
 ********************************************************************/
unsigned char rfidr_query_card_number_len(void);

/*********************************************************************
 * 函数名        rfidr_query_uuid
 * 功能            查询卡UUID
 * 参数
 * 返回           卡UUID
 ********************************************************************/
unsigned char* rfidr_query_uuid(void);

/*********************************************************************
 * 函数名        rfidr_query_uuid_len
 * 功能            查询卡UUID长度
 * 参数
 * 返回           卡UUID长度
 ********************************************************************/
unsigned char rfidr_query_uuid_len(void);

/*********************************************************************
 * 函数名        rfidr_query_info_type
 * 功能            查询刷卡信息类型
 * 参数
 * 返回           信息类型
 ********************************************************************/
unsigned char rfidr_query_info_type(void);

/*********************************************************************
 * 函数名        rfidr_query_swipe_state
 * 功能            查询刷卡状态
 * 参数            port   端口(枪号)
 * 返回           非0：刷卡了    0：未刷卡
 ********************************************************************/
uint8_t rfidr_query_swipe_state(unsigned char port);

/*********************************************************************
 * 函数名        rfidr_clear_swipe_state
 * 功能            清除刷卡状态
 * 参数            port   端口(枪号)
 * 返回
 ********************************************************************/
void rfidr_clear_swipe_state(unsigned char port);


#endif /* RFID_READER_INC_APP_RFID_READER_H_ */
