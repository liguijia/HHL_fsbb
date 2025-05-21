// 此文件定义CAN通讯
#pragma once
#ifndef __COMM_H__
#define __COMM_H__

#include "main.h"

#define RMCS_ID     (0x1FE)
#define LEGGED_ID   (0x427)
#define SUPERCAP_ID (0x300)
// #define SUPERCAP_ID              (0x209)//test
#define CAN_DISCONNECT_MAX_COUNT (500)

typedef enum {
    DCDC_OUTPUT_OUTPUT_DISABLED,        // 关闭输出
    DCDC_OUTPUT_TRANSITION_TO_ENABLED,  // 从关闭到开启的过渡状态
    DCDC_OUTPUT_TRANSITION_TO_DISABLED, // 从开启到关闭的过渡状态
    DCDC_OUTPUT_OUTPUT_ENABLED          // 开启输出
} DcdcOutputState;

typedef struct
{
    uint8_t targetChassisPower; // 底盘功率
    uint8_t enabled;            // 是否开启栅极驱动
} RxData;

typedef struct
{
    uint16_t chassis_power;    // 底盘功率
    uint16_t supercap_voltage; // 超级电容电压
    uint16_t chassis_voltage;  // 底盘电压
    uint8_t enabled;           // 栅极驱动开启状态
    uint8_t unused;            // 未使用
} TxData;

extern DcdcOutputState get_dcdc_output_state(void);
extern RxData can_rx_data;
extern TxData can_tx_data;
extern void comm_init(void);
extern void can_send(void);
extern DcdcOutputState UpdateDcdcOutputState(uint8_t IsEnabled);
extern void can_recevie_cnt_add(void);
extern void can_recevie_cnt_reset(void);
extern uint16_t can_recevie_cnt_get(void);

// 下面是发过来的消息
// struct SupercapStatus
// {
//     uint16_t chassis_power;
//     uint16_t supercap_voltage;
//     uint16_t chassis_voltage;
//     uint8_t  enabled;
//     uint8_t  unused;
// };

// struct SupercapCommand
// {
//     uint8_t power_limit;
//     bool    enabled;
// };

#endif // !__COMM_H__
