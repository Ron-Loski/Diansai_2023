#include "My_Math.h"

float Threshold11 = 0.0f;
float Threshold12 = 0.017f;
float Threshold21 = 0.0f;
float Threshold22 = 0.015;
float Threshold31 = 0.0f;
float Threshold32 = 0.04f;

float Threshold4 = 0.0427f;


/*
FFT_Complex用于备份FFT后的复数，用于计算相位
*/
void Math_FFT(uint16_t *ADC_Buff, float *FFT_IN,  float *FFT_OUT,float *FFT_Complex, uint16_t Sample_Size)
{
	for (uint16_t i = 0; i < Sample_Size; i ++)
	  {    
		  FFT_Complex[i] = ADC_Buff[i];
		  FFT_IN[i * 2] = ADC_Buff[i]/ 65535.f * 3.3f;
//		  printf("%f\r\n", FFT_IN[i * 2]);
		  FFT_IN[i * 2 + 1] = 0;
	  }
	  
	  arm_cfft_f32(&cfft_handler, FFT_IN, 0, 1);	
	  arm_cmplx_mag_f32(FFT_IN, FFT_OUT, Sample_Size / 2);
}


void Math_BubbleSort(float *Buff, uint16_t Size)
{
	float temp;
    for (int i = 0; i < Size - 1; i++) {
        for (int j = 0; j < Size - 1 - i; j++) {
            if (Buff[j] < Buff[j + 1]) { 
                temp = Buff[j];
                Buff[j] = Buff[j + 1];
                Buff[j + 1] = temp;
            }
        }
    }
}




SignalPara_t Math_CaculatePhase(float *FeedbackSignalAmp, float *FeedbackFFT_INPlural, uint16_t Sample_Size)//第二个数组为复数数组
{
	float MaxAmp = 0.0f;
	uint16_t MaxIndex = 0;
	
	SignalPara_t p = {0};
	
	for (uint16_t i = 1; i < Sample_Size / 2; i ++)
	{
		if (FeedbackSignalAmp[i] > MaxAmp)
		{
			MaxAmp = FeedbackSignalAmp[i];
			MaxIndex = i;
		}
	}
	
	p.Freq = 512000 / 512 * MaxIndex;
	float Real = FeedbackFFT_INPlural[2 * MaxIndex];
    float Imag = FeedbackFFT_INPlural[2 * MaxIndex + 1];
	p.Phase = atan2f(Imag, Real) * 180.0f / PI;
//	if (Phase < 180.0f) Phase += 180;
	return p;
}


void Math_FindMaxAndSecondMaxFreqAndAmp(float *FFT_OUT, SignalPara_t *pFirst, SignalPara_t *pSecond, uint16_t Sample_Size)
{
	pFirst->Amp = 0.0f;
	pFirst->Index = 0;
	pSecond->Amp = 0.0f;
	pSecond->Index = 0;
	
	for (uint16_t i = 1; i < (Sample_Size / 2); i ++)		//提取两个基波频率和幅度
	{
		if (FFT_OUT[i] > pFirst->Amp)
		{
			pSecond->Amp = pFirst->Amp;
			pSecond->Index = pFirst->Index;
			  
			pFirst->Amp = FFT_OUT[i];
			pFirst->Index = i;
		}
		else if (FFT_OUT[i] > pSecond->Amp)
		{
			pSecond->Amp = FFT_OUT[i];
		    pSecond->Index = i;
		}
	}
	
	pFirst->Freq = 1024000 * pFirst->Index / Sample_Size ;		//计算频率
	pSecond->Freq = 1024000 * pSecond->Index / Sample_Size ;
	
	if (pFirst->Freq < pSecond->Freq)	//始终设置First为较大的频率值
	{
		uint32_t Temp1, Temp2;
		float Temp3;
		
		Temp1 = pFirst->Freq;
		pFirst->Freq = pSecond->Freq;
		pSecond->Freq = Temp1;
		
		Temp2 = pFirst->Index;
		pFirst->Index = pSecond->Index;
		pSecond->Index = Temp2;
		
		Temp3 = pFirst->Amp;
		pFirst->Amp = pSecond->Amp;
		pSecond->Amp = Temp3;
	}
}

