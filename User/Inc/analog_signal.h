#pragma once
#ifndef __ANALOG_SIGNAL_H__
#define __ANALOG_SIGNAL_H__

extern void BSP_ADC_Convert_Start(void);

extern float get_voltage_chassis();
extern float get_voltage_motor();
extern float get_voltage_cap();
extern float get_current_chassis();
extern float get_current_motor();
extern float get_current_cap();

#endif // !__ANALOG_SIGNAL_H__
