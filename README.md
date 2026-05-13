English Versio:
This project presents a solution to Problem H(Signal Separation Device) of the 2023 Electronic Design Competition. The design approach is as follows:
First, acquire the mixed signal processed by the adder, then analyze the frequency and phase of the input signal. 
Next, generate the extracted frequency and waveform via DDS, collect the DDS output signal as feedback, calculate the phase of the feedback 
signal, and fine-tune the DDS frequency through PID control to achieve the effect of synchronous frequency display.
The main controller adopted is STM32H743IIT6 with a main frequency up to 480 MHz. To make the frequency resolution closer to an integer and 
reduce spectral leakage, the sampling frequency of the mixed signal is set to approximately 1.024 MHz with 1024 sampling points, delivering an 
actual frequency resolution of about 1 kHz.
Taking advantage of the harmonic characteristics of triangular waves, the amplitudes of the 3rd and 5th harmonics of the input signal are 
extracted and compared with the fundamental wave amplitude. Glitches will occur due to noise. Printing the harmonic ratios on Vofa reveals an 
intersection phenomenon between the third and fifth harmonic ratios. Therefore, complementary filtering is adopted for smoothing processing. 
After processing, the difference in harmonic ratios becomes highly distinct, enabling easy waveform identification. Only the phase of the 
feedback signal needs to be calculated, so a high sampling rate is not required. It should be noted that the timing sequence between waveform 
identification and signal tracking must be well matched.
This project has certain limitations. For the second sub-question in the extended requirement regarding initial phase configuration, 
the proportional (P) parameter needs to be adjusted according to the magnitude of the initial phase difference feedback. This part is omitted in
the project due to time constraints; users may modify it independently if needed.
中文版：
该工程为2023年电子设计大赛H题信号分离装置的解决方案。解题思路如下：
采集经过加法器混叠后的信号分析出输入信号的频率和相位，再通过DDS发出提取到的频率和波形并采集DDS发出的信号作为反馈，计算反馈信号的相位再通过PID微调DDS的频率从而达到同频显示的效果。
采用STM32H743IIT6为主控，主频高达480MHz,为了是分辨率更趋近于整数，削弱频谱泄露现象，混叠信号的采集频率约为1.024Mhz，采样点数为1024，实际分辨率约为1kHz，
利用三角波的谐波特性，提取输入信号的3次和5次谐波幅值，并与基波幅值作比，因为噪声的原因会有毛刺，在vofa上打印发现三次谐波比例和五次谐波比例会出现交叉现象，考虑所以使用
互补滤波平滑处理，处理后谐波比例的差距非常明显，极易判断波形。反馈信号需要计算相位即可，采样率不用太高，需要注意的是，判断波形和跟踪的时序关系需要匹配好。
该工程有一定的不足，发挥部分的第二问设置初相位，初相位差的不同需要根据相位差的反馈大小更改P项参数，该工程因时间问题已省略，需要的人烦请自行更改。
