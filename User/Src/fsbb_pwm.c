/**************************************************************************************
 * @file fsbb_pwm.c
 * @author Hong HongLin (wxxdada@163.com)
 * @brief
 * @version 1.0
 * @date 2024 - 12 - 11
 *
 * @copyright Copyright (C) 2024 Hong HongLin.  All Rights Reserved.
 *
 * Encoding = UTF-8
 *
 * ***********************************************************************************
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2024 Hong HongLin
 *
 * ************************************************************************************
 *
 *************************************************************************************/

#include "main.h"
#include "hrtim.h"
#include "fsbb_pwm.h"
#include "tim.h"
#include "analog_signal.h"
#include "incremental_pid.h"
#include "comm.h"

#define FSBB_GENERAL_TO_NARROW_RATIO  0.9f                   // 广义占空比到狭义占空比的比例
#define FSBB_PERIOD_FULL              (27200U)               // 周期长度全位置
#define FSBB_PERIOD_HALF              (FSBB_PERIOD_FULL / 2) // 周期长度半位置
#define FSBB_PERIOD_ZERO              (0U)                   // 周期长度零位置

#define TARGET_POWER_MAX              (200.0f) // 补血区底盘功率上限为200W
#define TARGET_POWER_MIN              (15.0f)  // 一级步兵底盘45W,虚弱状态降到1/3

#define MAX_POWERLOSED_DETECTION_TIME (1145U) // 最大掉电检测时间

incremental_pid_t pid_cap_voltage_h;
incremental_pid_t pid_cap_voltage_l;
incremental_pid_t pid_power;
incremental_pid_t pid_current;

float voltage_cap;
float voltage_motor;
float current_cap;
float current_chassis;

float general_duty = 1.0f;
float pid_cap_voltage_h_output;
float pid_cap_voltage_l_output;
float pid_power_output;

float calculatedChassisPower = 0.0f;

//
float test_target_power = 15.0f;
void fsbb_pwm_init(void)
{
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_MASTER);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_D);
}

void fsbb_pwm_output_start(void)
{
    // 先采集cap和chssis端的占空比,转换成广义占空比再启动,能最大程度减少开启瞬间的电流变化
    HAL_Delay(5);
    fsbb_pwm_output_restart();
}

void fsbb_pwm_output_restart(void)
{
    // 重新开启hrtim的时候要设置软启动
    float voltage_cap   = get_voltage_cap();
    float voltage_motor = get_voltage_motor();

    if (voltage_motor >= 20.0f && voltage_motor <= 28.0f) {
        fsbb_pwm_set_factor(voltage_cap / voltage_motor);
        pid_current.output = voltage_cap / voltage_motor;
    }

    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
}

