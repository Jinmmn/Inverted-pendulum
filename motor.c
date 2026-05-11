#include "MKL25Z4.h"
#include "motor.h"

#define MOTOR_PWM_MAX       100
#define MOTOR_PWM_FREQ_HZ   20000u
#define MOTOR_TPM           TPM1
#define MOTOR_TPM_CH_IN1    0u
#define MOTOR_TPM_CH_IN2    1u
#define MOTOR_PORT          PORTA
#define MOTOR_PORT_CLOCK    SIM_SCGC5_PORTA_MASK
#define MOTOR_PIN_IN1       12u
#define MOTOR_PIN_IN2       13u
#define MOTOR_PIN_MUX_TPM   3u

static uint16_t motor_pwm_mod;

static void motor_set_channel(uint8_t channel, uint16_t duty)
{
    if (duty > motor_pwm_mod) {
        duty = motor_pwm_mod;
    }
    MOTOR_TPM->CONTROLS[channel].CnV = duty;
}

void Motor_Init(void)
{
    uint32_t tpm_clock;

    SIM->SCGC5 |= MOTOR_PORT_CLOCK;
    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
    SIM->SOPT2 = (SIM->SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) | SIM_SOPT2_TPMSRC(1);

    MOTOR_PORT->PCR[MOTOR_PIN_IN1] = PORT_PCR_MUX(MOTOR_PIN_MUX_TPM);
    MOTOR_PORT->PCR[MOTOR_PIN_IN2] = PORT_PCR_MUX(MOTOR_PIN_MUX_TPM);

    MOTOR_TPM->SC = 0u;
    MOTOR_TPM->CNT = 0u;

    tpm_clock = SystemCoreClock;
    motor_pwm_mod = (uint16_t)((tpm_clock / MOTOR_PWM_FREQ_HZ) - 1u);
    MOTOR_TPM->MOD = motor_pwm_mod;

    MOTOR_TPM->CONTROLS[MOTOR_TPM_CH_IN1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    MOTOR_TPM->CONTROLS[MOTOR_TPM_CH_IN2].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;

    Motor_Stop();
    MOTOR_TPM->SC = TPM_SC_CMOD(1) | TPM_SC_PS(0);
}

void Motor_SetPWM(int16_t pwm)
{
    uint16_t duty;

    if (pwm > MOTOR_PWM_MAX) {
        pwm = MOTOR_PWM_MAX;
    } else if (pwm < -MOTOR_PWM_MAX) {
        pwm = -MOTOR_PWM_MAX;
    }

    if (pwm == 0) {
        Motor_Stop();
        return;
    }

    if (pwm > 0) {
        duty = (uint16_t)(((uint32_t)pwm * (motor_pwm_mod + 1u)) / MOTOR_PWM_MAX);
        motor_set_channel(MOTOR_TPM_CH_IN1, duty);
        motor_set_channel(MOTOR_TPM_CH_IN2, 0u);
    } else {
        duty = (uint16_t)(((uint32_t)(-pwm) * (motor_pwm_mod + 1u)) / MOTOR_PWM_MAX);
        motor_set_channel(MOTOR_TPM_CH_IN1, 0u);
        motor_set_channel(MOTOR_TPM_CH_IN2, duty);
    }
}

void Motor_Stop(void)
{
    motor_set_channel(MOTOR_TPM_CH_IN1, 0u);
    motor_set_channel(MOTOR_TPM_CH_IN2, 0u);
}
