#ifndef _LCXL_STRING_H_
#define _LCXL_STRING_H_

#include <string>
// �����locale��stringͷ�ļ���ʹ��setlocale������
std::wstring StringToWstring(const std::string &str);
std::string WstringToString(const std::wstring &str);

#endif