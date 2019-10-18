1. refs.py 为IDAPython脚本,功能为查找对函数的引用信息并输出,其中包含了函数调用信息
2. 输出字段说明:

"%08x\t%08x\t%08x\t%50s\t%s\n" % (call_addr, from_addr, to_addr, from_name, to_name)

	call_addr: 引用时的指令位置
	from_addr: 引用时的指令所属的函数地址
	to_addr  : 被引用的函数地址或者其他类型数据地址
	from_name: 引用时的指令所属的函数名称
	to_name  : 被引用的函数名称或者其他类型名称


举例：
MFC示例程序notepad.exe中 CnotepadApp::OnMyFileOpen函数调用了CFileDialog::DoModal函数

.text:00404E30		CnotepadApp__OnMyFileOpen proc near 
.text:00404E8B		call    ds:__imp_?DoModal@CFileDialog@@UAEHXZ ; CFileDialog::DoModal(void)


refs.py输出结果如下（其中数据段地址0x0040a8d8在实际运行过程中将被填充DoModal函数的绝对地址）：

00404e8b	00404e30	0040a8d8	                         CnotepadApp::OnMyFileOpen	__imp_?DoModal@CFileDialog@@UAEHXZ

