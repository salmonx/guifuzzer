# 文件说明
- application目录-Qt示例项目“记事本”
- applicaton\windeploy目录-手工创建的Qt发布版目录，其中的application.exe可以直接运行
- applicaton\windeploy\poc.txt-手工创建的POC
文件，运行记事本，打开文件，选择poc.txt 程序崩溃退出


# 编译环境
- 操作系统： Windows 10 x64
- Visual Studio版本：VS2012
- QT IDE： Qt Creater 4.9
- Qt 版本： 5.2.0 （选择该版本是因为Qt和VS版本需要对应，本机已存在VS2012）

- 下载链接：
[http://iso.mirrors.ustc.edu.cn/qtproject/archive/qt/5.2/5.2.0/qt-windows-opensource-5.2.0-msvc2012-x86-offline.exe](https://note.youdao.com/)
- 编译程序为32位GUI程序

# 测试环境
- Windows 7 x64

# 编译过程

1. 使用Qt Creater打开示例项目
	> \Qt5.2.0\5.2.0\msvc2012\examples\widgets\mainwindows\application

2. 默认构建项目会失败，需要将构建配置中的“Shadow build”选项取消勾选
    （bug： [https://lists.qt-project.org/pipermail/qt-creator-old/2010-February/006004.html](https://note.youdao.com/)）

3. 添加自定义逻辑。mainwindow.cpp文件中MainWindow::loadFile函数逻辑默认为打开文件并显示，添加逻辑为判断打开的文件前八个字节是否为“password”，如果是，调用“abort”函数触发崩溃推测，如果不是，正常显示文件内容。修改后的代码为：

	```
void MainWindow::loadFile(const QString &fileName)
//! [42] //! [43]
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();

    char * str = (char *)malloc(fileContent.length());

    strcpy(str, fileContent.toStdString().c_str());

    if (strlen(str) >= 8)
        if (str[0] == 'p')
            if (str[1] == 'a')
                if (str[2] == 's')
                    if (str[3] == 's')
                        if (str[4] =='w')
                            if (str[5] == 'o')
                                if (str[6] == 'r')
                                    if (str[7] == 'd'){
                                        printf("Right password!\n");
                                        abort();
                                    }

    in.seek(0);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}
	```

4. 构建成功后，可以在IDE中成功运行，单独运行程序可能失败，提示：

	> 由于找不到 Qt5Widgets.dll，无法继续执行代码。重新安装程序可能会解决此问题。 

	> 由于找不到 Qt5Core.dll，无法继续执行代码。重新安装程序可能会解决此问题。 

	解决办法为将所需Qt库目录加入系统环境变量中
	> \Qt5.2.0\5.2.0\msvc2012\bin

5. 部署Qt程序，即打包Qt程序，使其可以在没有Qt、Visual Studio环境上运行。新建文件夹“windeploy”， 拷贝“\Qt5.2.0\5.2.0\msvc2012\bin\windeployqt.exe”和Release目录下的application.exe到该文件夹，并在命令行窗口执行

	```
	windeployqt.exe application.exe
	```

6. 拷贝打包目录到测试环境，运行程序，提示缺少“msvcp110.dll”和“msvcr110.dll”错误，需将其拷贝到程序运行目录进行修复，打包项目已修复。
	> C:\Windows\SysWOW64\msvcp110.dll
	
	> C:\Windows\SysWOW64\msvcr110.dll

7. 检测运行是否正常，运行示例程序，打开普通文件正常显示，打开poc.txt文件，程序崩溃退出。
