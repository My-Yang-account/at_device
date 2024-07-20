
#include <rtthread.h>
#include "string.h"

#include "app_card.h"
#include "app_ofsm.h"
#include "app_osupport.h"
#include "app_support_func.h"
#include "bsp_serial.h"
#include "chargepile_config.h"

#define DBG_TAG "app.card"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct card{
    uint16_t data_count;
    uint8_t data[64];

    uint8_t card_uid_len;
    uint8_t card_uid[8];
    uint8_t card_numbe[16];
    uint8_t card_numbe_len;
    uint8_t info_type;
    uint8_t state;

    uint8_t port;
};
static struct card s_card_info;
static uint8_t s_reader_error_count;

static uint8_t s_card_active[2] = {0x00, 0x26};
static uint8_t s_card_info_sector = 0x09;
static uint8_t s_thaisen_card_key[6] = {0x41, 0x31, 0x53, 0x4D, 0x31, 0x50};
static uint8_t s_ykc_card_key[6] = {0x72, 0x28, 0x92, 0x63, 0x46, 0x23};

static enum card_state s_card_state = CARD_STATE_DOWN;
static enum buzzon_state s_buzzon_state_array[BUZZON_STATE_MAX_COUNT];
static enum buzzon_state s_buzzon_state = BUZZON_STATE_NULL;
struct rt_messagequeue g_buzzon_mq;

void buzzer_ipc_init(void)
{
    rt_mq_init(&g_buzzon_mq, "buzzon_mq", s_buzzon_state_array, sizeof(enum buzzon_state), sizeof(s_buzzon_state_array), RT_IPC_FLAG_PRIO);
}

static int16_t reader_get_char(void)
{
    uint8_t ch = 0x00;
    int8_t result = 0;

    while (rt_device_read(g_reader_serial, -1, &ch, 1) != 1) {
        result = rt_sem_take(&g_reader_rx_sem, SEARCH_CARD_PERIOD);
        if(result != RT_EOK){
            return -1;
        }
    }
    return ch;
}

static int8_t readline_data_from_reader_uart(void)
{
    char ch;
    int16_t result = 0;
    uint16_t data_len = 0;
    uint16_t frame_check = 0, calculate_check = 0;
    s_card_info.data_count = 0;

    while(1)
    {
        result = reader_get_char();
        if(result < 0){               /* 读卡器未回复 */
            return -1;
        }
        ch = (uint8_t)result;

//        LOG_D("receive data|0x%x", ch);

        if(ch != DEVICE_ADDRESS && s_card_info.data_count == 0){
            continue;
        }
        s_card_info.data[s_card_info.data_count++] = ch;

        if(s_card_info.data_count == 8){
            data_len = calculate_data_from_byte(s_card_info.data + 6, 2, START_FROM_HIGH_BYTE);
        }

        if(s_card_info.data_count >= 8 + data_len + 2){
            frame_check = calculate_data_from_byte(s_card_info.data + 8 + data_len, 2, START_FROM_HIGH_BYTE);
            for(uint8_t count = 0; count < 8 + data_len; count++){
                calculate_check += s_card_info.data[count];
            }
            if(frame_check != (uint16_t)(~calculate_check)){
                LOG_E("check code error|%x |%x", frame_check, (uint16_t)(~calculate_check));
                s_card_info.data_count = 0;
                continue;
            }
            break;
        }
    }
    return 0;
}


static void send_frame_to_reader(uint16_t cmd, uint8_t* data, uint16_t len)
{
    uint16_t check_code = 0;
    uint8_t request[len + 0x0A];

    request[0] = 0xB2;
    request[1] = 0x00;
    request[2] = 0x00;
    request[3] = MIFARE_S50_S70_CLASS;
    request[4] = cmd;
    request[5] = cmd >>8;
    request[6] = len;
    request[7] = len >>8;

    for(uint16_t count = 0; count < len; count++){
        request[8 + count] = data[count];
    }
    check_code = get_check_sum(request, len + 0x0A - 0x02);
    check_code = ~check_code;

    request[len + 0x0A - 0x02] = check_code;
    request[len + 0x0A - 0x01] = check_code >>8;

    reader_send_data(request, len + 0x0A);
}

static int8_t reader_offline_judge(void)
{
    if(readline_data_from_reader_uart() < 0){  /* 读卡器未回复 */
        s_reader_error_count++;
        if(s_reader_error_count >= READER_OFFLINE){
            s_reader_error_count = 0;
            s_card_state = CARD_STATE_OFFLINE;
            if(*(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0)) == 1){
                for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    LOG_E("card reader offline");
                    app_set_system_fault_enum(APP_SYS_FAULT_CARD_READER, APP_GENERAL_SYSTEM_FAULT_SET_LOW, gunno);
                }
            }
        }
        return -1;
    }
    s_reader_error_count = 0;
    return 0;
}

