#ifndef __MY_MATH_H_
#define __MY_MATH_H_

#include "stm32h7xx_hal.h"
#include "arm_math.h"
#include "math.h"
#include "usart.h"
#include <stdio.h>

#define Alpha	0.1


extern arm_cfft_instance_f32 cfft_handler;
extern arm_cfft_instance_f32 cfft_handler_512;

typedef struct {
	float Amp;
	uint32_t Freq;
	uint16_t Index;
	uint8_t WaveType;
	float Phase;
}SignalPara_t;

typedef struct {
	float Ratio3rd;
	float Ratio5th;
}SignalRatio_t;

typedef enum{
//	Unknow = 0,
	Triangle = ((0 << 5) | (1 << 1) | (0 << 3)),//计算得2
	Sine = ((0 << 5) | (0 << 1) | (0 << 3)),//计算得0
	}WaveformType_t;

void Math_FFT(uint16_t *ADC_Buff, float *FFT_IN,  float *FFT_OUT, float *FFT_Complex, uint16_t Sample_Size);	
SignalPara_t Math_CaculatePhase(float *FeedbackSignalAmp, float *FeedbackFFT_INPlural, uint16_t Sample_Size);					  
void Math_FindMaxAndSecondMaxFreqAndAmp(float *FFT_OUT, SignalPara_t *pFirst, SignalPara_t *pSecond, uint16_t Sample_Size);
SignalPara_t Math_Get3rdHarmonicPara(float *FFT_OUT, SignalPara_t *p, uint16_t Sample_Size);
SignalPara_t Math_Get5thHarmonicPara(float *FFT_OUT, SignalPara_t *p, uint16_t Sample_Size);
float Math_LowPassFilter(float Last_val, float Curr_val);					  
SignalRatio_t Math_CaculateRatio(SignalPara_t *Signal, SignalPara_t *Harmonic3rd, SignalPara_t *Harmonic5th);
WaveformType_t Math_WaveformJudegeSecond(SignalPara_t *FirstSignal, SignalRatio_t *FirstRatio, SignalPara_t *SecondSignal, SignalRatio_t *SecondRatio);
WaveformType_t Math_Math_WaveformJudegeFirst(SignalRatio_t *FirstRatio);
	
void Math_Feedback_SeparateChannel(uint16_t *Mix_Buff, float *FeedbackFFT_INFirst,
                                   float *FeedbackFFT_INSecond, uint16_t Sample_Size);
void Math_Feedback_FFTAndExtractPhase(SignalPara_t *pSignal, float *FFT_IN,
                                      float *FFT_OUT, uint16_t Sample_Size);

					  

#endif
