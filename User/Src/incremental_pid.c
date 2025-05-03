#include "incremental_pid.h"
#include <stdio.h>
#include <string.h> // For memset

/**************************************************************************************
 * @brief   初始化增量式PID控制器结构体，并设置其比例、积分、微分系数及输出限幅。
 *
 * @param  pid              指向incremental_pid_t结构体的指针，用于存储PID控制器的相关数据。
 * @param  kp               PID控制器的比例系数（Proportional gain）。
 * @param  ki               PID控制器的积分系数（Integral gain）。
 * @param  kd               PID控制器的微分系数（Derivative gain）。
 * @param  min_output       PID控制器输出的最小限制值。
 * @param  max_output       PID控制器输出的最大限制值。
 * @version 1.0
 * @author Hong HongLin (wxxdada@163.com)
 * @date 2025 - 01 - 11
 * @copyright Copyright (C) 2025 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
void incremental_pid_init(incremental_pid_t* pid, float kp, float ki, float kd, float min_output, float max_output)
{
    memset(pid, 0, sizeof(incremental_pid_t)); // 将结构体pid的所有成员初始化为0

    pid->Kp             = kp;         // 设置比例系数
    pid->Ki             = ki;         // 设置积分系数
    pid->Kd             = kd;         // 设置微分系数
    pid->outputMinLimit = min_output; // 设置输出下限
    pid->outputMaxLimit = max_output; // 设置输出上限
}

/**************************************************************************************
 * @brief   计算增量式PID控制器的输出。
 *          该函数根据新的实际值计算PID控制算法，并返回更新后的输出值。
 *
 * @param   pid              指向incremental_pid_t结构体的指针，包含PID控制器的所有参数和状态变量。
 * @param   newActualValue   新测量到的实际值（例如传感器读数）。
 * @return  float            返回经过PID计算后的新输出值。
 * @version 1.0
 * @author  Hong HongLin (wxxdada@163.com)
 * @date    2025 - 01 - 11
 * @copyright Copyright (C) 2025 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
float incremental_pid_compute(incremental_pid_t* pid, float newActualValue)
{
    // 更新PID控制器中的实际值为最新的测量值
    pid->actualValue = newActualValue;

    // 初始化增量变量
    float increment;

    // 计算当前误差（设定值与实际值之差）
    pid->error = pid->setValue - pid->actualValue;

    // 计算比例项：当前误差的变化量乘以比例系数Kp
    pid->P = pid->Kp * (pid->error - pid->errorPre);

    // 计算积分项：当前误差乘以积分系数Ki
    pid->I = pid->Ki * pid->error;

    // 计算微分项：误差变化率乘以微分系数Kd
    pid->D = pid->Kd * (pid->error - 2 * pid->errorPre + pid->errorPrePre);

    // 根据比例、积分和微分项计算总的增量
    increment = pid->P + pid->I + pid->D;

    // 更新输出值并添加增量
    pid->output += increment;

    // 对输出值进行限幅处理，确保不超过最大或最小限制
    if (pid->output > pid->outputMaxLimit)
    {
        pid->output = pid->outputMaxLimit;
    }
    else if (pid->output < pid->outputMinLimit)
    {
        pid->output = pid->outputMinLimit;
    }

    // 更新前两次的误差值，用于下一次计算
    pid->errorPrePre = pid->errorPre;
    pid->errorPre    = pid->error;

    // 返回更新后的输出值
    return pid->output;
}

/**************************************************************************************
 * @brief   重置增量式PID控制器的状态，但保留其配置参数。
 *          此函数将重置所有与控制过程有关的状态变量，
 *          包括误差、输出值等，而不影响诸如比例、积分、微分增益和设定值等配置参数。
 *
 * @param   pid 指向incremental_pid_t结构体的指针，包含PID控制器的所有参数和状态变量。
 * @version 1.0
 * @author  Hong HongLin (wxxdada@163.com)
 * @date    2025 - 01 - 11
 * @copyright Copyright (C) 2025 Hong HongLin.  All Rights Reserved.
 *************************************************************************************/
void incremental_pid_reset(incremental_pid_t* pid)
{
    // 保存不需要重置的配置参数，即PID增益和输出限制以及设定值
    float kp         = pid->Kp;
    float ki         = pid->Ki;
    float kd         = pid->Kd;
    float setValue   = pid->setValue;
    float min_output = pid->outputMinLimit;
    float max_output = pid->outputMaxLimit;

    // 使用memset将PID控制器结构体中的所有成员初始化为零
    memset(pid, 0, sizeof(incremental_pid_t));

    // 恢复之前保存的配置参数，确保PID控制器的行为不会改变
    pid->Kp             = kp;
    pid->Ki             = ki;
    pid->Kd             = kd;
    pid->setValue       = setValue;
    pid->outputMinLimit = min_output;
    pid->outputMaxLimit = max_output;
}