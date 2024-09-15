#ifndef Kalman_H
#define Kalman_H

typedef struct 
{
    float Last_P;//上次估算协方差 不可以为0 ! ! ! ! ! 
    float Now_P;//当前估算协方差
    float out;//卡尔曼滤波器输出
    float Kg;//卡尔曼增益
    float Q;//过程噪声协方差
    float R;//观测噪声协方差
}Kalman;

void Kalman_Init(void);
void Kalman1_Init(void);
void Kalman2_Init(void);
void Kalman3_Init(void);

float kalman_filter(Kalman *kfp,float input);

extern Kalman kfp;
extern Kalman kfp1;
extern Kalman kfp2;
extern Kalman kfp3;

#endif
