#include "angle_sensor.h"
#include "control.h"
#include "encoder.h"
#include "motor.h"
#include "pid.h"

#define CENTER_ANGLE       CENTER_ANGLE_DEFAULT
#define CENTER_RANGE       500u
#define START_PWM          35
#define START_TIME_MS      100u
#define SWING_SAMPLE_MS    40u
#define BALANCE_PERIOD_MS  5u
#define LOCATION_PERIOD_MS 50u
#define MOTOR_POLARITY     (-1)

enum {
    CONTROL_STOP = 0,
    CONTROL_SWING_WATCH = 1,
    CONTROL_BALANCE = 4,
    CONTROL_SWING_POS_KICK = 21,
    CONTROL_SWING_POS_HOLD = 22,
    CONTROL_SWING_POS_BACK = 23,
    CONTROL_SWING_POS_BACK_HOLD = 24,
    CONTROL_SWING_NEG_KICK = 31,
    CONTROL_SWING_NEG_HOLD = 32,
    CONTROL_SWING_NEG_BACK = 33,
    CONTROL_SWING_NEG_BACK_HOLD = 34
};

static PID_t angle_pid = {
    CENTER_ANGLE, 0.0f, 0.0f,
    0.2f, 0.01f, 0.5f,
    0.0f, 0.0f, 0.0f,
    100.0f, -100.0f
};

static PID_t location_pid = {
    0.0f, 0.0f, 0.0f,
    0.3f, 0.0f, 3.5f,
    0.0f, 0.0f, 0.0f,
    100.0f, -100.0f
};

static volatile uint8_t run_state;
static volatile uint16_t angle_adc;
static volatile int16_t speed;
static volatile int32_t location;
static volatile int16_t motor_output;

static uint8_t angle_is_near_center(uint16_t angle)
{
    return (angle > (CENTER_ANGLE - CENTER_RANGE))
        && (angle < (CENTER_ANGLE + CENTER_RANGE));
}

static void set_control_pwm(int16_t pwm)
{
    motor_output = pwm;
    Motor_SetPWM((int16_t)(MOTOR_POLARITY * pwm));
}

static void reset_balance_control(void)
{
    location = 0;
    PID_Reset(&angle_pid);
    PID_Reset(&location_pid);
    angle_pid.Target = CENTER_ANGLE;
}

void Control_Init(void)
{
    reset_balance_control();
    run_state = CONTROL_STOP;
    angle_adc = 0u;
    speed = 0;
    motor_output = 0;
    Motor_Stop();
}

void Control_Start(void)
{
    reset_balance_control();
    run_state = CONTROL_SWING_POS_KICK;
}

void Control_Stop(void)
{
    run_state = CONTROL_STOP;
    motor_output = 0;
    Motor_Stop();
}

void Control_AddLocationTarget(int16_t step)
{
    float target = location_pid.Target + (float)step;

    if (target > LOCATION_TARGET_MAX) {
        target = LOCATION_TARGET_MAX;
    } else if (target < -LOCATION_TARGET_MAX) {
        target = -LOCATION_TARGET_MAX;
    }

    location_pid.Target = target;
}

static void run_swing_watch(uint16_t angle)
{
    static uint16_t sample_count;
    static uint16_t angle0;
    static uint16_t angle1;
    static uint16_t angle2;

    sample_count++;
    if (sample_count < SWING_SAMPLE_MS) {
        return;
    }
    sample_count = 0u;

    angle2 = angle1;
    angle1 = angle0;
    angle0 = angle;

    if (angle0 > CENTER_ANGLE + CENTER_RANGE
     && angle1 > CENTER_ANGLE + CENTER_RANGE
     && angle2 > CENTER_ANGLE + CENTER_RANGE
     && angle1 < angle0
     && angle1 < angle2) {
        run_state = CONTROL_SWING_POS_KICK;
        return;
    }

    if (angle0 < CENTER_ANGLE - CENTER_RANGE
     && angle1 < CENTER_ANGLE - CENTER_RANGE
     && angle2 < CENTER_ANGLE - CENTER_RANGE
     && angle1 > angle0
     && angle1 > angle2) {
        run_state = CONTROL_SWING_NEG_KICK;
        return;
    }

    if (angle_is_near_center(angle0) && angle_is_near_center(angle1)) {
        reset_balance_control();
        run_state = CONTROL_BALANCE;
    }
}

