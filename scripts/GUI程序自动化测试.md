# GUI程序自动化测试

总的思路是使用winafl进行模糊测试，但是在测试GUI程序的时候由于程序不会主动关闭输入文件，导致winafl的单次进行测试迭代的时候用例会timeout，所以借助pywinauto生成模拟运行脚本模拟关闭过程，同时使用pywinauto希望可以触发更多的回调函数增加代码覆盖率。



**Winafl中对于GUI程序fuzz的要求**

```
GUI
1. There is a target function that behaves as explained in "How to select a target function"
		1. Open the input file. This needs to happen withing the target function so that you can read a new input file for each iteration as the input file is rewritten between target function runs).
		2. Parse it (so that you can measure coverage of file parsing)
		3. Close the input file. This is important because if the input file is not closed WinAFL won't be able to rewrite it.
		4. Return normally (So that WinAFL can "catch" this return and redirect execution. "returning" via ExitProcess() and such won't work)
		
2.  The target function is reachable without user interaction
3.  The target function runs and returns without user interaction
```



## Winafl

1. <https://github.com/googleprojectzero/winafl> 源程序

2. 我的电脑上直接运行afl-fuzz.exe**会报`Client library targets an incompatible API version`相关错误**，运到此问题可以用cmd在当前文件夹下执行**make**重新编译生成afl-fuzz.exe，生成的可执行文件在bin32\bin\Release中

   ```shell
   md b32
   cd b32
   cmake .. -DDynamoRIO_DIR=pathtodynamoRIO\cmake
   cmake --build . --config Release
   ```

3. cmd中winafl命令如下形式

```
afl-fuzz [afl options] -- [instrumentation options] -- target_cmd_line
```

​	常用的afl-fuzz选项有：

```
  -i dir        - 测试用例的文件夹 (必要)
  -o dir        - fuzz过程以及输出结果的目录 （必要）
  -D dir        - 包含DynamoRIO二进制程序的文件夹(drrun, drconfig)
  -t msec       - 每一次迭代的超时设置
  -M \\ -S id   - 分布式模式
  -m limit      - 目标进程的内存设置
```

​	常用instrumentation options选项如下：

```
-coverage_module  – fuzzing对象程序会调用到的模块
-target_module    – fuzzing对象模块，也就是-target_offset所在的模块
-target_offset    – fuzzing的偏移地址，也就是会被instrumentation的偏移
-fuzz_iterations  – 再fuzzing对象程序重新启动一次内运行目标函数的最大迭代数
-debug            – debug模式
```



### DynamoRIO

使用winafl前还需要使用DynamoRIO在运行前先进行插桩

 <https://github.com/DynamoRIO/dynamorio/wiki/Downloads>



### IDA

在使用winafl过程中使用target module参数时可以使用ida进行反编译寻找偏移地址



例：寻找main函数的时候一些小经验

VC编译器的版本不同，mainCRTSartup函数也可能会有所不同，IDA中将mainCRTSartup函数命名为___tmainCRTStartup。main函数有三个参数，分别为命令行参数个数、命令行参数信息和环境变量信息。根据main函数调用的特征会将3个参数压入栈内作为函数的参数。所以查找用户入口main()前必然会有3个push指令。



> 一些有价值的参考博客
>
> <https://www.cnblogs.com/Ox9A82/p/5877531.html>
>
> <https://blog.csdn.net/qq_27446553/article/details/52282480>
>
> <https://www.4hou.com/technology/2800.html>
>
> **注：一些播放器软件会自动关闭输入文件可以直接使用winafl进行fuzz**



### 使用流程：

```
E:\winafl\DynamoRIO-Windows-7.1.0-1\bin64\drrun.exe -c winafl.dll -debug -target_module notepad++.exe -fuzz_iterations 10 -nargs 2 -- notepad++.exe in\xxx.xxx
```

```
E:\winafl\winafl-test\b32\bin\Release\afl-fuzz.exe -i in -o out -D E:\winafl\DynamoRIO-Windows-7.1.0-1\bin32 -t 20000+ -- -fuzz_iterations 5000 -target_module notepad++.exe -target_offset 0x103CD0 -nargs 1 -- "notepad++.exe" @@
```



## Pywinauto

<https://pywinauto.readthedocs.io/en/latest/>一个python的第三方模块

pywinauto的安装依赖 **推荐使用pip安装**，python推荐3.5版本以上

- [pyWin32](http://sourceforge.net/projects/pywin32/files/pywin32/Build 220/)
- [comtypes](https://github.com/enthought/comtypes/releases)
- [six](https://pypi.python.org/pypi/six)



#### **流程** 

获取组件 -> 写入json文件 -> 生成测试用例并生成测试脚本

#### 获取组件

* fetchComponents类
  * open_application方法 打开目标程序并返回app对象
  * add_component_to_dict方法 递归获取窗口中的对象加入到dict对象中
  * add_menu_to_dict方法 将菜单栏中的所有对象加入到dict中
  * filter_texteditor_menu方法 删去每个菜单栏对象的无用属性
  * write_into_file方法 将dict写入json文件
  
* genTestSeq类
  * read_component_from_json方法 从文件读取组件列表
  * random_select方法 未完成
  * generate_autorun 未完成
  * 还需要加上一个去重的函数，可以建表做映射记录+使用bitmap压缩 or 哈希记录

* AutoRunScript文件夹

  * notepadRun.py示范的目标运行脚本

* componentList.json

  从notepad++中读取到的所有组件列表



### Notepad++

**目前发现的一个notepad++的一个崩溃是**

在npp32\themes文件夹下放入poc.xml文件

"Settings -> Style Configurator -> Select theme->poc  "

可以触发崩溃，还未用windbg调试



最后还要改写winafl的源码，让winafl开一个进程执行自动运行脚本，同时需要改写输入参数