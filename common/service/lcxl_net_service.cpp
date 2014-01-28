#include "lcxl_net_service.h"

DWORD CNetServiceBase::SerHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData)
{
	// Handle the requested control code.
	switch (dwControl){
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		// �رշ���
		OutputDebugStr(_T("����˽��յ��ر�����\n"));
		SetExitEvent();
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
		SetCurrentState(GetCurrentState());
		return ERROR_CALL_NOT_IMPLEMENTED;
		break;
	}
	return NO_ERROR;
}

void CNetServiceBase::SerRun()
{
	if (m_ListenPort == 0) {
		throw std::exception("port must be set");
	}
	mIOCPMgr = new CIOCPManager();
	mSerList = new CIOCPBaseList(mIOCPMgr);

	DOnIOCPBaseEvent iocp_event(this, reinterpret_cast<EOnIOCPBaseEvent>(&CNetServiceBase::IOCPEvent));
	mSerList->SetIOCPEvent(iocp_event);

	mExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	mSockLst = new CSocketLst();
	// ��������
	if (mSockLst->StartListen(mSerList, m_ListenPort)) {
		//
		SetCurrentState(SERVICE_RUNNING);
		// �ȴ��˳�
		WaitForSingleObject(mExitEvent, INFINITE);
		mSockLst->Close();
	} else {
		OutputDebugStr(_T("��������ʧ�ܣ�\n"));
		delete mSockLst;
		mSockLst = NULL;
	}

	CloseHandle(mExitEvent);
	delete mSerList;
	delete mIOCPMgr;

	mSerList = NULL;
	mIOCPMgr = NULL;
}

void CNetServiceBase::SetExitEvent()
{
	SetEvent(mExitEvent);
}

CNetServiceBase::~CNetServiceBase()
{

}

int CNetServiceBase::GetListenPort()
{
	return m_ListenPort;
}

void CNetServiceBase::SetListenPort(int Port)
{
	m_ListenPort = Port;
}

CNetServiceBase::CNetServiceBase()
{
	m_ListenPort = 0;
}
