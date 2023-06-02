# 一、ISP概述

ISP（Image Signal Processor），即图像信号处理器，主要用来对前端图像传感器（如CMOS/CCD图像传感器）输出的图像信号（一般是数字信号）做后处理。通过ISP的处理，包括内插、去噪、去坏点、边缘加强、白平衡等操作后，可以使图像更加细腻，贴近真实世界中的图像。下面通过开源项目fast-openISP具体介绍ISP处理流程并分析相关算法和基本算子。

# 二、ISP流程

对于彩色图像传感器，因其对色彩的分辨方式不同（主要是覆盖在传感器感光阵列上的彩色滤波阵列的不同），其输出图像的格式也有所不同。对于使用Bayer滤镜的彩色图像传感器，它产生的原始数据叫做Bayer模式图像（Bayer pattern image），这是大多数彩色传感器输出的图像格式，项目fast-opneISP处理的也是Bayer格式的原始图像。Bayer滤镜的最大特点是阵列中由50%是绿色，25%是红色，另外25%是蓝色，显微镜下Bayer滤镜和感光单元的图像如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\bayer_filter.png" style="zoom:67%;" />

从传感器获取到原始图像后ISP的处理流程如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\isp_pipeline.png"  />

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\isp_process.png)

## Bayer Domain Processing  

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

### 4.BNR（Bayer Noise Reduce）

拜尔域减噪，降噪

### 5. Auto White Balance（AWB)

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

### 6.CFAI（Color Filter Array Interpolation）/ Demosaicing

彩色图像的每一个像素点都需要红、绿、蓝三个色彩分量的信息，而由Bayer阵列所产生的raw图中的每一个像素点只有红或绿或黄一个色彩分量的信息，因此ISP必须估算出每个像素处缺失的另外两个颜色值，这个过程叫做色彩滤波阵列插值（CFAI），也叫去马赛克（Demosaicing）。

最简单的去马赛克算法是双线性插值算法，它的基本思想是通过取相邻像素的平均值来内插每个通道中的遗漏值，我们以下图中点R13、G14和B19为例具体计算如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\demosaicing-1685416203627.png" style="zoom: 150%;" />
$$
R13_G=\frac{1}{4}(G8+G12+G14+G18)\\
R13_B=\frac{1}{4}(B7+B9+B17+B19)\\
G14_R=\frac{1}{2}(R13+R15)\\
G14_B=\frac{1}{2}(B9+B19)\\
B19_R=\frac{1}{4}(R13+R15+R23+R25)\\
B19_G=\frac{1}{4}(G14+G18+G20+G24)\\
$$
线性插值没有考虑到各个颜色通道之间的相关性，也没有进行边缘的判断，在使用过程中容易产生伪像。2004年，Malvar提出了一种改进的线性插值算法，由于跟好的效果和相对低的复杂度，该算法广泛用于ISP处理过程中。

该算法改进了双线性插值算法，使用了拉普拉斯算子对原始线性插值进行校正，对于红色像素点的绿色分量值，该算法用一个离散的5点拉普拉斯变换估计如下：
$$
\hat{G}(i,j)=\hat{G}^{bl}(i,j)+\alpha\nabla_R(i,j)\\
\hat{G}^{bl}(i,j)=\frac{1}{4}(G(i-1,j)+G(i+1,j)+G(i,j-1)+G(i,j+1))\\
\nabla_R(i,j)=R(i,j)-\frac{1}{4}(R(i-2,j)+R(i+2,j)+R(i,j-2)+R(i,j+2))
$$
对于蓝色像素点的绿色分量也用同样方法求出：
$$
\hat{G}(i,j)=\hat{G}^{bl}(i,j)+\alpha\nabla_B(i,j)\\
\hat{G}^{bl}(i,j)=\frac{1}{4}(G(i-1,j)+G(i+1,j)+G(i,j-1)+G(i,j+1))\\
\nabla_B(i,j)=B(i,j)-\frac{1}{4}(B(i-2,j)+B(i+2,j)+B(i,j-2)+B(i,j+2))
$$
对于蓝色像素点的红色分量值，该算法用一个离散的5点拉普拉斯变化估计如下：
$$
\hat{R}(i,j)=\hat{R}^{bl}(i,j)+\gamma\nabla_B(i,j)\\
\hat{R}^{bl}(i,j)=\frac{1}{4}(R(i-1,j-1)+R(i-1,j+1)+R(i+1,j-1)+R(i+1,j+1))
$$
对于红色像素点的蓝色分量值，可用同样的方法求出：
$$
\hat{B}(i,j)=\hat{B}^{bl}(i,j)+\gamma\nabla_R(i,j)\\
\hat{B}^{bl}(i,j)=\frac{1}{4}(B(i-1,j-1)+B(i-1,j+1)+B(i+1,j-1)+B(i+1,j+1))
$$
对于绿色像素点的红色和蓝色分量值，该算法用一个离散的9点拉普拉斯变换估计如下：
$$
\hat{R}(i,j)=\hat{R}^{bl}(i,j)+\beta\nabla_G(i,j)\\
\hat{B}(i,j)=\hat{B}^{bl}(i,j)+\beta\nabla_G(i,j)
$$
以上式子中三个参数取值如下：
$$
\alpha=\frac{1}{2}\\
\beta=\frac{5}{8}\\
\gamma=\frac{3}{4}
$$
最终的过滤器如下图，注意该图中所有点的系数都扩大了8倍，因此最终结果要缩小8倍：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CFAI.png"  />



