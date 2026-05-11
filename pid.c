#include "pid.h"

void PID_Update(PID_t *pid)
{
    pid->Error1 = pid->Error0;
    pid->Error0 = pid->Target - pid->Actual;

    if (pid->Ki != 0.0f) {
        pid->ErrorInt += pid->Error0;
    } else {
        pid->ErrorInt = 0.0f;
    }

    pid->Out = pid->Kp * pid->Error0
             + pid->Ki * pid->ErrorInt
             + pid->Kd * (pid->Error0 - pid->Error1);

    if (pid->Out > pid->OutMax) {
        pid->Out = pid->OutMax;
    }
    if (pid->Out < pid->OutMin) {
        pid->Out = pid->OutMin;
    }
}

void PID_Reset(PID_t *pid)
{
    pid->Actual = 0.0f;
    pid->Out = 0.0f;
    pid->Error0 = 0.0f;
    pid->Error1 = 0.0f;
    pid->ErrorInt = 0.0f;
}
