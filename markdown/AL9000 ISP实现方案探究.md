# AL9000 ISP实现方案探究

## 一、爱芯元智AX620芯片

AX620A是爱芯元智第二代自研边缘侧智能视觉芯片，搭载四核Cortex-A7 CPU以及14.4TOPs@INT4或3.6TOPs@INT8的高算力NPU。该芯片不仅能能够满足传统智慧城市、智能家居应用需求，超低功耗还能够满足电池应用方案的功耗需求，兼顾AIoT、手机等应用场景，尤其在快速唤醒类产品领域拥有优越表现。

AX620A产品规格：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\AI_ISP_AX620A.png)

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

xkISP是基于Xilinx开发工具的开源图像信号处理器 (ISP)，由复旦大学VIP实验室和阿里巴巴DAMO CTL实验室联合开发。到目前为止，xkISP支持处理任意分辨率的12位原始图像数据。

xkISP Pipeline：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\xkISP_Pipeline.png)

xkISP通过使用Vitis HLS将ISP各个模块源代码综合并进行验证，同时，也可将验证通过后的RTL代码导出并封装成对应IP供给后续Vivado设计使用。该项目也可以直接使用Vitis GUI进行ISP全流程集成验证。

项目文件目录如下：

```
xkISP
├─fpga
│      host.cpp
│      top.cpp
│      top.h
│      xcl2.cpp
│      xcl2.h
│
├─src
│    isp_top.h
│    file_define.h
│    "*module*".cpp
│    "*module*".h
│    ...
│
├─tb
│     tb_"*module*".cpp
│    ...
│
├─tcl
│      Makefile
│      "*module*".tcl
│      "*module*"_directives.tcl
│    ...
│
├─tv
│     Makefile
│     hls_param.txt
│     input.raw
│     isp
│     readme_for_tv
├─  LICENSE
├─  setup_env.sh
└─  README.md
```

rawdns模块ip：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\xkISP_rawdns_ip.png)





## 四、补充

### 1.降噪与HDR

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

### 2.Vitis HLS

HLS即High-level Synthesis（高层次综合），Vitis HLS工具会将C或C++函数综合到RTL代码中，以便在Versal自适应SoC、Zynq MPSoC或AMD FPGA器件的可编程逻辑 (PL) 区域内实现。设计师需要以C/C++创建高层次的算法宏架构，在高层次上并不需要制定微架构决策，如创建状态机、数据路径、寄存器流水线等，这些具体细节可以留给 HLS工具，设计者只需要提供简单输入约束（如时钟速度、性能编译指示、目标器件等）即可生成经过最优化的 RTL。

使用HLS的开发步骤如下：

1. 基于设计原则建立算法架构
2. （C 语言仿真）利用C/C++测试激励文件验证C/C++代码
3. （C 语言综合）使用HLS生成RTL
4. （协同仿真）验证使用C++输出生成的内核
5. （分析）复查HLS综合报告和协同仿真报告，并进行分析
6. 重新运行前述步骤直至满足性能目标为止。

使用HLS进行验证的实例如下：

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Vitis_HLS_1.png)

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Vitis_HLS_2.png)

![](D:\notes\markdown\AL9000 ISP实现方案探究.assets\Vitis_HLS_3.png)

### 3.Vitis

Vitis工具支持的硬件：



## 五、参考









## 五、参考

[Vitis高层次综合用户指南](https://docs.xilinx.com/r/zh-CN/ug1399-vitis-hls/)、[高层次综合技术原理浅析](https://support.xilinx.com/s/article/1186018?language=zh_CN)

[xkISP](https://github.com/openasic-org/xkISP)、[fast-openISP](https://github.com/QiuJueqin/fast-openISP)

