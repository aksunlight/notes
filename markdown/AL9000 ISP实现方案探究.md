# AL9000 ISP实现方案探究

## 一、爱芯元智AX620芯片

AI ISP Pipeline：

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\AI_ISP_Pipeline.png"  />

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\AI_ISP_Pipeline1.png)

AI架构：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\AI_ISP_Struct.png)

Google HDR NET：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\google_hdr_net.png)

## 二、Xfuse Phoenix HDR ISP

Phoenix HDR ISP是一款200万像素HDR图像传感器解决方案，与Xilinx KV260视觉AI入门套件搭配使用。Phoenix HDR ISP是一款完全可编程的端到端的ISP解决方案，Phoenix ISP改编自Xfuse的专利HDR技术，该技术以人类视觉为模型，专有技术可确保保留局部图像对比度以及高光和阴影细节，同时不会产生光晕或色偏。Phoenix ISP还具有独特的适应性，可支持不同分辨率、不同彩色滤波阵列和不同HDR拍摄方法的传感器。

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\Phoenix_HDR_ISP_1.jpg" style="zoom: 67%;" />

Phoenix ISP Pipeline：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Xfuse_Phoenix_ISP_Pipeline.jpg)

| PHOENIX FEATURES                                             |
| ------------------------------------------------------------ |
| Main processing executed in the RAW Bayer domain             |
| Image Processes performed in HW freeing GPU & CPU for higher level tasks |
| 20-bit processing pipeline                                   |
| 120dB/20EV High Dynamic Range                                |
| Maintains color and tonal fidelity                           |
| Extremely low latency                                        |
| Low power consumption                                        |
| Processes video optimized for human or machine vision appications |
| 2, 3 or 4 frame multiple exposure merge                      |
| Auto de-ghosting and motion compensation                     |
| Auto halo removal                                            |
| Transition noise suppression                                 |
| Auto EV bracketing                                           |
| Auto and manual brightness adaption (gamma)                  |
| Ability to capture separate hdr and Tone mapped output video streams concurrently for ADAS application |
| Black level and gain compensation                            |
| Auto/manual white balance                                    |
| Auto gain                                                    |
| Lens distortion correction                                   |
| Veiling glare correction                                     |
| Noise reduction                                              |
| Local contrast adjustment                                    |
| Bad pixel correction                                         |
| Automatic and manual Region Of Interest (ROI) selection      |
| SNR Statistics                                               |
| Histogram calculation                                        |
| De-Mosaic-ing                                                |
| 50/60 Hz ambient artificial lighting sync                    |
| 180 degree rotation                                          |

Xfuse Phoenix ISP application package：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Phoenix_ISP_application_package.png)

Xfuse Camera Control App：

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\Xfuse_Camera_Control_App.png"  />

Phoenix HDR ISP效果：

 ![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Phoenix_ISP_LDR.png)

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Phoenix_ISP_HDR.png)

## 三、xkISP



## 四、补充

降噪(Noise Reduction)：目的在于去除图像噪点，具体来说它分为2DNR和3DNR两种类型，2DNR即空域降噪，它只分析和处理一帧图形内部的噪声；3DNR即时域降噪，它主要利用多帧图像在时间上的相关性实现降噪。

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\Noise.png" style="zoom: 50%;" />

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\Noise1.png" style="zoom: 50%;" />

hdr/histogram equalization/tone mapping：三者目的都是调整图片亮度和对比度，hdr往往通过多帧曝光来实现，后面两个主要使用算法对数字图像进行调整，直方图均衡化是图像处理领域中利用图像直方图对对比度进行调整的方法，色调映射是在有限动态范围媒介上近似显示高动态范围图像的一项计算机图形学技术。

亮度调整：

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\luminance.png"  />

对比度调整：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\contrast_ratio.png)

亮度和对比图调整示意图：

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\tone mapping.png" style="zoom:150%;" />

亮度和对比度调整效果：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\fullcompare.png)

图像质量评价：MSE，RMSE，PSNR，SSIM

<img src="D:\notes\markdown\AL9000 ISP实现方案探究.assets\judge.png"  />

