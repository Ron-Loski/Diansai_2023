#include "Table.h"


uint16_t sin_table[TABLE_SIZE];

uint32_t phase = 0;          // 当前相位（32位）
uint32_t phase_step = 0;     // 相位步进（控制频率）

float fs = 1024000.0f;         // DAC更新频率（定时器频率）

void Generate_SinTable(void)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        float angle = 2.0f * 3.1415926f * i / TABLE_SIZE;
        float s = sinf(angle);

        // 映射到 DAC（0~4095）
        sin_table[i] = (uint16_t)((s + 1.0f) * (DAC_MAX / 2));
    }
}