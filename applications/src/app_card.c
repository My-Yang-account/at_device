
#include <rtthread.h>
#include "string.h"

#include "app_card.h"
#include "app_ofsm.h"
#include "app_osupport.h"
#include "app_support_func.h"
#include "app_data_info_interface.h"
#include "chargepile_config.h"
#include "mw_storage.h"
#include "notfs_cfg.h"

#define DBG_TAG "app.card"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define CARD_BLOCK_SIZE                                0x10          /* 卡一个块的大小(字节) */

#define CARD_OFFLINE_BILLING_SECTOR                    0x02          /* 离线计费卡信息扇区 */
#ifdef RFIDR_USING_XJ_CARD
#define CARD_XJ_CRC_SECTOR                             0x01          /* 小桔卡CRC信息扇区 */
#define CARD_RANDOM_NUMBER_SECTOR                      0x05          /* 小桔卡随机数信息扇区 */

#define CARD_XJ_CRC_SERIAL_NUMBER_BLOCK                0x04          /* 小桔卡计算CRC用序列数信息扇区 */
#define CARD_XJ_CRC_VALUE_BLOCK                        0x05          /* 小桔卡CRC信息扇区 */

#define CARD_RANDOM_NUMBER_1_BLOCK                     0x14          /* 小桔卡随机数1信息扇区 */
#define CARD_RANDOM_NUMBER_2_BLOCK                     0x15          /* 小桔卡随机数2信息扇区 */
#define CARD_RANDOM_NUMBER_3_BLOCK                     0x16          /* 小桔卡随机数3信息扇区 */

#define CARD_XJ_CRC_POLYNOM                            0xA001
#endif /* #ifdef RFIDR_USING_XJ_CARD */

#pragma pack(1)

union block{
    struct{
        uint32_t start_time;               /** 开始时间戳(s) */
        uint32_t ballance;                 /** 卡内余额(0.01元) */
        uint8_t ballance_check;            /** 卡内余额和校验 */
        uint8_t is_lock;                   /** 已锁卡 */
        uint8_t reserve[6];                /** 预留 */
    }detail;
    uint8_t data[16];
};

struct card_info_sector2{
    uint8_t device_id[16];                     /** 启动桩号 */
    uint8_t card_number[16];                   /** 卡号 */
    union block block_10;                      /** 块10 */
};

#ifdef RFIDR_USING_XJ_CARD
struct card_info_sector1{
    uint8_t serial_number[16];                 /** 计算CRC用序列数 */
    uint8_t crc[16];                           /** CRC */
};

struct card_info_sector5{
    uint8_t random_number_1[16];               /** 随机数1 */
    uint8_t random_number_2[16];               /** 随机数2 */
    uint8_t random_number_3[16];               /** 随机数3 */
};
#endif /* #ifdef RFIDR_USING_XJ_CARD */
#pragma pack()

#define APP_ENDIANNESS_CONVERT(value)        \
        (((value >>24) &0xff) |((value >>8) &0xff00) |((value <<8) &0xff0000) |((value <<24) &0xff000000))

APP_DEF_SRAM2 static rfid_reader *s_rfidr = NULL;
APP_DEF_SRAM2 static int8_t s_card_operate_ret[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM2 static struct card_info_sector2 s_card_info_sector2;
APP_DEF_SRAM2 static uint32_t s_card_ballance[APP_SYSTEM_GUNNO_SIZE];  /** 卡内余额(0.0001) */
APP_DEF_SRAM2 static struct rt_event s_card_event[APP_SYSTEM_GUNNO_SIZE];
#ifdef RFIDR_USING_XJ_CARD
APP_DEF_SRAM2 static struct card_info_sector1 s_card_info_sector1;
APP_DEF_SRAM2 static struct card_info_sector5 s_card_info_sector5;

RFID_DEF_SRAM2 static uint8_t s_card_sector1_key[0x06] = {0x34, 0x71, 0x4C, 0x80, 0x01, 0x77};
RFID_DEF_SRAM2 static uint8_t s_card_sector5_key[0x06] = {0x92, 0x5C, 0x9A, 0x4B, 0x83, 0x74};
#endif /* #ifdef RFIDR_USING_XJ_CARD */


#ifdef RFIDR_USING_XJ_CARD
/******************************************
 * 函数名     xj_card_crc16
 * 功能         计算小桔卡信息CRC16校验码
 * 参数         ptr     数据
 *       len     数据长度
 * 返回        CRC16校验码
 * ***************************************/
static uint16_t xj_card_crc16(uint8_t *ptr, uint16_t len)
{
    uint8_t i;
    uint16_t crc = 0xffff;

    if (len == 0x00){
        len = 1;
    }
    while(len--){
        crc ^= *ptr;
        for (i = 0x00; i < 0x08; i++){
            if (crc & 0x01){
                crc >>= 0x01;
                crc ^= CARD_XJ_CRC_POLYNOM;
            }else{
                crc >>= 0x01;
            }
        }
        ptr++;
    }
    return(crc);
}

/******************************************
 * 函数名     xj_card_info_verify
 * 功能         确认小桔卡信息
 * 参数         handle        卡操作句柄
 * 返回        1：校验成功        0：校验失败
 * ***************************************/
static uint8_t xj_card_info_verify(void *handle)
{
    uint8_t entry = 0x00, verify_data[20], *uuid = NULL;
    uint16_t verify_crc = 0x00, storage_crc;
    s_rfidr = (rfid_reader*)handle;

    memset(s_card_info_sector1.serial_number, 0x00, sizeof(s_card_info_sector1.serial_number));
    memset(s_card_info_sector1.crc, 0x00, sizeof(s_card_info_sector1.crc));

    memset(s_card_info_sector5.random_number_1, 0x00, sizeof(s_card_info_sector5.random_number_1));
    memset(s_card_info_sector5.random_number_2, 0x00, sizeof(s_card_info_sector5.random_number_2));
    memset(s_card_info_sector5.random_number_3, 0x00, sizeof(s_card_info_sector5.random_number_3));

    while(1){
        if(s_rfidr->active_card() <= 0x00){
            if(++entry >= 0x02){
                LOG_W("xj card is not found 0");
                return -0x01;
            }
            rt_thread_mdelay(10);
            continue;
        }
        break;
    }
    /** 扇区1 */
    if(s_rfidr->key_authenticate(CARD_XJ_CRC_SECTOR, CARD_XJ_CRC_SERIAL_NUMBER_BLOCK, s_card_sector1_key, sizeof(s_card_sector1_key)) < 0x00){
        LOG_W("xj card sector 1 key authenticate fail");
        return -0x01;
    }
    if(s_rfidr->bolck_read(CARD_XJ_CRC_SECTOR, CARD_XJ_CRC_SERIAL_NUMBER_BLOCK, s_card_info_sector1.serial_number, CARD_BLOCK_SIZE) < 0x00){
        LOG_W("xj card sector 1 block 0 data read fail");
        return -0x01;
    }
    if(s_rfidr->bolck_read(CARD_XJ_CRC_SECTOR, CARD_XJ_CRC_VALUE_BLOCK, s_card_info_sector1.crc, CARD_BLOCK_SIZE) < 0x00){
        LOG_W("xj card sector 1 block 1 data read fail");
        return -0x01;
    }

    entry = 0x00;
    while(1){
        if(s_rfidr->active_card() <= 0x00){
            if(++entry >= 0x02){
                LOG_W("xj card is not found 1");
                return -0x01;
            }
            rt_thread_mdelay(10);
            continue;
        }
        break;
    }
    /** 扇区5 */
    if(s_rfidr->key_authenticate(CARD_RANDOM_NUMBER_SECTOR, CARD_RANDOM_NUMBER_1_BLOCK, s_card_sector5_key, sizeof(s_card_sector5_key)) < 0x00){
        LOG_W("xj card sector 5 key authenticate fail");
        return -0x01;
    }
    if(s_rfidr->bolck_read(CARD_RANDOM_NUMBER_SECTOR, CARD_RANDOM_NUMBER_1_BLOCK, s_card_info_sector5.random_number_1, CARD_BLOCK_SIZE) < 0x00){
        LOG_W("xj card sector 5 block 0 data read fail");
        return -0x01;
    }
    if(s_rfidr->bolck_read(CARD_RANDOM_NUMBER_SECTOR, CARD_RANDOM_NUMBER_2_BLOCK, s_card_info_sector5.random_number_2, CARD_BLOCK_SIZE) < 0x00){
        LOG_W("xj card sector 5 block 1 data read fail");
        return -0x01;
    }
    if(s_rfidr->bolck_read(CARD_RANDOM_NUMBER_SECTOR, CARD_RANDOM_NUMBER_3_BLOCK, s_card_info_sector5.random_number_3, CARD_BLOCK_SIZE) < 0x00){
        LOG_W("xj card sector 5 block 2 data read fail");
        return -0x01;
    }

    memcpy(verify_data, s_card_info_sector1.serial_number, CARD_BLOCK_SIZE);
    uuid = rfidr_query_uuid();
    for(uint8_t i = 0x00; i < 0x04; i++){
        verify_data[CARD_BLOCK_SIZE + i] = uuid[0x04 - 0x01 - i];
    }
    verify_crc = xj_card_crc16(verify_data, (CARD_BLOCK_SIZE + 0x04));
    storage_crc = s_card_info_sector1.crc[CARD_BLOCK_SIZE - 0x02];
    storage_crc <<=0x08;
    storage_crc |= s_card_info_sector1.crc[CARD_BLOCK_SIZE - 0x01];

    if(verify_crc != storage_crc){
        return 0;
    }
    return 0x01;
}
#endif /* #ifdef RFIDR_USING_XJ_CARD */

/******************************************
 * 函数名     card_node_init_hook
 * 功能         读卡器部分线程初始化回调
 * 参数         node   节点句柄
 *      para     可选参数
 *      plen     参数长度(B)
 *      option   选项字
 * 返回
 * ***************************************/
static int32_t card_node_init_hook(void *node, void *para, uint32_t plen, uint32_t option)
{
    extern int32_t app_thread_monitor_add(void *thread, void *para, uint32_t plen, uint32_t option);

    if(option &RFIDR_NODE_RUNNING_OPTION_ENTRY_MAX){
        app_thread_monitor_add(node, para, plen, APP_THREAD_MONITOR_OPT_ENTRY);
    }else if(option &RFIDR_NODE_RUNNING_OPTION_NAME){
        app_thread_monitor_add(node, para, plen, APP_THREAD_MONITOR_OPT_NAME);
    }

    return 0x00;
}

/******************************************
 * 函数名     card_node_running
 * 功能         读卡器部分线程运行回调
 * 参数         node   节点句柄
 *      para     可选参数
 *      plen     参数长度(B)
 *      option   选项字
 * 返回
 * ***************************************/
static int32_t card_node_running(void *node, void *para, uint32_t plen, uint32_t option)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);
    uint32_t _option = 0x00;

    if(option &RFIDR_NODE_RUNNING_OPTION_URGENT){
        _option |= APP_THREAD_MONITOR_OPT_URGENT;
    }
    app_thread_monitor_process(node, NULL, 0x00, _option);

    return 0x00;
}

