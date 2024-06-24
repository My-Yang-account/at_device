/**
 ******************************************************************************
 * @file app_card.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef APP_CARD_H_
#define APP_CARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define SEARCH_CARD_PERIOD      100                         /* 寻卡周期(单位 ms) */
#define READER_OFFLINE          5000 /SEARCH_CARD_PERIOD    /* 读卡器离线确认时间 ms */
#define READER_EXIT_COUNT       10                          /* 读卡器存在计数 */
#define CARD_LEAVE_COUNT        10                          /* 卡离场计数 */

#define BUZZON_STATE_MAX_COUNT   8

/* device addr */
#define DEVICE_ADDRESS    0xB3

/* region */
#define RESPONSE_FRAME_REGION_STATE           0x04          /* 响应帧状态域 */
#define RESPONSE_FRAME_REGION_UID_LEN         0x0B          /* 响应帧uid长度域 */
#define RESPONSE_FRAME_REGION_UID             0x0C          /* 响应帧uid域 */

enum buzzon_state {
    BUZZON_STATE_NULL = 0,
    BUZZON_STATE_OK,
    BUZZON_STATE_FAILED,  /* 平台鉴权卡片失败 */
    BUZZON_STATE_WARRING, /* 密钥验证或获取卡号失败，本地鉴权失败 */
    BUZZON_STATE_AUTHING, /* 平台正在鉴权卡片 */
    BUZZON_STATE_SUCCESS, /* 平台鉴权卡片成功 */
};
/* 卡信息类型 */
enum info_type{
    CARD_INFO_TYPE_CARD_UID,
    CARD_INFO_TYPE_CARD_NUMBER,
};
/* 读卡器指令 */
enum cmd{
    CARD_REQUEST = 0x41,
    CARD_KEY_AUTHEN = 0x46,
    CARD_READ_INFO = 0x47,
    CARD_ACTIVE = 0x4D,
    CARD_AUTO_DETECT = 0x4E,
    CARD_BUZZER = 0x5A,
};
/* 读卡器指令类型 */
enum cmd_type{
    DEVICE_CONTROL_CLASS = 0x01,
    MIFARE_S50_S70_CLASS,
    ISO7816_3_CLASS,
    ISO14443_PICC_CLASS,
    PLUS_CPU_CLASS,
    ISO15693_VICC_CLASS,
    ISO18000_6C_CLASS,
    ISO18092_NFCIP_1_CLASS,
    SGR_ID_CARD,
};
/* 读卡器指令操作结果 */
enum state{
    OPERATION_STATE_SUCCESS = 0,
    OPERATION_STATE_FAIL = -1,
};
/* 卡请求类型 */
enum card_atqa_type {
    CARD_ATQA_TYPE_MIFAREL1_S50 = 0x0004,
    CARD_ATQA_TYPE_MIFAREL1_S70 = 0x0002,
};
/* 卡指令响应类型 */
enum card_sak_type {
    CARD_SAK_TYPE_MIFAREL1_S50 = 0x08,
    CARD_SAK_TYPE_MIFAREL1_S70 = 0x18,
};

enum card_state {
    CARD_STATE_DOWN = 0,    /* 掉电状态（属于正常状态，RST引脚为低电平） */
    CARD_STATE_IDLE,        /* 闲置状态 */
    CARD_STATE_READY,       /* 准备状态 */
    CARD_STATE_ACTIVATION,  /* 激活状态 */
    CARD_STATE_FINISH,      /* 结束（完成）状态 */
    CARD_STATE_OFFFIELD,    /* 离场状态 */
    CARD_STATE_OFFLINE,     /* 离线状态（属于异常状态） */
    CARD_STATE_SIZE,
};

void buzzer_ipc_init(void);
void card_thread_entry(void *parameter);

uint8_t* get_card_number(void);
uint8_t get_card_number_len(void);

uint8_t* get_card_uid(void);
uint8_t get_card_uid_len(void);

uint8_t get_card_info_type(void);

uint8_t get_swipe_card_state(uint8_t gunno);
void clear_swipe_card_state(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* APP_CARD_H_ */