SignalPara_t Math_Get3rdHarmonicPara(float *FFT_OUT, SignalPara_t *p, uint16_t Sample_Size)
{
	SignalPara_t Harmonic3rd = {0};
	
	Harmonic3rd.Freq = 3 * p->Freq;
	Harmonic3rd.Index = Harmonic3rd.Freq * Sample_Size / 1024000;
	
	if (Harmonic3rd.Index <= Sample_Size / 2){
		Harmonic3rd.Amp = FFT_OUT[Harmonic3rd.Index];
	}
	else{
		Harmonic3rd.Amp = 0;
	}
	
	return Harmonic3rd;
}

SignalPara_t Math_Get5thHarmonicPara(float *FFT_OUT, SignalPara_t *p, uint16_t Sample_Size)
{
	SignalPara_t Harmonic5th = {0};
	
	Harmonic5th.Freq = 5 * p->Freq;
	Harmonic5th.Index = Harmonic5th.Freq * Sample_Size / 1024000;
	if (Harmonic5th.Index <= Sample_Size / 2){
		Harmonic5th.Amp = FFT_OUT[Harmonic5th.Index];
	}
	else{
		Harmonic5th.Amp = 0;
	}
	
	return Harmonic5th;
}

SignalRatio_t Math_CaculateRatio(SignalPara_t *Signal, SignalPara_t *Harmonic3rd, SignalPara_t *Harmonic5th)
{
	SignalRatio_t WaveRatio = {0};
	
	WaveRatio.Ratio3rd = Harmonic3rd->Amp / Signal->Amp;
	WaveRatio.Ratio5th = Harmonic5th->Amp / Signal->Amp;
	
	return WaveRatio;
}

float Math_LowPassFilter(float Last_val, float Curr_val)
{
	return (1.0 - Alpha) * Last_val + Alpha * Curr_val;
}

WaveformType_t Math_WaveformJudegeSecond(SignalPara_t *FirstSignal, SignalRatio_t *FirstRatio, SignalPara_t *SecondSignal, SignalRatio_t *SecondRatio)
{
	if ((FirstSignal->Freq != SecondSignal->Freq * 3) && (FirstSignal->Freq != SecondSignal->Freq * 5))
	{
		if ((SecondRatio->Ratio3rd > Threshold11) && (SecondRatio->Ratio3rd < Threshold12)){
			return Sine;
		}
		else{
			return Triangle;
		}
	}
	else if (FirstSignal->Freq == SecondSignal->Freq * 3)
	{
		if ((SecondRatio->Ratio5th > Threshold21) && (SecondRatio->Ratio5th < Threshold22)){
			return Sine;
		}
		else{
			return Triangle;
		}
	}
	else if (FirstSignal->Freq == SecondSignal->Freq * 5)
	{
		if ((SecondRatio->Ratio3rd > Threshold31) && (SecondRatio->Ratio3rd < Threshold32)){
			return Sine;
		}
		else{
			return Triangle;
		}
	}
	else
	{
		return 1;
	}
}

WaveformType_t Math_Math_WaveformJudegeFirst(SignalRatio_t *FirstRatio)
{
	if (FirstRatio->Ratio3rd < Threshold4){
		return Sine;
	}
	else{
		return Triangle;
	}
}

/*
 * @brief  从ADC2 DMA交错缓冲区中分离两路通道数据，整理为FFT复数输入格式
 *         Mix_Buff布局: [CH6_0, CH7_0, CH6_1, CH7_1, ... CH6_511, CH7_511]
 *         每路各512点，分别存入复数数组(实部/虚部交错，虚部填0)
 *
 * @param  Mix_Buff          ADC2 DMA原始交错缓冲区，长度 Sample_Size(1024)
 * @param  FeedbackFFT_INFirst   CH6输出复数数组，长度 Sample_Size(1024) = 512点 × 2
 * @param  FeedbackFFT_INSecond  CH7输出复数数组，长度 Sample_Size(1024) = 512点 × 2
 * @param  Sample_Size       DMA总采样点数(两路合计，本工程为1024)
 */
