#include "PLL.h"

#include "math.h"

SPLL_TypeDef my_pll;

// 初始化 PLL，假设目标频率约为 200kHz
void SPLL_Init(SPLL_TypeDef *pll, float fs, float target_f) {
    pll->Ts = 1.0f / fs;
    pll->center_omega = 2.0f * 3.14159265f * target_f;
    pll->omega = pll->center_omega;
    pll->theta = 0.0f;
    
    // PI 参数需要根据控制理论整定，此处给出一组经验初值
    pll->kp = 0.5f;
    pll->ki = 50.0f; 
    pll->pi_integral = 0.0f;
    
    pll->sogi_x = 0.0f;
    pll->sogi_y = 0.0f;
}

// 每次 ADC 采样得到新数据时调用此函数
// input_v: 去除直流偏置后的 ADC 采样电压值
void SPLL_Update(SPLL_TypeDef *pll, float input_v) {
    // 1. SOGI 离散化计算 (采用双线性变换法，k 为 SOGI 带宽系数，通常取 1.414)
    float k = 1.414f;
    float err = input_v - pll->v_alpha;
    
    // SOGI 核心状态机更新
    pll->sogi_x += err * k * pll->center_omega * pll->Ts;
    pll->v_beta += (pll->sogi_x - pll->v_beta) * pll->center_omega * pll->Ts;
    pll->v_alpha = pll->sogi_x;

    // 2. Park 变换 (计算 q 轴电压，代表相位误差)
    pll->v_q = -pll->v_alpha * sinf(pll->theta) + pll->v_beta * cosf(pll->theta);

    // 3. PI 控制器 (驱动 v_q 趋向于 0)
    pll->pi_integral += pll->v_q * pll->ki * pll->Ts;
    pll->pi_out = pll->v_q * pll->kp + pll->pi_integral;

    // 4. 频率与相位更新
    pll->omega = pll->center_omega + pll->pi_out;
    pll->theta += pll->omega * pll->Ts;

    // 约束相位在 0 ~ 2*PI 之间
    if (pll->theta > 6.2831853f) {
        pll->theta -= 6.2831853f;
    } else if (pll->theta < 0.0f) {
        pll->theta += 6.2831853f;
    }
}
