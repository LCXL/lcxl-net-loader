// LCXLNetLoaderService.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "LCXLNetLoaderService.h"

//ȫ�ֱ���  
SERVICE_STATUS         MyServiceStatus;  
SERVICE_STATUS_HANDLE  MyServiceStatusHandle;  

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	//��ʼ��һ�������  
	SERVICE_TABLE_ENTRY ServiceTable[] =  
	{  
		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },  
		{ NULL, NULL } //���ɱ�����һ������Ƿ������ͷ������������ NULL ָ�룬����������ΪNULL  
	};  
	if(!StartServiceCtrlDispatcher(ServiceTable))  // ��������Ŀ��Ʒ��ɻ��߳�  
	{  
		OutputDebugString(_T("���ɻ���������ʧ�ܣ�"));  
	} 
	return 0;
}


void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)   
{  
	int error;  

	//ָ�������������䵱ǰ״̬  
	MyServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;  //��������  
	MyServiceStatus.dwCurrentState = SERVICE_START_PENDING;     //ָ������ĵ�ǰ״̬  
	MyServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;   //�����Ա��ʾ��Щ����ִͨ�����ǿɽ��ܵ�  
	MyServiceStatus.dwWin32ExitCode = 0;                        //��������������ֹ���񲢱����˳�ϸ��ʱ�����á�  
	MyServiceStatus.dwServiceSpecificExitCode = 0;              //��ʼ������ʱ�����˳�����ˣ����ǵ�ֵΪ0  
	MyServiceStatus.dwCheckPoint = 0;                           //�������Ա����  
	MyServiceStatus.dwWaitHint = 0;                             //һ������ر����Ľ���  

	//ע��������  
	MyServiceStatusHandle = RegisterServiceCtrlHandler(szServiceName, ControlHandler);   

	if (NULL == MyServiceStatusHandle)  
	{  
		//ע��ʧ�ܾͷ���  
		OutputDebugString(TEXT("ע�����ʧ�ܣ�"));  
		return;   
	}  

	error = InitService(); //��ʼ������  

	if (error)   
	{  
		// ��ʼ��ʧ�ܣ���ֹ����  
		MyServiceStatus.dwWin32ExitCode = -1; 
		 //�� SCM ��������״̬  
		ReporttoSCM(SERVICE_STOPPED);
		return; // �˳� ServiceMain  
	}  

	// �����ʼ���ɹ����� SCM ��������״̬     
	ReporttoSCM(SERVICE_RUNNING);
	while (MyServiceStatus.dwCurrentState == SERVICE_RUNNING)  
	{  
		//�������Ҫʵ�ֵĹ��ܺ���  

		MessageBeep(0);  
		Sleep(3000);  
	}  

	return;   

}   

int InitService()  
{  
	//��ȡϵͳĿ¼��ַ��ʧ�ܾͷ���-1  
	
	return 0;  
}  

void WINAPI ControlHandler(DWORD dwMsg)   
{   
	switch(dwMsg)   
	{  
	case SERVICE_CONTROL_STOP:   
		//��Ӧֹͣ�������  
		MyServiceStatus.dwWin32ExitCode = 0;
		ReporttoSCM(SERVICE_STOPPED);
		return;   

	default:  
		break;  
	}   
	//�� SCM ��������״̬  
	SetServiceStatus(MyServiceStatusHandle, &MyServiceStatus);  
	return;   
}  

BOOL ReporttoSCM( DWORD dwCurrentState )
{
	MyServiceStatus.dwCurrentState = dwCurrentState;   
	return SetServiceStatus(MyServiceStatusHandle, &MyServiceStatus);  
}
