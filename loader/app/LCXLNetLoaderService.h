#pragma once

#include "resource.h"

TCHAR szServiceName[] = _T("LCXLNetLoaderService");  

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);  //����������  
void WINAPI ControlHandler(DWORD dwMsg);            //������ƺ���
BOOL ReporttoSCM(DWORD dwCurrentState);