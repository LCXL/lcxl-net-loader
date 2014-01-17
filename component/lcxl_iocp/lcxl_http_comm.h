#ifndef _LCXL_HTTP_COMM_H_
#define _LCXL_HTTP_COMM_H_
#include <string>
#include <list>
struct _URLRec;
typedef struct _URLRec  URLRec, *PURLRec;

URLRec DecodeURL(const std::string &url);
std::string EncodeURL(const URLRec &url_rec, bool IsPostMethod);

struct _URLRec {
	// Э�����ƣ�����ֻ֧��HTTP
	std::string Scheme;
	// ��www.baidu.com
	std::string Host;
	// �˿ںţ�Ĭ��Ϊ80
	int Port;
	// ��ѯ·������/sss/sss/ssd.asp
	std::string Query;
	// ��������sss=ssd&sdfs=sdr����POST�����У���Ϊ��
	std::string Fragment;
public:
	_URLRec();
public:
	void SetQueryAndFragment(const std::string &Value);
	std::string GetQueryAndFragment();
public:
	_URLRec & operator = (const std::string &Source){
		*this = DecodeURL(Source);
		return *this;
	}
	std::string ToString(bool IsPostMethod = false) {
		return EncodeURL(*this, IsPostMethod);
	}
};

typedef struct _HeadRec {
	std::string Key;
	std::string Value;
} HeadRec, *PHeadRec;

class CHeadList :public std::list<HeadRec> {


	///	<summary>
	///	  ���浽�ַ�����
	///	</summary>
	///	<param name="Separator">
	///	  �ָ�����Ĭ��Ϊ": "��ð�żӿո�
	///	</param>
	///	<param name="LineBreak">
	///	  ���з���Ĭ��Ϊ#13#10
	///	</param>
	///	<returns>
	///	  ��ʽ���õ�����/��Ӧ�ַ����б��ַ���ĩβ�Ի��з���β
	///	</returns>
	std::string SaveToString(const std::string Separator, const std::string LineBreak);


	///	<summary>
	///	  ���ַ�����������/��Ӧͷ
	///	</summary>
	///	<param name="AStr">
	///	  �ַ�����ĩβ��Ҫ��LinkBreak��β
	///	</param>
	///	<param name="Separator">
	///	  �ָ�����Ϊ�˱�֤�������ԣ��˴�Ӧ��ֻ����":"
	///	</param>
	///	<param name="LineBreak">
	///	  ���з���һ����#13#10
	///	</param>
	bool LoadFromString(const std::string AStr, const std::string Separator, const std::string LineBreak);

	std::string GetHeadItems(const std::string Index);
	void SetHeadItems(const std::string Index, const std::string Value);
};
#endif