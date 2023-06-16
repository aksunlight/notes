# 一、ISP概述

ISP（Image Signal Processor），即图像信号处理器，主要用来对前端图像传感器（如CMOS/CCD图像传感器）输出的图像信号（一般是数字信号）做后处理。通过ISP的处理，包括内插、去噪、去坏点、边缘加强、白平衡等操作后，可以使图像更加细腻，贴近真实世界中的图像。下面通过开源项目fast-openISP具体介绍ISP处理流程并分析相关算法和基本算子。

# 二、ISP流程

对于彩色图像传感器，因其对色彩的分辨方式不同（主要是覆盖在传感器感光阵列上的彩色滤波阵列的不同），其输出图像的格式也有所不同。对于使用Bayer滤镜的彩色图像传感器，它产生的原始数据叫做Bayer模式图像（Bayer pattern image），这是大多数彩色传感器输出的图像格式，项目fast-opneISP处理的也是Bayer格式的原始图像。Bayer滤镜的最大特点是阵列中有50%是绿色，25%是红色，另外25%是蓝色，显微镜下Bayer滤镜和感光单元的图像如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\bayer_filter.png" style="zoom:67%;" />

从传感器获取到原始图像后ISP的处理流程如下：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\isp_pipeline_3.jpg"  />

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\isp_process.png"  />

## Bayer Domain Processing

### 1. DPC（Dead Pixel Correction）

因为传感器制造的原因，出厂时传感器中可能会有一定数量的缺陷像素单元，这些缺陷像素是在制造过程中产生的，在拍照时无法避免，只能通过后处理进行消除。根据缺陷像素的值的不同，缺陷像素可分为死点（总是偏低）、热点（总是偏高）、卡点（总是某一固定值）三种类型，对于这些缺陷像素的纠正有两个步骤，第一步是检测缺陷像素，第二步是用内插值替换异常值，其过程如下图：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\dead_pixel.png)

对于卡点异常，通过拍摄黑色或者白色图像很容易找到位置，然后将这些异常像素的位置存储到ISP存储器中以供更正。对于动态缺陷（值总是偏低或偏高），通常使用相邻像素来估计，即通过中心像素与周围像素（通常取周围8个相同颜色的像素值）的差值来判断差异，如果差值均大于预设的阈值t则判断为异常像素。计算如下：
$$
P_0=
	\begin{cases}
		dead/hot, & abs(P_i-P_0)>t,i=1..8\\
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

由于相机镜头的构造，光照在透过镜头照射在感光单元上时，边角处的焦点夹角大于中心焦点夹角，造成边角失光（边缘处透射的光的量减少），表现在图像上的效果就是亮度从图像中心到四周逐渐衰减，这种情况叫做luma shading（亮度阴影），下图为均匀光照下拍摄灰色图像时产生的亮度阴影。

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\luma_shading.png" style="zoom: 48%;" />

另一方面，CCD和CMOS型的传感器对红色光更敏感，并且还显著扩展至近红外线（IR）光谱中，这将引起颜色再现的误差，此外由于IR滤波器的存在，具有较大入射角度的图像传感器的边缘处较多的阻挡较长波长（如红光），从而导致图像中的在空间上的不均匀色温。这种情况叫做color shading（颜色阴影），下图为颜色阴影示意图。

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\color_shading.png" style="zoom: 80%;" />

对于lens shading的纠正一般使用查找表（LUT）校正法，其主要思路是使用一张表存储所有像素的增益值（达到目标值需要乘上的系数，又叫gain值），最终纠正的值就是增益值乘上初始值。在实际操作中，ISP芯片一般不会存储所有像素点的gain值，而是将图像分块，存储各块顶点的gain值，具体步骤如下：

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

