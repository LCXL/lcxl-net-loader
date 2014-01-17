// LCXLNetLoaderService.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "LCXLNetLoaderService.h"

//ȫ�ֱ���  
SerCore g_SerCore;

#ifdef LCXL_SHADOW_SER_TEST
int _tmain(int argc, _TCHAR* argv[]) 
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	SetErrorMode(SEM_FAILCRITICALERRORS);//ʹ��������쳣ʱ������
	//��ʼ��һ�������  
	g_SerCore.Run();
	return 0;
}
#else

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	SetErrorMode(SEM_FAILCRITICALERRORS);//ʹ��������쳣ʱ������
	//��ʼ��һ�������  
	g_SerCore.Run();
	return 0;
}

#endif

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	g_SerCore.SerMain(argc, argv);
}

void WINAPI ServiceHandler(DWORD dwControl)
{
	g_SerCore.SerHandler(dwControl);
}

void SerCore::SerHandler(DWORD dwControl)
{
	// Handle the requested control code.
	switch (dwControl){
	case SERVICE_CONTROL_STOP:case SERVICE_CONTROL_SHUTDOWN:
		// �رշ���
		OutputDebugStr(_T("����˽��յ��ر�����\n"));
		SetEvent(mExitEvent);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
		// invalid control code
	default:
		// update the service status.
		ReportStatusToSCMgr();
		break;
	}
}

void SerCore::SerRun()
{
	mIOCPMgr = new CIOCPManager();
	mSerList = new CIOCPBaseList(mIOCPMgr);

	DOnIOCPEvent iocp_event(this, reinterpret_cast<EOnIOCPEvent>(&SerCore::IOCPEvent));
	mSerList->SetIOCPEvent(iocp_event);

	mExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	mSockLst = new CSocketLst();
	// ��������
	if (mSockLst->StartListen(mSerList, 9999)) {
		// �ȴ��˳�
		WaitForSingleObject(mExitEvent, INFINITE);
		mSockLst->Close();
	} else {
		OutputDebugStr(_T("��������ʧ�ܣ�\n"));
		delete mSockLst;
	}

	CloseHandle(mExitEvent);
	delete mSerList;
	delete mIOCPMgr;

}

void SerCore::IOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped)
{
	vector<CSocketObj*> *SockList;
	switch (EventType) {
	case ieAddSocket:
		SockList = mSerList->GetSockObjList();
		OutputDebugStr(_T("SerCore::IOCPEvent ieAddSocket %d\n"), SockList->size());
		break;
	case ieDelSocket:
		SockList = mSerList->GetSockObjList();
		OutputDebugStr(_T("SerCore::IOCPEvent ieDelSocket %d\n"), SockList->size());
		break;
	case ieCloseSocket:
		SockList = mSerList->GetSockObjList();
		OutputDebugStr(_T("SerCore::IOCPEvent ieCloseSocket %d\n"), SockList->size());
		break;
	case ieError:
		break;
	case ieRecvPart:
		break;
	case ieRecvAll:
		break;
	case ieRecvFailed:
		break;
	case ieSendPart:
		break;
	case ieSendAll:
		break;
	case ieSendFailed:
		break;
	default:
		break;
	}
}

BOOL SerMgrBase::ReportStatusToSCMgr()
{
	if (mSerStatus.dwCurrentState == SERVICE_START_PENDING){

		mSerStatus.dwControlsAccepted  = 0;
	} else {
		mSerStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if (mSerStatus.dwCurrentState == SERVICE_RUNNING || mSerStatus.dwCurrentState == SERVICE_STOPPED) {

		mSerStatus.dwCheckPoint = 0;
	} else {
		mSerStatus.dwCheckPoint++;
	}
	return SetServiceStatus(mServiceStatusHandle, &mSerStatus);
}

BOOL SerMgrBase::Run()
{
#ifdef LCXL_SHADOW_SER_TEST
	ServiceMain(0, NULL);
	return TRUE;
#else
	return StartServiceCtrlDispatcher(mServiceTableEntry);
#endif
}

void SerMgrBase::SerMain(DWORD dwNumServicesArgs, LPTSTR lpServiceArgVectors[])
{
#ifdef LCXL_SHADOW_SER_TEST 
	SerRun();
#else
	// ע�����
	mServiceStatusHandle = RegisterServiceCtrlHandler(LCXLSHADOW_SER_NAME, &ServiceHandler);
	mSerStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	mSerStatus.dwServiceSpecificExitCode = 0;
	mSerStatus.dwCheckPoint = 1;
	mSerStatus.dwWaitHint = 0;
	mSerStatus.dwWin32ExitCode = 0;
	// ������������
	mSerStatus.dwCurrentState = SERVICE_START_PENDING;
	ReportStatusToSCMgr();
	// ���������ɹ�
	mSerStatus.dwCurrentState = SERVICE_RUNNING;
	ReportStatusToSCMgr();

	SerRun();
	// �������ǰ��״̬��������ƹ�����
	mSerStatus.dwCurrentState = SERVICE_STOP_PENDING;
	ReportStatusToSCMgr();
	mSerStatus.dwCurrentState = SERVICE_STOPPED;
	ReportStatusToSCMgr();
#endif
}

SerMgrBase::SerMgrBase()
{
	mServiceTableEntry[0].lpServiceName = LCXLSHADOW_SER_NAME;
	mServiceTableEntry[0].lpServiceProc = &ServiceMain;
	mServiceTableEntry[1].lpServiceName = NULL;
	mServiceTableEntry[1].lpServiceProc = NULL;
}

SerMgrBase::~SerMgrBase()
{

}