/**************************************************************************************
 * @brief 关闭HRTIM PWM输出
 *
 * 该函数用于停止HRTIM模块中特定通道的PWM信号输出。注意，关闭PWM输出时，
 * 不应该直接关闭计数器(counter)，而应仅关闭输出(output)以避免可能的问题。
 *
 * @version 1.0
 * @author Hong HongLin (wxxdada@163.com)
 * @date 2024 - 12 - 25
 * @copyright Copyright (C) 2024 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
void fsbb_pwm_output_stop(void)
{
    // 调用HAL库函数来停止指定的HRTIM PWM输出通道。
    // 这里关闭的是TA1, TA2, TD1 和 TD2通道的PWM输出。
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);

    // 注意：可以关闭PWM输出，但切勿在未关闭输出的情况下停止计数器(counter)，
    // 否则可能会导致不可预期的行为或错误。
}

void fsbb_pwm_set_cap(float general_duty)
{
    const float general_duty_min = 0.5f; // 广义占空比最低值
    const float general_duty_max = 1.0f; // 广义占空比占空比最高值

    // 使用条件运算符限制数值
    general_duty = (general_duty < general_duty_min)   ? general_duty_min
                   : (general_duty > general_duty_max) ? general_duty_max
                                                       : general_duty;

    // 在电容端移相了 180°
    // 低侧管低电平持续时间就是高侧管高电平持续时间
    int32_t LowSide_LowLevel_Start_Comp1 = FSBB_PERIOD_HALF + general_duty * FSBB_GENERAL_TO_NARROW_RATIO * FSBB_PERIOD_HALF;

    int32_t LowSide_LowLevel_End_Comp3 = FSBB_PERIOD_HALF - general_duty * FSBB_GENERAL_TO_NARROW_RATIO * FSBB_PERIOD_HALF;

    // 低侧管低电平 and 高侧管高电平 开始
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, LowSide_LowLevel_Start_Comp1);
    // 低侧管低电平 and 高侧管高电平 结束
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, LowSide_LowLevel_End_Comp3);
}

void fsbb_pwm_set_motor(float general_duty)
{
    const float general_duty_min = 0.15f; // 广义占空比最低值
    const float general_duty_max = 1.0f;  // 广义占空比占空比最高值

    // 使用条件运算符限制数值
    general_duty = (general_duty < general_duty_min)   ? general_duty_min
                   : (general_duty > general_duty_max) ? general_duty_max
                                                       : general_duty;

    // 低侧管低电平持续时间就是高侧管高电平持续时间
    int32_t LowSide_LowLevel_Start_Comp1 = FSBB_PERIOD_ZERO + general_duty * FSBB_GENERAL_TO_NARROW_RATIO * FSBB_PERIOD_HALF;

    int32_t LowSide_LowLevel_End_Comp3 = FSBB_PERIOD_FULL - general_duty * FSBB_GENERAL_TO_NARROW_RATIO * FSBB_PERIOD_HALF;

    // 低侧管低电平 and 高侧管高电平 开始
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, LowSide_LowLevel_Start_Comp1);
    // 低侧管低电平 and 高侧管高电平 结束
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, LowSide_LowLevel_End_Comp3);
}

void fsbb_pwm_set_factor(float scaling_factor)
{
    const float factor_range_min = FACTOR_MIN; // 允许的倍数的最小值
    const float factor_range_max = FACTOR_MAX; // 允许的倍数的最大值

    // 把倍数限制在范围内
    scaling_factor = (scaling_factor < factor_range_min)   ? factor_range_min
                     : (scaling_factor > factor_range_max) ? factor_range_max
                                                           : scaling_factor;

    if (scaling_factor <= 1.0f) {
        fsbb_pwm_set_motor(scaling_factor); // 设置电机占空比
        fsbb_pwm_set_cap(1.0f);             // 设置电容为广义常开
    } else {
        fsbb_pwm_set_motor(1.0f);                // 设置电机为广义常开
        fsbb_pwm_set_cap(1.0f / scaling_factor); // 设置电容占空比
    }
}

//
static uint16_t powerlosed_cnt = 0; // 掉电计数器

void powerlosed_detection(void)
{
    if (current_cap <= -0.2f && current_chassis <= 0.2f) {
        powerlosed_cnt++;
    } else if (voltage_motor <= 19 || voltage_motor >= 27) {
        powerlosed_cnt = powerlosed_cnt + 10;
    } else {
        // 复位掉电计数器
        powerlosed_cnt = 0;
    }
    if (powerlosed_cnt >= MAX_POWERLOSED_DETECTION_TIME) {
        // 掉电保护
        fsbb_pwm_output_stop();
        HAL_GPIO_WritePin(USR_LED_GPIO_Port, USR_LED_Pin, GPIO_PIN_RESET);
    } else {
        //
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM16) {
        // 2ms定时器用于发送can消息
        can_send();

        // 2ms定时器用于计数CAN断联时间
        can_recevie_cnt_add();

        // chassis&cap负向电流时，认为下电
        powerlosed_detection();

        HAL_GPIO_TogglePin(USR_LED_GPIO_Port, USR_LED_Pin);
    } else if (htim->Instance == TIM6) {
        // adc线性映射
        voltage_cap   = get_voltage_cap();
        voltage_motor = get_voltage_motor();

        current_cap     = get_current_cap();
        current_chassis = get_current_chassis();

        // test

        DcdcOutputState dcdc_output_state = UpdateDcdcOutputState(can_rx_data.enabled);

        // DcdcOutputState dcdc_output_state = DCDC_OUTPUT_OUTPUT_ENABLED;

        //

        // 如果CAN断联超时了
        if (CAN_DISCONNECT_MAX_COUNT <= can_recevie_cnt_get()) {
            can_rx_data.enabled = 0;
            dcdc_output_state   = DCDC_OUTPUT_OUTPUT_DISABLED;
        }

        // 更新dcdc输出状态
        if (DCDC_OUTPUT_OUTPUT_DISABLED == dcdc_output_state) {
            fsbb_pwm_output_stop();
            HAL_GPIO_WritePin(USR_LED_GPIO_Port, USR_LED_Pin, GPIO_PIN_SET);
            can_rx_data.targetChassisPower = DEFAULT_TARGET_POWER;
        } else if (DCDC_OUTPUT_TRANSITION_TO_DISABLED == dcdc_output_state) {
            fsbb_pwm_output_stop();
        } else if (DCDC_OUTPUT_TRANSITION_TO_ENABLED == dcdc_output_state) {

            my_pid_init();
            fsbb_pwm_output_restart();
        }

        if (DCDC_OUTPUT_OUTPUT_ENABLED == dcdc_output_state) {
            HAL_GPIO_WritePin(USR_LED_GPIO_Port, USR_LED_Pin, GPIO_PIN_RESET);
            // 计算chassis端的功率值
            calculatedChassisPower = voltage_motor * current_chassis;

            // error检查
            // 暂无

            // pid环路计算

            // test
            pid_power.setValue = (float)can_rx_data.targetChassisPower;

            // pid_power.setValue = test_target_power;

            if (pid_power.setValue >= TARGET_POWER_MAX) {
                pid_power.setValue = TARGET_POWER_MAX;
            } else if (pid_power.setValue <= TARGET_POWER_MIN) {
                pid_power.setValue = TARGET_POWER_MIN;
            }

            pid_cap_voltage_h_output = incremental_pid_compute(&pid_cap_voltage_h, voltage_cap);
            pid_cap_voltage_l_output = incremental_pid_compute(&pid_cap_voltage_l, voltage_cap);
            pid_power_output         = incremental_pid_compute(&pid_power, calculatedChassisPower);

            float current_ref = pid_power_output;
            if (current_ref > pid_cap_voltage_h_output) {
                pid_power.output = pid_cap_voltage_h_output;
                current_ref      = pid_cap_voltage_h_output;
            } else if (current_ref < pid_cap_voltage_l_output) {
                pid_power.output = pid_cap_voltage_l_output;
                current_ref      = pid_cap_voltage_l_output;
            } else {
                pid_cap_voltage_h.output = current_ref;
                pid_cap_voltage_l.output = current_ref;
            }

            pid_current.setValue = current_ref;
            general_duty         = incremental_pid_compute(&pid_current, current_cap);
            // pwm输出
            fsbb_pwm_set_factor(general_duty);
        } else {
            // Do nothing
        }
    }
}
