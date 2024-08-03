#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "protocol_data_def.h"

#define EVS_LINKKIT_OPEN 0
#define EVS_LINKKIT_CONNECT 1
#define EVS_LINKKIT_POLL 2

#define EVS_LINKKIT_OPEN_FAULT -1    // 设备linkkit打开失败
#define EVS_LINKKIT_OPEN_WAIT -2     // 设备linkkit打开重试等待
#define EVS_LINKKIT_CONNECT_FAULT -3 // 设备linkkit连接失败
#define EVS_LINKKIT_CONNECT_WAIT -4  // 设备linkkit连接重试等待
#define EVS_LINKKIT_YIELD_FAULT -5   // 设备linkkit接收失败

#define EVS_IS_NOT_READY -1          // 设备未准备就绪
#define EVS_GET_REG_CODE_FAULT -2    // 获取设备注册码失败
#define EVS_SET_CERT_FAULT -3        // 设置设备证书失败

/*****************************智能枪读卡器相关********************/

#define EVS_OPT_READER_SUCCESS 0    // 读卡器操作成功
#define EVS_SET_READER_BAUD_FAIL -1 // 设置读卡器波特率失败
#define EVS_CHECK_READER_FAIL -2    // 获取读卡器版本失败
#define EVS_CTRL_BEEMER_FAIL -3     // 设置蜂鸣器失败
#define EVS_OPEN_RF_FAIL -4         // 打开射频失败
#define EVS_CLOSE_RF_FAIL -5        // 关闭射频失败

#define EVS_HANDLE_AUTHING 0              // 正在鉴权
#define EVS_HANDLE_AUTH_SUCCESS 1         // 鉴权成功
#define EVS_HANDLE_CARD_FAILED -1         // 读卡失败
#define EVS_HANDLE_TERMINAL_TIMEOUT -2    // 获取终端机编号服务超时
#define EVS_HANDLE_GET_MAC1 -3            // 读取卡内MAC1失败
#define EVS_HANDLE_AUTHCODE_TIMEOUT -4    // 获取加密机鉴权码服务超时
#define EVS_HANDLE_CHECK_MAC2 -5          // 验证MAC2失败
#define EVS_HANDLE_AUTH_RESULT_TIMEOUT -6 // 获取鉴权结果服务超时
#define EVS_HANDLE_GUN_NUM_WRONG -7       // 枪编号错误
#define EVS_HANDLE_GUN_PULL_OUT -8        // 鉴权中拔枪

