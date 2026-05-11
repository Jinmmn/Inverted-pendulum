#include "MKL25Z4.h"
#include "angle_sensor.h"
#include "control.h"
#include "encoder.h"
#include "motor.h"

#define MOTOR_TEST_MODE  0
#define MOTOR_TEST_PWM   40

#if MOTOR_TEST_MODE == 2
static void MotorGpioTest_Forward(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;

    PORTA->PCR[12] = PORT_PCR_MUX(1);
    PORTA->PCR[13] = PORT_PCR_MUX(1);

    PTA->PDDR |= (1u << 12) | (1u << 13);
    PTA->PSOR = (1u << 12);
    PTA->PCOR = (1u << 13);
}
#endif

int main(void)
{
    SystemCoreClockUpdate();

    AngleSensor_Init();
    Encoder_Init();
    Motor_Init();

#if MOTOR_TEST_MODE == 1
    Motor_SetPWM(MOTOR_TEST_PWM);
    while (1) {
    }
#elif MOTOR_TEST_MODE == 2
    MotorGpioTest_Forward();
    while (1) {
    }
#else
    Control_Init();
    Control_Start();

    SysTick_Config(SystemCoreClock / 1000u);

    while (1) {
        __WFI();
    }
#endif
}

void SysTick_Handler(void)
{
#if !MOTOR_TEST_MODE
    Control_Tick1ms();
#endif
}
