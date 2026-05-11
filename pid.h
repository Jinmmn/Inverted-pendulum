#ifndef PID_H
#define PID_H

typedef struct {
    float Target;
    float Actual;
    float Out;

    float Kp;
    float Ki;
    float Kd;

    float Error0;
    float Error1;
    float ErrorInt;

    float OutMax;
    float OutMin;
} PID_t;

void PID_Update(PID_t *pid);
void PID_Reset(PID_t *pid);

#endif