#ifdef APP_USING_OFFLINE_BILLING
/*****************************************************************************
 * 函数名                app_card_query_funpay_bill_fees_total
 * 功能                    查询历史订单中与指定UUID匹配的第一条订单的消费金额
 * 参数                    uuid   UUID
 *          ulen   UUID 长度
 *          stime  开始时间
 *          fees   用于保存订单的消费电量
 *          gunno  用于保存历史记录是哪把枪的
 * 返回                   >=0：成功    <0：失败
 ****************************************************************************/
static int32_t app_card_query_funpay_bill_fees_total(void *buf, uint16_t blen, uint8_t *uuid, uint8_t ulen, uint32_t stime, \
        uint32_t *fees, uint8_t *gunno, int32_t *index)
{
    if((fees == NULL) || (gunno == NULL) || (index == NULL)){
        return -0x01;
    }
    if((uuid == NULL) || ((ulen < 0x04) || (ulen > 0x08))){
        return -0x01;
    }
    if(blen < sizeof(thaisen_transaction_t)){
        return -0x01;
    }

    LOG_D("query unpay bill, uuid:");
    for(uint8_t count = 0x00; count < ulen; count++){
        LOG_D("%02x", uuid[count]);
    }

    uint8_t count = 0x00;
    int32_t sindex = -0x01, total_num = 0x00, result = 0x00;
    thaisen_transaction_t *transaction = (thaisen_transaction_t*)buf;

    if((total_num = mw_storage_record_get_unverify_record_num(RECORD_REGION_CHARGE_RECORDA)) > 0x00){
        LOG_D("gunno A unverify record num:%d", total_num);
        for(count = 0x00; count < total_num; count++){
            if(sindex >= 0x00){
                sindex = sindex > 0x00 ? (sindex - 0x01) : (NOTFS_ORDER_USER_FILE_MAX_COUNT - 0x01);
            }
            if((sindex = mw_storage_record_get_first_index_unverify(RECORD_REGION_CHARGE_RECORDA, sindex)) >= 0x00){
                result = mw_storage_record_get_designate_index_record((uint8_t*)transaction, sizeof(thaisen_transaction_t), RECORD_REGION_CHARGE_RECORDA, sindex);
                if((result == STORAGE_ERR_NONE) || (result == STORAGE_ERR_CHECK_ERROR)){
                    if(memcmp(uuid, transaction->physics_card_number, ulen) == 0x00){
                        struct ofsm_info *ofsm = get_ofsm_info(APP_SYSTEM_GUNNOA);

                        *index = sindex;
                        *gunno = APP_SYSTEM_GUNNOA;
                        *fees = transaction->total_fee;
                        ofsm->base.charge_time = transaction->charge_time;
                        ofsm->base.elect_a = transaction->total_elect;
                        ofsm->base.fees_total = transaction->total_fee;
                        ofsm->base.reason_code = transaction->stop_reason;
                        ofsm->base.current_soc = transaction->stop_soc;

                        LOG_D("history bill info:");
                        LOG_D("total_fee:%d", *fees);
                        LOG_D("charge_time:%d", ofsm->base.charge_time);
                        LOG_D("total_elect:%d", ofsm->base.elect_a);
                        LOG_D("total_fee:%d", ofsm->base.fees_total);
                        LOG_D("stop_reason:%d", ofsm->base.reason_code);
                        LOG_D("stop_soc:%d", ofsm->base.current_soc);
                        return 0x00;
                    }else{
                        LOG_D("card uuid no match in first unverify record gunno A [%02x, %02x, %02x, %02x][%02x, %02x, %02x, %02x]",
                                transaction->physics_card_number[0x00], transaction->physics_card_number[0x01],
                                transaction->physics_card_number[0x02], transaction->physics_card_number[0x03],
                                uuid[0x00], uuid[0x01], uuid[0x02], uuid[0x03]);
                    }
                }else{
                    LOG_D("get first unverify record data fail in gunno A (%d)", result);
                }
            }else{
                LOG_D("get first unverify record index fail in gunno A (%d)", sindex);
            }
        }
    }else{
        LOG_D("there is no unverify record is gunno A (%d)", total_num);
    }

#ifdef APP_USING_DOUBLEGUN
    sindex = -0x01;
    if((total_num = mw_storage_record_get_unverify_record_num(RECORD_REGION_CHARGE_RECORDB)) > 0x00){
        LOG_D("gunno B unverify record num:%d", total_num);
        for(count = 0x00; count < total_num; count++){
            if(sindex >= 0x00){
                sindex = sindex > 0x00 ? (sindex - 0x01) : (NOTFS_ORDER_USER_FILE_MAX_COUNT - 0x01);
            }
            if((sindex = mw_storage_record_get_first_index_unverify(RECORD_REGION_CHARGE_RECORDB, sindex)) >= 0x00){
                result = mw_storage_record_get_designate_index_record((uint8_t*)transaction, sizeof(thaisen_transaction_t), RECORD_REGION_CHARGE_RECORDB, sindex);
                if((result == STORAGE_ERR_NONE) || (result == STORAGE_ERR_CHECK_ERROR)){
                    if(memcmp(uuid, transaction->physics_card_number, ulen) == 0x00){
                        struct ofsm_info *ofsm = get_ofsm_info(APP_SYSTEM_GUNNOB);

                        *index = sindex;
                        *gunno = APP_SYSTEM_GUNNOB;
                        *fees = transaction->total_fee;
                        ofsm->base.charge_time = transaction->charge_time;
                        ofsm->base.elect_a = transaction->total_elect;
                        ofsm->base.fees_total = transaction->total_fee;
                        ofsm->base.reason_code = transaction->stop_reason;
                        ofsm->base.current_soc = transaction->stop_soc;

                        LOG_D("history bill info:");
                        LOG_D("total_fee:%d", *fees);
                        LOG_D("charge_time:%d", ofsm->base.charge_time);
                        LOG_D("total_elect:%d", ofsm->base.elect_a);
                        LOG_D("total_fee:%d", ofsm->base.fees_total);
                        LOG_D("stop_reason:%d", ofsm->base.reason_code);
                        LOG_D("stop_soc:%d", ofsm->base.current_soc);
                        return 0x00;
                    }else{
                        LOG_D("card uuid no match in first unverify record gunno B [%02x, %02x, %02x, %02x][%02x, %02x, %02x, %02x]",
                                transaction->physics_card_number[0x00], transaction->physics_card_number[0x01],
                                transaction->physics_card_number[0x02], transaction->physics_card_number[0x03],
                                uuid[0x00], uuid[0x01], uuid[0x02], uuid[0x03]);
                    }
                }else{
                    LOG_D("get first unverify record data fail in gunno B (%d)", result);
                }
            }else{
                LOG_D("get first unverify record index fail in gunno B (%d)", sindex);
            }
        }
    }else{
        LOG_D("there is no unverify record is gunno B (%d)", total_num);
    }
#endif /* APP_USING_DOUBLEGUN */

    return -0x01;
}


