#include "lcxl_string.h"
// �����locale��stringͷ�ļ���ʹ��setlocale������
#include <locale>
#include <algorithm> 
#include <functional> 
#include <cctype>

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

// trim from start
std::string &ltrim(std::string &_Str) {
	_Str.erase(_Str.begin(), std::find_if(_Str.begin(), _Str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return _Str;
}

std::wstring & ltrim(std::wstring &_Str)
{
	_Str.erase(_Str.begin(), std::find_if(_Str.begin(), _Str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return _Str;
}

// trim from end
std::string &rtrim(std::string &_Str) {
	_Str.erase(std::find_if(_Str.rbegin(), _Str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _Str.end());
	return _Str;
}

std::wstring & rtrim(std::wstring &_Str)
{
	_Str.erase(std::find_if(_Str.rbegin(), _Str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _Str.end());
	return _Str;
}

// trim from both ends
std::string &trim(std::string &_Str) {
	return ltrim(rtrim(_Str));
}

std::wstring & trim(std::wstring &_Str)
{
	return ltrim(rtrim(_Str));
}
