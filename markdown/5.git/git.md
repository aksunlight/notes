# git

## 1.git基本操作

### 1.1从本地初始化仓库

~~~
cd /home/user/my_project
git init
git add *.cpp
git add ./src/
git add LICENSE
git commit -m 'initial my project'
~~~

### 1.2克隆现有仓库

~~~
git clone <url>
git clone <url> <set repository name>
echo '# test line' >> CONTRIBUTING.md
注意：如果你使用clone命令克隆了一个仓库，命令会自动将其添加为远程仓库并默认以origin为简写。默认情况下，git clone 命令会自动设置本地 master 分支跟踪克隆的远程仓库的 master 分支（或其它名字的默认分支）。

# 查看已暂存的修改和未暂存的修改
git status / git status -s/git status --short
# 比较工作目录中当前文件和暂存区域快照之间的差异，就是修改之后还没有暂存起来的变化内容
git diff
# 比对已暂存文件与最后一次提交的文件差异，查看已暂存的将要添加到下次提交里的内容
git diff --staged
git commit -m 'xxxx'
git commit -a -m 'first commit'	
~~~

### 1.3和远程仓库交互
~~~
注意：如果你使用clone命令克隆了一个仓库，命令会自动将其添加为远程仓库并默认以origin为简写。默认情况下，git clone 命令会自动设置本地 master 分支跟踪克隆的远程仓库的 master 分支（或其它名字的默认分支）。

# 查看远程仓库，该命令列出远程仓库服务器的简写
cd projectpath
git remote / git remote -v

# 添加远程仓库服务器，同时指定一个方便使用的简写
git remote add <shortname> <url>

# 从远程仓库中抓取数据，它不会自动合并或修改当前工作
git fetch <remote>

# 推送数据到远程仓库
git push <remote> <branch>
# 推送本地的branchwx分支来更新远程仓库上的branchwx分支
git push origin branchwx / git push origin branchwx:branchwx
# 将本地的branchwx分支推送到远程仓库上的awesomebranch分支
git push origin branchwe:awesomebranch

# 其他协作者抓取仓库数据，并将branchwx分支合入当前分支
git fetch origin
git merge origin/branchwx
# 或者在本地创建一个新的分支，并将其建立在远程跟踪分支之上
git checkout -b branchwx origin/branchwx / git checkout --track origin/branchwx
# 修改远程跟踪分支
git branch -u origin/branchwx

# 查找当前分支所跟踪的服务器与分支，从服务器上抓取数据然后尝试合并入那个远程分支
git pull

# 删除远程分支，从服务器上删除branchwx分支
git push origin --delete branchwx

# 查看远程仓库具体信息
git remote shou <remote>	

# 仓库重命名和移除
git remote reanme oldname newname
git remote remove oldname
~~~
### 1.4其他
~~~
# 移除暂存区中的文件使其不再纳入版本管理（连带在工作区中删除指定文件）
git rm <file>
# 移除暂存区中的文件使其不再纳入版本管理（保留磁盘文件）
git rm --cached <file>
# 文件重命名
git mv filenameA filenameB
# 等价于
mv README.md README
git rm README.md
git add README
# 取消暂存
git reset filename
# 取消对文件的修改
git checkout -- filename

# 显示提交日志
git log -p/--patch -4

# 重新提交
git commit --amend

# 列出标签
git tag
git tag -l "v1.8.5*"
~~~

### 1.5git分支

~~~
git分支本质上仅仅是指向提交对象的可变指针
HEAD指向当前所在的分支
HEAD分支随着提交操作自动向前移动

# 列出当前所有分支
git branch

# 创建分支
git branch branchname

# 分支切换
git checkout branchname

# 创建分支并切换过去
git checkout -b <newbranchname>

# 分支合并，先切换到需要合入的分支再合并
git checkout master
git merge subbranch

# 删除分支
git branch -d branchname

# 查看每一个分支的最后一次提交
git branch -v 
~~~








