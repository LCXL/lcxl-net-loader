// LCXLNetLoaderService.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "LCXLNetLoaderService.h"

//ȫ�ֱ���  
SERVICE_STATUS         g_ser_status;  
SERVICE_STATUS_HANDLE  g_ser_ctrl_handler;  

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
	g_ser_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;  //��������  
	g_ser_status.dwCurrentState = SERVICE_START_PENDING;     //ָ������ĵ�ǰ״̬  
	g_ser_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;   //�����Ա��ʾ��Щ����ִͨ�����ǿɽ��ܵ�  
	g_ser_status.dwWin32ExitCode = 0;                        //��������������ֹ���񲢱����˳�ϸ��ʱ�����á�  
	g_ser_status.dwServiceSpecificExitCode = 0;              //��ʼ������ʱ�����˳�����ˣ����ǵ�ֵΪ0  
	g_ser_status.dwCheckPoint = 0;                           //�������Ա����  
	g_ser_status.dwWaitHint = 0;                             //һ������ر����Ľ���  

	//ע��������  
	g_ser_ctrl_handler = RegisterServiceCtrlHandler(szServiceName, ControlHandler);   

	if (NULL == g_ser_ctrl_handler)  
	{  
		//ע��ʧ�ܾͷ���  
		OutputDebugString(TEXT("ע�����ʧ�ܣ�"));  
		return;   
	}  

	error = InitService(); //��ʼ������  

	if (error)   
	{  
		// ��ʼ��ʧ�ܣ���ֹ����  
		g_ser_status.dwWin32ExitCode = -1; 
		 //�� SCM ��������״̬  
		ReporttoSCM(SERVICE_STOPPED);
		return; // �˳� ServiceMain  
	}  

	// �����ʼ���ɹ����� SCM ��������״̬     
	ReporttoSCM(SERVICE_RUNNING);
	while (g_ser_status.dwCurrentState == SERVICE_RUNNING)  
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
		g_ser_status.dwWin32ExitCode = 0;
		ReporttoSCM(SERVICE_STOPPED);
		return;   

	default:  
		break;  
	}   
	//�� SCM ��������״̬  
	SetServiceStatus(g_ser_ctrl_handler, &g_ser_status);  
	return;   
}  

BOOL ReporttoSCM( DWORD dwCurrentState )
{
	g_ser_status.dwCurrentState = dwCurrentState;   
	return SetServiceStatus(g_ser_ctrl_handler, &g_ser_status);  
}
