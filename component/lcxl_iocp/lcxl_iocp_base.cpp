#include "lcxl_iocp_base.h"
#include <stdlib.h>
#include <mstcpip.h>

#include <ws2tcpip.h>
#include <mswsock.h>
#include <process.h> 

#pragma comment(lib, "Ws2_32.lib")

static LPFN_ACCEPTEX g_AcceptEx;
static LPFN_GETACCEPTEXSOCKADDRS g_GetAcceptExSockaddrs;

#ifdef _DEBUG
void OutputDebugStr(const TCHAR fmt[], ...)
{
	va_list argptr;
	PTCHAR buf;
	va_start(argptr, fmt);
	int bufsize = _vsntprintf(NULL, 0, fmt, argptr) + 2;
	buf = (PTCHAR)malloc(bufsize*sizeof(TCHAR));
	_vsntprintf(buf, bufsize, fmt, argptr);
	OutputDebugString(buf);
	free(buf);
}
#endif // _DEBUG


/************************************************************************/
/* IOCP�����߳�                                                         */
/************************************************************************/
static unsigned __stdcall IocpWorkThread(void *CompletionPortID)
{
	HANDLE CompletionPort = (HANDLE)CompletionPortID;

	while (TRUE) {
		BOOL FIsSuc;
		PIOCPOverlapped FIocpOverlapped;
		CSocketBase *SockBase;
		CSocketObj *SockObj = NULL;
		CSocketLst *SockLst = NULL;
		BOOL _IsSockObj = FALSE;
		DWORD BytesTransferred;
		INT resuInt;

		FIsSuc = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred,
			(PULONG_PTR)&SockBase, (LPOVERLAPPED*)&FIocpOverlapped, INFINITE);
		if (SockBase != NULL) {
			assert(SockBase == FIocpOverlapped->AssignedSockObj);
			_IsSockObj = SockBase->mSocketType == STObj;
			if (_IsSockObj) {
				SockObj = static_cast<CSocketObj*>(SockBase);
			} else {
				SockLst = static_cast<CSocketLst*>(SockBase);
			}
		} else {
			// IOCP �߳��˳�ָ��
			// �˳�
			OutputDebugStr(_T("����˳�����˳���������һ�߳��˳���\n"));
			// ֪ͨ��һ�������߳��˳�
			PostQueuedCompletionStatus(CompletionPort, 0, 0, NULL);
			break;
		}
		if (FIsSuc) {
			// ������˳��߳���Ϣ�����˳�
			BOOL _NeedContinue = FALSE;
			SOCKET tmpSock;
			PCSocketObj _NewSockObj;

			switch (FIocpOverlapped->OverlappedType) {
			case otRecv:case otSend:
				if (BytesTransferred == 0) {
					assert(FIocpOverlapped == SockObj->GetAssignedOverlapped());
					OutputDebugStr(_T("socket(%d)�ѹر�:%d\n"), SockObj->GetSocket(), WSAGetLastError());
					// ��������
					SockObj->InternalDecRefCount();
					// ����
					_NeedContinue = TRUE;
					break;
				}
				// socket�¼�
				switch (FIocpOverlapped->OverlappedType){
				case otRecv:
					assert(FIocpOverlapped == SockObj->GetAssignedOverlapped());
					// �ƶ���ǰ���ܵ�ָ��
					FIocpOverlapped->RecvDataLen = BytesTransferred;
					FIocpOverlapped->RecvData = SockObj->GetRecvBuf();
					// ��ȡ�¼�ָ��
					// ���ͽ��
					// �����¼�
					try{
						SockObj->GetOwner()->OnIOCPEvent(ieRecvAll, SockObj, FIocpOverlapped);
					}
					catch (...) {
						OutputDebugStr(_T("SockObj->GetOwner()->OnIOCPEvent ieRecvAll throw an exception\n"));
					}
					// Ͷ����һ��WSARecv
					if (!SockObj->WSARecv()) {
						// �������
						OutputDebugStr(_T("WSARecv��������socket=%d:%d\n"), SockObj->GetSocket(), WSAGetLastError());
						// ��������
						SockObj->InternalDecRefCount();
					}
					break;
				case otSend:
					// ��ȡ�¼�ָ��
					// ����ָ�����
					FIocpOverlapped->CurSendData = (PVOID)((PBYTE)FIocpOverlapped->CurSendData + BytesTransferred);
					// �����ȫ��������ɣ��ͷ��ڴ�
					if ((ULONG_PTR)FIocpOverlapped->CurSendData -
						(ULONG_PTR)FIocpOverlapped->SendData == (ULONG_PTR)FIocpOverlapped->SendDataLen) {
						BOOL _NeedDecSockObj;

						// �����¼�
						try{
							SockObj->GetOwner()->OnIOCPEvent(ieSendAll, SockObj, FIocpOverlapped);
						}
						catch (...) {
							OutputDebugStr(_T("SockObj->GetOwner()->OnIOCPEvent ieSendAll throw an exception\n"));
						}
						SockObj->GetOwner()->GetOwner()->DelOverlapped(FIocpOverlapped);
						// ��ȡ�����͵�����
						FIocpOverlapped = NULL;
						SockObj->GetOwner()->Lock();
						assert(SockObj->GetIsSending());
						if (SockObj->GetSendDataQueue().size() > 0) {
							FIocpOverlapped = SockObj->GetSendDataQueue().front();
							SockObj->GetSendDataQueue().pop();
							OutputDebugStr(_T("Socket(%d)ȡ������������\n"), SockObj->GetSocket());
						} else {
							SockObj->SetIsSending(FALSE);
						}
						SockObj->GetOwner()->Unlock();
						// Ĭ�ϼ���Socket����
						_NeedDecSockObj = TRUE;
						if (FIocpOverlapped != NULL) {
							if (!SockObj->WSASend(FIocpOverlapped)) {
								// ����д���
								OutputDebugStr(_T("IocpWorkThread:WSASend����ʧ��(socket=%d):%d\n"), SockObj->GetSocket(), WSAGetLastError());
								// �����¼�
								try{
									SockObj->GetOwner()->OnIOCPEvent(ieSendFailed, SockObj, FIocpOverlapped);
								}
								catch (...) {
									OutputDebugStr(_T("SockObj->GetOwner()->OnIOCPEvent ieSendFailed throw an exception\n"));
								}
								SockObj->GetOwner()->GetOwner()->DelOverlapped(FIocpOverlapped);
							} else {
								// ���ͳɹ�������������
								_NeedDecSockObj = FALSE;
							}
						}
						if (_NeedDecSockObj) {
							// ��������
							SockObj->InternalDecRefCount();
						}
					} else {
						// û��ȫ���������
						FIocpOverlapped->DataBuf.len = FIocpOverlapped->SendDataLen +
							(ULONG)((ULONG_PTR)FIocpOverlapped->SendData -
							(ULONG_PTR)FIocpOverlapped->CurSendData);
						FIocpOverlapped->DataBuf.buf = (CHAR *)FIocpOverlapped->CurSendData;
						try{
							SockObj->GetOwner()->OnIOCPEvent(ieSendPart, SockObj, FIocpOverlapped);
						}
						catch (...) {
							OutputDebugStr(_T("SockObj->GetOwner()->OnIOCPEvent ieSendAll throw an exception\n"));
						}
						// ����Ͷ��WSASend
						if (!SockObj->WSASend(FIocpOverlapped)) {
							// ����д���
							OutputDebugStr(_T("IocpWorkThread:WSASend����ʧ��(socket=%d):%d\n"), SockObj->GetSocket(), WSAGetLastError());
							// �����¼�
							try{
								SockObj->GetOwner()->OnIOCPEvent(ieSendFailed, SockObj, FIocpOverlapped);
							}
							catch (...) {
								OutputDebugStr(_T("SockObj->GetOwner()->OnIOCPEvent ieSendFailed throw an exception\n"));
							}
							SockObj->GetOwner()->GetOwner()->DelOverlapped(FIocpOverlapped);
							// ��������
							SockObj->InternalDecRefCount();
						}
					}
					break;


				default:
					break;
				}
				break;
			case otListen:
				assert(FIocpOverlapped == SockLst->GetAssignedOverlapped());
				/*
				GetAcceptExSockaddrs(SockLst->mLstBuf, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, local, localLen, remote, remoteLen);
				*/
				tmpSock = SockLst->GetSocket();
				// ����������
				resuInt = setsockopt(FIocpOverlapped->AcceptSocket, SOL_SOCKET,
					SO_UPDATE_ACCEPT_CONTEXT, (char *)&tmpSock, sizeof(tmpSock));
				if (resuInt != 0) {
					OutputDebugStr(_T("socket(%d)����setsockoptʧ��:%d\n"),
						FIocpOverlapped->AcceptSocket, WSAGetLastError());
				}
				// ����
				// �����¼������SockObj�����ʧ�ܣ���close֮
				_NewSockObj = NULL;
				// �����µ�SocketObj��
				SockLst->CreateSockObj(_NewSockObj);
				// ���Socket���
				_NewSockObj->mSock = FIocpOverlapped->AcceptSocket;
				// ����Ϊ����socket
				_NewSockObj->mIsSerSocket = TRUE;
				// ��ӵ�Socket�б���
				SockLst->GetOwner()->AddSockBase(_NewSockObj);
				// Ͷ����һ��Accept�˿�
				if (!SockLst->Accept()){
					OutputDebugStr(_T("AcceptEx����ʧ��: %d\n"), WSAGetLastError());
					SockLst->InternalDecRefCount();
				}
				break;
			}
			if (_NeedContinue) {
				continue;
			}
		} else {
			if (FIocpOverlapped != NULL) {
				OutputDebugStr(_T("GetQueuedCompletionStatus����ʧ��(socket=%d): %d\n"),
					SockBase->GetSocket(), GetLastError());
				// �ر�
				if (FIocpOverlapped != SockBase->GetAssignedOverlapped()) {
					// ֻ��otSend��FIocpOverlapped
					assert(FIocpOverlapped->OverlappedType == otSend);
					SockBase->GetOwner()->GetOwner()->DelOverlapped(FIocpOverlapped);
				}
				// ��������
				SockBase->InternalDecRefCount();
			} else {
				OutputDebugStr(_T("GetQueuedCompletionStatus����ʧ��: %d\n"), GetLastError());
			}
		}
	}
	_endthreadex(0);
	return 0;
}