void card_thread_entry(void *parameter)
{
    (void)parameter;

    uint8_t reader_exit_count = 0;   /* 读卡器存在 */
    uint8_t card_leave_count = 0;    /* 卡离场 */
    uint16_t operation_state = 0;    /* 指令操作回复状态 */

    while (1)
    {
        switch (get_ofsm_info(0x00)->base.ota_state) {
        case APP_OTA_STATE_NULL:
            break;
        case APP_OTA_STATE_UP:
        case APP_OTA_STATE_LINK_UP:
        case APP_OTA_STATE_INTERNET_UP:
            break;
        case APP_OTA_STATE_AUTHING:
            break;
        case APP_OTA_STATE_AUTH_SUCCESS:
        case APP_OTA_STATE_UPDATEING:
            rt_thread_mdelay(100);
            continue;
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        if((*(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0))) != 1){
            for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                app_clear_system_fault_enum(APP_SYS_FAULT_CARD_READER, APP_GENERAL_SYSTEM_FAULT_SET_LOW, gunno);
            }
            rt_thread_mdelay(1000);
            continue;                      /* 读卡器不配置将不进行寻卡操作 */
        }

        if(rt_mq_recv(&g_buzzon_mq, &s_buzzon_state, sizeof(enum buzzon_state), 0) == RT_EOK){
            send_frame_to_reader(CARD_BUZZER, &s_buzzon_state, 1);  /* 蜂鸣器 */
            reader_offline_judge();
        }

        switch (s_card_state) {
        case CARD_STATE_DOWN:
            send_frame_to_reader(CARD_ACTIVE, s_card_active, sizeof(s_card_active));  /* 卡激活(寻卡) */
            if(reader_offline_judge() < 0){
                break;   /* 读卡器未回复 */
            }
            operation_state = calculate_data_from_byte(&s_card_info.data[RESPONSE_FRAME_REGION_STATE], 2, START_FROM_HIGH_BYTE);
            /* 寻到卡 */
            if(operation_state == OPERATION_STATE_SUCCESS ){
                s_card_info.card_uid_len = s_card_info.data[RESPONSE_FRAME_REGION_UID_LEN];
                s_card_info.card_uid_len = s_card_info.card_uid_len > sizeof(s_card_info.card_uid) ? sizeof(s_card_info.card_uid) : s_card_info.card_uid_len;

                memset(s_card_info.card_uid, 0x00, sizeof(s_card_info.card_uid));
                memcpy(s_card_info.card_uid, &s_card_info.data[RESPONSE_FRAME_REGION_UID], s_card_info.card_uid_len);

                s_card_state = CARD_STATE_IDLE;
            }
            break;
        case CARD_STATE_IDLE:
        {
            uint8_t card_key_authen[12];
            card_key_authen[0] = 0x60;
            card_key_authen[11] = 0x02;

            memcpy((card_key_authen + 1), s_card_info.card_uid, s_card_info.card_uid_len);
            memcpy((card_key_authen + 1 + s_card_info.card_uid_len), s_thaisen_card_key, 6);

            send_frame_to_reader(CARD_KEY_AUTHEN, card_key_authen, sizeof(card_key_authen));  /* 密钥直接认证 */
            if(reader_offline_judge() < 0){
                break;   /* 读卡器未回复 */
            }
            operation_state = calculate_data_from_byte(s_card_info.data + 4, 2, START_FROM_HIGH_BYTE);
            if(operation_state != OPERATION_STATE_SUCCESS){
                rt_thread_mdelay(100);
                send_frame_to_reader(CARD_ACTIVE, s_card_active, sizeof(s_card_active));  /* 卡激活(寻卡) */
                if(reader_offline_judge() < 0){
                    break;   /* 读卡器未回复 */
                }
                operation_state = calculate_data_from_byte(&s_card_info.data[RESPONSE_FRAME_REGION_STATE], 2, START_FROM_HIGH_BYTE);
                /* 寻到卡 */
                if(operation_state == OPERATION_STATE_SUCCESS ){
                    memcpy((card_key_authen + 1 + s_card_info.card_uid_len), s_ykc_card_key, 6);

                    send_frame_to_reader(CARD_KEY_AUTHEN, card_key_authen, sizeof(card_key_authen));  /* 密钥直接认证 */
                    if(reader_offline_judge() < 0){
                        break;   /* 读卡器未回复 */
                    }
                    operation_state = calculate_data_from_byte(s_card_info.data + 4, 2, START_FROM_HIGH_BYTE);
                }
            }

            /* 卡密钥认证成功 */
            if(operation_state == OPERATION_STATE_SUCCESS){
                s_card_state = CARD_STATE_ACTIVATION;
                LOG_D("card key authen success");
            }else{
                s_card_info.info_type = CARD_INFO_TYPE_CARD_UID;
                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    if(get_current_port() == gunno){
                        s_card_info.state |= (1 <<gunno);
                    }
                }
                s_card_state = CARD_STATE_OFFFIELD;
                LOG_D("card key authen fail");
            }
        }
            break;
        case CARD_STATE_ACTIVATION:
            send_frame_to_reader(CARD_READ_INFO, &s_card_info_sector, sizeof(s_card_info_sector));  /* 读取卡信息 */
            if(reader_offline_judge() < 0){
                break;   /* 读卡器未回复 */
            }
            operation_state = calculate_data_from_byte(&s_card_info.data[RESPONSE_FRAME_REGION_STATE], 2, START_FROM_HIGH_BYTE);
            /* 已成功获取到卡信息 */
            if(operation_state == OPERATION_STATE_SUCCESS){
                memcpy(s_card_info.card_numbe, s_card_info.data + 8, 16);  /* 获取卡号 */
                s_card_info.info_type = CARD_INFO_TYPE_CARD_NUMBER;
                s_card_info.card_numbe_len = 16;
                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    if(get_current_port() == gunno){
                        s_card_info.state |= (1 <<gunno);
                    }
                }
            }else{
                LOG_E("read card number fail!!");
            }
            s_card_state = CARD_STATE_OFFFIELD;
            break;
        case CARD_STATE_OFFFIELD:
            send_frame_to_reader(CARD_ACTIVE, s_card_active, sizeof(s_card_active));  /* 卡激活(寻卡)  */
            if(reader_offline_judge() < 0){
                break;   /* 读卡器未回复 */
            }
            operation_state = calculate_data_from_byte(&s_card_info.data[RESPONSE_FRAME_REGION_STATE], 2, START_FROM_HIGH_BYTE);
            if(operation_state != OPERATION_STATE_SUCCESS){
                card_leave_count++;
            }else{
                card_leave_count = 0;
            }
            if(card_leave_count >= CARD_LEAVE_COUNT){
                s_card_state = CARD_STATE_DOWN;
                card_leave_count = 0;
                LOG_D("card have leaved reader");
            }
            break;
        case CARD_STATE_OFFLINE:
            send_frame_to_reader(CARD_ACTIVE, s_card_active, sizeof(s_card_active));  /* 卡激活(寻卡)  */
            if(readline_data_from_reader_uart() < 0){
                reader_exit_count = 0;
                break;
            }
            reader_exit_count++;
            if(reader_exit_count > READER_EXIT_COUNT){
                reader_exit_count = 0;
                s_card_state = CARD_STATE_DOWN;
                LOG_D("reader is exit!!");
                for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    app_clear_system_fault_enum(APP_SYS_FAULT_CARD_READER, APP_GENERAL_SYSTEM_FAULT_SET_LOW, gunno);
                }
            }
            break;
        default:
            break;
        }
        rt_thread_mdelay(SEARCH_CARD_PERIOD);
    }
}