/*****************************************************************************
 *  函数名   app_card_swip_card_stop
 *  功能       充电中刷卡停处理
 *  参数      gunno    枪号
 * 返回      >=0：成功   <0：失败
 ****************************************************************************/
static int32_t app_card_swip_card_stop(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_CARD_OPERATE_RET_INTERNAL_ERROR;
    }

    LOG_D("gunno(%d) swip_card_stop", gunno);

    struct ofsm_info *ofsm = get_ofsm_info(gunno);
    uint8_t *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00), card_info_block, locked = s_card_info_sector2.block_10.detail.is_lock;
    uint32_t ballance = s_card_info_sector2.block_10.detail.ballance, stime = s_card_info_sector2.block_10.detail.start_time;

    s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_SUCCESS;

    memcpy(s_card_info_sector2.device_id, dev_id, CARD_BLOCK_SIZE);

    LOG_D("this charge infomation consume:%d, ballance:%d", ofsm->base.fees_total, s_card_info_sector2.block_10.detail.ballance);
    if(s_card_info_sector2.block_10.detail.ballance >= (ofsm->base.fees_total /100)){
        s_card_info_sector2.block_10.detail.ballance -= (ofsm->base.fees_total /100);
        s_card_info_sector2.block_10.detail.ballance = APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.ballance);
        s_card_info_sector2.block_10.detail.ballance_check = get_check_sum((uint8_t*)&s_card_info_sector2.block_10.detail.ballance, sizeof(s_card_info_sector2.block_10.detail.ballance));
        s_card_info_sector2.block_10.detail.is_lock = 0x00;

        /** 保存设备ID */
        card_info_block = 0x08;
        if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.device_id, CARD_BLOCK_SIZE) < 0x00){
            LOG_E("card storage dev id fail!!");

            s_card_info_sector2.block_10.detail.is_lock = locked;
            s_card_info_sector2.block_10.detail.ballance = ballance;
            s_card_info_sector2.block_10.detail.start_time = stime;

            s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
            return s_card_operate_ret[gunno];
        }
        /** 保存充电信息 */
        card_info_block = 0x0A;
        if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.block_10.data, CARD_BLOCK_SIZE) < 0x00){
            LOG_E("card storage charge info fail!!");
            s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;

            s_card_info_sector2.block_10.detail.is_lock = locked;
            s_card_info_sector2.block_10.detail.ballance = ballance;
            s_card_info_sector2.block_10.detail.start_time = stime;

            return s_card_operate_ret[gunno];
        }

        LOG_D("swip card stop info:");
        LOG_D("total_fee:%d", ofsm->base.fees_total /100);
        LOG_D("charge_time:%d", ofsm->base.charge_time);
        LOG_D("total_elect:%d", ofsm->base.elect_a);
        LOG_D("total_fee:%d", ofsm->base.fees_total /100);
        LOG_D("account_ballance_after:%d", ofsm->base.account_ballance_after);
        LOG_D("stop_reason:%d", ofsm->base.reason_code);
        LOG_D("stop_soc:%d", ofsm->base.current_soc);
    }else{
        /** 触发跳页 */
        LOG_E("this card is not enough to pay the bill(%d, %d)!!", ofsm->base.fees_total, s_card_info_sector2.block_10.detail.ballance);
        s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_NO_BALLANCE;
        return s_card_operate_ret[gunno];
    }

    return s_card_operate_ret[gunno];
}

/*****************************************************************************
 *  函数名   app_card_pay_history_bill
 *  功能       结算历史订单
 *  参数      gunno    用于保存枪号
 * 返回      >=0：成功   <0：失败
 ****************************************************************************/