## RGB Domain Processing  

### 7.Gamma Correction（GC）

CMOS和CCD型传感器的感光是线性的，即光强和电平之间的转换为线性关系，但人眼对自然光的感受是非线性的（呈对数关系），人眼对亮度的相应曲线如下图：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\HVS.png)

一般而言人眼对暗区的变化更敏感，因此我们需要对图片进行校正，使得暗处细节更加明显。伽马校正是一种非线性操作，在最简单的情况下，伽马校正值由以下幂函数计算：
$$
Y_{out}=Y_{in}^\gamma
$$
在伽马校正过程中，我们一般让伽马值r<1，从而让暗部细节更明显，通过调节伽马值，我们可以得到具有较好对比度和动态范围的图像。不同伽马值下校正效果如下图，(a)图中的伽马值r=0.5，(b)图中r=1，(c)图中r=2：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\gamma_correction.webp"  />

ISP中实现伽马校正的方法很简单，我们只需存储一张查找表（LUT）即可，表中存储着各个灰阶值对应的伽马校正值，ISP只需要把查表输出校正值即可。

### 8.CCM（Color Correction Matrix）

CCM指的是颜色校正矩阵，由于sensor的R/G/B响应曲线和人眼的R/G/B响应曲线是不一致的，因此需要使用CCM校正矩阵消除这种颜色误差。下图显示了颜色校正之前和颜色校正之后的图像：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CCM_1.jpg"  />

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CCM_2.jpg)

为了达到上述的要求，我们可以理解为人眼对物体感受的颜色是我们的目标，那么就需要将sensor感光数据经过某种变换达到我们的目标。假设人眼能感受到的颜色种类有m种，那么自然界的颜色就可以表示为一个3Xm的矩阵，同理sensor对自然界的感光也可以得到一个3Xm的矩阵。我们需要做的就是将右侧sensor感光的数据转换到左侧人眼感光的数据上来。
$$
\left[
	\begin{matrix}
	R_1 & R_1 & ... & R_m\\
	G_2 & G_2 & ... & G_m\\
	B_3 & B_3 & ... & B_m\\
	\end{matrix}
\right]
<==
\left[
	\begin{matrix}
	R'_1 & R'_1 & ... & R'_m\\
	G'_2 & G'_2 & ... & G'_m\\
	B'_3 & B'_3 & ... & B'_m\\
	\end{matrix}
\right]
$$


