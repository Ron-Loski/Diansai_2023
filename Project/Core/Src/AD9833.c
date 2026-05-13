#include "AD9833.h"

#define FCLK 	25000000   // 参考时钟25MHz
#define RealFreDat 	268435456.0/FCLK

/*
	DDS1
*/
static void AD98331_SPI_Write(uint16_t data)
{
    uint8_t txBuf[2];
    txBuf[0] = (uint8_t)((data & 0xFF00) >> 8); // 高8位
    txBuf[1] = (uint8_t)(data & 0x00FF);        // 低8位

    AD98331_FSYNC_CLR();                         // 拉低片选，开始通信
    HAL_SPI_Transmit(&hspi3, txBuf, 2, HAL_MAX_DELAY);
    AD98331_FSYNC_SET();                         // 拉高片选，结束通信
}

void AD98331_SetRegisterValue(unsigned short regValue)
{
    AD98331_SPI_Write(regValue);
}

void AD98331_Init(void)
{
    
    AD98331_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
}

void AD98331_Reset(void)
{
    AD98331_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
//    HAL_Delay(10);
}

void AD98331_ClearReset(void)
{
    AD98331_SetRegisterValue(AD9833_REG_CMD);
}

/*
函数名称 ：void AD9833_SetFrequency(unsigned short reg, float fout,unsigned short type)
函数功能 ：写入频率寄存器
入口参数 ：reg：要写入的频率寄存器。
val：要写入的值。
type：波形类型；AD9833_OUT_SINUS正弦波、AD9833_OUT_TRIANGLE三角波、AD9833_OUT_MSB方波
出口参数 ：无
函数说明 ：无
*/
void AD98331_SetFrequency(unsigned short reg, float fout, unsigned short type)
{
    uint16_t freqHi = reg;
    uint16_t freqLo = reg;
    uint32_t val = (uint32_t)(RealFreDat * fout);

    freqHi |= (val & 0xFFFC000) >> 14;
    freqLo |= (val & 0x3FFF);

    AD98331_SetRegisterValue(AD9833_B28 | type);
    AD98331_SetRegisterValue(freqLo);
    AD98331_SetRegisterValue(freqHi);
}
/*
函数名称 ：void AD9833_SetFrequencyQuick(float fout,unsigned short type)
函数功能 ：写入频率寄存器
入口参数 ：val：要写入的频率值。
type：波形类型；AD9833_OUT_SINUS正弦波、AD9833_OUT_TRIANGLE三角波、AD9833_OUT_MSB方波
出口参数 ：无
函数说明 ：时钟速率为25 MHz时， 可以实现0.1 Hz的分辨率；而时钟速率为1 MHz时，则可以实现0.004 Hz的分辨率。
*/
void AD98331_SetFrequencyQuick(float fout, unsigned short type)
{
    AD98331_SetFrequency(AD9833_REG_FREQ0, fout, type);
}

void AD98331_SetPhase(unsigned short reg, unsigned short val)
{
    uint16_t phase = reg | val;
    AD98331_SetRegisterValue(phase);
}

void AD98331_Setup(unsigned short freq, unsigned short phase, unsigned short type)
{
    uint16_t val = freq | phase | type;
    AD98331_SetRegisterValue(val);
}

/*
	DDS2
*/
static void AD98332_SPI_Write(uint16_t data)
{
    uint8_t txBuf[2];
    txBuf[0] = (uint8_t)((data & 0xFF00) >> 8); // 高8位
    txBuf[1] = (uint8_t)(data & 0x00FF);        // 低8位

    AD98332_FSYNC_CLR();                         // 拉低片选，开始通信
    HAL_SPI_Transmit(&hspi2, txBuf, 2, HAL_MAX_DELAY);
    AD98332_FSYNC_SET();                         // 拉高片选，结束通信
}

void AD98332_SetRegisterValue(unsigned short regValue)
{
    AD98332_SPI_Write(regValue);
}

void AD98332_Init(void)
{
    
    AD98332_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
}

void AD98332_Reset(void)
{
    AD98332_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
//    HAL_Delay(10);
}

void AD98332_ClearReset(void)
{
    AD98332_SetRegisterValue(AD9833_REG_CMD);
}

/*
函数名称 ：void AD9833_SetFrequency(unsigned short reg, float fout,unsigned short type)
函数功能 ：写入频率寄存器
入口参数 ：reg：要写入的频率寄存器。
val：要写入的值。
type：波形类型；AD9833_OUT_SINUS正弦波、AD9833_OUT_TRIANGLE三角波、AD9833_OUT_MSB方波
出口参数 ：无
函数说明 ：无
*/
void AD98332_SetFrequency(unsigned short reg, float fout, unsigned short type)
{
    uint16_t freqHi = reg;
    uint16_t freqLo = reg;
    uint32_t val = (uint32_t)(RealFreDat * fout);

    freqHi |= (val & 0xFFFC000) >> 14;
    freqLo |= (val & 0x3FFF);

    AD98332_SetRegisterValue(AD9833_B28 | type);
    AD98332_SetRegisterValue(freqLo);
    AD98332_SetRegisterValue(freqHi);
}
/*
函数名称 ：void AD9833_SetFrequencyQuick(float fout,unsigned short type)
函数功能 ：写入频率寄存器
入口参数 ：val：要写入的频率值。
type：波形类型；AD9833_OUT_SINUS正弦波、AD9833_OUT_TRIANGLE三角波、AD9833_OUT_MSB方波
出口参数 ：无
函数说明 ：时钟速率为25 MHz时， 可以实现0.1 Hz的分辨率；而时钟速率为1 MHz时，则可以实现0.004 Hz的分辨率。
*/
void AD98332_SetFrequencyQuick(float fout, unsigned short type)
{
    AD98332_SetFrequency(AD9833_REG_FREQ0, fout, type);
}

void AD98332_SetPhase(unsigned short reg, unsigned short val)
{
    uint16_t phase = reg | val;
    AD98332_SetRegisterValue(phase);
}

void AD98332_Setup(unsigned short freq, unsigned short phase, unsigned short type)
{
    uint16_t val = freq | phase | type;
    AD98332_SetRegisterValue(val);
}

