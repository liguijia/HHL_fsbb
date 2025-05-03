#pragma once
#ifndef __FSBB_PWM_H__
    #define __FSBB_PWM_H__
    #ifdef __cplusplus
extern "C"
{
    #endif

    #include "incremental_pid.h"

    #define DEFAULT_TARGET_POWER (45.0f) // 默认目标功率为45W,这是一级血量优先步兵的功率
    
    #define CAP_VOLTAGE_MAX      (26.0f) // 电容组最大电压
    #define CAP_VOLTAGE_MIN      (8.0f)  // 电容组最小电压
    #define CAP_CURRENT_MAX      (15.0f) // 电容组最大电流

    #define FACTOR_MAX           (1.23f) // 27V电容组 / 22V 底盘
    #define FACTOR_MIN           (0.15f) // 4V电容组 / 26V 底盘

    extern void fsbb_pwm_init(void);
    extern void fsbb_pwm_output_start(void);
    extern void fsbb_pwm_output_restart(void);
    extern void fsbb_pwm_output_stop(void);
    extern void fsbb_pwm_set_cap(float general_duty);
    extern void fsbb_pwm_set_motor(float general_duty);
    extern void fsbb_pwm_set_factor(float scaling_factor);

    extern incremental_pid_t pid_cap_voltage_h;
    extern incremental_pid_t pid_cap_voltage_l;
    extern incremental_pid_t pid_power;
    extern incremental_pid_t pid_current;

    #ifdef __cplusplus
}
    #endif
#endif // !__FSBB_PWM_H__