#define EVS_CARD_ERROR_READER_COMM 10        // 读卡器通信异常
#define EVS_CARD_ERROR_OPEN_RF 11            // 开射频失败
#define EVS_CARD_ERROR_FOUND_CARD 12         // 寻卡失败
#define EVS_CARD_ERROR_GET_USER_ID 13        // 获取用户ID失败
#define EVS_CARD_ERROR_GET_PSY_ID 14         // 获取物理卡号失败
#define EVS_CARD_ERROR_CHECK_PIN 15          // 验密失败
#define EVS_CARD_ERROR_GET_KEY_VER 16        // 获取秘钥版本失败
#define EVS_CARD_ERROR_GET_MAC1 17           // 获取MAC1失败
#define EVS_CARD_ERROR_CHECK_MAC2 18         // 获取MAC1失败
#define EVS_CARD_ERROR_CLOSE_RF 19           // 关射频失败
#define EVS_CARD_ERROR_TERMINAL_FAULT 20     // 获取终端机编号数据异常
#define EVS_CARD_ERROR_TERMINAL_TIMEOUT 21   // 获取终端机编号超时
#define EVS_CARD_ERROR_CHECK_MAC1_FAILED 22  // 平台验证MAC1失败
#define EVS_CARD_ERROR_CHECK_MAC1_TIMEOUT 23 // 平台验证MAC1超时
#define EVS_CARD_ERROR_AUTH_TIMEOUT 24       // 接收启动鉴权结果超时
#define EVS_CARD_ERROR_AUTH_FAILED 25        // 鉴权结果不允许启动
#define EVS_CARD_ERROR_GUN_PULL_OUT 26       // 鉴权过程中用户拔枪

    /*************************************************************/

    /**
     *函数名称： int evs_card_ctrl_init(int fd)
     *函数功能： 当智能卡初始化成功后可操作智能充电枪读取车辆枪座智能卡信息
     *
     *
     *输入参数： fd
     *读卡器通讯句柄
     *返回值:
     *
     *
     *EVS_OPT_READER_SUCCESS 0    //读卡器操作成功
     *EVS_SET_READER_BAUD_FAIL -1 //设置读卡器波特率失败
     *EVS_CHECK_READER_FAIL -2  //获取读卡器版本失败
     *EVS_CTRL_BEEMER_FAIL -3     //设置蜂鸣器失败
     *EVS_OPEN_RF_FAIL -4         //打开射频失败
     *EVS_CLOSE_RF_FAIL -5        //关闭射频失败
     **/
    int evs_card_ctrl_init(int fd);

    /**
     *函数名称： int evs_card_handle(int fd, evs_smart_gun_auth_param *param)
     *函数功能：  初始化智能卡充电枪
     *
     *
     *输入参数： fd
     *读卡器通讯句柄
     *返回值:
     *
     *
     *EVS_OPT_READER_SUCCESS 0    //读卡器操作成功
     *EVS_SET_READER_BAUD_FAIL -1 //设置读卡器波特率失败
     *EVS_CHECK_READER_FAIL -2  //获取读卡器版本失败
     *EVS_CTRL_BEEMER_FAIL -3     //设置蜂鸣器失败
     *EVS_OPEN_RF_FAIL -4         //打开射频失败
     *EVS_CLOSE_RF_FAIL -5        //关闭射频失败
     **/
    int evs_card_handle(int fd, evs_smart_gun_auth_param *param);

    /**
     *函数名称： int evs_linkkit_file_upload(int fd, evs_smart_gun_auth_param *param)
     *函数功能：  upload file to IOT
     *
     *
     *输入参数： filePath
     *file path
     *返回值:
     *
     *
     *EVS_UPLOAD_SUCCESS 0        //upload file success
     *EVS_UPLOAD_FAIL -1          //upload file failed
     **/
    int evs_linkkit_file_upload(char *filePath);

    /*
函数名称： int evs_linkkit_new(const int evs_access,const int is_device_uid)
函数功能： 当设备就绪后（可以读取到设备的设备证书信息和设备注册码），创建SDK套件
输入参数： evs_access
ACCESS_IS_CUSTOM 3：自定义
ACCESS_IS_TEST 2：  接入测试平台
ACCESS_IS_DEBUG 1： 接入调试平台
ACCESS_IS_FORMAL 0: 接入正式平台
is_device_uid
0:使用注册码获取证书
1：使用设备唯一编码获取证书
返回值:
0                          设备执行成功
EVS_IS_NOT_READY           设备未准备就绪
EVS_GET_REG_CODE_FAULT     获取设备注册码失败
EVS_SET_CERT_FAULT         设置设备证书失败
*/
    int evs_linkkit_new(const int evs_access, const int is_device_uid);

    /*
函数名称： int evs_linkkit_time_sync(void)
函数功能： 当设备与iot平台连接成功后，进行校时
输入参数： 无

返回值:
0   校时请求发送成功
1  校时 请求发送失败
*/
    int evs_linkkit_time_sync(void);

    /*
函数名称： int evs_linkkit_fota(unsigned char *buffer, int buffer_length)
函数功能： 当iot平台连下发固件升级时，进行ota固件包下载
输入参数：
1 buffer固件缓存区
2 buffer_length 缓存区大小
返回值:
0   ota固件下载成功
1   ota固件下载失败
*/
    int evs_linkkit_fota(unsigned char *buffer, int buffer_length);

    /*
函数名称： int evs_linkkit_free(void)
函数功能： 释放SDK套件
输入参数： 无
*/
    int evs_linkkit_free(void);

    /*
函数名称： void evs_set_firmware_version(const char *verion)
函数功能： 固件信息设置函数
输入参数： verion固件版本信息
返回值:无
*/
    void evs_set_firmware_version(const char *verion);

    int evs_mainopen(void);
    int evs_mainconnect(void);
    int evs_mainclose(void);
    int evs_mainyield(void);

    /**
     * @brief Send pile event data to SDK.
     *---
     * @param [in] event_type: @n the event you want to send.
     * @param [in] param: @n the event data will be written.
     * @return failed -1 success 0.
     * @see None.
     * @note None.
     */
    void evs_send_event(evs_cmd_event_enum event_type, void *param);

    /**
 *
 * 函数 evs_send_property() SDK内部实现的事件发送接口, 供使用者调用。
 * ---
 * Interface of evs_send_property() implemented by SDK， provide for user of SDK.
 *
 * 
 */
    /**
     * @brief Send pile porperty data to SDK.
     *---
     * @param [in] event_type: @n the property you want to send.
     * @param [in] param: @n the property data will be written.
     * @return failed -1 success 0.
     * @see None.
     * @note None.
     */
    void evs_send_property(evs_cmd_property_enum property_type, void *param);

#ifdef __cplusplus
}
#endif
#endif /* interface.h */