int CSocketBase::InternalIncRefCount(int Count/*=1*/, BOOL UserMode/*=FALSE*/)
{
	int resu;
	mOwner->Lock();
	mRefCount += Count;
	if (UserMode) {
		mUserRefCount += Count;
		resu = mUserRefCount;
	} else {
		resu = mRefCount;
	}
	mOwner->Unlock();
	assert(resu > 0);
	return resu;
}

int CSocketBase::InternalDecRefCount(int Count/*=1*/, BOOL UserMode/*=FALSE*/)
{
	// socket�Ƿ�ر�
	BOOL _IsSocketClosed1;
	BOOL _IsSocketClosed2;
	BOOL _CanFree;
	int resu;
	mOwner->Lock();
	_IsSocketClosed1 = mRefCount == mUserRefCount;
	mRefCount -= Count;
	if (UserMode) {
		mUserRefCount -= Count;
		resu = mUserRefCount;
	} else
	{
		resu = mRefCount;
	}
	_IsSocketClosed2 = mRefCount == mUserRefCount;
	_CanFree = 0 == mRefCount;
	mOwner->Unlock();
	// socket�Ѿ��ر�
	if (!_IsSocketClosed1 && _IsSocketClosed2) {
		// ����close�¼�
		if (this->mSocketType == STObj) {
			mOwner->OnIOCPEvent(ieCloseSocket, static_cast<CSocketObj*>(this), NULL);
		} else {
			mOwner->OnListenEvent(leCloseSockLst, static_cast<CSocketLst*>(this));
		}
	}
	if (_CanFree){
		// �Ƴ����������ͷ�
		mOwner->RemoveSockBase(this);
	}
	return resu;
}

CSocketBase::CSocketBase()
{
	mSock = INVALID_SOCKET;
	// ���ü���Ĭ��Ϊ1
	mRefCount = 0;
	// �û�����Ĭ��Ϊ0
	mUserRefCount = 0;

	mIniteStatus = sisInitializing;
	mOwner = NULL;
	mIOComp = INVALID_HANDLE_VALUE;
	mAssignedOverlapped = NULL;
	mTag = 0;
}

CSocketBase::~CSocketBase()
{
	if (mAssignedOverlapped != NULL) {
		assert(mOwner != NULL);
		assert(mOwner->GetOwner() != NULL);
		mOwner->GetOwner()->DelOverlapped(mAssignedOverlapped);
	}
}

void CSocketBase::Close()
{
	shutdown(mSock, SD_BOTH);
	if (closesocket(mSock) != ERROR_SUCCESS) {
		OutputDebugStr(_T("closesocket failed:%d\n"), WSAGetLastError());
	}
	mSock = INVALID_SOCKET;
}

int CSocketBase::IncRefCount(int Count/*=1*/)
{
	assert(Count > 0);
	return InternalIncRefCount(Count, TRUE);
}

