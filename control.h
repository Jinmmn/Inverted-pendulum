#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

#define CENTER_ANGLE_DEFAULT  2010u
#define LOCATION_TARGET_MAX   4080.0f
#define LOCATION_STEP         408.0f

void Control_Init(void);
void Control_Start(void);
void Control_Stop(void);
void Control_Tick1ms(void);
void Control_AddLocationTarget(int16_t step);

uint8_t Control_GetState(void);
uint16_t Control_GetAngle(void);
int16_t Control_GetMotorOutput(void);
int16_t Control_GetSpeed(void);
int32_t Control_GetLocation(void);

#endif
