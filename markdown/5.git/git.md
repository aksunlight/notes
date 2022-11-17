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
~~~







