#ifndef _LCXL_NET_LOADER_SERVICE_H_
#define _LCXL_NET_LOADER_SERVICE_H_

#include "resource.h"

TCHAR szServiceName[] = _T("LCXLNetServerService");  

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);  //����������  
void WINAPI ControlHandler(DWORD dwMsg);            //������ƺ���
BOOL ReporttoSCM(DWORD dwCurrentState);
int InitService();

#endif