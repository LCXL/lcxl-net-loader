#ifndef _LCXL_NET_LOADER_SERVICE_H_
#define _LCXL_NET_LOADER_SERVICE_H_

#include "resource.h"
#include "../common/dll_interface.h"
#include "../../component/lcxl_iocp/lcxl_iocp_base.h"

TCHAR LCXLSHADOW_SER_NAME[] = _T("LCXLNetLoaderService");

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);  //����������  
void WINAPI ControlHandler(DWORD dwMsg);            //������ƺ���

/// <summary>
/// ���������
/// </summary>
class SerMgrBase {
private:
	SERVICE_TABLE_ENTRY mServiceTableEntry[2];
	// ������
	SERVICE_STATUS_HANDLE mServiceStatusHandle;
protected:
	SERVICE_STATUS mSerStatus;
	/// <summary>
	/// �������״̬�����������
	/// </summary>
	/// <param name="SerStatus">
	/// ����״̬
	/// </param>
	/// <returns>
	/// �Ƿ�ɹ�
	/// </returns>
	BOOL ReportStatusToSCMgr();
	void SerMain(DWORD dwNumServicesArgs, LPTSTR lpServiceArgVectors[]);
	virtual void SerHandler(DWORD dwControl) = 0;
	virtual void SerRun() = 0;
public:
	SerMgrBase();
	virtual ~SerMgrBase();
	/// <summary>
	/// ��ʼ������
	/// </summary>
	/// <returns>
	/// �Ƿ�ɹ�
	/// </returns>
	virtual BOOL Run();
};

class SerCore : public SerMgrBase{
private:
	//�˳��¼�
	HANDLE mExitEvent;
	CIOCPBaseList *mSerList;
	CIOCPManager *mIOCPMgr;
	CSocketLst *mSockLst;
	void IOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped);
protected:
	virtual void SerHandler(DWORD dwControl);
	virtual void SerRun();
public:
	friend void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
	friend void WINAPI ServiceHandler(DWORD dwControl);
};

#endif