/*************************************
 * 函数名            get_card_number
 * 功能               获取卡号
 ************************************/
uint8_t* get_card_number(void)
{
    return s_card_info.card_numbe;
}
/*************************************
 * 函数名            get_card_number_len
 * 功能               获取卡号长度
 ************************************/
uint8_t get_card_number_len(void)
{
    return s_card_info.card_numbe_len;
}

/*************************************
 * 函数名            get_card_uid
 * 功能               获取卡UID
 ************************************/
uint8_t* get_card_uid(void)
{
    return s_card_info.card_uid;
}
/*************************************
 * 函数名            get_card_uid_len
 * 功能               获取卡UID长度
 ************************************/
uint8_t get_card_uid_len(void)
{
    return s_card_info.card_uid_len;
}

/*************************************
 * 函数名            get_card_info_type
 * 功能               获取刷卡信息类型
 ************************************/
uint8_t get_card_info_type(void)
{
    return s_card_info.info_type;
}

/*************************************
 * 函数名            获取刷卡状态
 * 功能               获取卡号
 ************************************/
uint8_t get_swipe_card_state(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0;
    }
    return (s_card_info.state &(1 <<gunno));
}
/*************************************
 * 函数名            clear_swipe_card_state
 * 功能               清除刷卡状态
 ************************************/
void clear_swipe_card_state(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    s_card_info.state &= (~(1 <<gunno));
}

/*************************************
 * 函数名            get_current_port
 * 功能               获取当前端口
 ************************************/
uint8_t get_current_port(void)
{
    return s_card_info.port;
}
/*************************************
 * 函数名          set_current_port
 * 功能              设置当前端口
 ************************************/
void set_current_port(uint8_t port)
{
    if(port >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    s_card_info.port = port;
}




