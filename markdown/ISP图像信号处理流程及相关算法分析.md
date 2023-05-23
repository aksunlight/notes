# 一、ISP概述

ISP（Image Signal Processor），即图像信号处理器，主要用来对前端图像传感器（如CMOS/CCD图像传感器）输出的图像信号（一般是数字信号）做后处理。通过ISP的处理，包括内插、去噪、去坏点、边缘加强、白平衡等操作后，可以使图像更加细腻，贴近真实世界中的图像。下面通过开源项目fast-openISP具体介绍ISP处理流程并分析相关算法和基本算子。

# 二、ISP流程

对于彩色图像传感器，因其对色彩的分辨方式不同（主要是覆盖在传感器感光阵列上的彩色滤波阵列的不同），其输出图像的格式也有所不同。对于使用Bayer滤镜的彩色图像传感器，它产生的原始数据叫做Bayer模式图像（Bayer pattern image），这是大多数彩色传感器输出的图像格式，项目fast-opneISP处理的也是Bayer格式的原始图像。Bayer滤镜的最大特点是阵列中由50%是绿色，25%是红色，另外25%是蓝色，显微镜下Bayer滤镜和感光单元的图像如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\bayer_filter.png" style="zoom:67%;" />

从传感器获取到原始图像后ISP的处理流程如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\isp_pipeline.png"  />

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\isp_process.png)

### 1. DPC（Dead Pixel Correction）

因为传感器制造的原因，出厂时传感器中可能会有一定数量的缺陷像素单元，这些缺陷像素是在制造过程中产生的，在拍照时无法避免，只能通过后处理进行消除。根据缺陷像素的值的不同，缺陷像素可分为死点（总是偏低）、热点（总是偏高）、卡点（总是某一固定值）三种类型，对于这些缺陷像素的纠正有两个步骤，第一步是检测缺陷像素，第二步是用内插值替换异常值，其过程如下图：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\dead_pixel.png)

对于卡点异常，通过拍摄黑色或者白色图像很容易找到位置，然后将这些异常像素的位置存储到ISP存储器中以供更正。对于动态缺陷（值总是偏低或偏高），通常使用相邻像素来估计，即通过中心像素与周围像素（通常取周围8个相同颜色的像素值）的差值来判断差异，如果差值均大于预设的阈值t则判断为异常像素。计算如下：
$$
P_0=
	\begin{cases}
		dead, & abs(P_i-P_0)>t,i=1..8\\
		normal, & others
	\end{cases}
$$
判断出异常像素后就可以对异常值进行内插纠正了，有两种方法，一种是基于平均值，另一种是基于梯度的，基于梯度的计算方法是使用更多的，我们按下图所示方向计算梯度：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\image_gradient.png)
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

主流的CMOS型传感器的感光单元一般由光敏二极管和MOS管组成的逻辑电路构成。从传感器特性来看，感光单元的最低输出电压是无光照时（黑色）的电压，理想情况下该电压值应该为0，经AD转换之后数字量也应为0，但由于电路设计的原因（光敏二极管的暗电流、晶体管截止时的漏电流、AD的灵敏度），该电压值不可能为0。ISP在处理raw图数据时应该对该偏差进行修正来还原图像，这一过程就叫做黑电平补偿。调整很简单，我们只需对每个通道扣除相应固定值即可，具体步骤如下：

1. 采集黑帧raw图，将其分为R，Gr，Gb，B四个通道；
2. 对四个通道求平均值（也有算法使用中位值或别的方式）；
3. 后续图像每个通道都减去2中计算出的各个通道的校正值；
4. ~~对Gr和Gb通道进行归一化，使得其像素值的范围恢复到0-255；~~

计算公式如下：
$$
R'=R-R\_offset\\
B'=B-B\_offset\\
Gr'=Gr-Gr\_offset+\alpha R\\
Gb'=Gb-Gb\_offset+\beta B\\
$$

### 3. LSC（Lens Shading Correction）

由于相机镜头的构造，图像光照在透过镜头照射在感光单元上时，边角处的焦点夹角大于中心焦点夹角，造成边角失光（边缘处透射的光的量减少），表现在图像上的效果就是亮度从图像中心到四周逐渐衰减，这种情况叫做luma shading（亮度阴影），下图为均匀光照下拍摄灰色图像时产生的亮度阴影。

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\luma_shading.png" style="zoom: 48%;" />

