#include "MKL25Z4.h"
#include "angle_sensor.h"

#define ANGLE_ADC_CHANNEL  8u

void AngleSensor_Init(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

    PORTB->PCR[0] = 0u;

    ADC0->CFG1 = ADC_CFG1_ADICLK(0)
               | ADC_CFG1_MODE(1)
               | ADC_CFG1_ADIV(2);
    ADC0->CFG2 = 0u;
    ADC0->SC2 = 0u;
    ADC0->SC3 = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3);
}

uint16_t AngleSensor_ReadRaw(void)
{
    ADC0->SC1[0] = ADC_SC1_ADCH(ANGLE_ADC_CHANNEL);
    while ((ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0u) {
    }

    return (uint16_t)ADC0->R[0];
}
