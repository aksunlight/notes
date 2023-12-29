# OCR编码识别

0.数据标注

标框(labeling)



1.文本检测

专用：

​	CTPN网络：CNN(Faster RCNN)+**LSTM Model**

​	EAST网络：

​	DBNet网络：

通用：

​	商汤mmdection库：优化后的Faster RCNN，

​	SSD网络



2.文本识别（基于图像的序列识别）

步骤：Feature extraction（特征提取，可使用VGG/ResNet/RCNN等网络结构，一般用CNN网络提取特征）

​			Sequence modeling（序列建模，可使用LSTM/GRU等网络结构，两者都是基于RNN改进而来，一般用

​                                                    RNN网络产生序列化标签）

​			Prediction（字符预测，可使用CTC网络结构或者Attention方法，CTC是一种基于RNN的序列到序列模型

​								（sequence-to-sequence models））

​			CNN网络（去除全连接层）用来提取特征向量序列：从卷积层组件产生的特征图中提取特征向量序列，这			些特征向量序列作为循环层的输入。具体地，特征序列的每一个特征向量在特征图上按列从左到右生成。			这意味着第i个特征向量是所有特征图第i列的连接。在我们的设置中每列的宽度固定为单个像素。

​			RNN网络（如LSTM）用来预测特征序列的标签分布：一个深度双向循环神经网络是建立在卷积层的顶部，			作为循环层。循环层预测特征序列x=x1,…,xT中每一帧xt的标签分布yt。

​			CTC转录：转录是将RNN所做的每帧预测转换成标签序列的过程。数学上，转录是根据每帧预测找到具有			最高概率的标签序列。使用CTC算法进行转录不需要做输入和输出之间的对齐。*Y*∗=*Y*argmax**p**(*Y*∣*X*).

专用：

​	CRNN网络：CNN(VGG16、ResNet18)+RNN(GRU、LSTM) Model with **CTC as loss funciton**

​					  					CNN+LSTM+CTC(实例deep-text-recognition-benchmark：               

​                                                                          https:://arxiv.org/abs/1904.01906)

​	Attention网络：

总结：[CRNN-CTC 模型是一种端到端的文本行图片识别方法，模型首先利用 CNN 提取图像卷积特征，然后使用 RNN 进一步提取图像卷积特征中的序列特征，最后引入 CTC 解决训练时字符无法对齐的问题](https://www.paddlepaddle.org.cn/modelbasedetail/crnnctc)

​	

3329张图的数据集进行非常多轮的训练，在900张左右的验证集上的准确率最高99%左右，无法达到100%的准确率。这可能的原因是我们用的crnn网络太简单，没有足够多的参数，从而无法拟合出复杂的模型；另外也可能是数据太少，数据代表性不够，比如验证集中有一个特别的数据，而训练集中没有相似特征的数据，从而训练出的模型对这样特征的图片预测准确率低。为了提高模型准确率我们可以使用第一次训练出的准确率最高的模型来预测新图片，然后将预测出错的图片加入数据集接续训练。