static void run_balance(uint16_t angle)
{
    static uint16_t balance_count;
    static uint16_t location_count;

    if (!angle_is_near_center(angle)) {
        Control_Stop();
        balance_count = 0u;
        location_count = 0u;
        return;
    }

    balance_count++;
    if (balance_count >= BALANCE_PERIOD_MS) {
        balance_count = 0u;
        angle_pid.Actual = angle;
        PID_Update(&angle_pid);
        set_control_pwm((int16_t)angle_pid.Out);
    }

    location_count++;
    if (location_count >= LOCATION_PERIOD_MS) {
        location_count = 0u;
        location_pid.Actual = (float)location;
        PID_Update(&location_pid);
        angle_pid.Target = CENTER_ANGLE - location_pid.Out;
    }
}

void Control_Tick1ms(void)
{
    static uint16_t swing_time;

    angle_adc = AngleSensor_ReadRaw();
    speed = Encoder_Get();
    location += speed;

    switch (run_state) {
    case CONTROL_STOP:
        Motor_Stop();
        motor_output = 0;
        break;

    case CONTROL_SWING_WATCH:
        run_swing_watch(angle_adc);
        break;

    case CONTROL_SWING_POS_KICK:
        set_control_pwm(START_PWM);
        swing_time = START_TIME_MS;
        run_state = CONTROL_SWING_POS_HOLD;
        break;

    case CONTROL_SWING_POS_HOLD:
        if (swing_time > 0u) {
            swing_time--;
        }
        if (swing_time == 0u) {
            run_state = CONTROL_SWING_POS_BACK;
        }
        break;

    case CONTROL_SWING_POS_BACK:
        set_control_pwm(-START_PWM);
        swing_time = START_TIME_MS;
        run_state = CONTROL_SWING_POS_BACK_HOLD;
        break;

    case CONTROL_SWING_POS_BACK_HOLD:
        if (swing_time > 0u) {
            swing_time--;
        }
        if (swing_time == 0u) {
            Motor_Stop();
            motor_output = 0;
            run_state = CONTROL_SWING_WATCH;
        }
        break;

    case CONTROL_SWING_NEG_KICK:
        set_control_pwm(-START_PWM);
        swing_time = START_TIME_MS;
        run_state = CONTROL_SWING_NEG_HOLD;
        break;

    case CONTROL_SWING_NEG_HOLD:
        if (swing_time > 0u) {
            swing_time--;
        }
        if (swing_time == 0u) {
            run_state = CONTROL_SWING_NEG_BACK;
        }
        break;

    case CONTROL_SWING_NEG_BACK:
        set_control_pwm(START_PWM);
        swing_time = START_TIME_MS;
        run_state = CONTROL_SWING_NEG_BACK_HOLD;
        break;

    case CONTROL_SWING_NEG_BACK_HOLD:
        if (swing_time > 0u) {
            swing_time--;
        }
        if (swing_time == 0u) {
            Motor_Stop();
            motor_output = 0;
            run_state = CONTROL_SWING_WATCH;
        }
        break;

    case CONTROL_BALANCE:
        run_balance(angle_adc);
        break;

    default:
        Control_Stop();
        break;
    }
}

uint8_t Control_GetState(void)
{
    return run_state;
}

uint16_t Control_GetAngle(void)
{
    return angle_adc;
}

int16_t Control_GetMotorOutput(void)
{
    return motor_output;
}

int16_t Control_GetSpeed(void)
{
    return speed;
}

int32_t Control_GetLocation(void)
{
    return location;
}