int CSocketBase::DecRefCount(int Count/*=1*/)
{
	assert(Count > 0);
	if (mUserRefCount == 0) {
		throw invalid_argument("IncRefCount function must be called before call this function!");
	}
	return InternalDecRefCount(Count, TRUE);
}


BOOL CSocketLst::Accept()
{
	DWORD BytesReceived;
	BOOL resu;

	assert(mAssignedOverlapped != NULL);
	assert(mAssignedOverlapped->OverlappedType == otListen);
	ZeroMemory(&mAssignedOverlapped->lpOverlapped, sizeof(mAssignedOverlapped->lpOverlapped));
	mAssignedOverlapped->AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
		WSA_FLAG_OVERLAPPED);

	resu = (g_AcceptEx(mSock, mAssignedOverlapped->AcceptSocket, mLstBuf, 0,
		sizeof(sockaddr_storage)+16, sizeof(sockaddr_storage)+16, &BytesReceived,
		&mAssignedOverlapped->lpOverlapped) == TRUE) || (WSAGetLastError() == WSA_IO_PENDING);
	// Ͷ��AcceptEx
	if (!resu) {
		OutputDebugStr(_T("AcceptEx����ʧ��: %d\n"), WSAGetLastError());
		closesocket(mAssignedOverlapped->AcceptSocket);
		mAssignedOverlapped->AcceptSocket = INVALID_SOCKET;
	}
	return resu;
}

BOOL CSocketLst::Init()
{
	// ����������ݵ��ڴ�
	mLstBuf = malloc(mLstBufLen);
	return TRUE;
}

void CSocketLst::CreateSockObj(CSocketObj* &SockObj)
{
	assert(SockObj == NULL);
	SockObj = new CSocketObj;
	SockObj->SetTag(GetTag());
}

CSocketLst::CSocketLst()
{
	mSocketType = STLst;
	mSocketPoolSize = 10;
	mLstBufLen = (sizeof(sockaddr_storage)+16) * 2;

	mPort = 0;
	mLstBuf = NULL;

}

CSocketLst::~CSocketLst()
{
	if (mLstBuf != NULL) {
		free(mLstBuf);
	}
}

void CSocketLst::SetSocketPoolSize(int Value)
{
	if (mIniteStatus == sisInitializing) {
		if (Value > 0) {
			mSocketPoolSize = Value;
		}
	} else {
		throw invalid_argument("SocketPoolSize can't be set after StartListen");
	}
}

BOOL CSocketLst::StartListen(CCustomIOCPBaseList *IOCPList, int Port, u_long InAddr /*= INADDR_ANY*/)
{
	SOCKADDR_IN InternetAddr;
	PSOCKADDR sockaddr;
	int ErrorCode;
	BOOL resu = FALSE;

	mPort = Port;
	mSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mSock == INVALID_SOCKET) {
		ErrorCode = WSAGetLastError();
		OutputDebugStr(_T("WSASocket ����ʧ�ܣ�%d\n"), ErrorCode);
		return resu;
	}
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(InAddr);
	InternetAddr.sin_port = htons(Port);
	sockaddr = (PSOCKADDR)&InternetAddr;
	// �󶨶˿ں�
	if (bind(mSock, sockaddr, sizeof(InternetAddr)) == SOCKET_ERROR) {
		ErrorCode = WSAGetLastError();
		OutputDebugStr(_T("bind ����ʧ�ܣ�%d\n"), ErrorCode);
		closesocket(mSock);
		WSASetLastError(ErrorCode);
		mSock = INVALID_SOCKET;
		return resu;
	}
	// ��ʼ����
	if (listen(mSock, SOMAXCONN) == SOCKET_ERROR) {
		ErrorCode = WSAGetLastError();
		OutputDebugStr(_T("listen ����ʧ�ܣ�%d\n"), ErrorCode);
		closesocket(mSock);
		WSASetLastError(ErrorCode);
		mSock = INVALID_SOCKET;
		return resu;
	}
	mOwner = IOCPList;
	// ��ӵ�SockLst
	resu = IOCPList->AddSockBase(this);
	if (!resu) {
		ErrorCode = WSAGetLastError();
		OutputDebugStr(_T("AddSockBase ����ʧ�ܣ�%d\n"), ErrorCode);
		closesocket(mSock);
		WSASetLastError(ErrorCode);
		mSock = INVALID_SOCKET;
	}
	return resu;
}



RELEASE_INLINE BOOL CSocketObj::WSARecv()
{
	DWORD Flags;

	// ���Overlapped
	ZeroMemory(&mAssignedOverlapped->lpOverlapped, sizeof(mAssignedOverlapped->lpOverlapped));
	// ����OverLap
	mAssignedOverlapped->DataBuf.len = mRecvBufLen;
	mAssignedOverlapped->DataBuf.buf = (CHAR *)mRecvBuf;
	Flags = 0;
	return (::WSARecv(mSock, &mAssignedOverlapped->DataBuf, 1, NULL, &Flags,
		&mAssignedOverlapped->lpOverlapped, NULL) == 0) || (WSAGetLastError() == WSA_IO_PENDING);
}

RELEASE_INLINE BOOL CSocketObj::WSASend(PIOCPOverlapped Overlapped)
{
	// OutputDebugStr(Format('WSASend:Overlapped=%p,Overlapped=%d\n',[Overlapped, Integer(Overlapped.OverlappedType)]));
	// ���Overlapped
	ZeroMemory(&Overlapped->lpOverlapped, sizeof(Overlapped->lpOverlapped));
	assert(Overlapped->OverlappedType == otSend);
	assert((Overlapped->DataBuf.buf != NULL) && (Overlapped->DataBuf.len > 0));
	return (::WSASend(mSock, &Overlapped->DataBuf, 1, NULL, 0,
		&Overlapped->lpOverlapped, NULL) == 0) || (WSAGetLastError() == WSA_IO_PENDING);
}

BOOL CSocketObj::Init()
{
	assert(mRecvBufLen > 0);
	// ����������ݵ��ڴ�
	mRecvBuf = malloc(mRecvBufLen);
	if (mRecvBuf == NULL) {
		return FALSE;
	}
	// ��ʼ��
	//FSendDataQueue: = TList.Create;
	return TRUE;
}

CSocketObj::CSocketObj()
{
	mSocketType = STObj;
	// ���ó�ʼ������Ϊ4096
	mRecvBufLen = 4096;

	mRecvBuf = NULL;
	mIsSerSocket = FALSE;
	mIsSending = FALSE;
}

CSocketObj::~CSocketObj()
{
	while (!mSendDataQueue.empty()) {
		PIOCPOverlapped _IOCPOverlapped;

		_IOCPOverlapped = mSendDataQueue.front();
		mSendDataQueue.pop();
		mOwner->GetOwner()->DelOverlapped(_IOCPOverlapped);
	}
	if (mRecvBuf != NULL) {
		free(mRecvBuf);
	}
}

