#ifndef __PLL_H_
#define __PLL_H_

#include "stm32h7xx_hal.h"

// 定义 SPLL 结构体

typedef struct {
    float v_alpha;
    float v_beta;
    float v_q;
    
    // SOGI 内部状态变量
    float sogi_x; 
    float sogi_y;
    
    // PI 控制器变量
    float kp;
    float ki;
    float pi_out;
    float pi_integral;
    
    // 频率与相位
    float center_omega; // 中心角频率 2*pi*f
    float omega;        // 实时锁定的角频率
    float theta;        // 实时锁定的相位 (0 到 2pi)
    
    float Ts;           // 采样周期 (1 / 1.024MHz)
} SPLL_TypeDef;

#endif
