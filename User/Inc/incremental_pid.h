#pragma once
#ifndef __INCREMENTAL_PID_H__
    #define __INCREMENTAL_PID_H__

    #ifdef __cplusplus
extern "C"
{
    #endif

    typedef struct incremental_pid_t_
    {
        float actualValue;    // 实际值 每次反馈回来的值
        float Kd, Ki, Kp;     // 比例、积分、微分常数
        float P, I, D;        // 比例项 积分项 微分项
        float error;          // 本次误差值
        float errorPre;       // E[k-1] 上一次误差
        float errorPrePre;    // E[k-2] 上上次误差
        float setValue;       // 设定值 你期望系统达到的值
        float output;         // 当前输出值
        float outputMaxLimit; // 输出的最大限制
        float outputMinLimit; // 输出的最小限制
    } incremental_pid_t;

    void incremental_pid_init(incremental_pid_t* pid, float kp, float ki, float kd, float min_output, float max_output);
    float incremental_pid_compute(incremental_pid_t* pid, float newActualValue);
    void  incremental_pid_reset(incremental_pid_t* pid);

    #ifdef __cplusplus
}
    #endif

#endif // !__INCREMENTAL_PID_H__