void Math_Feedback_SeparateChannel(uint16_t *Mix_Buff, float *FeedbackFFT_INFirst,
                                   float *FeedbackFFT_INSecond, uint16_t Sample_Size)
{
    uint16_t HalfSize = Sample_Size / 2;    // 每路实际点数 = 512

    for (uint16_t i = 0; i < HalfSize; i ++)
    {
        // CH6(Rank1): Mix_Buff偶数索引
        FeedbackFFT_INFirst[i * 2]= (float)Mix_Buff[i * 2] / 65535.f * 3.3f;
        FeedbackFFT_INFirst[i * 2 + 1] = 0.0f;

        // CH7(Rank2): Mix_Buff奇数索引
        FeedbackFFT_INSecond[i * 2]= (float)Mix_Buff[i * 2 + 1] / 65535.f * 3.3f;
        FeedbackFFT_INSecond[i * 2 + 1] = 0.0f;
		
//		printf("%f,%f\r\n", FeedbackFFT_INFirst[i * 2], FeedbackFFT_INSecond[i * 2]);
    }
}


/*
 * @brief  对单路反馈信号做FFT并提取基波的频率、幅值和相位
 *         注意：FFT_IN数组会被arm_cfft_f32原地覆盖，调用后不可再还原原始时域数据
 *         FFT点数固定为 Sample_Size/2 = 512，使用已初始化的 cfft_handler_512
 *         等效采样率 Fs = APB1_CLK / (TIM4_Period + 1) / NbrOfConversion
 *                       = 120MHz / 2 / 117 / 2 ≈ 256410 Hz (每路)
 *
 * @param  pSignal       输出，填充Freq/Amp/Phase/Index
 * @param  FFT_IN        复数输入数组，长度 Sample_Size(1024)，函数内原地FFT
 * @param  FFT_OUT       幅值输出数组，长度 Sample_Size/2(512)
 * @param  Sample_Size   DMA总点数(1024)，函数内自动取半作为FFT点数
 */
void Math_Feedback_FFTAndExtractPhase(SignalPara_t *pSignal, float *FFT_IN,
                                      float *FFT_OUT, uint16_t Sample_Size)
{
    uint16_t HalfSize  = Sample_Size / 2;    // FFT实际点数 = 512
    uint16_t MaxIndex  = 0;
    float    MaxAmp    = 0.0f;

    // 1. 原地FFT，结果覆盖FFT_IN(复数)
    arm_cfft_f32(&cfft_handler_512, FFT_IN, 0, 1);

    // 2. 计算各频点幅值，存入FFT_OUT
    arm_cmplx_mag_f32(FFT_IN, FFT_OUT, HalfSize / 2);

//	for (uint16_t i = 1; i < HalfSize / 2; i ++)
//	{
//		printf("%f\r\n", FFT_OUT[i]);
//	}
	
    // 3. 寻找基波（跳过i=0直流分量，只看前半段）
    for (uint16_t i = 1; i < HalfSize / 2; i ++)
    {
        if (FFT_OUT[i] > MaxAmp)
        {
            MaxAmp= FFT_OUT[i];
            MaxIndex  = i;
        }
    }

    pSignal->Amp   = MaxAmp;
    pSignal->Index = MaxIndex;

    // 4. 计算频率
    // 每路等效Fs = TIM4触发频率 / NbrOfConversion
    pSignal->Freq = 512000 * MaxIndex / HalfSize;

    // 5. 从FFT原地复数结果中取对应频点的实部/虚部计算相位
    //    arm_cfft_f32执行后 FFT_IN[2*k]=实部, FFT_IN[2*k+1]=虚部
    float Real   = FFT_IN[2 * MaxIndex];
    float Imag   = FFT_IN[2 * MaxIndex + 1];
    pSignal->Phase = atan2f(Imag, Real) * 180.0f / PI;    // 单位:度, 范围[-180, 180]
}



















