#ifndef __PWM_H
#define __PWM_H

void PWM_Init(void);
void PWM_SetCompare1(uint16_t Compare);  // 左电机PWM
void PWM_SetCompare2(uint16_t Compare);  // 右电机PWM

#endif
