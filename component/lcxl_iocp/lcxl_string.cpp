#include "lcxl_string.h"
// �����locale��stringͷ�ļ���ʹ��setlocale������
#include <locale.h>

std::wstring string_to_wstring(const std::string &str)
{
	// stringתwstring
	size_t len = str.size() * 2;// Ԥ���ֽ���
	setlocale(LC_CTYPE, "");     //������ô˺���
	wchar_t *p = new wchar_t[len];// ����һ���ڴ���ת������ַ���
	mbstowcs(p, str.c_str(), len);// ת��
	std::wstring str1(p);
	delete[] p;// �ͷ�������ڴ�
	return str1;
}

std::string wstring_to_string(const std::wstring &str)
{
	// wstringתstring
	size_t len = str.size() * 4;
	setlocale(LC_CTYPE, "");
	char *p = new char[len];
	wcstombs(p, str.c_str(), len);
	std::string str1(p);
	delete[] p;
	return str1;
}