BNR即拜尔域降噪，旨在消除RAW图中的噪声。Sensor输出的RAW图像本身是携带了噪声的，Sensor噪声的种类主要包括热噪声、光散粒噪声、读出噪声、固定模式噪声等（参考：[详解图像传感器噪声](https://blog.csdn.net/weixin_44580210/article/details/104141914)）。图像传感器噪声会造成各种各样的图像噪声（参考：[图像噪声模型](https://blog.csdn.net/weixin_44580210/article/details/105087523)）。当Sensor温度较高、增益较大、环境较暗的情况下各种噪声会变得更加明显，成为影响图像质量的主要因素。

目前主流的ISP产品中一般会选择在RAW域、RGB域、YUV域等多个环节设置降噪模块以控制不同类型和特性的噪声。在YUV域降噪的方法已经得到了广泛的研究并且出现了很多非常有效的算法，但是在RAW域进行降噪则因为RAW数据本身的一些特点而受到不少限制，主要的限制是RAW图像中相邻的像素点分别隶属于不同的颜色通道，所以相邻像素之间的相关性较弱，不具备传统意义上的像素平滑性，所以很多基于灰度图像的降噪算法都不能直接使用，又因为RAW数据每个像素点只含有一个颜色通道的信息，所以很多针对彩色图像的降噪算法也不适用。

目前在RAW域降噪基本都需要将RAW图像按照颜色分成四个通道(R,Gr,Gb,B)，然后在各个通道上分别应用滤波器进行平滑，根据滤波器的特点和复杂度大致可以分成以下几类：

1. 经典低通滤波器，如均值滤波、中值滤波、高斯滤波、维纳滤波等。这类方法的优点是比较简单，占用资源少，速度快，缺点是滤波器是各向同性的，容易破坏图像中的边缘。另外由于没有考虑颜色通道之间的相关性所以也容易引入伪彩等噪声，而人眼对这种颜色噪声是比较敏感的。
2. 改进的经典滤波器，如Eplison滤波、双边滤波(bilateral filter)，在经典滤波器的基础上增加了阈值检测用于区分同类像素和异类像素，同类像素分配较大的滤波权重，异类像素则权重很小因而基本不参与滤波。这类方法的优点是可以有效地保护图像边缘，复杂度增加也不大，其它特点与经典滤波器基本相同。
3. 基于块匹配的滤波算法，利用图像的自相似特性，在以当前像素为中心的一个滤波窗口内找到与当前块最相似的几个块，当前像素的滤波值即等于几个相似块的中心像素的加权平均值。此类算法以非局部均值滤波(Non-Local Means)和BM3D(Block Matching 3D)算法为代表，它们的优点是平滑性能和边缘保持性能很好，缺点是计算量很大，资源消耗大，不太适合处理实时视频。已有情报显示，华为在某些型号手机芯片中实现了BM3D算法用于对照相图片做降噪处理，估计这主要是为了体现行业龙头企业占领技术高地的决心。
4. 引导滤波器(guided filter)，由何凯明博士早期提出的一种算法，引入了引导图像的概念。对于任一颜色通道的图像，以当前位置像素**P**(x,y)为中心，在一个固定大小的滤波窗口内为**P**(x,y)的所有邻近像素{**P**(i,j)}计算权重{**W**(i,j)}，计算权重的方法是以某个引导图像**I**作为参考，在引导图像的对应滤波窗口内，凡是与像素**I**(x,y)性质“类似”的像素都得到较大的权重，与**I**(x,y)性质“相反”的像素则得到较小的权重。举例来说，如果当前像素**P**(x,y)是亮的，则滤波窗口{**P**(i,j)}中的全部亮像素会参与平滑，平滑结果仍是亮的，同理，如果与**P**(x,y)相邻的某个像素**P**(x‘,y’)是暗的，则对应的{**P**(i‘,j’)}中的全部暗像素会参与平滑，平滑结果仍是暗的，这样**P**(x,y)与**P**(x',y')之间的明暗边界就得到了保持。这种方法的基本假设是图像各通道的颜色梯度分布与引导图像是一致的，如果假设不成立，则从引导图像计算出的权重反而容易破坏其它通道中的边缘。有人建议使用demosaic还原出的G通道图像作为引导图像会得到较理想的效果，因为G通道集中了图像的大部分能量，也包含了丰富的梯度信息，是比较理想的候选对象。

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

彩色图像的每一个像素点都需要红、绿、蓝三个色彩分量的信息，而由Bayer阵列所产生的RAW图中的每一个像素点只有红或绿或黄一个色彩分量的信息，因此ISP必须估算出每个像素处缺失的另外两个颜色值，这个过程叫做色彩滤波阵列插值（CFAI），也叫去马赛克（Demosaicing）。

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
线性插值没有考虑到各个颜色通道之间的相关性，也没有进行边缘的判断，在使用过程中容易产生伪像。2004年，Malvar提出了一种改进的线性插值算法，由于更好的效果和相对低的复杂度，该算法广泛用于ISP处理过程中。

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

我们可以理解为人眼对物体感受的颜色是我们的目标，那么就需要将sensor感光数据经过某种变换达到我们的目标。假设人眼能感受到的颜色种类有m种，那么自然界的颜色就可以表示为一个3Xm的矩阵，同理sensor对自然界的感光也可以得到一个3Xm的矩阵。我们需要做的就是通过CCM矩阵将右侧sensor感光的数据转换到左侧人眼感光的数据上来。
$$
\left[
	\begin{matrix}
	R_1 & R_1 & ... & R_m\\
	G_2 & G_2 & ... & G_m\\
	B_3 & B_3 & ... & B_m\\
	\end{matrix}
\right]
<==M*
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
2. 可以将Y和UV分开处理，即将亮度信号和色度信号分开处理，这样更符合HVS，因为HVS中人眼对亮度信号更敏感，对色度信号相对不敏感，那么在去噪等一些处理的时候就可以针对不同层面的信号做不同强度的处理，从而最大程度的保护图像效果；
3. 为后续的数据压缩做准备，因为通常现在用的多的MJPG和网络传输用的H264和H265等信号都是基于YUV信号的基础上做进一步的数据压缩得来的；

## YUV Domain Processing  

### 10.Edge Enhancement

边缘锐化(Edge crispening)是一种用滤波器进行影像处理的过程，通过加强影像或是影片的边缘对比度来获得在视觉上较受使用者喜爱的具有清晰边界的影像。单纯使用高通滤波器来锐化边缘，会导致噪声同时被放大，故常用非锐化滤波(Unsharp masking)来处理，非锐化滤镜结合全通滤波器和低通滤波器以达到既锐化边缘，又不放大噪声的效果，但通常处理后影像中的物体会变得较不清晰。在YUV图像中，该滤波过程在Y通道中处理。

非锐化滤波的过程如下：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\unsharp-masking-step-1686019065012.jpg)

