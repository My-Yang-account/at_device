/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-12     chenyong     first version
 * 2018-08-12     Marcus       port to ec20
 * 2019-05-13     chenyong     multi AT socket client support
 */

#include <stdio.h>
#include <string.h>
#include <app.h>
#include "net_netdev.h"

#include <at_device_ec20.h>

#define LOG_TAG                        "at.dev.ec20"
#include <at_log.h>

#ifdef AT_DEVICE_USING_EC20

#define EC20_WAIT_CONNECT_TIME          5000
#define EC20_THREAD_STACK_SIZE          2048
#define EC20_THREAD_PRIORITY            (RT_THREAD_PRIORITY_MAX/2)

#define EC20_POWER_CTRL_TIME            30 *60 *1000

/* AT+QICSGP command default*/
static char *QICSGP_CHINA_MOBILE = "AT+QICSGP=1,1,\"CMNET\",\"\",\"\",0";
static char *QICSGP_CHINA_UNICOM = "AT+QICSGP=1,1,\"UNINET\",\"\",\"\",0";
static char *QICSGP_CHINA_TELECOM = "AT+QICSGP=1,1,\"CTNET\",\"\",\"\",0";

///////////////////////////////////////////////////////////////////////////////
struct at_device_appinfo {
    int  boot;            /* 开机状态 */
    int  at;              /* 0：AT连接正常；1：AT连接异常 */
    int  card;            /* 0：有卡；1：无卡 */
    int  signal_strength; /* 信号强度 */
    int  cgreg;           /* 0：GPRS网络未注册；1：GPRS网络已注册 */
    int  mnc;             /* 网络运营商 */
    int  init_complete;   /* 4G模块初始化完成 */
    char iccid[21];       /* SIM卡号码 */
    char imei[18];        /* IMEI码 */
};

struct dev_control{
    uint32_t tick;
    uint8_t power_on;
};

static struct dev_control s_dev_control_info =
{
    .power_on = 0x00,
};
static struct at_device_appinfo s_at_device_appinfo;

int get_at_device_appinfo_boot(void)
{
    return s_at_device_appinfo.boot;
}
int get_at_device_appinfo_at(void)
{
    return s_at_device_appinfo.at;
}
int get_at_device_appinfo_check_card(void)
{
    return s_at_device_appinfo.card;
}
int get_at_device_appinfo_signal_strength(void)
{
    return s_at_device_appinfo.signal_strength;
}
int get_at_device_appinfo_check_gprs_registered(void)
{
    return s_at_device_appinfo.cgreg;
}
int get_at_device_appinfo_mnc(void)
{
    return s_at_device_appinfo.mnc;
}
char *get_at_device_appinfo_iccid(void)
{
    s_at_device_appinfo.iccid[sizeof(s_at_device_appinfo.iccid) - 1] = '\0';
    return s_at_device_appinfo.iccid;
}
char *get_at_device_appinfo_imei(void)
{
    return s_at_device_appinfo.imei;
}
int get_at_device_appinfo_is_complete(void)
{
    return s_at_device_appinfo.init_complete;
}
void check_simcard_signal_strength(void)
{
    extern struct at_device_ec20 *get_ec20_device(void);
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;
    int qi_arg[2] = {0};

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(300));
    device = (struct at_device *)(&(get_ec20_device()->device));
    if (resp == RT_NULL)
    {
        LOG_E("no memory for resp create.check_simcard_signal_strength");
        return;
    }
    resp = at_resp_set_info(resp, 128, 0, rt_tick_from_millisecond(300));
    at_obj_exec_cmd(device->client, resp, "AT+CSQ");

    at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %d,%d", &qi_arg[0], &qi_arg[1]);

    s_at_device_appinfo.signal_strength = qi_arg[0];

    if (resp)
    {
        at_delete_resp(resp);
    }
}

void ec20_at_device_reset(void)
{
    LOG_E("ec20 AT device close");
    netdev_set_down(netdev_default);
    rt_thread_mdelay(5000);

    s_at_device_appinfo.boot = 0;
    s_at_device_appinfo.at = 0;
    s_at_device_appinfo.card = 0;
    rt_memset(s_at_device_appinfo.iccid, 0x00, sizeof(s_at_device_appinfo.iccid));
    rt_memset(s_at_device_appinfo.imei, 0x00, sizeof(s_at_device_appinfo.imei));
    s_at_device_appinfo.signal_strength = 0;
    s_at_device_appinfo.cgreg = 0;
    s_at_device_appinfo.mnc = -1;
    s_at_device_appinfo.init_complete = 0;

    LOG_E("ec20 AT device open");
    netdev_set_up(netdev_default);
}
///////////////////////////////////////////////////////////////////////////////

