// LCXLNetLoaderService.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "LCXLNetLoaderService.h"
#include "resource.h"
#include "../common/dll_interface.h"

TCHAR LCXLSHADOW_SER_NAME[] = _T("LCXLNetLoaderService");


#ifdef LCXL_SHADOW_SER_TEST

class CTestNetLoaderSer : public CNetLoaderSer {
public:
	virtual BOOL Run() {
		SerRun();
		return TRUE;
	}
};
//ȫ�ֱ���  
CTestNetLoaderSer g_NetLoadSer;

int _tmain(int argc, _TCHAR* argv[]) 
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	SetErrorMode(SEM_FAILCRITICALERRORS);//ʹ��������쳣ʱ������
	//��ʼ��һ�������  
	g_NetLoadSer.SetServiceName(LCXLSHADOW_SER_NAME);
	g_NetLoadSer.SetListenPort(9999);
	g_NetLoadSer.Run();
	return 0;
}
#else

//ȫ�ֱ���  
CNetLoaderSer g_NetLoadSer;

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
	g_NetLoadSer.SetServiceName(LCXLSHADOW_SER_NAME);
	g_NetLoadSer.SetListenPort(9999);
	//��ʼ��һ�������  
	g_NetLoadSer.Run();
	return 0;
}

#endif

void CNetLoaderSer::IOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped)
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