static int32_t app_card_pay_history_bill(uint8_t *gunno)
{
    if(gunno == NULL){
        return APP_CARD_OPERATE_RET_INTERNAL_ERROR;
    }
    if(*gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_CARD_OPERATE_RET_INTERNAL_ERROR;
    }

    LOG_D("gunno(%d) pay_history_bill");

    uint8_t card_info_block = 0x00, locked = s_card_info_sector2.block_10.detail.is_lock;
    int32_t result = 0x00, index;
    uint32_t money = 0x00, ballance = s_card_info_sector2.block_10.detail.ballance, stime = s_card_info_sector2.block_10.detail.start_time;
    struct ofsm_info *ofsm = NULL;
    thaisen_transaction_t *transaction = (thaisen_transaction_t*)rt_malloc(sizeof(thaisen_transaction_t));

    if(transaction == NULL){
        /** 没有足够的内存 */
        LOG_W("gunno(%d) no enough memory for transaction(%d)", *gunno, sizeof(thaisen_transaction_t));
        s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_INTERNAL_ERROR;
        return s_card_operate_ret[*gunno];
    }

    result = app_card_query_funpay_bill_fees_total(transaction, sizeof(thaisen_transaction_t), s_rfidr->uuid, s_rfidr->uuid_len, \
            APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.start_time), &money, gunno, &index);

    if((result < 0x00) || (*gunno >= APP_SYSTEM_GUNNO_SIZE)){
        /** 获取订单信息失败，提示无效卡 */
        LOG_D("current port(%d) query history bill fail(%d)", *gunno, result);
        if(*gunno >= APP_SYSTEM_GUNNO_SIZE){
            *gunno = APP_SYSTEM_GUNNO_SIZE - 0x01;
            s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_HISTORY_BILL_ERROR;
        }else{
            s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_HISTORY_BILL_ERROR;
        }
        rt_free(transaction);
        return s_card_operate_ret[*gunno];
    }

    ofsm = get_ofsm_info(*gunno);
    s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_SUCCESS;

    if(s_card_info_sector2.block_10.detail.ballance >= (ofsm->base.fees_total /100)){
        s_card_info_sector2.block_10.detail.ballance -= (ofsm->base.fees_total /100);
        ofsm->base.account_ballance_before = s_card_info_sector2.block_10.detail.ballance;

        s_card_info_sector2.block_10.detail.ballance = APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.ballance);
        s_card_info_sector2.block_10.detail.ballance_check = get_check_sum((uint8_t*)&s_card_info_sector2.block_10.detail.ballance, sizeof(s_card_info_sector2.block_10.detail.ballance));
        s_card_info_sector2.block_10.detail.is_lock = 0x00;

        /** 保存设备ID */
        card_info_block = 0x08;
        if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.device_id, CARD_BLOCK_SIZE) < 0x00){
            LOG_E("card storage dev id fail(history bill)!!");

            s_card_info_sector2.block_10.detail.is_lock = locked;
            s_card_info_sector2.block_10.detail.ballance = ballance;
            s_card_info_sector2.block_10.detail.start_time = stime;

            rt_free(transaction);
            s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
            return s_card_operate_ret[*gunno];
        }
        /** 保存充电信息 */
        card_info_block = 0x0A;
        if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.block_10.data, CARD_BLOCK_SIZE) < 0x00){
            LOG_E("card storage charge info fail(history bill)!!");

            s_card_info_sector2.block_10.detail.is_lock = locked;
            s_card_info_sector2.block_10.detail.ballance = ballance;
            s_card_info_sector2.block_10.detail.start_time = stime;

            rt_free(transaction);
            s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
            return s_card_operate_ret[*gunno];
        }

        LOG_D("gunno(%d) storage modified transaction(%d)(history bill)", *gunno, index);
        mw_storage_record_designate_index_updated(transaction, sizeof(thaisen_transaction_t), USER_DATA_TYPE_REPORTED,  \
                0x00, 0x01, *gunno, index);

    }else{
        rt_free(transaction);
        LOG_E("this card is not enough to pay the bill(%d, %d)(history bill)!!", ofsm->base.fees_total, s_card_info_sector2.block_10.detail.ballance);
        s_card_operate_ret[*gunno] = APP_CARD_OPERATE_RET_NO_BALLANCE;
        return s_card_operate_ret[*gunno];
    }

    rt_free(transaction);
    return s_card_operate_ret[*gunno];
}

/*****************************************************************************
 *  函数名   app_card_non_swip_card_stop
 *  功能       充电中非刷卡停处理
 *  参数      gunno    枪号
 * 返回      >=0：成功   <0：失败
 ****************************************************************************/
static int32_t app_card_non_swip_card_stop(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_CARD_OPERATE_RET_INTERNAL_ERROR;
    }

    LOG_D("gunno(%d) non_swip_card_stop");

    struct ofsm_info *ofsm = get_ofsm_info(gunno);
    uint8_t *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00), card_info_block = 0x00, locked = s_card_info_sector2.block_10.detail.is_lock;
    uint32_t ballance = s_card_info_sector2.block_10.detail.ballance, stime = s_card_info_sector2.block_10.detail.start_time;
    thaisen_transaction_t *transaction = (thaisen_transaction_t*)rt_malloc(sizeof(thaisen_transaction_t));
    int32_t index = mw_storage_record_get_current_index(gunno);

    if(transaction == NULL){
        /** 没有足够的内存 */
        LOG_W("gunno(%d) no enough memory for transaction(%d)", gunno, sizeof(thaisen_transaction_t));
        s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_INTERNAL_ERROR;
        return s_card_operate_ret[gunno];
    }

    s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_SUCCESS;

    if(gunno == APP_SYSTEM_GUNNOA){
        mw_storage_record_get_designate_index_record((uint8_t*)transaction, sizeof(thaisen_transaction_t), RECORD_REGION_CHARGE_RECORDA, index);
    }else{
        mw_storage_record_get_designate_index_record((uint8_t*)transaction, sizeof(thaisen_transaction_t), RECORD_REGION_CHARGE_RECORDB, index);
    }

    memcpy(s_card_info_sector2.device_id, dev_id, CARD_BLOCK_SIZE);

    if(s_card_info_sector2.block_10.detail.ballance >= (ofsm->base.fees_total /100)){
        s_card_info_sector2.block_10.detail.ballance -= (ofsm->base.fees_total /100);
        s_card_info_sector2.block_10.detail.ballance = APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.ballance);
        s_card_info_sector2.block_10.detail.ballance_check = get_check_sum((uint8_t*)&s_card_info_sector2.block_10.detail.ballance, sizeof(s_card_info_sector2.block_10.detail.ballance));
        s_card_info_sector2.block_10.detail.is_lock = 0x00;

        /** 保存设备ID */
        card_info_block = 0x08;
        if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.device_id, CARD_BLOCK_SIZE) < 0x00){
            LOG_E("card storage dev id fail(non swip card)!!");

            s_card_info_sector2.block_10.detail.is_lock = locked;
            s_card_info_sector2.block_10.detail.ballance = ballance;
            s_card_info_sector2.block_10.detail.start_time = stime;

            rt_free(transaction);
            s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
            return s_card_operate_ret[gunno];
        }
        /** 保存充电信息 */
        card_info_block = 0x0A;
        if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.block_10.data, CARD_BLOCK_SIZE) < 0x00){
            LOG_E("card storage charge info fail(non swip card)!!");

            s_card_info_sector2.block_10.detail.is_lock = locked;
            s_card_info_sector2.block_10.detail.ballance = ballance;
            s_card_info_sector2.block_10.detail.start_time = stime;

            rt_free(transaction);
            s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
            return s_card_operate_ret[gunno];
        }

        LOG_D("swip non card stop info:");
        LOG_D("total_fee:%d", ofsm->base.fees_total /100);
        LOG_D("charge_time:%d", ofsm->base.charge_time);
        LOG_D("total_elect:%d", ofsm->base.elect_a);
        LOG_D("total_fee:%d", ofsm->base.fees_total /100);
        LOG_D("account_ballance_after:%d", ofsm->base.account_ballance_after);
        LOG_D("stop_reason:%d", ofsm->base.reason_code);
        LOG_D("stop_soc:%d", ofsm->base.current_soc);

        LOG_D("gunno(%d) storage modified transaction", gunno);
        mw_storage_record_designate_index_updated(transaction, sizeof(thaisen_transaction_t), USER_DATA_TYPE_REPORTED,  \
                0x00, 0x01, gunno, index);
    }else{
        rt_free(transaction);
        /** 提示余额不足 */
        LOG_E("this card is not enough to pay the bill(%d, %d)(non swip card)!!", ofsm->base.fees_total, s_card_info_sector2.block_10.detail.ballance);
        s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_NO_BALLANCE;
        return s_card_operate_ret[gunno];
    }

    rt_free(transaction);
    return s_card_operate_ret[gunno];
}