1. 载入一张原始影像，对于YUV影像则只使用Y表征的明度值，得〈Result 1〉
2. 对〈Result 1〉做 Sobel Operator，会得到一张一阶微分的〈Result 2〉
3. 对〈Result 1〉做 Laplace Operator ，会得到一张二阶微分的〈Result 3〉
4. 对〈Result 2〉做 算术平均滤波(Mean Filter)，将做完的结果归一化[0,1]得到〈Result 4〉
5. 将〈Result 3〉与〈Result 4〉做相乘的动作，得到我们真正要 Enhance 的数值〈Result 5〉
6. 用〈Result 5〉对原始影像(Source Image) 做 Enhancement

其中〈Result 4〉即是我们的Mask；〈Result 3〉为从原图过滤出來的边缘细节(detail)。
步骤(5) 利用〈Result 4〉做为Mask 把〈Result 3〉非影像边缘的数值(即被放大过后的杂波)给滤掉。
步骤(6) 利用〈Result 5〉对原图做Enhancement 即完成了所谓的Unsharp masking。

~~具体计算如下：~~

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\unsharp-masking.jpg)

~~如上图所示，输入为![{\displaystyle F(j,k)}](https://wikimedia.org/api/rest_v1/media/math/render/svg/bbc188ee0344adccf4024a62a6f4bb633f32579a)，输出为![{\displaystyle G(j,k)}](https://wikimedia.org/api/rest_v1/media/math/render/svg/dc2225c61092ccd802b14caeeadff36e58e37963)。![{\displaystyle G(j,k)}](https://wikimedia.org/api/rest_v1/media/math/render/svg/dc2225c61092ccd802b14caeeadff36e58e37963)是全通滤波器和低通滤波器![{\displaystyle LP}](https://wikimedia.org/api/rest_v1/media/math/render/svg/3272aece287bc6081ba5e195a4a57fc4a1f7ccc4)分别对![{\displaystyle F(j,k)}](https://wikimedia.org/api/rest_v1/media/math/render/svg/bbc188ee0344adccf4024a62a6f4bb633f32579a)进行处理后的线性组合。~~

### 11.Hue/Saturation Control

色调（Hue）是颜色的主要属性之一（称为颜色外观参数），在技术上定义为“刺激可以被描述为与被描述为红色、绿色、蓝色和黄色的刺激相似或不同的程度”（在某些色觉理论中称为独特色调）。色调通常可以用单个数字定量表示，通常对应于色彩空间坐标图（例如色度图）或色轮上围绕中心或中性轴的角度位置，或者通过其主波长或其主波长互补色。大多数可见光都包含一定波长范围内的能量。色调是可见光谱中光源输出能量最大的波长。这在随附的强度与波长曲线图中显示为曲线的峰值。 颜色的饱和度（Saturation）由光强度和它在不同波长的光谱中分布的程度共同决定。随着饱和度的增加，颜色显得更加“纯净”。随着饱和度降低，颜色显得更加“褪色”。饱和度是颜色的纯度或鲜艳度，饱和度为 100% 的颜色不含白色。饱和度为 0% 的颜色对应于灰色阴影。

色调和饱和度的调整都应用于色度通道，即UV通道。色调调整涉及色调角度的变化，其计算如下，其中正余弦函数值通过查表得到：
$$
Cb’=(Cb-128)\cos{\theta}+(Cr-128)\sin{\theta}+128\\
Cr'=(Cr-128)\cos{\theta}-(Cb-128)\sin{\theta}+128\\
(Cb",Cr")=clip((Cb",Cr"),0,255)
$$
饱和度计算如下，其中k为饱和度调整比例：
$$
(Cb",Cr")=k*((Cb,Cr)-128)+128
$$




# 三、ISP高级处理功能

## HDR

HDR即高动态范围（High Dynamic Range），动态范围就是sensor在一幅图像里能够同时体现高光和阴影部分内容的能力，实际上指的是摄像头同时可以看清楚图像最亮和最暗部分的照度对比。用公式表达这种能力就是：DR = 20log10(i_max / i_min)，其中i_max 是sensor的最大不饱和电流，i_min是sensor的暗电流（blacklevel）。

在自然界的真实情况，有些场景的动态范围要大于100dB，而普通sensor的动态范围一般在60-70dB。所以当sensor的动态范围小于图像场景动态范围的时候就会出现HDR问题，不是暗处看不清，就是亮处看不清，有的甚至两头都看不清，显示效果如下：

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\HDR1.jpg)

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\HDR2.jpg)

