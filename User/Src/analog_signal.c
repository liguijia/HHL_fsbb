/**************************************************************************************
 * @file analog_signal.c
 * @author Hong HongLin (wxxdada@163.com)
 * @brief
 * @version 1.0
 * @date 2024 - 12 - 12
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
#include "analog_signal.h"
#include "mean_filter.h"
#include "adc.h"

// 定义数据类型和结构体
typedef struct
{
    float k;
    float b;
} adc_calibration_t;

typedef struct
{
    uint32_t stm32id[3];         // 96 bit stm32 id
    adc_calibration_t v_motor; // motor电压线性拟合数据
    adc_calibration_t i_chassis; // chassis电流线性拟合数据
    adc_calibration_t v_cap;     // cap电压线性拟合数据
    adc_calibration_t i_cap;     // cap电流线性拟合数据
} board_adc_calibration_t;

// 全局变量定义
static board_adc_calibration_t adc_cali_array[] = {
    // board_adc_calibration insert start
    {
        {0x00000000, 0x00000000, 0x00000222},
        {0.0005554504f, 0},
        {0.0004999009f, -16.28000000f},
        {0.0005544604f, 0},
        {0.0004996450f, -16.35000000f},
    },
    {
        {0x00000000, 0x00000000, 0x00000111},
        {0.0005204504f, -0.2722371015f},
        {0.0010059302f, -0.6575082288f},
        {0.0009980598f, -32.6001642915f},
        {0.0005210866f, -0.2409683303f},
    }
    // board_adc_calibration insert stop
};

#define ADC1_DATA_LEN (1U)
#define ADC2_DATA_LEN (2U)
#define ADC3_DATA_LEN (1U)

uint16_t adc1_data[ADC1_DATA_LEN * 2] = {0};
uint16_t adc2_data[ADC2_DATA_LEN * 2] = {0};
uint16_t adc3_data[ADC3_DATA_LEN * 2] = {0};

#define FILTER_WINDOW_SIZE 8 // 定义滤波窗口大小

static mean_filter_t v_cap_filter = {0};
// static mean_filter_t v_chassis_filter = {0};
static mean_filter_t i_cap_filter   = {0};
static mean_filter_t v_motor_filter = {0};
// static mean_filter_t i_motor_filter   = {0};
static mean_filter_t i_chassis_filter = {0};

// 线性映射,用于将ADC采样值映射到实际值
static inline float linear_map(float value, adc_calibration_t calibration)
{
    return calibration.k * value + calibration.b;
}

float get_voltage_motor()
{
    uint16_t adc_value_average = mean_filter_calculate_average(&v_motor_filter);
    float mapped_value         = linear_map(adc_value_average, adc_cali_array[0].v_motor);
    return mapped_value;
}

float get_voltage_cap()
{
    uint16_t adc_value_average = mean_filter_calculate_average(&v_cap_filter);
    float mapped_value         = linear_map(adc_value_average, adc_cali_array[0].v_cap);
    return mapped_value;
}

float get_current_chassis()
{
    uint16_t adc_value_average = mean_filter_calculate_average(&i_chassis_filter);
    float mapped_value         = linear_map(adc_value_average, adc_cali_array[0].i_chassis);
    return mapped_value;
}

// float get_current_motor()
// {
//     uint16_t adc_value_average = mean_filter_calculate_average(&i_motor_filter);
//     float    mapped_value      = linear_map(adc_value_average, adc_cali_array[0].i_motor);
//     return mapped_value;
// }

float get_current_cap()
{
    uint16_t adc_value_average = mean_filter_calculate_average(&i_cap_filter);
    float mapped_value         = linear_map(adc_value_average, adc_cali_array[0].i_cap);
    return mapped_value;
}

void BSP_ADC_Convert_Start(void)
{
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_Delay(5);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_data, ADC1_DATA_LEN * 2);

    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_Delay(5);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc2_data, ADC2_DATA_LEN * 2);

    HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
    HAL_Delay(5);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc3_data, ADC3_DATA_LEN * 2);

    mean_filter_init(&v_cap_filter, FILTER_WINDOW_SIZE);
    mean_filter_init(&v_motor_filter, FILTER_WINDOW_SIZE);
    mean_filter_init(&i_cap_filter, FILTER_WINDOW_SIZE);
    // mean_filter_init(&i_motor_filter, FILTER_WINDOW_SIZE);
    mean_filter_init(&i_chassis_filter, 32);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        mean_filter_update(&i_chassis_filter, adc1_data[0]);
    } else if (hadc->Instance == ADC2) {
        mean_filter_update(&i_cap_filter, adc2_data[0]);
        mean_filter_update(&v_cap_filter, adc2_data[1]);
    } else if (hadc->Instance == ADC3) {
        mean_filter_update(&v_motor_filter, adc3_data[0]);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        mean_filter_update(&i_chassis_filter, adc1_data[0 + ADC1_DATA_LEN]);
    } else if (hadc->Instance == ADC2) {
        mean_filter_update(&i_cap_filter, adc2_data[0 + ADC2_DATA_LEN]);
        mean_filter_update(&v_cap_filter, adc2_data[1 + ADC2_DATA_LEN]);
    } else if (hadc->Instance == ADC3) {
        mean_filter_update(&v_motor_filter, adc3_data[0 + ADC3_DATA_LEN]);
    }
}