/*****************************************************************************
 *  函数名   app_card_swip_card_start
 *  功能      刷卡启动处理
 *  参数      gunno    枪号
 * 返回      >=0：成功   <0：失败
 ****************************************************************************/
static int32_t app_card_swip_card_start(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_CARD_OPERATE_RET_INTERNAL_ERROR;
    }

    LOG_D("gunno(%d) swip_card_start", gunno);

    struct ofsm_info *ofsm = get_ofsm_info(gunno);
    uint8_t *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00), card_info_block = 0x00, locked = s_card_info_sector2.block_10.detail.is_lock;
    uint32_t ballance = s_card_info_sector2.block_10.detail.ballance, stime = s_card_info_sector2.block_10.detail.start_time;

    s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_SUCCESS;

    if(s_card_info_sector2.block_10.detail.ballance <= 100){  /** 启动时余额不能小于1元 */
        /** 提示余额不足 */
        LOG_E("card no ballance when swip card start(%d)!!", s_card_info_sector2.block_10.detail.ballance);
        s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_NO_BALLANCE;
        return s_card_operate_ret[gunno];
    }

    memcpy(s_card_info_sector2.device_id, dev_id, CARD_BLOCK_SIZE);
    s_card_info_sector2.block_10.detail.start_time = APP_ENDIANNESS_CONVERT(ofsm->base.current_time);
    s_card_info_sector2.block_10.detail.is_lock = 0x01;
    s_card_info_sector2.block_10.detail.ballance = APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.ballance);
    s_card_info_sector2.block_10.detail.ballance_check = get_check_sum((uint8_t*)&s_card_info_sector2.block_10.detail.ballance, sizeof(s_card_info_sector2.block_10.detail.ballance));

    /** 保存设备ID */
    card_info_block = 0x08;
    if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.device_id, CARD_BLOCK_SIZE) < 0x00){
        LOG_E("card storage dev id fail(swip card start)!!");

        s_card_info_sector2.block_10.detail.is_lock = locked;
        s_card_info_sector2.block_10.detail.ballance = ballance;
        s_card_info_sector2.block_10.detail.start_time = stime;

        s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
        return s_card_operate_ret[gunno];
    }
    /** 保存充电信息 */
    card_info_block = 0x0A;
    if(s_rfidr->bolck_write(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.block_10.data, CARD_BLOCK_SIZE) < 0x00){
        LOG_E("card storage charge info fail(swip card start)!!");

        s_card_info_sector2.block_10.detail.is_lock = locked;
        s_card_info_sector2.block_10.detail.ballance = ballance;
        s_card_info_sector2.block_10.detail.start_time = stime;

        s_card_operate_ret[gunno] = APP_CARD_OPERATE_RET_STORAGE_ERROR;
        return s_card_operate_ret[gunno];
    }

    return s_card_operate_ret[gunno];
}
#endif /* APP_USING_OFFLINE_BILLING */


/*****************************************************************************
 *  函数名   app_card_online_status
 *  功能       读卡器在线、离线判断
 *  参数       state   在线、离线状态
 * 返回
 ****************************************************************************/
static void app_card_online_status(uint8_t state)
{
    if(state == APP_RFIDR_OFFLINE){
        if(*(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0)) == 0x01){
            for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                app_set_system_fault_enum(APP_SYS_FAULT_CARD_READER, APP_GENERAL_SYSTEM_FAULT_SET_LOW, gunno);
            }
        }
    }else{
        for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            app_clear_system_fault_enum(APP_SYS_FAULT_CARD_READER, APP_GENERAL_SYSTEM_FAULT_SET_LOW, gunno);
        }
    }
}
/*****************************************************************************
 *  函数名   app_card_data_update
 *  功能       实时更新数据(是否使能读卡器、当前端口赋值)
 *  参数       handle   卡信息总句柄
 * 返回
 ****************************************************************************/
static void app_card_data_update(void* handle)
{
    s_rfidr = (rfid_reader*)handle;

    switch (get_ofsm_info(0x00)->base.ota_state) {
    case APP_OTA_STATE_AUTH_SUCCESS:
    case APP_OTA_STATE_UPDATEING:
        s_rfidr->flag.is_forbid = 0x01;
        return;
    default:
        break;
    }

    if((*(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0))) != 0x01){
        for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            app_clear_system_fault_enum(APP_SYS_FAULT_CARD_READER, APP_GENERAL_SYSTEM_FAULT_SET_LOW, gunno);
        }
        s_rfidr->flag.is_forbid = 0x01;
    }else{
        /** 并充时，若要刷卡结束则停止主枪，不管屏幕当前页面 */
        if((get_ofsm_info(0x00)->base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) ||
                (get_ofsm_info(0x00)->base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            s_rfidr->current_port = get_ofsm_info(0x00)->base.main_gunno;
        }else{
            s_rfidr->current_port = thaisen_get_hci_page_pos();
        }
        s_rfidr->flag.is_forbid = 0x00;
    }
}

#ifdef APP_USING_OFFLINE_BILLING
/*****************************************************************************
 *  函数名   app_card_another_gun_judge
 *  功能       判断另一把枪是否要停止
 *  参数       gunno    当前枪号
 * 返回        1：需要停止   0：不需要停止
 ****************************************************************************/
static uint8_t app_card_another_gun_judge(uint8_t gunno)
{
    int32_t ret = 0x00;
    uint8_t another_port = 0x00, *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
    struct ofsm_info *ofsm = NULL;

    if(APP_SYSTEM_GUNNO_SIZE >= 0x02){   /** 双枪情况下才进行此判断 */
        another_port = APP_SYSTEM_GUNNOA;
        if(gunno == another_port){
            another_port = APP_SYSTEM_GUNNOA + 0x01;
        }
        ofsm = get_ofsm_info(another_port);
        switch(ofsm->base.state.current){
        case APP_OFSM_STATE_STARTING:
            if(memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len) == 0x00){  /** 这是情况2，退出 */
                s_rfidr->current_port = another_port;
                s_card_operate_ret[another_port] = APP_CARD_OPERATE_RET_NULL;
                return 0x01;
            }
        case APP_OFSM_STATE_CHARGING:
            if(((memcmp(dev_id, s_card_info_sector2.device_id, CARD_BLOCK_SIZE)) == 0x00) &&
                    ((memcmp(s_card_info_sector2.card_number, ofsm->base.card_number, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING)) == 0x00) &&
                    (memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len) == 0x00)){
                app_card_event_send(APP_CARD_EVENT_IS_PAYING, another_port, NULL);  /** 此时需要应用到业务的充电数据，先告诉业务正在结算，业务不能修改业务充电数据 */

                ret = app_card_swip_card_stop(another_port);
                s_rfidr->current_port = another_port;
                if(ret < 0x00){
                    app_card_event_recv(APP_CARD_EVENT_IS_PAYING, 0x00, another_port, NULL, 0x01);
                    s_card_operate_ret[another_port] = APP_CARD_OPERATE_RET_INTERNAL_ERROR;
                    return 0x01;
                }else{
                    app_card_event_send(APP_CARD_EVENT_PAY_COMPLETE, another_port, NULL);
                }
                s_card_operate_ret[another_port] = APP_CARD_OPERATE_RET_SUCCESS;
                return 0x01;
            }
            break;
        default:
            break;
        }
    }

    return 0x00;
}
#endif /* APP_USING_OFFLINE_BILLING */