BOOL CSocketObj::ConnectSer(CCustomIOCPBaseList &IOCPList, LPCTSTR SerAddr, int Port, int IncRefNumber)
{
	BOOL resu = FALSE;
	ADDRINFOT _Hints;
	int _Retval;
	PADDRINFOT _ResultAddInfo;
	PADDRINFOT _NextAddInfo;
	DWORD _AddrStringLen;
	LPTSTR _AddrString;

	assert(mIsSerSocket == FALSE);
	ZeroMemory(&_Hints, sizeof(_Hints));
	_Hints.ai_family = AF_UNSPEC;
	_Hints.ai_socktype = SOCK_STREAM;
	_Hints.ai_protocol = IPPROTO_TCP;
	_Retval = GetAddrInfo(SerAddr, to_tstring(Port).c_str(), &_Hints, &_ResultAddInfo);
	if (_Retval != 0) {
		return FALSE;
	}
	_NextAddInfo = _ResultAddInfo;

	// ���뻺����
	_AddrString = new TCHAR[ADDR_STRING_MAX_LEN];

	while (_NextAddInfo != NULL) {
		_AddrStringLen = ADDR_STRING_MAX_LEN;
		// ��ȡ
#ifdef _DEBUG
		if (WSAAddressToString(_NextAddInfo->ai_addr, (DWORD)_NextAddInfo->ai_addrlen, NULL,
			_AddrString, &_AddrStringLen) == 0) {
			// ��Ϊ��ʵ����,�����_AddrStringLen������ĩβ���ַ�#0������Ҫ��ȥ���#0�ĳ���
			_AddrStringLen--;
			OutputDebugStr(_T("ai_addr:%s,ai_flags:%d,ai_canonname=%s\n"),
				_AddrString, _NextAddInfo->ai_flags, _NextAddInfo->ai_canonname);
		} else {
			OutputDebugStr(_T("WSAAddressToString Error:%d\n"), WSAGetLastError());
		}
#endif
		mSock = WSASocket(_NextAddInfo->ai_family, _NextAddInfo->ai_socktype,
			_NextAddInfo->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (mSock != INVALID_SOCKET) {
			if (connect(mSock, _NextAddInfo->ai_addr, (INT)_NextAddInfo->ai_addrlen) == SOCKET_ERROR) {
				DWORD LastError = WSAGetLastError();

				OutputDebugStr(_T("����%sʧ�ܣ�%d\n"), _AddrString, LastError);

				closesocket(mSock);
				WSASetLastError(LastError);
				mSock = INVALID_SOCKET;
			} else {
				mOwner = &IOCPList;
				// ��������
				IncRefCount(IncRefNumber);
				resu = IOCPList.AddSockBase(this);
				if (!resu) {
					DWORD LastError = WSAGetLastError();
					OutputDebugStr(_T("���%s���б���ʧ�ܣ�%d\n"), _AddrString, LastError);
					closesocket(mSock);
					WSASetLastError(LastError);
					mSock = INVALID_SOCKET;
					// ��������
					DecRefCount(IncRefNumber);
				}
				break;
			}
		}
		_NextAddInfo = _NextAddInfo->ai_next;
	}
	delete[] _AddrString;
	FreeAddrInfo(_ResultAddInfo);
	return resu;
}

BOOL CSocketObj::GetRemoteAddr(tstring &Address, WORD &Port)
{
	SOCKADDR_STORAGE name;
	int namelen;
	char addrbuf[NI_MAXHOST];
	char portbuf[NI_MAXSERV];

	Address.clear();
	Port = 0;

	namelen = sizeof(name);
	if (getpeername(mSock, (PSOCKADDR)&name, &namelen) == 0) {
		if (getnameinfo((PSOCKADDR)&name, namelen, addrbuf, NI_MAXHOST, portbuf, NI_MAXSERV, NI_NUMERICHOST || NI_NUMERICSERV) == 0) {
			Address = string_to_tstring(string(addrbuf));
			Port = stoi(portbuf);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CSocketObj::GetLocalAddr(tstring &Address, WORD &Port)
{
	SOCKADDR_STORAGE name;
	int namelen;
	char addrbuf[NI_MAXHOST];
	char portbuf[NI_MAXSERV];

	Address.clear();
	Port = 0;

	namelen = sizeof(name);
	if (getsockname(mSock, (PSOCKADDR)&name, &namelen) == 0) {
		if (getnameinfo((PSOCKADDR)&name, namelen, addrbuf, NI_MAXHOST, portbuf, NI_MAXSERV, NI_NUMERICHOST || NI_NUMERICSERV) == 0) {
			Address = string_to_tstring(string(addrbuf));
			Port = stoi(portbuf);
		}
		return TRUE;
	}
	return FALSE;
}

RELEASE_INLINE void CSocketObj::SetRecvBufLenBeforeInit(DWORD NewRecvBufLen)
{
	assert(this->mIniteStatus == sisInitializing && NewRecvBufLen > 0);
	if (mRecvBufLen != NewRecvBufLen) {
		mRecvBufLen = NewRecvBufLen;
	}
}

BOOL CSocketObj::SendData(LPVOID Data, DWORD DataLen, BOOL UseGetSendDataFunc /*= FALSE*/)
{
	PIOCPOverlapped FIocpOverlapped;
	PVOID _NewData;
	BOOL _PauseSend;
	BOOL resu;

	if (DataLen == 0) {
		return TRUE;
	}
	// ����������
	InternalIncRefCount();
	_NewData = NULL;
	assert(Data != NULL);
	resu = FALSE;
	FIocpOverlapped = mOwner->GetOwner()->NewOverlapped(this, otSend);
	if (FIocpOverlapped != NULL) {
		// ��䷢�������йص���Ϣ
		if (UseGetSendDataFunc) {
			FIocpOverlapped->SendData = Data;
		} else {
			_NewData = malloc(DataLen);
			CopyMemory(_NewData, Data, DataLen);
			FIocpOverlapped->SendData = _NewData;
		}
		FIocpOverlapped->CurSendData = FIocpOverlapped->SendData;
		FIocpOverlapped->SendDataLen = DataLen;
		FIocpOverlapped->DataBuf.buf = (CHAR *)FIocpOverlapped->CurSendData;
		FIocpOverlapped->DataBuf.len = FIocpOverlapped->SendDataLen;
		mOwner->Lock();
		_PauseSend = mIsSending || (mIniteStatus == sisInitializing);
		// ������������ڷ��͵�
		if (_PauseSend) {
			mSendDataQueue.push(FIocpOverlapped);
			OutputDebugStr(_T("Socket(%d)�еķ������ݼ��뵽�����Ͷ���\n"), mSock);
		} else {
			mIsSending = TRUE;
		}
		mOwner->Unlock();
		if (!(_PauseSend)) {
			// OutputDebugStr(_T("SendData:Overlapped=%p,Overlapped=%d\n"),FIocpOverlapped, Integer(FIocpOverlapped.OverlappedType)));
			// Ͷ��WSASend
			if (!WSASend(FIocpOverlapped)){
				// ����д���
				OutputDebugStr(_T("SendData:WSASend����ʧ��(socket=%d):%d\n"),
					mSock, WSAGetLastError());
				// ɾ����Overlapped
				mOwner->GetOwner()->DelOverlapped(FIocpOverlapped);
				mOwner->Lock();
				mIsSending = FALSE;
				mOwner->Unlock();
			} else {
				resu = TRUE;
			}
		} else {
			// ��ӵ������Ͷ��е����ݲ����������ã������Ҫȡ����ǰ��Ԥ����
			InternalDecRefCount();
			resu = TRUE;
		}
	}
	if (!resu) {
		if (!UseGetSendDataFunc) {
			if (_NewData != NULL) {
				free(_NewData);
			}
		}
		// ��������
		InternalDecRefCount();
	}
	return resu;
}

RELEASE_INLINE LPVOID CSocketObj::GetSendData(DWORD DataLen)
{
	return malloc(DataLen);
}

RELEASE_INLINE void CSocketObj::FreeSendData(LPVOID Data)
{
	free(Data);
}

BOOL CSocketObj::SetKeepAlive(BOOL IsOn, int KeepAliveTime /*= 50000*/, int KeepAliveInterval /*= 30000*/)
{
	struct tcp_keepalive alive_in;
	struct tcp_keepalive alive_out;
	DWORD ulBytesReturn;

	alive_in.keepalivetime = KeepAliveTime; // ��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��
	alive_in.keepaliveinterval = KeepAliveInterval; // ����KeepAlive̽����ʱ����
	alive_in.onoff = (ULONG)IsOn;
	return WSAIoctl(mSock, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out,
		sizeof(alive_out), &ulBytesReturn, NULL, NULL) == 0;
}


RELEASE_INLINE void CCustomIOCPBaseList::Lock()
{
	EnterCriticalSection(&mSockBaseCS);
}

RELEASE_INLINE void CCustomIOCPBaseList::Unlock()
{
	LeaveCriticalSection(&mSockBaseCS);
}

BOOL CCustomIOCPBaseList::AddSockBase(CSocketBase *SockBase)
{
	BOOL _IsLocked;
	BOOL resu;

	assert(SockBase->GetSocket() != INVALID_SOCKET);
	assert(SockBase->GetRefCount() >= 0);
	SockBase->mOwner = this;
	// �������ü���+1�������ü�������Recv������
	SockBase->InternalIncRefCount();
	// ��ʼ��ʼ��Socket
	if (!SockBase->Init()) {
		// ieCloseSocket����û�м��뵽IOCP֮ǰ�����ô���
		SockBase->Close();
		SockBase->InternalDecRefCount();
		return FALSE;
	}
	Lock();
	// List�Ƿ���ס
	_IsLocked = mLockRefNum > 0;
	if (_IsLocked) {
		// ����ס�����ܶ�Socket�б������ӻ�ɾ���������ȼӵ�Socket�����List�С�
		mSockBaseAddList.push(SockBase);
		OutputDebugStr(_T("�б�������Socket(%d)�������Ӷ���\n"), SockBase->GetSocket());
	} else {
		// û�б���ס��ֱ����ӵ�Socket�б���
		mSockBaseList.push_back(SockBase);
		// ��ӵ�Ӱ��List
		if (SockBase->mSocketType == STObj) {
			mSockObjList.push_back(static_cast<CSocketObj*>(SockBase));
		} else {
			mSockLstList.push_back(static_cast<CSocketLst*>(SockBase));
		}
	}
	Unlock();
	if (!_IsLocked) {
		// ���û�б���ס�����ʼ��Socket
		resu = InitSockBase(SockBase);
		if (resu) {

		} else {
			// ��ʼ������
			assert(SockBase->GetRefCount() > 0);
		}
	} else {
		// �������ס���Ƿ���ֵ��Զ��True
		resu = TRUE;
	}
	return resu;
}

BOOL CCustomIOCPBaseList::RemoveSockBase(CSocketBase *SockBase)
{
	BOOL _IsLocked;

	assert(SockBase->GetRefCount() == 0);
	Lock();
	_IsLocked = mLockRefNum > 0;
	if (!_IsLocked) {
		vector<CSocketBase*>::iterator it;
		for (it = mSockBaseList.begin(); it != mSockBaseList.end(); it++) {
			if ((*it) == SockBase) {
				it = mSockBaseList.erase(it);
				break;
			}
		}
		if (SockBase->mSocketType == STObj) {
			vector<CSocketObj*>::iterator it;
			for (it = mSockObjList.begin(); it != mSockObjList.end(); it++) {
				if ((*it) == SockBase) {
					it = mSockObjList.erase(it);
					break;
				}
			}
		} else {
			vector<CSocketLst*>::iterator it;
			for (it = mSockLstList.begin(); it != mSockLstList.end(); it++) {
				if ((*it) == SockBase) {
					it = mSockLstList.erase(it);
					break;
				}
			}
		}
	} else {
		mSockBaseDelList.push(SockBase);
	}
	Unlock();
	if (!_IsLocked) {
		FreeSockBase(SockBase);
	}
	return TRUE;
}

BOOL CCustomIOCPBaseList::InitSockBase(CSocketBase *SockBase)
{
	// ���뵽�����˵���Ѿ���ӵ�socket�б����ˣ�����Ҫ����
	BOOL _IsSockObj;

	_IsSockObj = SockBase->mSocketType == STObj;
	try {
		if (_IsSockObj) {
			OnIOCPEvent(ieAddSocket, static_cast<CSocketObj*>(SockBase), NULL);
		} else {
			OnListenEvent(leAddSockLst, static_cast<CSocketLst*>(SockBase));
		}
	}
	catch (...){

	}
	// ����
	assert(SockBase->GetRefCount() > 0);
	// ��ӵ�Mgr
	if (!IOCPRegSockBase(SockBase)) {
		// ʧ�ܣ�
		// ieCloseSocket���Լ��ֶ�����
		SockBase->Close();
		SockBase->InternalDecRefCount();
		//?
		return TRUE;
	}
	// Result := True;
	// ע�ᵽϵͳ��IOCP�в����ʼ�����
	Lock();
	SockBase->mIniteStatus = sisInitialized;
	Unlock();
	if (_IsSockObj) {
		CSocketObj *_SockObj = static_cast<CSocketObj*>(SockBase);
		// ���Recv��Overlapped
		_SockObj->mAssignedOverlapped = mOwner->NewOverlapped(_SockObj, otRecv);
		// Ͷ��WSARecv
		if (!_SockObj->WSARecv()) {
			// �������
			OutputDebugStr(_T("InitSockObj:WSARecv��������socket=%d:%d\n"), _SockObj->GetSocket(), WSAGetLastError());
			try{
				OnIOCPEvent(ieRecvFailed, _SockObj, _SockObj->mAssignedOverlapped);
			}
			catch (...){

			}
			// ��������
			SockBase->InternalDecRefCount();
		}
	} else {
		CSocketLst *_SockLst = static_cast<CSocketLst*>(SockBase);
		// ���Listen��Overlapped
		_SockLst->mAssignedOverlapped = mOwner->NewOverlapped(_SockLst, otListen);
		// Ͷ��AcceptEx
		if (!_SockLst->Accept()) {
			// ��������
			SockBase->InternalDecRefCount();
		}
	}
	return TRUE;
}

BOOL CCustomIOCPBaseList::FreeSockBase(CSocketBase *SockBase)
{
	BOOL _IsSockObj;

	assert(SockBase->GetRefCount() == 0);
	_IsSockObj = SockBase->mSocketType == STObj;
	if (_IsSockObj) {
		try {
			OnIOCPEvent(ieDelSocket, static_cast<CSocketObj*>(SockBase), NULL);
		}
		catch (...) {

		}
	} else {
		try {
			OnListenEvent(leDelSockLst, static_cast<CSocketLst*>(SockBase));
		}
		catch (...) {

		}
	}
	delete SockBase;
	if (mIsFreeing) {
		CheckCanDestroy();
	}
	return TRUE;
}

RELEASE_INLINE BOOL CCustomIOCPBaseList::IOCPRegSockBase(CSocketBase *SockBase)
{
	BOOL resu;
	// ��IOCP��ע���Socket
	SockBase->mIOComp = CreateIoCompletionPort((HANDLE)SockBase->GetSocket(), mOwner->mCompletionPort,
		(ULONG_PTR)SockBase, 0);
	resu = SockBase->mIOComp != 0;
	if (!resu) {
		OutputDebugStr(_T("Socket(%d)IOCPע��ʧ�ܣ�Error:%d\n"), SockBase->GetSocket(), WSAGetLastError());
	}
	return resu;
}

void CCustomIOCPBaseList::WaitForDestroyEvent()
{
#define EVENT_NUMBER 1
	BOOL _IsEnd;
	HANDLE EventArray[EVENT_NUMBER];

	EventArray[0] = mCanDestroyEvent;
	_IsEnd = FALSE;
	// �ȴ��ͷ�����¼�
	while (!_IsEnd) {
		switch (MsgWaitForMultipleObjects(EVENT_NUMBER, EventArray, FALSE, INFINITE, QS_ALLINPUT)) {
		case WAIT_OBJECT_0:
			// �����ͷ���
			_IsEnd = TRUE;
			break;
		case WAIT_OBJECT_0 + EVENT_NUMBER:
			// ��GUI��Ϣ���ȴ���GUI��Ϣ
			OutputDebugStr(_T("TIOCPBaseList.Destroy:Process GUI Event\n"));
			ProcessMsgEvent();
			break;
		default:
			//�����¼���
			_IsEnd = TRUE;
		}
	}
}

void CCustomIOCPBaseList::CheckCanDestroy()
{
	BOOL _CanDestroy;

	Lock();
	_CanDestroy = (mSockBaseList.size() == 0) && (mSockBaseAddList.size() == 0) &&
		(mSockBaseDelList.size() == 0);
	Unlock();
	if (_CanDestroy) {
		SetEvent(mCanDestroyEvent);
	}
}

void CCustomIOCPBaseList::OnIOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped)
{

}

void CCustomIOCPBaseList::OnListenEvent(ListenEventEnum EventType, CSocketLst *SockLst)
{

}

CCustomIOCPBaseList::CCustomIOCPBaseList(CIOCPManager *AIOCPMgr)
{
	mOwner = AIOCPMgr;
	mLockRefNum = 0;
	mIsFreeing = FALSE;
	mCanDestroyEvent = NULL;

	assert(AIOCPMgr != NULL);
	InitializeCriticalSection(&mSockBaseCS);
	// �������
	mOwner->AddSockList(this);
}

CCustomIOCPBaseList::~CCustomIOCPBaseList()
{
	mCanDestroyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	mIsFreeing = TRUE;
	CloseAllSockBase();
	CheckCanDestroy();
	WaitForDestroyEvent();

	mOwner->RemoveSockList(this);
	CloseHandle(mCanDestroyEvent);
}

void CCustomIOCPBaseList::LockSockList()
{
	Lock();
	assert(mLockRefNum >= 0);
	mLockRefNum++;
	Unlock();
}

void CCustomIOCPBaseList::UnlockSockList()
{
	BOOL _IsEnd;

	do {
		CSocketBase *_SockBase = NULL;
		BOOL isAdd = FALSE;

		Lock();
		assert(mLockRefNum >= 1);
		// �ж��ǲ���ֻ�б��߳��������б�ֻҪ�ж�FLockRefNum�ǲ��Ǵ���1
		_IsEnd = mLockRefNum > 1;
		if (!_IsEnd) {
			// ֻ�б��߳���ס��socket��Ȼ��鿴socketɾ���б��Ƿ�Ϊ��
			if (mSockBaseDelList.size() > 0) {
				vector<CSocketBase*>::iterator it;
				BOOL _IsSockObj;

				// ��Ϊ�գ��ӵ�һ����ʼɾ
				_SockBase = mSockBaseDelList.front();
				mSockBaseDelList.pop();

				for (it = mSockBaseList.begin(); it != mSockBaseList.end(); it++) {
					if ((*it) == _SockBase) {
						mSockBaseList.erase(it);
						break;
					}
				}
				_IsSockObj = _SockBase->mSocketType == STObj;
				if (_IsSockObj) {
					vector<CSocketObj*>::iterator it;
					for (it = mSockObjList.begin(); it != mSockObjList.end(); it++) {
						if ((*it) == _SockBase) {
							mSockObjList.erase(it);
							break;
						}
					}
				} else {
					vector<CSocketLst*>::iterator it;
					for (it = mSockLstList.begin(); it != mSockLstList.end(); it++) {
						if ((*it) == _SockBase) {
							mSockLstList.erase(it);
							break;
						}
					}
				}
				isAdd = FALSE;
			} else {
				// �鿴socket����б��Ƿ�Ϊ��
				if (mSockBaseAddList.size() > 0) {
					BOOL _IsSockObj;


					isAdd = TRUE;
					// �����Ϊ�գ���popһ��sockobj��ӵ��б���
					_SockBase = mSockBaseAddList.front();
					mSockBaseAddList.pop();
					mSockBaseList.push_back(_SockBase);
					_IsSockObj = _SockBase->mSocketType == STObj;
					if (_IsSockObj) {
						mSockObjList.push_back(static_cast<CSocketObj*>(_SockBase));
					} else {
						mSockLstList.push_back(static_cast<CSocketLst*>(_SockBase));
					}
				} else {
					// ��Ϊ�գ����ʾ�Ѿ�������
					_IsEnd = TRUE;
				}
			}
		}
		// ���ûʲô��Ҫ������ˣ������б�����1
		if (_IsEnd) {
			mLockRefNum--;
		}
		Unlock();
		// �鿴sockobj�Ƿ�Ϊ�գ���Ϊ�����ʾ����List�ڼ���ɾ��sock�������sock����
		if (_SockBase != NULL) {
			if (isAdd) {
				// �����sock��������ʼ��sockobj�����ʧ�ܣ����Զ���Free���������ȡ����ֵ
				InitSockBase(_SockBase);
			} else {
				// ��ɾ��sock������ɾ��sockobk
				// InitSockBase(_SockBase);
				// RemoveSockBase(_SockBase);
				assert(_SockBase->GetRefCount() == 0);
				// _SockBase.Free;
				FreeSockBase(_SockBase);
			}
		}
	} while (!_IsEnd);
}

void CCustomIOCPBaseList::ProcessMsgEvent()
{
	MSG Msg;

	while (PeekMessage(&Msg, 0, 0, 0, PM_NOREMOVE)) {
		BOOL Unicode;
		BOOL MsgExists;

		Unicode = (Msg.hwnd == 0) || IsWindowUnicode(Msg.hwnd);
		if (Unicode) {
			MsgExists = PeekMessageW(&Msg, 0, 0, 0, PM_REMOVE);
		} else {
			MsgExists = PeekMessageA(&Msg, 0, 0, 0, PM_REMOVE);
		}
		if (MsgExists) {
			TranslateMessage(&Msg);
			if (Unicode) {
				DispatchMessageW(&Msg);
			} else {
				DispatchMessageA(&Msg);
			}
		}
	}
}

void CCustomIOCPBaseList::CloseAllSockObj()
{
	vector<CSocketObj*>::iterator it;

	LockSockList();
	for (it = mSockObjList.begin(); it != mSockObjList.end(); it++) {
		(*it)->Close();
	}
	UnlockSockList();
}

void CCustomIOCPBaseList::CloseAllSockLst()
{
	vector<CSocketLst*>::iterator it;

	LockSockList();
	for (it = mSockLstList.begin(); it != mSockLstList.end(); it++) {
		(*it)->Close();
	}
	UnlockSockList();
}

void CCustomIOCPBaseList::CloseAllSockBase()
{
	vector<CSocketBase*>::iterator it;

	LockSockList();
	for (it = mSockBaseList.begin(); it != mSockBaseList.end(); it++) {
		(*it)->Close();
	}
	UnlockSockList();
}

#ifdef _UNICODE
#define GetHostName GetHostNameW
#else
#define GetHostName gethostname
#endif // _UNICODE

void CCustomIOCPBaseList::GetLocalAddrs(vector<tstring> &Addrs)
{
	LPTSTR sHostName;
	ADDRINFOT _Hints;
	int _Retval;
	PADDRINFOT _ResultAddInfo;
	PADDRINFOT _NextAddInfo;

	Addrs.clear();
	sHostName = new TCHAR[MAX_PATH];
	if (GetHostName(sHostName, MAX_PATH) == SOCKET_ERROR) {
		return;
	}

	ZeroMemory(&_Hints, sizeof(_Hints));
	_Hints.ai_family = AF_UNSPEC;
	_Hints.ai_socktype = SOCK_STREAM;
	_Hints.ai_protocol = IPPROTO_TCP;
	_Retval = GetAddrInfo(sHostName, NULL, &_Hints, &_ResultAddInfo);
	if (_Retval == 0) {
		DWORD _AddrStringLen;
		LPTSTR _AddrString;

		_NextAddInfo = _ResultAddInfo;
		// ���뻺����
		_AddrString = new TCHAR[ADDR_STRING_MAX_LEN];

		while (_NextAddInfo != NULL) {
			_AddrStringLen = ADDR_STRING_MAX_LEN;
			// ��ȡ

			if (WSAAddressToString(_NextAddInfo->ai_addr, (DWORD)_NextAddInfo->ai_addrlen, NULL,
				_AddrString, &_AddrStringLen) == 0) {
				// ��Ϊ��ʵ����,�����_AddrStringLen������ĩβ���ַ�#0������Ҫ��ȥ���#0�ĳ���
				_AddrStringLen--;
				Addrs.push_back(tstring(_AddrString));
				OutputDebugStr(_T("ai_addr:%s,ai_flags:%d,ai_canonname=%s\n"),
					_AddrString, _NextAddInfo->ai_flags, _NextAddInfo->ai_canonname);
			} else {
				OutputDebugStr(_T("WSAAddressToString Error:%d\n"), WSAGetLastError());
			}

			_NextAddInfo = _NextAddInfo->ai_next;
		}
		delete[] _AddrString;
		FreeAddrInfo(_ResultAddInfo);
	}
	delete[] sHostName;
}

void CIOCPManager::AddSockList(CCustomIOCPBaseList *SockList)
{
	LockSockList();
	mSockList.push_back(SockList);
	UnlockSockList();
}

void CIOCPManager::RemoveSockList(CCustomIOCPBaseList *SockList)
{
	vector<CCustomIOCPBaseList*>::iterator it;
	LockSockList();
	for (it = mSockList.begin(); it != mSockList.end(); it++) {
		if ((*it) == SockList) {
			mSockList.erase(it);
			break;
		}
	}
	UnlockSockList();
}

void CIOCPManager::FreeOverLappedList()
{
	vector<PIOCPOverlapped>::iterator it;
	LockOverLappedList();
	for (it = mOverLappedList.begin(); it != mOverLappedList.end(); it++) {
		assert((*it)->IsUsed == FALSE);
		free(*it);
	}
	mOverLappedList.clear();
	UnlockOverLappedList();
}

void CIOCPManager::DelOverlapped(PIOCPOverlapped UsedOverlapped)
{
	assert(UsedOverlapped != NULL);
	// ����ʹ������ΪFalse
	assert(UsedOverlapped->IsUsed == TRUE);
	switch (UsedOverlapped->OverlappedType) {
	case otSend:
		assert(UsedOverlapped->SendData != NULL);
		if (UsedOverlapped->SendData != NULL) {
			free(UsedOverlapped->SendData);
			UsedOverlapped->SendData = NULL;
		}
		break;
	case otListen:
		if (UsedOverlapped->AcceptSocket != INVALID_SOCKET) {
			closesocket(UsedOverlapped->AcceptSocket);
			UsedOverlapped->AcceptSocket = INVALID_SOCKET;
		}
		break;
	}
	LockOverLappedList();
	// ����ʹ������ΪFalse
	UsedOverlapped->IsUsed = FALSE;
	mOverLappedList.push_back(UsedOverlapped);
	UnlockOverLappedList();
}

PIOCPOverlapped CIOCPManager::NewOverlapped(CSocketBase *SockObj, OverlappedTypeEnum OverlappedType)
{
	PIOCPOverlapped _NewOverLapped;
	LockOverLappedList();

	if (mOverLappedList.size() > 0) {
		_NewOverLapped = mOverLappedList[0];
		mOverLappedList.erase(mOverLappedList.begin());
	} else {
		_NewOverLapped = (PIOCPOverlapped)malloc(sizeof(IOCPOverlapped));
	}
	_NewOverLapped->IsUsed = TRUE;

	UnlockOverLappedList();

	// �Ѿ�ʹ��
	_NewOverLapped->AssignedSockObj = SockObj;
	_NewOverLapped->OverlappedType = OverlappedType;
	// ����
	switch (OverlappedType) {
	case otSend:
		_NewOverLapped->SendData = NULL;
		_NewOverLapped->CurSendData = NULL;
		_NewOverLapped->SendDataLen = 0;
	case otRecv:
		_NewOverLapped->RecvData = NULL;
		_NewOverLapped->RecvDataLen = 0;
	case otListen:
		_NewOverLapped->AcceptSocket = INVALID_SOCKET;
	default:
		break;
	}
	return _NewOverLapped;
}

BOOL CIOCPManager::PostExitStatus()
{
	OutputDebugStr(_T("�����߳��˳����\n"));
	return PostQueuedCompletionStatus(mCompletionPort, 0, 0, NULL);
}

CIOCPManager::CIOCPManager(int IOCPThreadCount /*= 0*/)
{
	SOCKET TmpSock;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes;
	INT i;

	OutputDebugStr(_T("IOCPManager::IOCPManager\n"));
	// ʹ�� 2.2���WS2_32.DLL
	if (WSAStartup(0x0202, &mwsaData) != 0) {
		throw exception("WSAStartup Fails");
	}
	// ��ȡAcceptEx��GetAcceptExSockaddrs�ĺ���ָ��
	TmpSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (TmpSock == INVALID_SOCKET) {
		throw exception("WSASocket Fails");
	}
	if (SOCKET_ERROR == WSAIoctl(TmpSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx), &g_AcceptEx, sizeof(g_AcceptEx), &dwBytes, NULL, NULL)) {
		throw exception("WSAIoctl WSAID_ACCEPTEX Fails");
	}
	if (SOCKET_ERROR == WSAIoctl(TmpSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockaddrs, sizeof(guidGetAcceptExSockaddrs), &g_GetAcceptExSockaddrs, sizeof(g_GetAcceptExSockaddrs), &dwBytes, NULL, NULL)) {
		throw exception("WSAIoctl WSAID_GETACCEPTEXSOCKADDRS Fails");
	}
	closesocket(TmpSock);
	// ��ʼ���ٽ���
	InitializeCriticalSection(&mSockListCS);
	InitializeCriticalSection(&mOverLappedListCS);
	// ��ʼ��IOCP��ɶ˿�
	mCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (IOCPThreadCount <= 0) {
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		IOCPThreadCount = SysInfo.dwNumberOfProcessors + 2;
	}
	mIocpWorkThreadCount = IOCPThreadCount;
	mIocpWorkThreads = new HANDLE[mIocpWorkThreadCount];
	// ����IOCP�����߳�
	for (i = 0; i < mIocpWorkThreadCount; i++) {
		mIocpWorkThreads[i] = (HANDLE)_beginthreadex(NULL, 0, IocpWorkThread, (PVOID)mCompletionPort, 0, NULL);
		if (mIocpWorkThreads[i] == NULL) {
			throw exception("CreateThread FIocpWorkThreads Fails");
		}
	}
}

CIOCPManager::~CIOCPManager()
{
	BOOL Resu;
	LockSockList();
	try {
		if (mSockList.size() > 0) {
			throw exception("SockList����ȫ���ͷ�");
		}
	}
	catch (...) {
		UnlockSockList();
		throw;
	}
	UnlockSockList();
	Resu = PostExitStatus();
	assert(Resu == TRUE);
	OutputDebugStr(_T("�ȴ���ɶ˿ڹ����߳��˳���\n"));
	// �ȴ������߳��˳�
	WaitForMultipleObjects(mIocpWorkThreadCount, mIocpWorkThreads, TRUE, INFINITE);
	delete[] mIocpWorkThreads;
	OutputDebugStr(_T("�ȴ���ɶ˿ھ����\n"));
	// �ر�IOCP���
	CloseHandle(mCompletionPort);
	// �ȴ�SockLst�ͷţ�����Ƚ�����
	// WaitSockLstFree;
	// �ͷ�
	FreeOverLappedList();
	DeleteCriticalSection(&mOverLappedListCS);
	assert(mSockList.size() == 0);
	DeleteCriticalSection(&mSockListCS);
	// �ر�Socket
	WSACleanup();
}

RELEASE_INLINE void CIOCPManager::LockSockList()
{
	EnterCriticalSection(&mSockListCS);
}

RELEASE_INLINE void CIOCPManager::UnlockSockList()
{
	LeaveCriticalSection(&mSockListCS);
}

RELEASE_INLINE void CIOCPManager::LockOverLappedList()
{
	EnterCriticalSection(&mOverLappedListCS);
}

RELEASE_INLINE void CIOCPManager::UnlockOverLappedList()
{
	LeaveCriticalSection(&mOverLappedListCS);
}

void CIOCPBaseList::OnIOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped)
{
	if (mIOCPEvent.IsAvaliable()) {
		TRIGGER_DELEGATE(mIOCPEvent)(EventType, SockObj, Overlapped);
	}
}

void CIOCPBaseList::OnListenEvent(ListenEventEnum EventType, CSocketLst *SockLst)
{
	if (mListenEvent.IsAvaliable()) {
		TRIGGER_DELEGATE(mListenEvent)(EventType, SockLst);
	}
}

CIOCPBaseList::CIOCPBaseList(CIOCPManager *AIOCPMgr) :CCustomIOCPBaseList(AIOCPMgr)
{

}
