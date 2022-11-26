# Lightpad，一个实验性的轻量命令行编辑器。

## 支持的设备

Lightpad 可以在 Linux 发行版上运行（MacOS 应该也可以，未测试）。

Windows 暂时无法使用 Lightpad。

## 编译及运行

您可以简单地运行以下命令来编译。

```bash
make build
make debug # 调试的场合
```

然后，输入以下命令即可打开编辑器。

```bash
./lightpad
```

Lightpad 也支持在参数中指定单个或多个文件，比如：

```bash
./lightpad foo.cpp bar.js
```

## 基本操作

进入 Lightpad 以后，您可以看到没有任何页面，此时也不能使用任何快捷键（部分命令除外）。

您可以使用以下命令打开内嵌帮助。

```bash
:help
```

您可以使用以下命令打开一个新页面：

```bash
:new # 打开一个无标题页面。
:new [filename] # 打开一个新文件。filename可以是存在的，也可以是不存在的。
```

在新打开的页面中，您可以使用**方向键**或者**WSAD**来进行上下左右光标移动。

但您可能已经发现，您仍然无法输入文字，这是因为您仍处于**普通**模式（界面的左下角是**NORMAL**）。这时，您可以进入**插入模式**来编辑文字。

请在需要的地方按下**I 键**来进入插入模式。这时，界面的左下角将变为**INSERT**，您不再可以使用**WSAD 键**进行移动，而**方向键**移动仍然可以使用。

在完成编辑后，可以按**Esc 键**退出插入模式。

若您想要保存您的修改，可以使用以下命令保存文件：

```bash
:w # 保存到当前的文件。当文件未命名时无法这样保存。
:w [filename] # 另存为名为filename的文件。如果正在编辑一个文件，则当前页面会转为编辑另存为的文件，您将需要重新打开原文件。
```

然后，您可以用这个命令关闭文件：

```bash
:q # 关闭页面。如果未保存，则无法关闭页面。在没有页面时，退出Lightpad。
:q! # 放弃修改并关闭页面（不推荐）。在没有页面时，退出Lightpad。
```

## 高级操作

### 多页面

相信您已经注意到，Lightpad 下方有 **\[0/0\]** 的标识。当您打开文件后，这个标识变为了 **\[1/1\]**。

实际上，这是 Lightpad 的 **多页面**。

斜杠左边代表**当前的页面位置**，而右边则代表**总共的页面数**。您可以按下**Z 键**来切换到上一个页面，**X 键**来切换到下一个页面。

注意：部分文件操作可能无法在未打开任何页面的情况下使用。

### 选择模式

当您需要选择并删除一段文本时，可以选择按**V 键**进入**选择模式**，此时界面左下角会变为**SELECT**。

您可以用**WSAD 键**或者**方向键**来进行选择。

当您选好了之后，可以按**Backspace 键**删除选中的文字。

当然，您也可以按**Esc 键**取消选择。

您可以使用 **find** 命令查找一段文本在文件中最先出现的位置（早期功能，在开发后期可能被删除）。

```bash
:find [text] # 寻找指定的文字，允许空格。
```

### 语法高亮（实验性功能）

Lightpad 带有内嵌的 C++ 和 Javascript 语法高亮（tomorrow-night-bright theme）。

一般来讲，Lightpad 会自动根据文件后缀名推测需要使用的语法高亮，但您也可以自己使用 **lang** 命令选择语法高亮：

```bash
:lang [cpp | js | plain] # 选择指定的语法高亮。"plain"代表不使用语法高亮。
```

### 版本

使用 **version** 命令可以查看当前 Lightpad 的版本号。

```bash
:version
```