根据前边动态范围公式DR = 20log10(i_max / i_min)，从数学本质上说要提高DR，就是提高i_max，减小 i_min；对于10bit输出的sensor，i_max =1023，i_min =1，动态范围DR = 60；对于12bit输出的sensor，DR = 72。所以从数学上来看，提高sensor 输出的bit width就可以提高动态范围，从而解决HDR问题。可是现实上却没有这么简单。提高sensor的bit width导致不仅sensor的成本提高，整个图像处理器的带宽都得相应提高，消耗的内存也都相应提高，这样导致整个系统的成本会大幅提高。

从Sensor的角度完整的动态范围DR公式如下：
$$
DR=20\lg\frac{i_{max}}{i_{min}}=20\lg\frac{\frac{qQ_{sat}}{t_{int}}-i_{dc}}{\frac{q}{t_{int}}\sqrt{b\frac{1}{q}i_{dc}t_{int}+\sigma^2_r}}
$$
Q_sat: Well Capacity；i_dc: 底电流；t_int: 曝光时间；σ:噪声。

目前解决HDR问题的主要方法有：

1. 提高Qsat –Well capacity。就是提高感光井的能力，这就涉及到sensor的构造，简单说，sensor的每个像素就像一口井，光子射到井里产生光电转换效应，井的容量如果比较大，容纳的电荷就比较多，这样i_max的值就更大。

2. 多帧曝光合成。本质上这种方法就是用短曝光获取高光处的图像，用长曝光获取阴暗处的图像。有的厂家用前后两帧长短曝光图像，或者前后三针长、中、短曝光图像进行融合。当该像素值大于一个门限时，这个像素的数值就是来自于短曝光，小于一个数值，该像素值就来自于长曝光，在中间的话，就用长短曝光融合。这种多帧融合的方法需要非常快的readout time，而且即使readout time再快，多帧图像也会有时间差，所以很难避免在图像融合时产生的鬼影问题。尤其在video HDR的时候，由于运算时间有限，无法进行复杂的去鬼影的运算，会有比较明显的问题。于是就出现了单帧的多曝光技术。