/*****************************************************************************
 *  函数名   app_card_info_process
 *  功能       卡信息读、写处理
 *  参数       handle  卡信息总句柄
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
static int32_t app_card_info_process(void* handle)
{
#ifndef APP_USING_OFFLINE_BILLING
    if(get_ofsm_info(0x00)->base.run_mode != APP_RUN_MODE_OFFLINE_BILLING){
#ifdef RFIDR_USING_XJ_CARD
        if(xj_card_info_verify(handle) == 0x00){
            LOG_W("xj card info verify fail");
            return -0x01;
        }
#endif /* RFIDR_USING_XJ_CARD */
        if((rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_UUID) ||
                (rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_CARD_NUMBER)){

            struct ofsm_info *ofsm = NULL;
            s_rfidr = (rfid_reader*)handle;
            for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                ofsm = get_ofsm_info(gunno);
                if((memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len) == 0x00) &&
                        (ofsm->base.state.current == APP_OFSM_STATE_CHARGING)){
                    s_rfidr->current_port = gunno;
                }
            }
        }
        return 0x00;
    }
#else
    if(get_ofsm_info(0x00)->base.run_mode != APP_RUN_MODE_OFFLINE_BILLING){
        if((rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_UUID) ||
                (rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_CARD_NUMBER)){

            struct ofsm_info *ofsm = NULL;
            s_rfidr = (rfid_reader*)handle;
            for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                ofsm = get_ofsm_info(gunno);
                if((memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len) == 0x00) &&
                        (ofsm->base.state.current == APP_OFSM_STATE_CHARGING)){
                    s_rfidr->current_port = gunno;
                }
            }
        }
        return 0x00;
    }
    s_rfidr = (rfid_reader*)handle;

    int32_t ret = 0x00;
    uint8_t card_info_block = 0x00, port = s_rfidr->current_port, another_port = s_rfidr->current_port;
    struct ofsm_info *ofsm = get_ofsm_info(port);

    if(port >= APP_SYSTEM_GUNNO_SIZE){
        port = (APP_SYSTEM_GUNNO_SIZE - 0x01);   /** 必需有一把枪 */
    }
    s_card_operate_ret[port] = APP_CARD_OPERATE_RET_SUCCESS;

    /** 读取设备ID */
    card_info_block = 0x08;
    if(s_rfidr->bolck_read(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.device_id, CARD_BLOCK_SIZE) < 0x00){
        /** 提示无效卡 */
        LOG_E("reader read device ID fail!!");
        s_card_operate_ret[port] = APP_CARD_OPERATE_RET_READ_ERROR;
        return s_card_operate_ret[port];
    }

    /** 读取充电信息 */
    card_info_block = 0x0A;
    if(s_rfidr->bolck_read(CARD_OFFLINE_BILLING_SECTOR, card_info_block, s_card_info_sector2.block_10.data, CARD_BLOCK_SIZE) < 0x00){
        /** 提示无效卡 */
        LOG_E("reader read charge info fail!!");
        s_card_operate_ret[port] = APP_CARD_OPERATE_RET_READ_ERROR;
        return s_card_operate_ret[port];
    }

    s_card_info_sector2.block_10.detail.ballance = APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.ballance);
    s_card_ballance[port] = s_card_info_sector2.block_10.detail.ballance;
    memcpy(s_card_info_sector2.card_number, s_rfidr->card_number, sizeof(s_card_info_sector2.card_number));

    LOG_D("gunno(%d) this card locked state(%d) ballance(%d) device id[%s] card number[%s]", port, s_card_info_sector2.block_10.detail.is_lock, \
            s_card_info_sector2.block_10.detail.ballance, s_card_info_sector2.device_id, s_card_info_sector2.card_number);
    /** 卡已被锁, 存在以下情况
     * 1.上一次订单未结算(需要到上一次充电的设备去解锁, 除了启动、充电状态外，都可以解锁)
     * 2.已经进行了充电(状态是启动中或充电中) */
    if(s_card_info_sector2.block_10.detail.is_lock == 0x01){
        uint8_t *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
        /** 卡被锁了,业务状态既不是启动也不是充电, 可能是充电结束需要刷卡结算, 也可能是上一笔订单未支付 */
        if((ofsm->base.state.current != APP_OFSM_STATE_STARTING) && (ofsm->base.state.current != APP_OFSM_STATE_CHARGING)){
            if(memcmp(dev_id, s_card_info_sector2.device_id, CARD_BLOCK_SIZE)){    /** 上一次充电不是在这个设备，需要到服务台解锁 */
                /** 提示到服务台解锁 */
                LOG_E("please unlock this card to service platform!!(%d)", APP_CARD_OPERATE_RET_IS_LOCKED);
                s_card_operate_ret[port] = APP_CARD_OPERATE_RET_IS_LOCKED;
                return s_card_operate_ret[port];
            }else{
                /** 卡被锁而且桩不是启动或充电状态，并且接收到了充电桩已停止充电事件，说明这是充电完成未结算场景 */
                if(app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, port, NULL, 0x00) >= 0x00){  /** 只有正确结算完才将此事件清除 */
                    if((memcmp(dev_id, s_card_info_sector2.device_id, CARD_BLOCK_SIZE)) ||
                            (memcmp(s_card_info_sector2.card_number, ofsm->base.card_number, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING)) ||
                            memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len)){

                        if(app_card_another_gun_judge(port)){
                            if(APP_SYSTEM_GUNNO_SIZE >= 0x02){   /** 双枪情况下才进行此判断 */
                                another_port = APP_SYSTEM_GUNNOA;
                                if(port == another_port){
                                    another_port = APP_SYSTEM_GUNNOA + 0x01;
                                }
                            }
                            return s_card_operate_ret[another_port];
                        }

                        if(memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len)){
                            s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NOT_START_CARD;
                        }else{
                            s_card_operate_ret[port] = APP_CARD_OPERATE_RET_HISTORY_BILL_ERROR;
                        }
                        LOG_W("this is not the start card 00");
                        app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, port, NULL, 0x01);
                        return s_card_operate_ret[port];
                    }
                    app_card_event_send(APP_CARD_EVENT_IS_PAYING, port, NULL);  /** 此时需要应用到业务的充电数据，先告诉业务正在结算，业务不能修改业务充电数据 */
                    ret = app_card_non_swip_card_stop(port);
                    if(ret < 0x00){
                        app_card_event_recv(APP_CARD_EVENT_IS_PAYING, 0x00, port, NULL, 0x01);
                        return -0x01;
                    }else{
                        app_card_event_send(APP_CARD_EVENT_PAY_COMPLETE, port, NULL);
                        app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, port, NULL, 0x01);
                    }
                    s_card_operate_ret[port] = APP_CARD_OPERATE_RET_PAYED;
                    return s_card_operate_ret[port];
                }
                /** 卡被锁而且桩不是启动或充电状态，但未接收到充电桩已停止充电事件，说明这是上一笔订单未结算场景 */
                else{
                    /**
                                                       * 此种情况分析：
                     * 1：卡被锁
                     * 2：此端口状态非启动或充电
                     * 3：此端口未接收到充电结束事件
                                                       * 有以下可能：
                     * 1：这是上一笔订单未结算
                     * 2：这是用这张卡先启了一把，然后选择了另一把枪(这把枪状态：非启动或空闲)然后刷卡
                     */
                    /** 判断情况2 */
                    if(app_card_another_gun_judge(port)){
                        if(APP_SYSTEM_GUNNO_SIZE >= 0x02){   /** 双枪情况下才进行此判断 */
                            another_port = APP_SYSTEM_GUNNOA;
                            if(port == another_port){
                                another_port = APP_SYSTEM_GUNNOA + 0x01;
                            }
                        }
                        return s_card_operate_ret[another_port];
                    }
                    thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAYING, 0x0A, APP_THA_ENUM_TRUE, port);
                    app_card_event_send(APP_CARD_EVENT_IS_PAYING, port, NULL);  /** 此时需要应用到业务的充电数据，先告诉业务正在结算，业务不能修改业务充电数据 */
                    another_port = port;
                    ret = app_card_pay_history_bill(&port);

                    if(port >= APP_SYSTEM_GUNNO_SIZE){
                        port = APP_SYSTEM_GUNNO_SIZE - 0x01;
                    }

                    if(another_port != port){
                        app_card_event_send(APP_CARD_EVENT_IS_NOT_SAME_PORT, another_port, NULL);
                    }
                    if(ret < 0x00){
                        app_card_event_recv(APP_CARD_EVENT_IS_PAYING, 0x00, port, NULL, 0x01);
                        return -0x01;
                    }else{
                        app_card_event_send(APP_CARD_EVENT_PAY_COMPLETE, port, NULL);
                    }
                    s_card_operate_ret[another_port] = APP_CARD_OPERATE_RET_PAYED;  /** 当前枪号和历史订单的枪号不对时，不让另一把枪显示告警 */
                    s_card_operate_ret[port] = APP_CARD_OPERATE_RET_PAYED;
                    return s_card_operate_ret[port];
                }
            }
        }
        /** 卡被锁了,业务状态为启动或充电, 说明这是正常充电流程, 此时刷卡了,是要停止充电*/
        else{   // OK
            uint8_t gunno = 0x00;
            if(ofsm->base.state.current == APP_OFSM_STATE_STARTING){
                if(app_card_another_gun_judge(port)){
                    if(APP_SYSTEM_GUNNO_SIZE >= 0x02){   /** 双枪情况下才进行此判断 */
                        another_port = APP_SYSTEM_GUNNOA;
                        if(port == another_port){
                            another_port = APP_SYSTEM_GUNNOA + 0x01;
                        }
                    }
                    return s_card_operate_ret[another_port];
                }

                LOG_D("gunno(%d) is starting, is not allow stop", port);
                s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NULL;
                return s_card_operate_ret[port];
            }

            for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                ofsm = get_ofsm_info(gunno);
                if(((memcmp(dev_id, s_card_info_sector2.device_id, CARD_BLOCK_SIZE)) == 0x00) &&
                        ((memcmp(s_card_info_sector2.card_number, ofsm->base.card_number, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING)) == 0x00) &&
                        (memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len) == 0x00)){
                    s_rfidr->current_port = gunno;
                    port = gunno;
                    break;
                }
            }

            if(gunno == APP_SYSTEM_GUNNO_SIZE){
                LOG_D("gunno(%d) card number and recorded card number is no match in first %d byte", port, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING);
                s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NOT_START_CARD;
                return s_card_operate_ret[port];
            }else{
                app_card_event_send(APP_CARD_EVENT_IS_PAYING, port, NULL);  /** 此时需要应用到业务的充电数据，先告诉业务正在结算，业务不能修改业务充电数据 */
                ret = app_card_swip_card_stop(port);
                if(ret < 0x00){
                    app_card_event_recv(APP_CARD_EVENT_IS_PAYING, 0x00, port, NULL, 0x01);
                    return -0x01;
                }else{
                    app_card_event_send(APP_CARD_EVENT_PAY_COMPLETE, port, NULL);
                }
            }
        }
    }
    /**
     * 卡未被锁, 存在以下情况
     * 1.充电结束，要刷卡结算(需要有业务已经处理结束事件)
     * 2.想要启动充电(需要有业务状态已正常切换事件)
     * 3.一张卡启动(被锁了)，用另一张卡结束
     */
    else{
        switch(ofsm->base.state.current){
        case APP_OFSM_STATE_WAIT_NET:
        case APP_OFSM_STATE_IDLEING:
            break;
        case APP_OFSM_STATE_CHARGING:
        {
            uint8_t *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00), gunno = 0x00;
            for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                ofsm = get_ofsm_info(gunno);
                if(((memcmp(dev_id, s_card_info_sector2.device_id, CARD_BLOCK_SIZE)) == 0x00) &&
                        ((memcmp(s_card_info_sector2.card_number, ofsm->base.card_number, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING)) == 0x00) &&
                        (memcmp(s_rfidr->uuid, ofsm->base.card_uid, s_rfidr->uuid_len) == 0x00)){
                    s_rfidr->current_port = gunno;
                    port = gunno;
                    break;
                }
            }

            if(gunno == APP_SYSTEM_GUNNO_SIZE){
                LOG_W("this is not the start card");
                if(thaisen_is_not_allow_swip_card()){
                    LOG_D("gunno(%d) current page is not allow swip card charge", port);
                    s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NULL;
                    return s_card_operate_ret[port];
                }
                s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NOT_START_CARD;
                return s_card_operate_ret[port];
            }else{
                LOG_W("occured error, card start but not be locked");
                app_card_event_send(APP_CARD_EVENT_IS_PAYING, port, NULL);  /** 此时需要应用到业务的充电数据，先告诉业务正在结算，业务不能修改业务充电数据 */
                ret = app_card_swip_card_stop(port);
                if(ret < 0x00){
                    app_card_event_recv(APP_CARD_EVENT_IS_PAYING, 0x00, port, NULL, 0x01);
                    return -0x01;
                }else{
                    app_card_event_send(APP_CARD_EVENT_PAY_COMPLETE, port, NULL);
                }
                s_card_operate_ret[port] = APP_CARD_OPERATE_RET_SUCCESS;
                return s_card_operate_ret[port];
            }
        }
            break;
        case APP_OFSM_STATE_READYING:
        case APP_OFSM_STATE_FINISHING:     // OK
            if(app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, port, NULL, 0x01) >= 0x00){
                uint8_t *dev_id = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
                if(memcmp(s_card_info_sector2.card_number, dev_id, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING)){
                    LOG_D("gunno(%d) card number and pile number is no match in first %d byte", port, APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING);
                    s_card_operate_ret[port] = APP_CARD_OPERATE_RET_INVALID_CARD;
                    return s_card_operate_ret[port];
                }
                if(thaisen_is_not_allow_swip_card()){
                    LOG_D("gunno(%d) current page is not allow swip card charge", port);
                    s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NULL;
                    return s_card_operate_ret[port];
                }
                ret = app_card_swip_card_start(port);
                if(ret < 0x00){
                    app_card_event_recv(APP_CARD_EVENT_IS_PAYING, 0x00, port, NULL, 0x01);
                    return -0x01;
                }else{
                    app_card_event_send(APP_CARD_EVENT_CHARGE_START, port, NULL);
                    s_card_ballance[port] = APP_ENDIANNESS_CONVERT(s_card_info_sector2.block_10.detail.ballance);

                    LOG_D("gunno(%d) swip card start charge, ballance:%d", port, s_card_ballance[port]);
                }
            }else{
                if(app_card_another_gun_judge(port)){
                    if(APP_SYSTEM_GUNNO_SIZE >= 0x02){   /** 双枪情况下才进行此判断 */
                        another_port = APP_SYSTEM_GUNNOA;
                        if(port == another_port){
                            another_port = APP_SYSTEM_GUNNOA + 0x01;
                        }
                    }
                    return s_card_operate_ret[another_port];
                }else{
                    if(thaisen_is_not_allow_swip_card()){
                        LOG_D("gunno(%d) current page is not allow swip card charge", port);
                        s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NULL;
                        return s_card_operate_ret[port];
                    }
                }
                LOG_D("chargepile state is not switch complete(%d)\n", port);
                return -0x01;
            }
            break;
        default:
            s_card_operate_ret[port] = APP_CARD_OPERATE_RET_NULL;
            return s_card_operate_ret[port];
            break;
        }
    }