#ifdef EC20_USING_CME
static void at_cme_errcode_parse(int result)
{
    switch(result)
    {
    case 0   : LOG_E("%d : Phone failure",                result); break;
    case 1   : LOG_E("%d : No connection to phone",       result); break;
    case 2   : LOG_E("%d : Phone-adaptor link reserved",  result); break;
    case 3   : LOG_E("%d : Operation not allowed",        result); break;
    case 4   : LOG_E("%d : Operation not supported",      result); break;
    case 5   : LOG_E("%d : PH-SIM PIN required",          result); break;
    case 6   : LOG_E("%d : PH-FSIM PIN required",         result); break;
    case 7   : LOG_E("%d : PH-FSIM PUK required",         result); break;
    case 10  : LOG_E("%d : SIM not inserted",             result); break;
    case 11  : LOG_E("%d : SIM PIN required",             result); break;
    case 12  : LOG_E("%d : SIM PUK required",             result); break;
    case 13  : LOG_E("%d : SIM failure",                  result); break;
    case 14  : LOG_E("%d : SIM busy",                     result); break;
    case 15  : LOG_E("%d : SIM wrong",                    result); break;
    case 16  : LOG_E("%d : Incorrect password",           result); break;
    case 17  : LOG_E("%d : SIM PIN2 required",            result); break;
    case 18  : LOG_E("%d : SIM PUK2 required",            result); break;
    case 20  : LOG_E("%d : Memory full",                  result); break;
    case 21  : LOG_E("%d : Invalid index",                result); break;
    case 22  : LOG_E("%d : Not found",                    result); break;
    case 23  : LOG_E("%d : Memory failure",               result); break;
    case 24  : LOG_E("%d : Text string too long",         result); break;
    case 25  : LOG_E("%d : Invalid characters in text string", result); break;
    case 26  : LOG_E("%d : Dial string too long",         result); break;
    case 27  : LOG_E("%d : Invalid characters in dial string", result); break;
    case 30  : LOG_E("%d : No network service",           result); break;
    case 31  : LOG_E("%d : Network timeout",              result); break;
    case 32  : LOG_E("%d : Network not allowed - emergency calls only", result); break;
    case 40  : LOG_E("%d : Network personalization PIN required", result); break;
    case 41  : LOG_E("%d : Network personalization PUK required", result); break;
    case 42  : LOG_E("%d : Network subset personalization PIN required", result); break;
    case 43  : LOG_E("%d : Network subset personalization PUK required", result); break;
    case 44  : LOG_E("%d : Service provider personalization PIN required", result); break;
    case 45  : LOG_E("%d : Service provider personalization PUK required", result); break;
    case 46  : LOG_E("%d : Corporate personalization PIN required", result); break;
    case 47  : LOG_E("%d : Corporate personalization PUK required", result); break;
    case 901 : LOG_E("%d : Audio unknown error",          result); break;
    case 902 : LOG_E("%d : Audio invalid parameters",     result); break;
    case 903 : LOG_E("%d : Audio operation not supported", result); break;
    case 904 : LOG_E("%d : Audio device busy",            result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_cms_errcode_parse(int result)
{
    switch(result)
    {
    case 300 : LOG_E("%d : ME failure",                   result); break;
    case 301 : LOG_E("%d : SMS ME reserved",              result); break;
    case 302 : LOG_E("%d : Operation not allowed",        result); break;
    case 303 : LOG_E("%d : Operation not supported",      result); break;
    case 304 : LOG_E("%d : Invalid PDU mode",             result); break;
    case 305 : LOG_E("%d : Invalid text mode",            result); break;
    case 310 : LOG_E("%d : SIM not inserted",             result); break;
    case 311 : LOG_E("%d : SIM pin necessary",            result); break;
    case 312 : LOG_E("%d : PH SIM pin necessary",         result); break;
    case 313 : LOG_E("%d : SIM failure",                  result); break;
    case 314 : LOG_E("%d : SIM busy",                     result); break;
    case 315 : LOG_E("%d : SIM wrong",                    result); break;
    case 316 : LOG_E("%d : SIM PUK required",             result); break;
    case 317 : LOG_E("%d : SIM PIN2 required",            result); break;
    case 318 : LOG_E("%d : SIM PUK2 required",            result); break;
    case 320 : LOG_E("%d : Memory failure",               result); break;
    case 321 : LOG_E("%d : Invalid memory index",         result); break;
    case 322 : LOG_E("%d : Memory full",                  result); break;
    case 330 : LOG_E("%d : SMSC address unknown",         result); break;
    case 331 : LOG_E("%d : No network",                   result); break;
    case 332 : LOG_E("%d : Network timeout",              result); break;
    case 500 : LOG_E("%d : Unknown",                      result); break;
    case 512 : LOG_E("%d : SIM not ready",                result); break;
    case 513 : LOG_E("%d : Message length exceeds",       result); break;
    case 514 : LOG_E("%d : Invalid request parameters",   result); break;
    case 515 : LOG_E("%d : ME storage failure",           result); break;
    case 517 : LOG_E("%d : Invalid service mode",         result); break;
    case 528 : LOG_E("%d : More message to send state error", result); break;
    case 529 : LOG_E("%d : MO SMS is not allow",          result); break;
    case 530 : LOG_E("%d : GPRS is suspended",            result); break;
    case 531 : LOG_E("%d : ME storage full",              result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}
#endif /* EC20_USING_CME */

#ifdef EC20_USING_MMS
static void at_mms_errcode_parse(int result)//MMS
{
    switch(result)
    {
    case 751 : LOG_E("%d : Unknown error",                result); break;
    case 752 : LOG_E("%d : URL length error",             result); break;
    case 753 : LOG_E("%d : URL error",                    result); break;
    case 754 : LOG_E("%d : Invalid proxy type",           result); break;
    case 755 : LOG_E("%d : Proxy address error",          result); break;
    case 756 : LOG_E("%d : Invalid parameter",            result); break;
    case 757 : LOG_E("%d : Recipient address full",       result); break;
    case 758 : LOG_E("%d : CC recipient address full",    result); break;
    case 759 : LOG_E("%d : BCC recipient address full",   result); break;
    case 760 : LOG_E("%d : Attachments full",             result); break;
    case 761 : LOG_E("%d : File error",                   result); break;
    case 762 : LOG_E("%d : No recipient",                 result); break;
    case 763 : LOG_E("%d : File not found",               result); break;
    case 764 : LOG_E("%d : MMS busy",                     result); break;
    case 765 : LOG_E("%d : Server response failed",       result); break;
    case 766 : LOG_E("%d : Error response of HTTP(S) post", result); break;
    case 767 : LOG_E("%d : Invalid report of HTTP(S) post", result); break;
    case 768 : LOG_E("%d : PDP activation failed",        result); break;
    case 769 : LOG_E("%d : PDP deactivated",              result); break;
    case 770 : LOG_E("%d : Socket creation failed",       result); break;
    case 771 : LOG_E("%d : Socket connection failed",     result); break;
    case 772 : LOG_E("%d : Socket read failed",           result); break;
    case 773 : LOG_E("%d : Socket write failed",          result); break;
    case 774 : LOG_E("%d : Socket closed",                result); break;
    case 775 : LOG_E("%d : Timeout",                      result); break;
    case 776 : LOG_E("%d : Encode data error",            result); break;
    case 777 : LOG_E("%d : HTTP(S) decode data error",    result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}
#endif /* EC20_USING_MMS */

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

#ifdef USING_DOUBLE_GUN
#define G4_STATUS_PIN             GET_PIN(G, 6)
#define G4_POWER_KEY_PIN          GET_PIN(A, 11)
#define G4_POWER_CTRL_PIN         GET_PIN(G, 4)
#else
#define G4_POWER_CTRL_PIN         GET_PIN(B, 12)
#endif

int net_power_init(void)
{
#ifdef USING_DOUBLE_GUN
    rt_pin_mode(G4_STATUS_PIN, PIN_MODE_INPUT);

    rt_pin_mode(G4_POWER_KEY_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(G4_POWER_KEY_PIN, PIN_HIGH);        /* 初始化 模块端 POEKEY 为高 */
#endif
    rt_pin_mode(G4_POWER_CTRL_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(G4_POWER_CTRL_PIN, PIN_LOW);

    return 0;
}
INIT_DEVICE_EXPORT(net_power_init);

#define AT_SEND_CMDS(client, resp, resp_line, timeout, cmd)                                        \
    do {                                                                                           \
        (resp) = at_resp_set_info((resp), 128, (resp_line), rt_tick_from_millisecond(timeout));    \
        at_obj_exec_cmd((client), (resp), (cmd));                                                  \
    } while(0)                                                                                     \


#ifdef USING_DOUBLE_GUN
/*
 * EC800M :
 * 关机状态：status:低电平      MCU ：1417B ML-4G 3.8v :高电平
 * 开机状态：status:高电平      MCU ：1417B ML-4G 0.0v :低电平
 * POWKEY:[1417B ML-4G]
 * MCU 端：低电平--->3.8v :高电平
 * MCU 端：高电平--->0.0v :低电平
 */

static int8_t ec20_power_on(struct at_device *device)
{
    uint16_t power_on = 0;
    struct at_device_ec20 *ec20 = RT_NULL;

    ec20 = (struct at_device_ec20 *)device->user_data;

    /* not nead to set pin configuration for ec20 device power on */
    if (ec20->power_pin == -1 || ec20->power_status_pin == -1)
    {
        return -1;
    }

    if(s_dev_control_info.power_on ==  0x00){
        s_dev_control_info.tick = rt_tick_get();
        s_dev_control_info.power_on = 0x01;
    }

    if (rt_pin_read(ec20->power_status_pin) == PIN_LOW)
    {
        LOG_I("4G MODULE ALREADY POWER ON");   /* 如果模块已是开机状态 */
        uint32_t ctick = rt_tick_get();
        if(s_dev_control_info.tick > ctick){
            if(((ctick + 0xFFFFFFFF) - s_dev_control_info.tick) > EC20_POWER_CTRL_TIME){
                rt_pin_write(G4_POWER_CTRL_PIN, PIN_HIGH);
                rt_thread_mdelay(8000);
                rt_pin_write(G4_POWER_CTRL_PIN, PIN_LOW);
                rt_thread_mdelay(2000);
                s_dev_control_info.tick = ctick;
            }
        }else{
            if((ctick - s_dev_control_info.tick) > EC20_POWER_CTRL_TIME){
                rt_pin_write(G4_POWER_CTRL_PIN, PIN_HIGH);
                rt_thread_mdelay(8000);
                rt_pin_write(G4_POWER_CTRL_PIN, PIN_LOW);
                rt_thread_mdelay(2000);
                s_dev_control_info.tick = ctick;
            }
        }
    }

    rt_pin_write(ec20->power_pin, PIN_LOW);     /* 拉高 POWKEY */
    rt_thread_mdelay(2000);
    rt_pin_write(ec20->power_pin, PIN_HIGH);    /* 拉低 POWKEY开机 */

    while (rt_pin_read(ec20->power_status_pin) == PIN_HIGH)
    {
        rt_thread_mdelay(100);

        if(++power_on >= 100)
        {
            LOG_W("4G POWER-ON TIMEOUT|%d", 300 *100);
            return -1;
        }
    }
    rt_pin_write(ec20->power_pin, PIN_LOW);     /* 拉高 POWKEY */

    LOG_I("4G POWER-ON OK");
    return 0;
}

static void ec20_power_off(struct at_device *device)
{
    uint16_t power_off= 0;
    struct at_device_ec20 *ec20 = RT_NULL;

    ec20 = (struct at_device_ec20 *)device->user_data;

    /* not nead to set pin configuration for ec20 device power on */
    if (ec20->power_pin == -1 || ec20->power_status_pin == -1)
    {
        return;
    }

    if (rt_pin_read(ec20->power_status_pin) == PIN_HIGH)
    {
        LOG_I("4G MODULE ALREADY POWER OFF");
        return;
    }

    rt_pin_write(ec20->power_pin, PIN_HIGH);    /* 拉低 POWKEY */
    rt_thread_mdelay(2000);
    rt_pin_write(ec20->power_pin, PIN_LOW);     /* 拉高 POWKEY关机 */

    while (rt_pin_read(ec20->power_status_pin) == PIN_LOW)
    {
        rt_thread_mdelay(100);

        if(++power_off >= 100)
        {
            LOG_W("4G POWER-OFF TIMEOUT|%d", 300 *100);
            return;
        }
    }
    rt_pin_write(ec20->power_pin, PIN_LOW);     /* 拉高 POWKEY */

    LOG_I("4G POWER-OFF OK");
}
#else
static int32_t ec20_power_on(struct at_device *device)
{
    struct at_device_ec20 *ec20 = RT_NULL;
    ec20 = (struct at_device_ec20 *)device->user_data;

    if(s_dev_control_info.power_on ==  0x00){
        s_dev_control_info.tick = rt_tick_get();
        s_dev_control_info.power_on = 0x01;
    }

    rt_pin_write(ec20->power_pin, PIN_HIGH);

    rt_thread_mdelay(10000);

    LOG_I("4G POWER-ON OK");
}

static void ec20_power_off(struct at_device *device)
{
    uint8_t try_num = 0;

    struct at_device_ec20 *ec20 = RT_NULL;
    ec20 = (struct at_device_ec20 *)device->user_data;

    at_response_t resp = RT_NULL;
    resp = at_create_resp(128, 0, rt_tick_from_millisecond(500));

__again:

    if (at_obj_exec_cmd(device->client, resp, "AT+QPOWD") < 0)
    {
        if(++try_num <= 5)          goto __again;

        LOG_I("4G POWER-OFF FAIL");
        return;
    }

    if(s_dev_control_info.tick > rt_tick_get()){
        if(((rt_tick_get() + 0xFFFFFFFF) - s_dev_control_info.tick) > EC20_POWER_CTRL_TIME){
            rt_pin_write(ec20->power_pin, PIN_LOW);
            s_dev_control_info.tick = rt_tick_get();
            LOG_I("4G POWER-OFF OK");
        }
    }else{
        if((rt_tick_get() - s_dev_control_info.tick) > EC20_POWER_CTRL_TIME){
            rt_pin_write(ec20->power_pin, PIN_LOW);
            s_dev_control_info.tick = rt_tick_get();
            LOG_I("4G POWER-OFF OK");
        }
    }

    rt_thread_mdelay(8000);
}
#endif

/* =============================  ec20 network interface operations ============================= */

/* set ec20 network interface device status and address information */
static int ec20_netdev_set_info(struct netdev *netdev)
{
#define EC20_IMEI_RESP_SIZE      32
#define EC20_IPADDR_RESP_SIZE    64
#define EC20_DNS_RESP_SIZE       96
#define EC20_INFO_RESP_TIMO      rt_tick_from_millisecond(300)

    int result = RT_EOK;
    ip_addr_t addr;
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;

    RT_ASSERT(netdev);

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get device(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    /* set network interface device status */
    netdev_low_level_set_status(netdev, RT_TRUE);
    netdev_low_level_set_link_status(netdev, RT_TRUE);
    netdev_low_level_set_dhcp_status(netdev, RT_TRUE);

    resp = at_create_resp(EC20_IMEI_RESP_SIZE, 0, EC20_INFO_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("no memory for resp create.");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* set network interface device hardware address(IMEI) */
    {
        #define EC20_NETDEV_HWADDR_LEN   8
        #define EC20_IMEI_LEN            15

        char imei[EC20_IMEI_LEN] = {0};
        int i = 0, j = 0;

        /* send "AT+GSN" commond to get device IMEI */
        if (at_obj_exec_cmd(device->client, resp, "AT+GSN") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args(resp, 2, "%s", imei) <= 0)
        {
            LOG_E("%s device prase \"AT+GSN\" cmd error.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("%s device IMEI number: %s", device->name, imei);

        netdev->hwaddr_len = EC20_NETDEV_HWADDR_LEN;
        /* get hardware address by IMEI */
        for (i = 0, j = 0; i < EC20_NETDEV_HWADDR_LEN && j < EC20_IMEI_LEN; i++, j+=2)
        {
            if (j != EC20_IMEI_LEN - 1)
            {
                netdev->hwaddr[i] = (imei[j] - '0') * 10 + (imei[j + 1] - '0');
            }
            else
            {
                netdev->hwaddr[i] = (imei[j] - '0');
            }
        }
    }

    /* set network interface device IP address */
    {
        #define IP_ADDR_SIZE_MAX    16
        char ipaddr[IP_ADDR_SIZE_MAX] = {0};

        resp = at_resp_set_info(resp, EC20_IPADDR_RESP_SIZE, 0, EC20_INFO_RESP_TIMO);

        /* send "AT+QIACT?" commond to get IP address */
        if (at_obj_exec_cmd(device->client, resp, "AT+QIACT?") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* parse response data "+QIACT: 1,<context_state>,<context_type>[,<IP_address>]" */
        if (at_resp_parse_line_args_by_kw(resp, "+QIACT:", "+QIACT: %*[^\"]\"%[^\"]", ipaddr) <= 0)
        {
            LOG_E("%s device \"AT+QIACT?\" cmd error.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("%s device IP address: %s", device->name, ipaddr);

        /* set network interface address information */
        inet_aton(ipaddr, &addr);
        netdev_low_level_set_ipaddr(netdev, &addr);
    }

    /* set network interface device dns server */
    {
        #define DNS_ADDR_SIZE_MAX   16
        char dns_server1[DNS_ADDR_SIZE_MAX] = {0}, dns_server2[DNS_ADDR_SIZE_MAX] = {0};

        resp = at_resp_set_info(resp, EC20_DNS_RESP_SIZE, 0, EC20_INFO_RESP_TIMO);

        /* send "AT+QIDNSCFG=1" commond to get DNS servers address */
        if (at_obj_exec_cmd(device->client, resp, "AT+QIDNSCFG=1") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* parse response data "+QIDNSCFG: <contextID>,<pridnsaddr>,<secdnsaddr>" */
        if (at_resp_parse_line_args_by_kw(resp, "+QIDNSCFG:", "+QIDNSCFG: 1,\"%[^\"]\",\"%[^\"]\"",
                dns_server1, dns_server2) <= 0)
        {
            LOG_E("%s device prase \"AT+QIDNSCFG=1\" cmd error.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("%s device primary DNS server address: %s", device->name, dns_server1);
        LOG_D("%s device secondary DNS server address: %s", device->name, dns_server2);

        inet_aton(dns_server1, &addr);
        netdev_low_level_set_dns_server(netdev, 0, &addr);

        inet_aton(dns_server2, &addr);
        netdev_low_level_set_dns_server(netdev, 1, &addr);
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

static void ec20_check_link_status_entry(void *parameter)
{
#define EC20_LINK_RESP_SIZE     64
#define EC20_LINK_RESP_TIMO     (3 * RT_TICK_PER_SECOND)
#define EC20_LINK_DELAY_TIME    (30 * RT_TICK_PER_SECOND)

    int link_stat = 0;
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;
    struct netdev *netdev = (struct netdev *) parameter;

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get device(%s) failed.", netdev->name);
        return;
    }

    resp = at_create_resp(EC20_LINK_RESP_SIZE, 0, EC20_LINK_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("no memory for resp ceate.");
        return;
    }

    while (1)
    {
        /* send "AT+CGREG" commond  to check netweork interface device link status */
        if (at_obj_exec_cmd(device->client, resp, "AT+CGREG?") < 0)
        {
            if (netdev_is_link_up(netdev))
            {
                netdev_low_level_set_link_status(netdev, RT_FALSE);
            }

            rt_thread_mdelay(EC20_LINK_DELAY_TIME);
            continue;
        }
        else
        {
            at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %*d,%d", &link_stat);

            /* 1 Registered, home network,5 Registered, roaming */
            if (link_stat == 1 || link_stat == 5)
            {
                if (netdev_is_link_up(netdev) == RT_FALSE)
                {
                    netdev_low_level_set_link_status(netdev, RT_TRUE);
                }
            }
            else
            {
                if (netdev_is_link_up(netdev))
                {
                    netdev_low_level_set_link_status(netdev, RT_FALSE);
                }
            }
        }

        rt_thread_mdelay(EC20_LINK_DELAY_TIME);
    }
}

static int ec20_netdev_check_link_status(struct netdev *netdev)
{
#define EC20_LINK_THREAD_TICK           20
#define EC20_LINK_THREAD_STACK_SIZE     (1024 + 512)
#define EC20_LINK_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX - 2)

    rt_thread_t tid;
    char tname[RT_NAME_MAX] = {0};

    RT_ASSERT(netdev);

    rt_snprintf(tname, RT_NAME_MAX, "%s", netdev->name);

    /* create ec20 link status polling thread  */
    tid = rt_thread_create(tname, ec20_check_link_status_entry, (void *)netdev,
                           EC20_LINK_THREAD_STACK_SIZE, EC20_LINK_THREAD_PRIORITY, EC20_LINK_THREAD_TICK);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}

static int ec20_net_init(struct at_device *device);

static int ec20_netdev_set_up(struct netdev *netdev)
{
    struct at_device *device = RT_NULL;

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get device(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    if (device->is_init == RT_FALSE)
    {
        ec20_net_init(device);
        device->is_init = RT_TRUE;

        netdev_low_level_set_status(netdev, RT_TRUE);
        LOG_D("network interface device(%s) set up status.", netdev->name);
    }

    return RT_EOK;
}

static int ec20_netdev_set_down(struct netdev *netdev)
{
    struct at_device *device = RT_NULL;

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get device(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    if (device->is_init == RT_TRUE)
    {
        ec20_power_off(device);
        device->is_init = RT_FALSE;

        netdev_low_level_set_status(netdev, RT_FALSE);
        LOG_D("network interface device(%s) set down status.", netdev->name);
    }

    return RT_EOK;
}

static int ec20_netdev_set_dns_server(struct netdev *netdev, uint8_t dns_num, ip_addr_t *dns_server)
{
#define EC20_DNS_RESP_LEN    8
#define EC20_DNS_RESP_TIMEO  rt_tick_from_millisecond(300)

    int result = RT_EOK;
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;

    RT_ASSERT(netdev);
    RT_ASSERT(dns_server);

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get device(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    resp = at_create_resp(EC20_DNS_RESP_LEN, 0, EC20_DNS_RESP_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_D("no memory for resp create.");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* send "AT+QIDNSCFG=<pri_dns>[,<sec_dns>]" commond to set dns servers */
    if (at_obj_exec_cmd(device->client, resp, "AT+QIDNSCFG=1,\"%s\"", inet_ntoa(*dns_server)) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    netdev_low_level_set_dns_server(netdev, dns_num, dns_server);

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

#ifdef NETDEV_USING_PING
static int ec20_netdev_ping(struct netdev *netdev, const char *host,
        size_t data_len, uint32_t timeout, struct netdev_ping_resp *ping_resp)
{
#define EC20_PING_RESP_SIZE       128
#define EC20_PING_IP_SIZE         16
#define EC20_PING_TIMEO           (5 * RT_TICK_PER_SECOND)

    rt_err_t result = RT_EOK;
    int response = -1, recv_data_len, ping_time, ttl;
    char ip_addr[EC20_PING_IP_SIZE] = {0};
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;

    RT_ASSERT(netdev);
    RT_ASSERT(host);
    RT_ASSERT(ping_resp);

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get device(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    resp = at_create_resp(EC20_PING_RESP_SIZE, 4, EC20_PING_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_E("no memory for resp create");
        return -RT_ENOMEM;
    }

    /* send "AT+QPING="<host>"[,[<timeout>][,<pingnum>]]" commond to send ping request */
    if (at_obj_exec_cmd(device->client, resp, "AT+QPING=1,\"%s\",%d,1", host, timeout / RT_TICK_PER_SECOND) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    at_resp_parse_line_args_by_kw(resp, "+QPING:", "+QPING:%d", &response);
    /* Received the ping response from the server */
    if (response == 0)
    {
        if (at_resp_parse_line_args_by_kw(resp, "+QPING:", "+QPING:%d,\"%[^\"]\",%d,%d,%d",
                                          &response, ip_addr, &recv_data_len, &ping_time, &ttl) <= 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }
    }

    /* prase response number */
    switch (response)
    {
    case 0:
        inet_aton(ip_addr, &(ping_resp->ip_addr));
        ping_resp->data_len = recv_data_len;
        ping_resp->ticks = ping_time;
        ping_resp->ttl = ttl;
        result = RT_EOK;
        break;
    case 569:
        result = -RT_ETIMEOUT;
        break;
    default:
        result = -RT_ERROR;
        break;
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}
#endif /* NETDEV_USING_PING */



const struct netdev_ops ec20_netdev_ops =
{
    ec20_netdev_set_up,
    ec20_netdev_set_down,

    RT_NULL,
    ec20_netdev_set_dns_server,
    RT_NULL,

#ifdef NETDEV_USING_PING
    ec20_netdev_ping,
#endif
    RT_NULL,
};

static struct netdev *ec20_netdev_add(const char *netdev_name)
{
#define ETHERNET_MTU        1500
#define HWADDR_LEN          8
    struct netdev *netdev = RT_NULL;

    netdev = netdev_get_by_name(netdev_name);
    if (netdev != RT_NULL)
    {
        return (netdev);
    }

    netdev = (struct netdev *)rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        LOG_E("no memory for netdev create.");
        return RT_NULL;
    }

    netdev->mtu = ETHERNET_MTU;
    netdev->ops = &ec20_netdev_ops;
    netdev->hwaddr_len = HWADDR_LEN;

#ifdef SAL_USING_AT
    extern int sal_at_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_at_netdev_set_pf_info(netdev);
#endif

    netdev_register(netdev, netdev_name, RT_NULL);

    return netdev;
}

/* =============================  ec20 device operations ============================= */

#define AT_SEND_CMD(client, resp, resp_line, timeout, cmd)                                         \
    do {                                                                                           \
        (resp) = at_resp_set_info((resp), 128, (resp_line), rt_tick_from_millisecond(timeout));    \
        if (at_obj_exec_cmd((client), (resp), (cmd)) < 0)                                          \
        {                                                                                          \
                                                                                                   \
        }                                                                                          \
    } while(0)                                                                                     \

/* initialize for ec20 */
static void ec20_init_thread_entry(void *parameter)
{
#define INIT_RETRY                     99999
#define CIMI_RETRY                     99
#define CSQ_RETRY                      99
#define CREG_RETRY                     99
#define CGREG_RETRY                    99
#define CPIN_RETRY                     20

#define POWER_OFF_COUNT                300 *1000

    int i, qi_arg[3] = {0}, is_digit = 0;
    int retry_num = INIT_RETRY;
    unsigned int power_off_tick = rt_tick_get();
    char parsed_data[20] = {0};
    rt_err_t result = RT_EOK;
    at_response_t resp = RT_NULL;
    struct at_device *device = (struct at_device *) parameter;
    struct at_client *client = device->client;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(300));
    if (resp == RT_NULL)
    {
        LOG_E("no memory for resp create.");
        return;
    }

    LOG_D("start init %s device.", device->name);

    while (retry_num--)
    {
        is_digit = 0;
        s_at_device_appinfo.boot = 0;
        s_at_device_appinfo.at = 0;
        s_at_device_appinfo.card = 0;
        rt_memset(s_at_device_appinfo.iccid, 0x00, sizeof(s_at_device_appinfo.iccid));
        rt_memset(s_at_device_appinfo.imei, 0x00, sizeof(s_at_device_appinfo.imei));
        s_at_device_appinfo.signal_strength = 0;
        s_at_device_appinfo.cgreg = 0;
        s_at_device_appinfo.mnc = -1;
        s_at_device_appinfo.init_complete = 0;

        if((net_query_netdev_type() &NET_NETDEV_TYPE_4G) == 0){
            LOG_W("net device selected is not 4G, quit");
            break;
        }
        /* power on the ec20 device */
        if(ec20_power_on(device) >= 0){
            s_at_device_appinfo.boot = 1;
        }

        rt_thread_mdelay(1000);

        /* wait ec20 startup finish, send AT every 500ms, if receive OK, SYNC success*/
        if (at_client_obj_wait_connect(client, EC20_WAIT_CONNECT_TIME))
        {
            result = -RT_ETIMEOUT;
            goto __exit;
        }

        s_at_device_appinfo.at = 1;

        /* set response format to ATV1 */
        AT_SEND_CMD(client, resp, 0, 300, "ATV1");
        /* disable echo */
        AT_SEND_CMD(client, resp, 0, 300, "ATE0");
        /* Use AT+CMEE=2 to enable result code and use verbose values */
        AT_SEND_CMD(client, resp, 0, 300, "AT+CMEE=2");
        /* Get the baudrate */
        AT_SEND_CMD(client, resp, 0, 300, "AT+IPR?");
        at_resp_parse_line_args_by_kw(resp, "+IPR:", "+IPR: %d", &i);
        LOG_D("%s device baudrate %d", device->name, i);
        /* get module version */
        AT_SEND_CMD(client, resp, 0, 300, "ATI");
        /* show module version */
        for (i = 0; i < (int) resp->line_counts - 1; i++)
        {
            LOG_D("%s", at_resp_get_line(resp, i + 1));
        }
        /* Use AT+GSN to query the IMEI of module */
        AT_SEND_CMD(client, resp, 0, 300, "AT+GSN");

        for(uint8_t count = 0, valid_i = 0; count < resp->buf_size; count++){
            if(is_digit == 0){
                if(isdigit(resp->buf[count])){
                    is_digit = 1;
                    s_at_device_appinfo.imei[valid_i] = resp->buf[count];
                    valid_i++;
                }
            }else{
                if(isdigit(resp->buf[count]) == 0){
                    break;
                }
                s_at_device_appinfo.imei[valid_i] = resp->buf[count];
                valid_i++;
            }
        }

        /* check SIM card */
        rt_thread_mdelay(5000);
        for (i = 0; i < CPIN_RETRY; i++)
        {
            rt_thread_mdelay(1000);
            AT_SEND_CMD(client, resp, 2, 5 * 1000, "AT+CPIN?");
            if (at_resp_get_line_by_kw(resp, "READY"))
            {
                s_at_device_appinfo.card = 1;
                break;
            }
            LOG_I("search sim card again(%d)", i);
        }

        if(i == CPIN_RETRY){
            LOG_E("%s device SIM card detection failed.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }

        /* waiting for dirty data to be digested */
        rt_thread_mdelay(10);

        /* Use AT+CIMI to query the IMSI of SIM card */
        // AT_SEND_CMD(client, resp, 2, 300, "AT+CIMI");
        i = 0;
        resp = at_resp_set_info(resp, 128, 0, rt_tick_from_millisecond(300));
        while(at_obj_exec_cmd(device->client, resp, "AT+CIMI") < 0)
        {
            i++;
            if(i > CIMI_RETRY)
            {
                LOG_E("%s device read CIMI failed.", device->name);
                result = -RT_ERROR;
                goto __exit;
            }
            rt_thread_mdelay(1000);
        }

        /* Use AT+QCCID to query ICCID number of SIM card */
        AT_SEND_CMD(client, resp, 0, 300, "AT+QCCID");
        at_resp_parse_line_args_by_kw(resp, "+QCCID:", "+QCCID: %s", &s_at_device_appinfo.iccid);

        /* check signal strength */
        for (i = 0; i < CSQ_RETRY; i++)
        {
            AT_SEND_CMD(client, resp, 0, 300, "AT+CSQ");
            at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %d,%d", &qi_arg[0], &qi_arg[1]);

            s_at_device_appinfo.signal_strength = qi_arg[0];

            if (qi_arg[0] != 99)
            {
                LOG_D("%s device signal strength: %d, channel bit error rate: %d",
                        device->name, qi_arg[0], qi_arg[1]);
                break;
            }
            rt_thread_mdelay(1000);

            LOG_I("query sim card signal strength again(%d)", i);
        }
        if (i == CSQ_RETRY)
        {
            LOG_E("%s device signal strength check failed (%s)", device->name, parsed_data);
            result = -RT_ERROR;
            goto __exit;
        }
        /* check the GSM network is registered */
        for (i = 0; i < CREG_RETRY; i++)
        {
            AT_SEND_CMD(client, resp, 0, 300, "AT+CREG?");
            at_resp_parse_line_args_by_kw(resp, "+CREG:", "+CREG: %s", &parsed_data);
            if (!rt_strncmp(parsed_data, "0,1", sizeof(parsed_data)) ||
                    !rt_strncmp(parsed_data, "0,5", sizeof(parsed_data)))
            {
                LOG_D("%s device GSM is registered(%s)", device->name, parsed_data);
                break;
            }
            rt_thread_mdelay(1000);

            LOG_I("query GSM network is registered again(%d)", i);
        }
        if (i == CREG_RETRY)
        {
            LOG_E("%s device GSM is register failed (%s)", device->name, parsed_data);
            result = -RT_ERROR;
            goto __exit;
        }
        /* check the GPRS network is registered */
        for (i = 0; i < CGREG_RETRY; i++)
        {
            AT_SEND_CMD(client, resp, 0, 300, "AT+CGREG?");
            at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %s", &parsed_data);
            if (!rt_strncmp(parsed_data, "0,1", sizeof(parsed_data)) ||
                    !rt_strncmp(parsed_data, "0,5", sizeof(parsed_data)))
            {
                s_at_device_appinfo.cgreg = 1;
                LOG_D("%s device GPRS is registered(%s)", device->name, parsed_data);
                break;
            }
            rt_thread_mdelay(1000);

            LOG_I("query GPRS network is registered again(%d)", i);
        }
        if (i == CGREG_RETRY)
        {
            LOG_E("%s device GPRS is register failed (%s)", device->name, parsed_data);
            result = -RT_ERROR;
            goto __exit;
        }
        /*Use AT+CEREG? to query current EPS Network Registration Status*/
        AT_SEND_CMD(client, resp, 0, 300, "AT+CEREG?");
        /* Use AT+COPS? to query current Network Operator */
        AT_SEND_CMD(client, resp, 0, 300, "AT+COPS?");
        at_resp_parse_line_args_by_kw(resp, "+COPS:", "+COPS: %*[^\"]\"%[^\"]", &parsed_data);
        if(rt_strcmp(parsed_data,"CHINA MOBILE") == 0)
        {
            s_at_device_appinfo.mnc = 0;
            /* "CMCC" */
            LOG_I("%s device network operator: %s", device->name, parsed_data);
            AT_SEND_CMD(client, resp, 0, 300, QICSGP_CHINA_MOBILE);
        }
        else if(strcmp(parsed_data,"CHN-UNICOM") == 0)
        {
            s_at_device_appinfo.mnc = 1;
            /* "UNICOM" */
            LOG_I("%s device network operator: %s", device->name, parsed_data);
            AT_SEND_CMD(client, resp, 0, 300, QICSGP_CHINA_UNICOM);
        }
        else if(rt_strcmp(parsed_data,"CHN-CT") == 0)
        {
            s_at_device_appinfo.mnc = 2;
            /* "CT" */
            LOG_I("%s device network operator: %s", device->name, parsed_data);
            AT_SEND_CMD(client, resp, 0, 300, QICSGP_CHINA_TELECOM);
        }
        /* Enable automatic time zone update via NITZ and update LOCAL time to RTC */
        AT_SEND_CMD(client, resp, 0, 300, "AT+CTZU=3");
        /* Get RTC time */
        AT_SEND_CMD(client, resp, 0, 300, "AT+CCLK?");

        /* Deactivate context profile */
        AT_SEND_CMD(client, resp, 0, 40 * 1000, "AT+QIDEACT=1"); /* 激活场景 */
        /* Activate context profile */
        AT_SEND_CMD(client, resp, 0, 150 * 1000, "AT+QIACT=1");
        /* Query the status of the context profile */
        AT_SEND_CMD(client, resp, 0, 150 * 1000, "AT+QIACT?");
        at_resp_parse_line_args_by_kw(resp, "+QIACT:", "+QIACT: %*[^\"]\"%[^\"]", &parsed_data);
        LOG_I("%s device IP address: %s", device->name, parsed_data);

        /* initialize successfully  */
        result = RT_EOK;
        break;

    __exit:
        if (result != RT_EOK)
        {
            /* power off the ec20 device */
            if((rt_tick_get() - power_off_tick) > POWER_OFF_COUNT)
            {
                s_at_device_appinfo.boot = 0;
                ec20_power_off(device);
                power_off_tick = rt_tick_get();
                LOG_I("4G module execute power off");
            }

            LOG_I("%s device initialize retry...(%d)", device->name, (rt_tick_get() - power_off_tick));
        }
    }

    if (resp)
    {
        at_delete_resp(resp);
    }

    if (result == RT_EOK)
    {
        /* set network interface device status and address information */
        ec20_netdev_set_info(device->netdev);
        /* check and create link staus sync thread  */
        if (rt_thread_find(device->netdev->name) == RT_NULL)
        {
            //ec20_netdev_check_link_status(device->netdev);
        }

        s_at_device_appinfo.init_complete = 1;
        LOG_I("%s device network initialize success.", device->name);
    }
    else
    {
        LOG_E("%s device network initialize failed(%d).", device->name, result);
    }

}

/* ec20 device network initialize */
static int ec20_net_init(struct at_device *device)
{
#ifdef AT_DEVICE_EC20_INIT_ASYN
    rt_thread_t tid;

    tid = rt_thread_create("ec20_net", ec20_init_thread_entry, (void *)device,
                           EC20_THREAD_STACK_SIZE, EC20_THREAD_PRIORITY, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("create %s device init thread failed.", device->name);
        return -RT_ERROR;
    }
#else
    ec20_init_thread_entry(device);
#endif /* AT_DEVICE_EC20_INIT_ASYN */

    return RT_EOK;
}

static int ec20_init(struct at_device *device)
{
    struct at_device_ec20 *ec20 = (struct at_device_ec20 *) device->user_data;

    /* initialize AT client */
    at_client_init(ec20->client_name, ec20->recv_line_num);

    device->client = at_client_get(ec20->client_name);
    if (device->client == RT_NULL)
    {
        LOG_E("get AT client(%s) failed.", ec20->client_name);
        return -RT_ERROR;
    }

    /* register URC data execution function  */
#ifdef AT_USING_SOCKET
    ec20_socket_init(device);
#endif

    /* add ec20 device to the netdev list */
    device->netdev = ec20_netdev_add(ec20->device_name);
    if (device->netdev == RT_NULL)
    {
        LOG_E("add netdev(%s) failed.", ec20->device_name);
        return -RT_ERROR;
    }

    /* initialize ec20 pin configuration */
    if (ec20->power_pin != -1 && ec20->power_status_pin != -1)
    {
        rt_pin_mode(ec20->power_pin, PIN_MODE_OUTPUT);
        rt_pin_mode(ec20->power_status_pin, PIN_MODE_INPUT);
    }

    /* initialize ec20 device network */
    return ec20_netdev_set_up(device->netdev);
}

static int ec20_deinit(struct at_device *device)
{
    return ec20_netdev_set_down(device->netdev);
}

static int ec20_control(struct at_device *device, int cmd, void *arg)
{
    int result = -RT_ERROR;

    RT_ASSERT(device);

    switch (cmd)
    {
    case AT_DEVICE_CTRL_POWER_ON:
    case AT_DEVICE_CTRL_POWER_OFF:
    case AT_DEVICE_CTRL_RESET:
    case AT_DEVICE_CTRL_LOW_POWER:
    case AT_DEVICE_CTRL_SLEEP:
    case AT_DEVICE_CTRL_WAKEUP:
    case AT_DEVICE_CTRL_NET_CONN:
    case AT_DEVICE_CTRL_NET_DISCONN:
    case AT_DEVICE_CTRL_SET_WIFI_INFO:
    case AT_DEVICE_CTRL_GET_SIGNAL:
    case AT_DEVICE_CTRL_GET_GPS:
    case AT_DEVICE_CTRL_GET_VER:
        LOG_W("not support the control command(%d).", cmd);
        break;
    default:
        LOG_E("input error control command(%d).", cmd);
        break;
    }

    return result;
}

const struct at_device_ops ec20_device_ops =
{
    ec20_init,
    ec20_deinit,
    ec20_control,
};

static int ec20_device_class_register(void)
{
    struct at_device_class *class = RT_NULL;

    class = (struct at_device_class *) rt_calloc(1, sizeof(struct at_device_class));
    if (class == RT_NULL)
    {
        LOG_E("no memory for device class create.");
        return -RT_ENOMEM;
    }

    /* fill ec20 device class object */
#ifdef AT_USING_SOCKET
    ec20_socket_class_register(class);
#endif
    class->device_ops = &ec20_device_ops;

    return at_device_class_register(class, AT_DEVICE_CLASS_EC20);
}
INIT_DEVICE_EXPORT(ec20_device_class_register);

#endif /* AT_DEVICE_USING_EC20 */