3. 单帧空间域多曝光。最开始的方法是在sensor的一些像素上加ND filter（Neutral density filter），让这些像素获得的光强度变弱，所以当其他正常像素饱和的时候，这些像素仍然没有饱和，不过这样做生产成本比较高，同时给后边的处理增加很多麻烦。通常的做法是进行隔行的多曝光，如下图所示，两行短曝光，再两行长曝光，然后做图像融合，这样可以较好的避免多帧融合的问题，从而有效的在video中实现HDR。

   ![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\exposure.jpg)

4. 使用logarithmic sensor。实际是一种数学方法，把图像从线性域压缩到log域，从而压缩了动态范围，在数字通信里也用类似的技术使用不同的函数进行压缩，在isp端用反函数再恢复到线性，再做信号处理。

5. 局部适应（local adaption）。这是种仿人眼的设计，人眼会针对局部的图像特点进行自适应，既能够增加局部的对比度，同时保留大动态范围。这种算法比较复杂，有很多论文单独讨论。目前在sensor端还没有使用这种技术，在ISP和后处理这种方法已经得到了非常好的应用。

以上方法中，方法1和方法4都需要传感器有相应的特性，而方法2、3和5主要通过ISP端配合实现。目前，主流的HDR方法是多帧融合曝光的方法，在这方面的代表是谷歌的HDR+技术。

## NR

在讲解Bayer域的NR步骤时，我们知道了降噪的主要方法，下面本文补充一些经典算法和基于深度学习的算法。

1. 双边滤波

   虽然高斯滤波相对于普通均值滤波而言性能有所提升，但是依然对边缘随时严重，为了进一步保留边缘信息，就需要对边缘做进一步处理。于是就有人提出在高斯分布的基础上再加一个权重，这个权重和像素值的差别挂上联系，这个算法就是双边滤波算法。

   ![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\NLM.png)

   从上图中可以看出双边滤波有两个高斯权重叠加而来。前面||p-q||就是高斯滤波中使用的距离权重，后面的|Ip-Iq|就是像素值的高斯分布的权重。其实也很好理解，去噪的时候肯定需要相似的区域有更大的贡献，不相同的给小的权重，而对于边缘而言，那么两侧的像素值的差距肯定很大，也就会导致离边缘另一侧不同的会分配一个小权重，而同侧相差小就会有一个很大的权重，这样就不会由于取平均的时候将边缘两侧的大差异变小导致边缘变弱了，从而起到保留边缘的目的

2. NLM（non-local mean）

   NLM是利用整个图像的信息来进行降噪滤波处理。如下图所示，对于p点而言，q1和q2点所在的邻域和p所在的邻域更相似，那么就给q1和a2较大的权重，而q3邻域和p邻域差别较大，就赋予一个较小的权重。具体权重的赋予方式其实也是高斯的一种方式，只不过e的对数是通过邻域简单欧拉距离来计算。

   <img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\NLM_1.jpg" style="zoom: 67%;" />

   具体权值的计算如下：

   <img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\NLM_2.jpg"  />

   从式子中可知，NLM算法相当于求了几邻域中对应位置像素差的平方和来当作分配权重的依据。当邻域相似时，这个方差就小，权重也就大，而差异很大时方差就很大，权重也就很小，满足了算法的需求。理论上每一个点进行去噪的时候会利用整个图像的信息来计算，但是为了降低运算量，一般不会用整个图像的信息来计算，而是在整个图像中先选择一个大的范围，然后用这个范围内的点的信息进行降噪处理。

