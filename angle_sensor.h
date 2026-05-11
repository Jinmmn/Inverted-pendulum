#ifndef ANGLE_SENSOR_H
#define ANGLE_SENSOR_H

#include <stdint.h>

void AngleSensor_Init(void);
uint16_t AngleSensor_ReadRaw(void);

#endif
