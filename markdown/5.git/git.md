# git

## 1.获取git仓库

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
# 查看已暂存的修改和为暂存的修改
git status / git status -s/git status --short
# 比较工作目录中当前文件和暂存区域快照之间的差异，就是修改之后还没有暂存起来的变化内容
git diff
# 比对已暂存文件与最后一次提交的文件差异，查看已暂存的将要添加到下次提交里的内容
git diff --staged
git commit -m 'xxxx'
git commit -a -m 'first commit'	
~~~

### 1.3其他

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
~~~