#endif /* APP_USING_OFFLINE_BILLING */
    return 0x00;
}

/*****************************************************************************
 *  函数名   app_card_event_send
 *  功能       卡事件发送
 *  参数       event      事件
 *     set        用于保存当前事件集
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_event_send(uint32_t event, uint8_t gunno, uint32_t *set)
{
#ifdef APP_USING_OFFLINE_BILLING
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        if(set){
            *set = 0x00;
        }
        return -0x01;
    }
    int32_t res = 0x00;

    res = rt_event_send(&s_card_event[gunno], event);
    if(set){
        *set = s_card_event[gunno].set;
    }
    return res;
#else
    return -0x01;
#endif /* APP_USING_OFFLINE_BILLING */
}

/*****************************************************************************
 *  函数名   app_card_event_recv
 *  功能       卡事件接收
 *  参数       event        事件
 *     timeout      事件等待时长
 *     set          用于保存当前事件集
 *     is_clear     事件接收完是否清除事件
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_event_recv(uint32_t event, uint32_t timeout, uint8_t gunno, uint32_t *set, uint8_t is_clear)
{
#ifdef APP_USING_OFFLINE_BILLING
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        if(set){
            *set = 0x00;
        }
        return -0x01;
    }

    if(is_clear){
        return rt_event_recv(&s_card_event[gunno], event, RT_EVENT_FLAG_OR |RT_EVENT_FLAG_CLEAR, timeout, set);
    }
    return rt_event_recv(&s_card_event[gunno], event, RT_EVENT_FLAG_OR, timeout, set);
#else
    return -0x01;
#endif /* APP_USING_OFFLINE_BILLING */
}

