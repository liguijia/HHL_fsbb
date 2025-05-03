#pragma once
#ifndef __MEAN_FILTER_H__
#define __MEAN_FILTER_H__

#include <stdint.h>

// 定义均值滤波器的最大窗口大小
#define MAX_WINDOW_SIZE 64

// 均值滤波器结构体
typedef struct
{
    uint16_t window[MAX_WINDOW_SIZE]; // 存储样本的窗口
    uint16_t window_size;             // 窗口大小
    uint16_t head;                    // 指向最新样本的位置
    uint32_t sum;                     // 窗口内样本的总和
} mean_filter_t;

// 初始化均值滤波器
void mean_filter_init(mean_filter_t* filter, uint16_t size);

// 更新均值滤波器状态
void mean_filter_update(mean_filter_t* filter, uint16_t new_sample);

// 计算当前平均值
uint16_t mean_filter_calculate_average(const mean_filter_t* filter);

#endif // !__MEAN_FILTER_H__
