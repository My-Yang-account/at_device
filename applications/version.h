/**
  ******************************************************************************
  * @file
  * @author
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __VERSION_H
#define __VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_ofsm.h"

                                         /*  ([1.2.5 (修复四组模块的柔性切换模式下，一组继电器被误断问题)]
                                         [1.3.7 为能佳新板子将能佳默认IP、端口、二维码前缀写入flash]
                                         [1.3.8为能佳修改并充时CCS输出电流值]
                                         [1.4.1 为模块离线或故障也不踢出，去掉CCS与BCS电流比较，保留CCS与BCL电流比较]
                                         [1.4.2 为模块离线或故障也不踢出，去掉CCS与BCS电流比较，去掉CCS与BCL电流比较]
                                         [1.4.3 为模块离线或故障则踢出，CCS电流与模块电流比较，模块电流与BCL电流比较，仅用于测试]
                                         [1.4.8 为打印开log查灯全亮和费率不显示问题，最后提交日期：20241016 16:03]
                                         [1.4.9 为打印开log费率不显示问题(增加打印信息)]
                                         [1.5.1 为打开电流比较，测试版本])
                                         [1.6.2 能佳测试版本(联网会断开)、绍兴临时版本]
                                         [1.6.3 能佳版本(修复断网问题)]
                                         [1.6.8 /1.7.0 为不启用网络部分、去掉联网图标、屏幕二维码只显示桩号+枪号]
                                         [1.7.3(单枪) 最后一次提交(增加厂商编码、注册码屏幕可输入，修改注册码后会清空三元组) 2025/05/06]
                                         [1.9.8(双枪) 最后一次提交(fix: 故障停也可以二次启动 + 并充模式下才发CFC报文) 2025/05/13]
                                         [2.0.0(双枪国网) 最后一次提交(fix: 上报模块故障开启重发机制) 2025/06/07]
                                         [2.0.2(双枪) 最后一次提交(fix: 不使能并联继电器时不检测并联继电器反馈) 2025/06/25]
                                         [2.0.3(双枪) 修改温度检测算法，继电器、电锁反馈检测增加滤波 2025/06/25]
                                         [2.0.4(双枪) 充电结束清除SOC，修复并充自动识别失败问题[空闲时总线有数据] 2025/07/24]
                                         [1.7.5(单枪) 屏幕使能本地停止功能后，无论何种方式启动都可以本地停止 2025/07/25]
                                         [2.0.H(2.0.7 双枪)  电瑞储能+特来电平台版本 2025/08/25]
                                         [1.7.H(1.7.7 单枪)  更新CROAA 阶段先发CCS(状态切换到CCS) 2025/08/12]
                                         [2.1.A(2.1.0 双枪)  实验测试：1.充电结束2min后风扇停转   2.启动时BHM最大允许值小于系统最小电压时报电池电压故障停充   3.CC1 4V下线改为 3.3V) 2025/10/10]
                                         [2.1.B(2.1.1 双枪枪)  实验测试 2025/10/13]
                                         [1.8.1(1.8.B 单枪) 飞宇- 广州-车会发标准帧报文，程序接收全部按扩展帧来，导致误判；处理：CAN帧正常接收，但是只处理扩展帧 2025/11/17]  */

#ifdef APP_USING_DOUBLEGUN

#define APP_SOFT_MODULE_USING_GB_WHOLE_7103F_ZG                 /* 软件型号使用国标双枪一体机7103F-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_GB_WHOLE_7104C_ZG                 /* 软件型号使用国标双枪一体机7104C-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_GB_TERMINAL_7103F_ZG              /* 软件型号使用国标双枪终端7103F-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_GB_HCABINET_7103F_ZG              /* 软件型号使用国标半矩主机柜7103F-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_GB_WCABINET7103F_ZG               /* 软件型号使用国标全矩主机柜7103F-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_EN_TCU_7103F_ZG                   /* 软件型号使用欧标含TCU7103F-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_EN_NTCU_7103F_ZG                  /* 软件型号使用欧标无TCU7103F-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_GB_SGUN_HOST_7103F_H7             /* 软件型号使用国标多枪主板7103F-470H7(芯片型号) */
//#define APP_SOFT_MODULE_USING_GB_SGUN_SLAVE_7103F_VG            /* 软件型号使用国标多枪从板7103F-470VG(芯片型号) */

#if defined(APP_SOFT_MODULE_USING_GB_WHOLE_7103F_ZG)
#define SOFTWARE_MODULE                                         "7103-V31"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_WHOLE_7104C_ZG)
#define SOFTWARE_MODULE                                         "7103-V41"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_TERMINAL_7103F_ZG)
#define SOFTWARE_MODULE                                         "7103-V51"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_HCABINET_7103F_ZG)
#define SOFTWARE_MODULE                                         "7103-V71"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_WCABINET7103F_ZG)
#define SOFTWARE_MODULE                                         "7103-V81"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_EN_TCU_7103F_ZG)
#define SOFTWARE_MODULE                                         "7103-V91"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_EN_NTCU_7103F_ZG)
#define SOFTWARE_MODULE                                         "7103-VA1"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_SGUN_HOST_7103F_H7)
#define SOFTWARE_MODULE                                         "7103-VC1"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_SGUN_SLAVE_7103F_VG)
#define SOFTWARE_MODULE                                         "7103-VC2"          /* 软件型号 */
#else
#define SOFTWARE_MODULE                                         "7103-V30"          /* 软件型号 */
#endif /* APP_SOFT_MODULE_USING_GB_WHOLE_7103F_ZG */

#define SOFTWARE_VERSION       2L
#define SOFTWARE_SUBVERSION    1
#define SOFTWARE_REVISION      4L  /* 01 */

#else

//#define APP_SOFT_MODULE_USING_GB_TERMINAL_7101H_ZG              /* 软件型号使用国标单枪终端7101H-470ZG(芯片型号) */
#define APP_SOFT_MODULE_USING_GB_WHOLE_7101H_ZG                 /* 软件型号使用国标单枪一体机7101H-470ZG(芯片型号) */
//#define APP_SOFT_MODULE_USING_EN_WHOLE_7101H_ZG                 /* 软件型号使用欧标单枪一体机7101H-470ZG(芯片型号) */

#if defined(APP_SOFT_MODULE_USING_GB_TERMINAL_7101H_ZG)
#define SOFTWARE_MODULE                                         "7101-V61"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_GB_WHOLE_7101H_ZG)
#define SOFTWARE_MODULE                                         "7101-V21"          /* 软件型号 */
#elif defined(APP_SOFT_MODULE_USING_EN_WHOLE_7101H_ZG)
#define SOFTWARE_MODULE                                         "7101-VB1"          /* 软件型号 */
#else
#define SOFTWARE_MODULE                                         "7101-V20"          /* 软件型号 */
#endif /* APP_SOFT_MODULE_USING_GB_TERMINAL_7101H_ZG */

#define SOFTWARE_VERSION       1L
#define SOFTWARE_SUBVERSION    8L
#define SOFTWARE_REVISION      2L  /* 02 */

#endif /* APP_USING_DOUBLEGUN */

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