另一方面，CCD和CMOS型的传感器对红色光更敏感，并且还显著扩展至近红外线（IR）光谱中，这将引起颜色再现的误差，此外由于IR滤波器的存在，具有较大入射角度的图像传感器的边缘处较多的阻挡较长波长（如红光），从而导致图像中的在空间上的不均匀色温。这种情况叫做color shading（颜色阴影），下图为颜色阴影示意图。

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\color_shading.png" style="zoom: 80%;" />

对于lens shading的纠正一般使用查找表（LUT）校正法，其主要思路是使用一张表存储所有像素的增益值（达到目标值需要乘上的系数，又叫gain值），最终纠正的值就是增益值乘上初始值。具体步骤如下：

1. 拍摄光线均匀的图像，然后将图像分成块，每个块的四个点都有一个校正系数，将系数存储到查找表中。 
2. 拍照时根据像素位置，计算像素落在哪个块中，然后从查找表中得到这个块的四个系数。 
3. 通过插值计算像素的校正增益。 
4. 将校正增益乘以像素。

插值计算如下：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\interpolation.png)
$$
\frac{Z_B-Z_A}{Y_1-Y_0}=\frac{Z_N-Z_A}{Y-Y_0}\\
\frac{Z_C-Z_D}{Y_1-Y_0}=\frac{Z_M-Z_D}{Y-Y_0}\\
\frac{Z_M-Z_N}{X_1-X_0}=\frac{Z-Z_N}{X-X_0}\\
Z=(Z_C-Z_A)*X+(Z_B-Z_A)*Y+(Z_D+Z_A-Z_B-Z_C)*X*Y+Z_A
$$


### 4. Auto White Balance（AWB)

白平衡是去除不真实的色偏的过程，其基本概念是“不管在任何光源下，都能将白色物体还原为白色”。对于不同的光源，因为色温的不同，在没有白平衡时，拍出来的照片往往会产生橙色、蓝色甚至绿色色偏。传统的相机白平衡需要我们给出当前光源的色温，然后相机才能根据设定的色温进行补色，实现白平衡，下图是相机设置不同色温以及自动白平衡情况下的成像效果：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\AWB.png)

自动白平衡的方法有很多，其中应用最广泛是基于灰度世界理论的方法。灰度世界理论认为，对于任意一副图像，当它有足够的色彩变化，则它的RGB分量的均值会趋于相等。使用灰度世界理论来进行白平衡的具体步骤如下：

1. 计算出R、G、B三通道的像素平均值R_mean、G_mean、B_mean
2. 以G通道作为参考分量，计算出R和B通道的增益值R_gain、B_gain
3. 将R通道和B通道像素乘以增益值，G通道像素保持不变，完成白平衡

计算式如下：
$$
R_{gain}=\frac{G_{mean}}{R_{mean}}\\
B_{gain}=\frac{G_{mean}}{B_{mean}}\\
R'=R*R_{gain}\\
B'=B*B_{gain}\\
G'=G
$$
除了基于灰度世界理论的方法，我们还可以使用基于色温的方法，该方法通过前期实验计算出不同色温下的R通道或B通道的增益值，然后根据实验计算出的R_gain和B_gain关系曲线求出另一个通道的增益值，得到R通道和B通道的增益值后就可以按步骤3完成白平衡了，其中环境色温往往由色温传感器采集获得。

### 5.Gamma Correction（GC）



# 三、参考

开源ISP项目：

[openISP(python)](https://github.com/cruxopen/openISP)、[fast-openISP(python)](https://github.com/QiuJueqin/fast-openISP)、[ISP-pipeline-hdrplus(c/c++)](https://github.com/jhfmat/ISP-pipeline-hdrplus)、[xk-isp(c/c++)](https://github.com/T-head-Semi/open_source_ISP)

ISP学习资料：

[awesome-ISP](https://github.com/starkfan007/awesome-ISP)、[ISPAlgorithmStudy](https://gitee.com/wtzhu13/ISPAlgorithmStudy)

