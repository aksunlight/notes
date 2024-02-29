# gdb

注意事项：一些老的GNU编译器需要python2.7共享库

`sudo apt install libpython2.7`

`sudo apt install libatlas3-base`

[ubuntu20解决error while loading shared libraries: libpython2.7.so.1.0: cannot open shared object file_阿槐123456的博客-CSDN博客](https://blog.csdn.net/weixin_43491077/article/details/121644091)



函数调用栈：

[手把手教你栈溢出从入门到放弃（上） - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/25816426)

[手把手教你栈溢出从入门到放弃（下） - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/25892385)



Linux进程：

top

ps id pid

ps -aux |grep -v grep|grep pid



GPU显卡：

watch -n 0.25 nvidia-smi #每0.25s刷新一次



VNC远程桌面：

服务端设置：vncserver -geometry 1920x1080 :27

​					   vncserver -localhost :27



pip安装包导出：

pip list --format=freeze > requirements.txt

pip freeze >requirements.txt

