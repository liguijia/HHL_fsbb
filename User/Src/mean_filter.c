#include "mean_filter.h"

/**************************************************************************************
 * @brief   初始化一个均值滤波器。
 *          该函数设置均值滤波器的窗口大小，并将窗口中的所有元素初始化为0。
 *
 * @param   filter          指向mean_filter_t结构体的指针，代表要初始化的均值滤波器。
 * @param   size            滤波器窗口的大小，即用于计算平均值的数据点数量。
 * @version 1.0
 * @author  Hong HongLin (wxxdada@163.com)
 * @date    2025 - 01 - 11
 * @copyright Copyright (C) 2025 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
void mean_filter_init(mean_filter_t* filter, uint16_t size)
{
    // 检查提供的窗口大小是否有效。窗口大小不能为0，也不能超过预定义的最大窗口尺寸。
    if (size == 0 || size > MAX_WINDOW_SIZE)
    {
        return; // 如果窗口大小无效，则不进行任何操作并退出函数。
    }

    // 设置滤波器的窗口大小。
    filter->window_size = size;

    // 初始化环形缓冲区的头部指针。
    filter->head = 0;

    // 初始化累积和为0，用于之后计算平均值。
    filter->sum = 0;

    // 将滤波器窗口的所有元素初始化为0。
    // 这是为了确保在开始使用滤波器时，所有旧的数据都被清空。
    for (uint16_t i = 0; i < size; ++i)
    {
        filter->window[i] = 0;
    }
}

/**************************************************************************************
 * @brief   更新均值滤波器的状态。
 *          该函数接收一个新的样本数据，并更新内部状态以反映新数据，
 *          同时保持窗口大小不变。它会自动移除最旧的数据点并加入新的数据点。
 *
 * @param   filter          指向mean_filter_t结构体的指针，代表要更新的均值滤波器。
 * @param   new_sample      新的样本数据，用于更新滤波器的状态。
 * @version 1.0
 * @author  Hong HongLin (wxxdada@163.com)
 * @date    2025 - 01 - 11
 * @copyright Copyright (C) 2025 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
void mean_filter_update(mean_filter_t* filter, uint16_t new_sample)
{
    // 如果滤波器未正确初始化（即窗口大小为0），则不执行任何操作并退出函数。
    if (filter->window_size == 0)
    {
        return;
    }

    // 减去即将被替换的旧样本的值，以维护累积和的准确性。
    // 这里的filter->head指向的是下一个要被覆盖的位置，因此是当前窗口中最旧的样本。
    filter->sum -= filter->window[filter->head];

    // 更新环形缓冲区中当前位置的样本为新的样本数据。
    filter->window[filter->head] = new_sample;

    // 添加新样本的值到累积和中，以便之后计算平均值。
    filter->sum += new_sample;

    // 移动头部指针到下一个位置。使用模运算确保指针在达到窗口末尾时回绕到起始位置。
    // 这样可以实现一个环形缓冲区，从而有效地管理固定数量的数据点。
    filter->head = (filter->head + 1) % filter->window_size;
}

/**************************************************************************************
 * @brief 
 * 
 * @param  filter
 * @return uint16_t
 * @version 1.0
 * @author Hong HongLin (wxxdada@163.com)
 * @date 2025 - 01 - 11
 * @copyright Copyright (C) 2025 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
uint16_t mean_filter_calculate_average(const mean_filter_t* filter)
{
    // 如果滤波器未正确初始化，则返回0
    if (filter->window_size == 0)
    {
        return 0;
    }
    
    // 计算并返回平均值
    return (uint16_t)(filter->sum / filter->window_size);
}