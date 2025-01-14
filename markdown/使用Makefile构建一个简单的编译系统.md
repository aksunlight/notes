# 使用Makefile构建一个简单的编译系统

一个MP3播放器应用，它由若干个组件组成：用户界面（ui）、编解码器（codec）和数据管理库（db）。将这些组件源码放到各自目录下，通过Makefile构建编译系统就可以方便的产生这个应用程序。具体的文件结构为:

![](D:\学习笔记\目录.png)

我们使用最外层的Makefile文件，即总控Makefile文件，来调用项目中各个子目录的Makefile文件的运行。同时通过主控Makefile我们也可以控制子Makefile的执行顺序，在这里，我们通过设置依赖关系，先编译db和codec模块再编译ui模块，最后编译player项目，总控Makefile文件编写如下：

```
lib_codec := lib/codec
lib_db    := lib/db
lib_ui    := lib/ui
libraries := $(lib_codec) $(lib_db) $(lib_ui)
player    := app/player

.PHONY : all $(player) $(libraries)
all : $(player)
$(player) : $(libraries)

$(lib_ui) : $(lib_db) $(lib_codec)

$(player) $(libraries) :
	$(MAKE) -C $@
```

总控Makefile对每个子目录的Makefile调用的代码是：

``` 
$(player) $(librarise) :
	$(MAKE) -C %@
```

在 Makefile 中，源文件的编译可能需要依赖一系列的头文件。在一个比较大型的工程中，你必需清楚每一个源文件包含了哪些头文件，并且在加入或删除头文件时，也需要小心地修改Makefile，这是一个很没有维护性的工作。为了避免这种繁重而又容易出错的事情，我们可以使用 C/C++ 编译器的一个功能。大多数的 C/C++ 编译器都支持一个 “-M” 的选项，即自动找寻源文件中包含的头文件，并生成一个依赖关系文件（.d文件）。在项目codec、ui、db和player中我们均使用了.d文件来引入.c文件的依赖关系，并把依赖关系添加到Makefile文件中，这样我们改变.c的依赖（比如添加头文件）就不需要再修改Makefile文件了。

lib/codec/目录下的Makefile文件内容如下：

```
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

.PHONY:clean

codec:$(OBJS)
	gcc $^ -o $@

%.d:%.c
	gcc $< -MM -MF $*.d -MP
	
%.o:%.c
	gcc -c $< -o $@
	
clean:
	rm -rf *.o codec
	
-include $(DEPS)
```

使用Makefile编译code模块结果如下，从执行结果可以看到，运行Makefile后会先生成依赖文件再去编译生成目标文件

![](D:\学习笔记\子makefile.png)

当我们去掉main.c所依赖的头文件codec.h后再次执行make会发现重新生成了main.c的依赖文件main.d，同时更新了main.o目标文件和codec最终目标

![](D:\学习笔记\改变依赖.png)



也可将Makefile文件内容设置如下，将.d和.o文件一起生成：

```
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

.PHONY:clean

codec:$(OBJS)
	gcc $^ -o $@

%.o:%.c
	gcc -c $< -o $@ -MMD -MF $*.d -MP
	
clean:
	rm -rf *.o codec
	
-include $(DEPS)
```

lib/db/、lib/ui/、app/player/目录下的Makefile文件内容和lib/codec/下的相似，唯一不同的是终极目标分别变为db、ui和player

当执行主控Makefile时显示结果如下，从make执行结果中可知，编译按我们想象的顺序执行

![](D:\学习笔记\执行结果.png)





