# 一、ISP概述

ISP（Image Signal Processor），即图像信号处理器，主要用来对前端图像传感器（如CMOS/CCD图像传感器）输出的图像信号（一般是数字信号）做后处理。通过ISP的处理，包括内插、去噪、去坏点、边缘加强、白平衡等操作后，可以使图像更加细腻，贴近真实世界中的图像。下面通过开源项目fast-openISP具体介绍ISP处理流程并分析相关算法和基本算子。

# 二、ISP流程

对于彩色图像传感器，因其对色彩的分辨方式不同（主要是覆盖在传感器感光阵列上的彩色滤波阵列的不同），其输出图像的格式也有所不同。对于使用Bayer滤镜的彩色图像传感器，它产生的原始数据叫做Bayer模式图像（Bayer pattern image），这是大多数彩色传感器输出的图像格式，项目fast-opneISP处理的也是Bayer格式的原始图像。Bayer滤镜的最大特点是阵列中由50%是绿色，25%是红色，另外25%是蓝色，显微镜下Bayer滤镜和感光单元的图像如下：

<img src="D:\notes\markdown\ISP\bayer_filter.png" alt="bayer_filter" style="zoom: 67%;" />

从传感器获取到原始图像后ISP的处理流程如下：

<img src="D:\notes\markdown\ISP\isp_pipeline.png" alt="isp_pipeline" style="zoom:;" />

### 1. DPC（Dead Pixel Correction）

因为传感器制造的原因，出厂时传感器中可能会有一定数量的缺陷像素单元，这些缺陷像素是在制造过程中产生的，在拍照时无法避免，只能通过后处理进行消除。根据缺陷像素的值的不同，缺陷像素可分为死点（总是偏低）、热点（总是偏高）、卡点（总是某一固定值）三种类型，对于这些缺陷像素的纠正有两个步骤，第一步是检测缺陷像素，第二步是用内插值替换异常值，其过程如下图：

<img src="C:\Users\xiang.weng\AppData\Roaming\Typora\typora-user-images\image-20230519110410995.png" alt="image-20230519110410995"  />

对于卡点异常，通过拍摄黑色或者白色图像很容易找到位置，然后将这些异常像素的位置存储到ISP存储器中以供更正。对于动态缺陷（值总是偏低或偏高），通常使用相邻像素来估计，即通过中心像素与周围像素（通常取周围8个相同颜色的像素值）的差值来判断差异，如果差值均大于预设的阈值t则判断为异常像素。计算如下：
$$
P_0=
	\begin{cases}
		dead, & abs(P_i-P_0)>t,i=1..8\\
		normal, & others
	\end{cases}
$$
判断出异常像素后就可以对异常值进行内插纠正了，有两种方法，一种是基于平均值，另一种是基于梯度的，基于梯度的计算方法是使用更多的，我们按下图所示方向计算梯度：

<img src="C:\Users\xiang.weng\AppData\Roaming\Typora\typora-user-images\image-20230519190535143.png" alt="image-20230519190535143"  />
$$
dv=|2P_0-P_2-P_7|\\
dh=|2P_0-P_4-P_5|\\
ddr=|2P_0-P_1-P_8|\\
ddl=|2P_0-P_3-P_6|
$$
计算出梯度值后，输出值就是所选方向上相邻像素的平均值，计算公式如下我们按如下：
$$
P_{ij}=
	\begin{cases}
		(P_{i,j-2}+P_{i,j+2}+1)/2 & min(dv,dh,ddr,ddl)=dv\\
		(P_{i-2,j}+P_{i+2,j}+1)/2 & min(dv,dh,ddr,ddl)=dh\\
		(P_{i-2,j-2}+P_{i+2,j+2}+1)/2 & min(dv,dh,ddr,ddl)=ddl\\
		(P_{i-2,j-2}+P_{i-2,j+2}+1)/2 & min(dv,dh,ddr,ddl)=ddr
	\end{cases}
$$


### 2. BLC（Block Level Compensation）

主流的CMOS型传感器的感光单元一般由光敏二极管和MOS管组成的逻辑电路构成。从传感器特性来看，感光单元的最低输出电压是无光照时（黑色）的电压，理想情况下该电压值应该为0，经AD转换之后数字量也应为0，但由于电路设计的原因（光敏二极管的暗电流、晶体管截止时的漏电流、AD的灵敏度），该电压值不可能为0。ISP在处理raw图数据时应该对该偏差进行修正来还原图像，这一过程就叫做黑电平补偿，调整很简单



# 三、参考

开源ISP项目：

[openISP(python)](https://github.com/cruxopen/openISP)、[fast-openISP(python)](https://github.com/QiuJueqin/fast-openISP)、[ISP-pipeline-hdrplus(c/c++)](https://github.com/jhfmat/ISP-pipeline-hdrplus)、[xk-isp(c/c++)](https://github.com/T-head-Semi/open_source_ISP)

ISP学习资料：

[awesome-ISP](https://github.com/starkfan007/awesome-ISP)、[ISPAlgorithmStudy](https://gitee.com/wtzhu13/ISPAlgorithmStudy)