3. BM3D

   参考：[从BM3D到VBM4D](https://blog.csdn.net/weixin_44580210/article/details/107139590)

4. 基于CNN网络的降噪方法

   参考：[DnCNN / FFDNet / CBDNet / RIDNet / PMRID / SID](https://blog.csdn.net/weixin_44580210/article/details/113828226)

目前，基于CNN网络的降噪算法已经优于传统的算法。

# 四、扩展与参考

## AI在去噪和HDR的应用

HDR处理还是以多帧曝光融合的方法为主，在这方面的代表是谷歌的HDR+技术。而在降噪方面，基于CNN神经网络的方法的效果已经超过了传统的算法。

## 色彩的基本理论

### 人眼对色彩的感知

人眼之所以能感受到自然界的颜色是因为人眼的视锥细胞在起作用，人眼主要通过三种视锥细胞感受三种不同波长的光。430nm波长的光被视为蓝色，主要激活S型锥形细胞；大多数波长以550nm为中心的光被视为绿色，主要刺激M型锥形细胞；600nm波长或更长波长的光被视为红色，主要刺激L型锥形细胞。如图所示是人眼感受不同波长的反应曲线，分别对应三种不同的波长，所以通常用RGB三原色来表示颜色：

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\sensitivity-1685673123148.png" style="zoom:80%;" />

### CIE RGB

<img src="D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_RGB.png"  />

为了模拟人眼对色彩的感知，人们进行了颜色匹配实验，该实验原理如上图所示，右侧有两个屏幕，一侧有一个可以改变波长的光源，一侧是固定三种波长（红绿蓝）的光源，然后人眼通过一个角度去看这两个屏幕。f光源改变不同的波长呈现出不同的颜色，然后通过P1，P2，P3三个波长的光源不同的强度的混合，使得人眼感受到两个屏幕的颜色一致，然后记录下此时的三色值，那么该三色值就表示f测波长对应的颜色值。通过该实验就得到了CIE RGB颜色空间的数据如下图。

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_RGB_data.png)

从图中可以看到CIE RGB有数据是负数，这个是因为某个波长颜色的光通过P1，P2，P3在一侧混合无法得到，那么就需要将某个光源放到f测去才能到达这种效果，那么此时就相当于对改颜色做了减法，那么就出现了负数。上图CIE RGB空间为三维的不方便操作，那么就对RGB做归一化，使得R+G+B=1，那么一直其中两个颜色就可以得到第三个颜色，从而将三维空间降维到二维方便操作，归一化结果如下图。

![](D:\notes\markdown\ISP图像信号处理流程及相关算法分析.assets\CIE_RGB_normalized.png)

### CIE XYZ

由于CIE RGB会出现负数，不适合理解和操作，所以人为定义一组新的三原色XYZ，实现通过正数表示所有颜色。经过一系列的坐标变换之后就可以重新得到一个XYZ的颜色空间，同理经过归一化之后可以投影到二维空间方便理解。CIE XYZ色彩空间如下图。

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
由一阶偏导我们可以得出常用的索贝尔算子卷子核如下：
$$
dx:
\left[
	\begin{matrix}
	-1 & -2 & -1\\
	0 & 0 & 0\\
	1 & 2 & 1\\
	\end{matrix}
\right]
----dy:
\left[
	\begin{matrix}
	-1 & 0 & 1\\
	-2 & 0 & 2\\
	-1 & 0 & 1\\
	\end{matrix}
\right]	
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
L(x,y)=\frac{\partial^2f}{\partial x^2}+\frac{\partial^2f}{\partial y^2}=f(x+1,y)+f(x-1,y)+f(x,y+1)+f(x,y-1)-4f(x,y)
$$

4邻域的拉普拉斯算子卷积核如下：
$$
\left[
	\begin{matrix}
	0 & -1 & 0\\
	-1 & 4 & -1\\
	0 & -1 & 0\\
	\end{matrix}
\right]
----
\left[
	\begin{matrix}
	-1 & -1 & -1\\
	-1 & 8 & -1\\
	-1 & -1 & -1\\
	\end{matrix}
\right]
$$


## 开源ISP项目

[openISP(python)](https://github.com/cruxopen/openISP)、[fast-openISP(python)](https://github.com/QiuJueqin/fast-openISP)、[ISP-pipeline-hdrplus(c/c++)](https://github.com/jhfmat/ISP-pipeline-hdrplus)、[xk-isp(c/c++)](https://github.com/T-head-Semi/open_source_ISP)

## ISP学习资料

[awesome-ISP](https://github.com/starkfan007/awesome-ISP)、[ISPAlgorithmStudy](https://gitee.com/wtzhu13/ISPAlgorithmStudy)、[光、颜色和色度图](https://zhuanlan.zhihu.com/p/129095380)、[Understanding ISP Pipeline](https://zhuanlan.zhihu.com/p/98820927)、[Understanding ISP Pipeline - Noise Reduction](https://zhuanlan.zhihu.com/p/102423615)、[Google Camera 发展梳理](https://zhihuhuhu.blogspot.com/2018/11/google-camera-development-review.html)