为了达到上述要求，在实际操作中，我们需要使用CCM把sensor RGB色彩空间转为sRGB色彩空间。下图可知，由sensor RGB空间分别经过 ![M^{2}](https://www.zhihu.com/equation?tex=M%5E%7B2%7D) 和 ![M^{1}](https://www.zhihu.com/equation?tex=M%5E%7B1%7D) 以及![\gamma](https://www.zhihu.com/equation?tex=%5Cgamma) 校正可以实现到sRGB色彩空间的转换。sensor RGB空间我们称之为“源色彩空间”，非线性sRGB空间称之为“目标颜色空间”，目前，我们能够得到源色彩空间的“不饱和图”对应的24色色快，也有非线性sRGB空间的“饱和图”对应的24色色快，而 ![M^{1}](https://www.zhihu.com/equation?tex=M%5E%7B1%7D) 和 ![\gamma](https://www.zhihu.com/equation?tex=%5Cgamma) 的数值是已知的，那么，只需要将非线性sRGB空间的图片经过反 ![\gamma](https://www.zhihu.com/equation?tex=%5Cgamma) 校正然后再转换到XYZ空间，那时就可以和sensorRGB数值联立从而求得矩阵 ![M^{2}](https://www.zhihu.com/equation?tex=M%5E%7B2%7D) ，继而求得 ![M](https://www.zhihu.com/equation?tex=M) 。在这里，矩阵 ![M](https://www.zhihu.com/equation?tex=M) 就是所求的颜色校正矩阵。

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CCM_M.png"  />

具体计算CCM矩阵的方法有多种，本文介绍两个典型算法，多项式拟合和三维查表法（3D-LUT）。

多项式拟合的方法具体做法是：

1. 对非线性sRGB做反Gamma校正得到线性sRGB，![sRGB_{linearity}](https://www.zhihu.com/equation?tex=sRGB_%7Blinearity%7D) = ![(sRGB_{nonlinearity})^{\gamma}](https://www.zhihu.com/equation?tex=%28sRGB_%7Bnonlinearity%7D%29%5E%7B%5Cgamma%7D)
2. 将线性sRGB转换至CIE XYZ空间，![XYZ_{m*n}](https://www.zhihu.com/equation?tex=XYZ_%7Bm%2An%7D) = ![sRGB_{m*n}](https://www.zhihu.com/equation?tex=sRGB_%7Bm%2An%7D) * ![(M_{n*n}^{1})^{-1}](https://www.zhihu.com/equation?tex=%28M_%7Bn%2An%7D%5E%7B1%7D%29%5E%7B-1%7D)

3. 由于![sensorRGB_{m*n}*M_{n*n}^{2}=XYZ_{m*n},](https://www.zhihu.com/equation?tex=sensorRGB_%7Bm%2An%7D%2AM_%7Bn%2An%7D%5E%7B2%7D%3DXYZ_%7Bm%2An%7D%2C)其中，m是色块的大小，如果取24色卡，则m=24，n为每个色块的平均颜色值，n=3，显然m>n，这是过定问题，可得，![M_{n*n}^{2}=(sensorRGB^{T}*sensorRGB)^{-1}*(sensorRGB^{T}*XYZ)](https://www.zhihu.com/equation?tex=M_%7Bn%2An%7D%5E%7B2%7D%3D%28sensorRGB%5E%7BT%7D%2AsensorRGB%29%5E%7B-1%7D%2A%28sensorRGB%5E%7BT%7D%2AXYZ%29)

4. 由m1、m2求得：![M=M^{2}*M^{1}](https://www.zhihu.com/equation?tex=M%3DM%5E%7B2%7D%2AM%5E%7B1%7D)

三维查表法很简单，就是将两个矩阵表示的颜色都通过一张表来表示对应的关系，那么进来一个颜色就可以通过查表快速得到想要的颜色。

### 9.CSC（Color Space Conversion）

CSC即色彩空间转换，通过一些线性变化，将原本图像的颜色空间转换到其他的颜色空间，常见的有RGB2YUV，RGB2SV等等，但是通常在ISP的Pipeline中用到的CSC转换只有RGB2YUV，YUV色彩空间中，“Y”表示明亮度（Luminance、Luma），“U”和“V”则是色度、浓度，ISP要实现RGB2YUV转换就必定会有一个转换公式，因为转换过程中采用的标准不同，所有转换公式也有所区别。由于CSC算法固定，所以它一般固化在ISP芯片中。

Pipeline中需要这么一个转换将RGB转为YUV主要有三个原因：

1. YUV是早期欧洲定义的一种信号格式，主要是为了解决黑白电视和彩色电视过渡时期的信号兼容问题，黑白电视只需要亮度值，不需要彩色信号，而彩色电视既需要亮度信号也需要色彩信号，所以如果直接使用RGB就会带来兼容问题，而采用YUV信号，黑白电视不处理彩色信号即可；
2. 可以将Y和UV分开处理，即将亮度信号和色度信号分开处理，这样更符合HVS，因为本专题前面的博文也提到过HVS中人眼对亮度信号更明高，对色度信号相对不明感，那么在去噪等一些处理的时候就可以针对不同层面的信号做不同强度的处理，从而最大程度的保护图像效果；
3. 为后续的数据压缩做准备，因为通常现在用的多的MJPG和网络传输用的H264和H265等信号都是基于YUV信号的基础上做进一步的数据压缩得来的

# 三、扩展与参考

## AI在去噪和hdr的应用



## 色彩的基本理论

### 人眼对色彩的感知

人眼之所以能感受到自然界的颜色是因为人眼的视锥细胞在起作用，人眼主要通过三种视锥细胞感受三种不同波长的光从而感受颜色。430nm波长的光被视为蓝色，主要激活S型锥形细胞；大多数波长以550nm为中心的光被视为绿色，主要刺激M型锥形细胞；600nm波长或更长波长的光被视为红色，主要刺激L型锥形细胞。正是如此，人类才能够产生各种颜色的感觉。如图所示是人眼感受不同波长的反应曲线，分别对应三种不同的波长，所以通常用RGB三原色来表示颜色：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\sensitivity-1685673123148.png" style="zoom:80%;" />

### CIE RGB

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_RGB.png"  />

为了模拟人眼对色彩的感知，人们进行了色彩匹配实验，该实验原理如上图所示，右侧有两个屏幕，一侧有一个可以改变波长的光源，一侧是固定三种波长（红绿蓝）的光源，然后人眼通过一个角度去看这两个屏幕。f光源改变不同的波长呈现出不同的颜色，然后通过P1，P2，P3三个波长的光源不同的强度的混合，使得人眼感受到两个屏幕的颜色一致，然后记录下此时的三色值，那么该三色值就表示f测波长对应的颜色值。通过该实验就得到了CIE RGB颜色空间的数据如下图。

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_RGB_data.png)

从图中可以看到CIE RGB有数据是负数，这个是因为某个波长颜色的光通过P1，P2，P3在一侧混合无法得到，那么就需要将某个光源放到f测去才能到达这种效果，那么此时就相当于对改颜色做了减法，那么就出现了负数。上图CIE RGB空间为三维的不方便操作，那么就对RGB做归一化，使得R+G+B=1，那么一直其中两个颜色就可以得到第三个颜色，从而将三维空间降维到二维方便操作，归一化结果如下图。

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_RGB_normalized.png)

### CIE XYZ

由于CIE RGB会出现负数，不适合理解和操作，所以人为定义一组新的三原色XYZ，实现通过整数表示所有颜色。经过一系列的坐标变换之后就可以重新得到一个XYZ的颜色空间，同理经过归一化之后可以投影到二维空间方便理解。CIE XYZ色彩空间如下图。

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_XYZ_1.png)

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_XYZ_2.png)

## 图像处理中的微积分

对于一个二维函数f(x,y)，其偏导数为：
$$
\frac{\partial f}{\partial x}=\lim\limits_{h\rightarrow 0}{\frac{f(x+h,y)-f(x,y)}{h}}\\
\frac{\partial f}{\partial y}=\lim\limits_{h\rightarrow 0}{\frac{f(x,y+h)-f(x,y)}{h}}
$$
因为图像时离散的二维函数，h不能无限小，图像是按照像素来离散的，最小的h就是1像素，因此，图像的偏导数变成了差分的形式：
$$
\frac{\partial f}{\partial x}={f(x+1,y)-f(x,y)}\\
\frac{\partial f}{\partial y}={f(x,y+1)-f(x,y)}
$$
根据梯度的定义，图像的垂直和水平方向的梯度为：
$$
g_x=\frac{\partial f}{\partial x}={f(x+1,y)-f(x,y)}\\
g_y=\frac{\partial f}{\partial y}={f(x,y+1)-f(x,y)}
$$
图像中某一像素点(x,y)处的梯度值为：
$$
M(x,y)=mag(\nabla f)=\sqrt{g_x^2+g_y^2}\approx|g_x|+|g_y|
$$
根据图像的一阶偏导，可以求出二阶偏导如下：
$$
\frac{\partial ^2f}{\partial x^2}={f_x'(x+1,y)-f_x'(x,y)}=f((x+1)+1,y)-f((x+1),y)-f(x+1,y)+f(x,y)=f(x+2,y)-2f(x+1,y)+f(x,y)\\
\frac{\partial ^2f}{\partial y^2}={f_y'(x,y+1)-f_y'(x,y)}=f(x,(y+1)+1)-f(x,(y+1))-f(x,y+1)+f(x,y)=f(x,y+2)-2f(x,y+1)+f(x,y)
$$

即：
$$
\frac{\partial^2f}{\partial x^2}=f(x+2,y)-2f(x+1,y)+f(x,y)\\
\frac{\partial^2f}{\partial y^2}=f(x,y+2)-2f(x,y+1)+f(x,y)
$$
令x=x-1，y=y-1得到：
$$
\frac{\partial^2f}{\partial x^2}=f(x+1,y)+f(x-1,y)-2f(x,y)\\
\frac{\partial^2f}{\partial y^2}=f(x,y+1)+f(x,y-1)-2f(x,y)
$$
我们把x方向和y方向的二阶导数结合在一起，得到拉普拉斯二阶微分算子：
$$
\frac{\partial^2f}{\partial x^2}+\frac{\partial^2f}{\partial y^2}=f(x+1,y)+f(x-1,y)+f(x,y+1)+f(x,y-1)-4f(x,y)
$$

## 开源ISP项目

[openISP(python)](https://github.com/cruxopen/openISP)、[fast-openISP(python)](https://github.com/QiuJueqin/fast-openISP)、[ISP-pipeline-hdrplus(c/c++)](https://github.com/jhfmat/ISP-pipeline-hdrplus)、[xk-isp(c/c++)](https://github.com/T-head-Semi/open_source_ISP)

## ISP学习资料

[awesome-ISP](https://github.com/starkfan007/awesome-ISP)、[ISPAlgorithmStudy](https://gitee.com/wtzhu13/ISPAlgorithmStudy)

