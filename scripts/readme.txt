1. refs.py ΪIDAPython�ű�,����Ϊ���ҶԺ�����������Ϣ�����,���а����˺���������Ϣ
2. ����ֶ�˵��:

"%08x\t%08x\t%08x\t%50s\t%s\n" % (call_addr, from_addr, to_addr, from_name, to_name)

	call_addr: ����ʱ��ָ��λ��
	from_addr: ����ʱ��ָ�������ĺ�����ַ
	to_addr  : �����õĺ�����ַ���������������ݵ�ַ
	from_name: ����ʱ��ָ�������ĺ�������
	to_name  : �����õĺ������ƻ���������������


������
MFCʾ������notepad.exe�� CnotepadApp::OnMyFileOpen����������CFileDialog::DoModal����

.text:00404E30		CnotepadApp__OnMyFileOpen proc near 
.text:00404E8B		call    ds:__imp_?DoModal@CFileDialog@@UAEHXZ ; CFileDialog::DoModal(void)


refs.py���������£��������ݶε�ַ0x0040a8d8��ʵ�����й����н������DoModal�����ľ��Ե�ַ����

00404e8b	00404e30	0040a8d8	                         CnotepadApp::OnMyFileOpen	__imp_?DoModal@CFileDialog@@UAEHXZ

