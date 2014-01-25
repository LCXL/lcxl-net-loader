#ifndef _LCXL_SERVICE_H_
#define _LCXL_SERVICE_H_
#include <Windows.h>
#include "../../component/lcxl_iocp/lcxl_string.h"
/// <summary>
/// ����������
/// </summary>
class CServiceBase {
private:
	SERVICE_TABLE_ENTRY m_ServiceTableEntry[2];
	// ������
	SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
	//��������
	std::tstring m_SerivceName;
	//�Ƿ������Run����
	BOOL m_IsCallRunFunc;
private:
	static CServiceBase* m_GlobalService;
	static void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
	static DWORD WINAPI HandlerEx(_In_  DWORD dwControl, _In_  DWORD dwEventType, _In_  LPVOID lpEventData, _In_  LPVOID lpContext);
protected:
	SERVICE_STATUS m_SerStatus;
	void SerMain(DWORD dwNumServicesArgs, LPTSTR lpServiceArgVectors[]);
	virtual void SerHandler(DWORD dwControl) = 0;
	//����������гɹ�������Ҫִ��SetCurrentState(SERVICE_RUNNING);
	virtual void SerRun() = 0;
public:
	CServiceBase();
	virtual ~CServiceBase();
	/// <summary>
	/// ��ʼ������
	/// </summary>
	/// <returns>
	/// �Ƿ�ɹ�
	/// </returns>
	virtual BOOL Run();
public:
	std::tstring GetSerivceName();
	void SetServiceName(std::tstring szServiceName);

	DWORD GetCurrentState();
	//���õ�ǰ����״̬����������������
	void SetCurrentState(DWORD dwCurrentState);

	DWORD GetServiceType();
	void SetServiceType(DWORD dwServiceType);
	DWORD GetServiceSpecificExitCode();
	void SetServiceSpecificExitCode(DWORD dwServiceSpecificExitCode);
	DWORD GetWaitHint();
	void SetWaitHint(DWORD dwWaitHint);
	DWORD GetWin32ExitCode();
	void SetWin32ExitCode(DWORD dwWin32ExitCode);
};

#endif