/*****************************************************************************
 *  函数名   app_card_ipc_init
 *  功能       卡IPC初始化
 *  参数
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_ipc_init(void)
{
    uint8_t name[8];

    memset(name, 0x00, sizeof(name));
    memset(&s_card_info_sector2, 0x00, sizeof(s_card_info_sector2));

    for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        sprintf(name, "%s%d", "carde_", gunno);
        if(rt_event_init(&s_card_event[gunno], (const char*)name, RT_IPC_FLAG_PRIO) != RT_EOK){
            LOG_E("card event set init fail(%d)", gunno);
            return -0x01;
        }
    }

    return 0x00;
}

/*****************************************************************************
 *  函数名   app_card_query_ballance
 *  功能       查询卡内余额
 *  参数      gunno    枪号
 * 返回       卡内余额
 ****************************************************************************/
uint32_t app_card_query_ballance(uint8_t gunno)
{
#ifdef APP_USING_OFFLINE_BILLING
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    return s_card_ballance[gunno];
#else
    return 0x00;
#endif /* APP_USING_OFFLINE_BILLING */
}

/*****************************************************************************
 *  函数名   app_card_query_operate_ret
 *  功能       查询卡信息处理结果
 *  参数      gunno    枪号
 * 返回       卡信息处理结果
 ****************************************************************************/
int8_t app_card_query_operate_ret(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_CARD_OPERATE_RET_SUCCESS;
    }

    return s_card_operate_ret[gunno];
}

#ifdef RFIDR_USING_XJ_CARD
/*****************************************************************************
 *  函数名   app_query_xj_crc_serial_number
 *  功能       查询小桔卡计算用CRC序列号
 *  参数
 *  返回       计算用CRC序列号
 ****************************************************************************/
uint8_t *app_query_xj_crc_serial_number(void)
{
    return s_card_info_sector1.serial_number;
}
/*****************************************************************************
 *  函数名   app_query_xj_crc
 *  功能       查询小桔卡CRC值
 *  参数
 * 返回       CRC值
 ****************************************************************************/
uint8_t *app_query_xj_crc(void)
{
    return s_card_info_sector1.crc;
}

/*****************************************************************************
 *  函数名   app_query_xj_random_number_1
 *  功能       查询小桔卡随机数1
 *  参数
 *  返回       随机数1
 ****************************************************************************/
uint8_t *app_query_xj_random_number_1(void)
{
    return s_card_info_sector5.random_number_1;
}
/*****************************************************************************
 *  函数名   app_query_xj_random_number_2
 *  功能       查询小桔卡随机数2
 *  参数
 *  返回       随机数2
 ****************************************************************************/
uint8_t *app_query_xj_random_number_2(void)
{
    return s_card_info_sector5.random_number_2;
}
/*****************************************************************************
 *  函数名   app_query_xj_random_number_3
 *  功能       查询小桔卡随机数3
 *  参数
 *  返回       随机数3
 ****************************************************************************/
uint8_t *app_query_xj_random_number_3(void)
{
    return s_card_info_sector5.random_number_3;
}
#endif /* #ifdef RFIDR_USING_XJ_CARD */

/*****************************************************************************
 *  函数名   app_card_init
 *  功能       卡部分初始化
 *  参数
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_init(void)
{
#ifdef APP_DESIGNATE_REGION
    for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        s_card_operate_ret[gunno] = 0x00;
        s_card_ballance[gunno] = 0x00;
    }

    s_rfidr = NULL;
    memset(&s_card_info_sector2, 0x00, sizeof(s_card_info_sector2));
#ifdef RFIDR_USING_XJ_CARD
    memset(&s_card_info_sector1, 0x00, sizeof(s_card_info_sector1));
    memset(&s_card_info_sector5, 0x00, sizeof(s_card_info_sector5));

    /** 小桔卡扇区1密钥 */
    s_card_sector1_key[0x00] = 0x34;
    s_card_sector1_key[0x01] = 0x71;
    s_card_sector1_key[0x02] = 0x4C;
    s_card_sector1_key[0x03] = 0x80;
    s_card_sector1_key[0x04] = 0x01;
    s_card_sector1_key[0x05] = 0x77;
    /** 小桔卡扇区5密钥 */
    s_card_sector5_key[0x00] = 0x92;
    s_card_sector5_key[0x01] = 0x5C;
    s_card_sector5_key[0x02] = 0x9A;
    s_card_sector5_key[0x03] = 0x4B;
    s_card_sector5_key[0x04] = 0x83;
    s_card_sector5_key[0x05] = 0x74;
#endif /* #ifdef RFIDR_USING_XJ_CARD */
#endif /* APP_DESIGNATE_REGION */

    app_rfidr_config_handle_fault(app_card_online_status);
    app_rfidr_config_handle_data_update(app_card_data_update);
    app_rfidr_config_handle_info_process(app_card_info_process);
    app_rfidr_config_handle_node_init(card_node_init_hook);
    app_rfidr_config_handle_node_running(card_node_running);

    return app_rfidr_init();
}




