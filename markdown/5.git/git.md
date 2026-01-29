# git

## 1.git基本操作

### 1.1从本地初始化仓库

~~~
cd /home/user/my_project
git init

git config --global user.name "[name]"
git config --global user.email "[email address]"
启用有帮助的彩色命令行输出
git config --global color.ui auto

git add *.cpp
git add ./src/
git add .	# 添加新文件和编辑过的文件但不包括删除的文件
git add -u	# 添加编辑过或者删除的文件

git commit -m 'initial my project'(-s选项可以用来添加"Signed-off-by"声明)
~~~

### 1.2克隆现有仓库

~~~
git clone <url>
git clone <url> <set repository name>
echo '# test line' >> CONTRIBUTING.md
注意：如果你使用clone命令克隆了一个仓库，命令会自动将其添加为远程仓库并默认以origin为简写。默认情况下，git clone 命令会自动设置本地 master 分支跟踪克隆的远程仓库的 master 分支（或其它名字的默认分支）。

# 查看分支状态，包括未暂存的修改/未跟踪的文件等
git status / git status -s / git status --short
# 比较工作目录中当前文件和暂存区域快照之间的差异，就是修改之后还没有暂存起来的变化内容
git diff
# 比对已暂存文件与最后一次提交的文件差异，就是查看已暂存的将要添加到下次提交里的内容
git diff --staged
# 显示某个文件在某两次提交之间的差异
git diff <提交1/哈希值1>..<提交2/哈希值2> -- filename
git diff abc123..def456 -- example.txt

git commit -m 'xxxx'(-s选项可以用来添加"Signed-off-by"声明)
git commit -a -m 'first commit'(-s选项可以用来添加"Signed-off-by"声明)
~~~

### 1.3和远程仓库交互
~~~
# 查看远程仓库，该命令列出远程仓库服务器的简写
cd projectpath
git remote / git remote -v

# 添加远程仓库服务器，同时指定一个方便使用的简写
git remote add <shortname> <url>

# 从远程仓库中抓取数据，它不会自动合并或修改当前工作
git fetch <remote>
# 从远程仓库中抓取数据，并将branchwx分支合入当前分支
git fetch origin
git merge origin/branchwx
# 查找当前分支所跟踪的服务器与分支，从服务器上抓取数据然后尝试合并入本地分支
git pull

# 推送数据到远程仓库
git push <remote> <branch>
# 推送本地的branchwx分支来更新远程仓库上的branchwx分支
git push origin branchwx / git push origin branchwx:branchwx
# 将本地的branchwx分支推送到远程仓库上的awesomebranch分支
git push origin branchwx:awesomebranch
# 在远程仓库创建同名分支，将本地分支推送上去并跟踪它
git push --set-upstream origin branchwx / git push -u origin branchwx

# 或者在本地创建一个新的分支，并将其建立在远程跟踪分支之上
git checkout -b branchwx origin/branchwx / git checkout --track origin/branchwx
# 修改远程跟踪分支
git branch -u origin/branchwx

# 创建分支
git branch branchname
# 删除分支(不能在要删除的分支上操作)
git branch -d branchname
# 删除远程分支，从服务器上删除branchwx分支
git push origin --delete branchwx

# 查看远程仓库具体信息
git remote show <remote>	

# 仓库重命名和移除
git remote reanme oldname newname
git remote remove oldname

# 打标签
git add [file_path]
git commit -m "xxxx"
git tag v1.4(轻量便签，推荐使用) / git tag -a v1.4 -m "my version 1.4"
git push origin master(默认情况下标签不会随着此命令一起被推送到远程仓库)
git push origin v1.4 / git push origin --tags(推送某个标签或者推送全部标签)

# 查看标签
git tag
git show v1.4

# 删除标签
git tag -d v1.4
git push origin --delete v1.4

# 获取远程版本
git fetch origin tag v1.3
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
# 清空暂存器
git reset .
# 取消对文件的修改
git checkout -- filename
# 取消所有对文件的修改
git checkout -- .

# 显示最近四次的提交内容
git log -p/--patch -4
# 显示文件的提交内容
git log -p/--patch filename
# 显示某次具体的提交
git show <commit_hash>

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

# 创建空白分支
git checkout -orphan emptybranch
git rm -rf .

# 分支切换
git checkout branchname

# 创建分支并切换过去
git checkout -b <newbranchname>

# 分支合并，先切换到需要合入的分支再合并
git checkout master
git merge subbranch

# 删除分支
git branch -d branchname

# 删除远程分支，从服务器上删除branchwx分支
git push origin --delete branchwx

# 查看每一个分支的最后一次提交
git branch -v 
~~~

## 撤销上一次提交

```
git reset HEAD~
```

撤销最新一次的提交，在本地保存更改

```
git reset [commit]
```

撤销所有 `[commit]` 后的的提交，在本地保存更改

```
git reset --hard [commit]
```

放弃所有历史，改回指定提交。

### 1.6.gitignore

```
# 忽略所有的 .a 文件
*.a

# 但跟踪所有的 lib.a，即便你在前面忽略了 .a 文件
!lib.a

# 忽略所有名字叫做test的目录及其所有内容
test/

# 试图在一个被忽略的目录内排除一个文件是行不通的
!test/example.md

# 只忽略根目录下的 TODO 文件，而不忽略 subdir/TODO
/TODO

# 忽略任何目录下名为 build 的文件夹
build/

# 忽略 doc/notes.txt，但不忽略 doc/server/arch.txt
doc/*.txt

# 忽略 doc/ 目录及其所有子目录下的 .pdf 文件
doc/**/*.pdf
```

### 1.7.将本地仓库上传到服务器

```
# 在服务器创建仓库
cd /path/
git init --bare xxx.git
--bare表示建立的是空的仓库，仓库地址为user@ip:/path/xxx.git

# 将本地的仓库上传
git remote add origin usr@ip:/path/xxx.git
添加一个新的远程仓库，给这个远程仓库定名为origin，远程仓库的URL地址为usr@ip:/path/xxx.git
或
git remote set-url origin usr@ip:/path/xxx.git
修改远程仓库origin，远程仓库的URL修改为usr@ip:/path/xxx.git

git push origin master
上传本地分支到远程仓库
```

