#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void Motor_Init(void);
void Motor_SetPWM(int16_t pwm);
void Motor_Stop(void);

#endif
