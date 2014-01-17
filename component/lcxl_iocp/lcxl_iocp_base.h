#ifndef _LCXL_IOCP_BASE_H_
#define _LCXL_IOCP_BASE_H_

#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <queue>
#include <assert.h>
#include "lcxl_string.h"

#include "lcxl_func_delegate.h"

using namespace std;
//����IP��ַ�ַ�����󳤶�
#define ADDR_STRING_MAX_LEN 1024

#ifdef _DEBUG
#define RELEASE_INLINE
#else
#define RELEASE_INLINE inline
#endif // DEBUG

#ifdef _DEBUG
void OutputDebugStr(const TCHAR fmt[], ...);
#else
#define OutputDebugStr(__fmt, ...)
#endif // _DEBUG



typedef enum _IocpEventEnum {ieAddSocket,

	/// <summary>
	/// socket��IOCP�������Ƴ��¼�
	/// </summary>
	ieDelSocket,

	/// <summary>
	/// socket��ϵͳ�ر��¼������������¼�ʱ���û������ͷŴ�socket�����ã��Ա�iocp�����������socket�����û������ͷ�֮�󣬻ᴥ��ieD
	/// elSocket�¼�
	/// </summary>
	ieCloseSocket,

	ieError,

	/// <summary>
	/// ieRecvPart �ڱ���Ԫ��û��ʵ�֣���չ��
	/// </summary>
	ieRecvPart,

	ieRecvAll,

	ieRecvFailed,

	ieSendPart,

	ieSendAll,

	ieSendFailed} IocpEventEnum, *PIocpEventEnum;
typedef enum _ListenEventEnum {leAddSockLst, leDelSockLst, leCloseSockLst, leListenFailed} ListenEventEnum, *PListenEventEnum;
//ǰ������
//Socket��
class CSocketBase;
// ����socket�࣬Ҫʵ�ֲ�ͬ�Ĺ��ܣ���Ҫ�̳в�ʵ��������
class CSocketLst;
//typedef IOCPOverlapped *PIOCPOverlapped;
class CSocketObj;
class CCustomIOCPBaseList;
class CIOCPBaseList;
class CIOCPManager;

typedef CSocketObj *PCSocketObj;

typedef enum _OverlappedTypeEnum {otRecv, otSend, otListen} OverlappedTypeEnum;
/// <summary>
/// socket���״̬
/// </summary>
typedef enum _SocketInitStatus {
	/// <summary>
	/// socket�����ڳ�ʼ��
	/// </summary>
	sisInitializing,

	/// <summary>
	/// socket���ʼ�����
	/// </summary>
	sisInitialized,

	/// <summary>
	/// socket����������
	/// </summary>
	sisDestroying} SocketInitStatus;

typedef struct _IOCPOverlapped {
	OVERLAPPED lpOverlapped;
	WSABUF DataBuf;
	BOOL IsUsed;
	OverlappedTypeEnum OverlappedType;
	CSocketBase *AssignedSockObj;
	LPVOID GetRecvData() {
		assert(OverlappedType == otRecv);
		return RecvData;
	}
	DWORD GetRecvDataLen() {
		assert(OverlappedType == otRecv);
		return RecvDataLen;
	}
	DWORD GetCurSendDataLen() {
		assert(OverlappedType == otSend);
		return (DWORD)((DWORD_PTR)CurSendData - (DWORD_PTR)SendData);
	}
	LPVOID GetSendData() {
		assert(OverlappedType == otSend);
		return SendData;
	}
	DWORD GetTotalSendDataLen() {
		assert(OverlappedType == otSend);
		return SendDataLen;
	}
	union
	{
		struct {
			LPVOID RecvData;
			DWORD RecvDataLen;
		};
		struct 
		{
			LPVOID SendData;
			LPVOID CurSendData;
			DWORD SendDataLen;
		};
		struct  
		{
			SOCKET AcceptSocket;
		};
	};
} IOCPOverlapped, *PIOCPOverlapped;

/// <summary>
/// Socket����
/// </summary>
typedef enum _SocketType{ STObj, STLst } SocketType;

class CSocketBase {
protected:
	SocketType mSocketType;
	int mRefCount;
	int mUserRefCount;
	SocketInitStatus mIniteStatus;
	SOCKET mSock;
	CCustomIOCPBaseList *mOwner;
	HANDLE mIOComp;
	PIOCPOverlapped mAssignedOverlapped;
	UINT_PTR mTag;
	virtual BOOL Init() = 0;
	int InternalIncRefCount(int Count=1, BOOL UserMode=FALSE);
	int InternalDecRefCount(int Count=1, BOOL UserMode=FALSE);
public:
	CSocketBase();
	virtual ~CSocketBase();
	virtual void Close();
	//Property
	CCustomIOCPBaseList *GetOwner() {
		return mOwner;
	}
	SOCKET GetSocket() {
		return mSock;
	}
	SocketInitStatus GetIniteStatus() {
		return mIniteStatus;
	}
	UINT_PTR GetTag() {
		return mTag;
	}
	void SetTag(UINT_PTR Value) {
		mTag = Value;
	}
	PIOCPOverlapped GetAssignedOverlapped() {
		return mAssignedOverlapped;
	}
	int IncRefCount(int Count=1);
	int DecRefCount(int Count=1);
	/// <summary>
	/// ���ü���
	/// </summary>
	int GetRefCount(){
		return mRefCount;
	}
	friend class CCustomIOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

class CSocketLst: public CSocketBase {
private:
	int mPort;
	PVOID mLstBuf;
	DWORD mLstBufLen;
	int mSocketPoolSize;
protected:
	BOOL Accept();
	virtual BOOL Init();
	virtual void CreateSockObj(CSocketObj* &SockObj);
public:
	CSocketLst();
	virtual ~CSocketLst();
	//Property
	int GetPort() {
		return mPort;
	}
	int GetSocketPoolSize() {
		return mSocketPoolSize;
	}
	void SetSocketPoolSize(int Value);
	BOOL StartListen(CCustomIOCPBaseList *IOCPList, int Port, u_long InAddr = INADDR_ANY);
	friend class CCustomIOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

/// <summary>
/// Socket�࣬һ�����һ���׽���
/// </summary>
class CSocketObj: public CSocketBase {
private:
	DWORD mRecvBufLen;
	LPVOID mRecvBuf;
	BOOL mIsSerSocket;
	BOOL mIsSending;
	queue<PIOCPOverlapped> mSendDataQueue;
	RELEASE_INLINE BOOL WSARecv();
	RELEASE_INLINE BOOL WSASend(PIOCPOverlapped Overlapped);
	void SetIsSending(BOOL value) {
		mIsSending = value;
	}
protected:
	virtual BOOL Init();
public:
	CSocketObj();
	virtual ~CSocketObj();
	/// <summary>
	/// ����ָ���������ַ��֧��IPv6
	/// </summary>
	/// <param name="IOCPList">
	/// Socket�б�
	/// </param>
	/// <param name="SerAddr">
	/// Ҫ���ӵĵ�ַ
	/// </param>
	/// <param name="Port">
	/// Ҫ���ӵĶ˿ں�
	/// </param>
	/// <param name="IncRefNumber">����ɹ��������Ӷ������ü��������ü�����Ҫ����Ա�Լ��ͷţ���Ȼ��һֱռ��</param>
	/// <returns>
	/// �����Ƿ����ӳɹ�
	/// </returns>
	BOOL ConnectSer(CCustomIOCPBaseList &IOCPList, LPCTSTR SerAddr, int Port, int IncRefNumber);
	//Windowsƽ̨��ʹ��WSAAddressToString 
	tstring GetRemoteIP() {
		tstring Address;
		WORD Port;
		GetRemoteAddr(Address, Port);
		return Address;
	}
	WORD GetRemotePort() {
		tstring Address;
		WORD Port;
		GetRemoteAddr(Address, Port);
		return Port;
	}
	BOOL GetRemoteAddr(tstring &Address, WORD &Port);
	tstring GetLocalIP() {
		tstring Address;
		WORD Port;
		GetLocalAddr(Address, Port);
		return Address;
	}
	WORD GetLocalPort() {
		tstring Address;
		WORD Port;
		GetLocalAddr(Address, Port);
		return Port;
	}
	BOOL GetLocalAddr(tstring &Address, WORD &Port);
	LPVOID GetRecvBuf() {
		return mRecvBuf;
	}
	RELEASE_INLINE void SetRecvBufLenBeforeInit(DWORD NewRecvBufLen);
	BOOL SendData(LPVOID Data, DWORD DataLen, BOOL UseGetSendDataFunc = FALSE);
	RELEASE_INLINE LPVOID GetSendData(DWORD DataLen);
	RELEASE_INLINE void FreeSendData(LPVOID Data);
	BOOL SetKeepAlive(BOOL IsOn, int KeepAliveTime = 50000, int KeepAliveInterval = 30000);

	BOOL GetIsSerSocket() {
		return mIsSerSocket;
	}

	BOOL GetIsSending() {
		return mIsSending;
	}
	queue<PIOCPOverlapped> &GetSendDataQueue() {
		return mSendDataQueue;
	}
	friend class CCustomIOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

/// <summary>
/// �洢Socket�б���࣬ǰ��Ϊ��TSocketMgr��
/// </summary>
class CCustomIOCPBaseList{
private:
	HANDLE mCanDestroyEvent;
	BOOL mIsFreeing;
	CIOCPManager *mOwner;
	int mLockRefNum;
	RTL_CRITICAL_SECTION mSockBaseCS;
	vector<CSocketBase*> mSockBaseList;
	queue<CSocketBase*> mSockBaseAddList;
	queue<CSocketBase*> mSockBaseDelList;
	vector<CSocketObj*> mSockObjList;
	vector<CSocketLst*> mSockLstList;
	
protected:
	/// <summary>
	/// ���ֻ�ǵ������ٽ�������Ҫ������Ч�������б�ʹ�� LockSockList
	/// </summary>
	RELEASE_INLINE void Lock();
	/// <summary>
	/// ���sockobj���б��У�����True��ʾ�ɹ�������False��ʾʧ�ܣ�ע������Ҫ����IsFreeingΪTrue�����
	/// </summary>
	RELEASE_INLINE void Unlock();
	/// <summary>
	/// ���sockobj���б��У�����True��ʾ�ɹ�������False��ʾʧ�ܣ�ע������Ҫ����IsFreeingΪTrue�����
	/// </summary>
	BOOL AddSockBase(CSocketBase *SockBase);
	/// <summary>
	/// �Ƴ�sockbase����� �б���������socket������ɾ��������
	/// </summary>
	BOOL RemoveSockBase(CSocketBase *SockBase);
	/// <summary>
	/// ��ʼ��SockBase
	/// </summary>
	BOOL InitSockBase(CSocketBase *SockBase);
	/// <summary>
	/// �ͷ�sockbase���������¼�����ʱsockbase�����Ѿ����б����Ƴ�
	/// </summary>
	BOOL FreeSockBase(CSocketBase *SockBase);
	/// <summary>
	/// ��IOCP��������ע��SockBase
	/// </summary>
	RELEASE_INLINE BOOL IOCPRegSockBase(CSocketBase *SockBase);
	void WaitForDestroyEvent();
	/// <summary>
	/// ����Ƿ�����ͷ�
	/// </summary>
	void CheckCanDestroy();
	/// <summary>
	/// IOCP�¼�
	/// </summary>
	virtual void OnIOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped);
	virtual void OnListenEvent(ListenEventEnum EventType, CSocketLst *SockLst);
public:
	CCustomIOCPBaseList(CIOCPManager *AIOCPMgr);
	virtual ~CCustomIOCPBaseList();
	/// <summary>
	/// �����б�ע����������ܶ��б�������ӣ�ɾ��������һ�ж���SocketMgr��ά��
	/// </summary>
	void LockSockList();

	void UnlockSockList();
	/// <summary>
	/// ������Ϣ���������д��ڵĳ�����ʹ��
	/// </summary>
	void ProcessMsgEvent();
	/// <summary>
	/// �ر����е�Socket
	/// </summary>
	void CloseAllSockObj();
	/// <summary>
	/// �ر����е�Socklst
	/// </summary>
	void CloseAllSockLst();
	/// <summary>
	/// �ر����е�Socket����������socket�ͷǼ���socket
	/// </summary>
	void CloseAllSockBase();
	/// <summary>
	/// �����ӵ����
	/// </summary>
	CIOCPManager *GetOwner() {
		return mOwner;
	}
	vector<CSocketBase*> *GetSockBaseList() {
		return &mSockBaseList;
	}
	vector<CSocketLst*> *GetSockLstList() {
		return &mSockLstList;
	}
	vector<CSocketObj*> *GetSockObjList() {
		return &mSockObjList;
	}
	/// <summary>
	/// ��ȡ����IP��ַ�б�
	/// </summary>
	/// <param name="Addrs">
	/// ��ȡ���ip��ַ������б���
	/// </param>
	static void GetLocalAddrs(vector<tstring> &Addrs);
	//���������
	friend class CSocketBase;       
	friend class CSocketLst;
	friend class CSocketObj;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

class CIOCPManager {
private:
	WSADATA mwsaData;
	vector<CCustomIOCPBaseList*> mSockList;
	RTL_CRITICAL_SECTION mSockListCS;
	vector<PIOCPOverlapped> mOverLappedList;
	RTL_CRITICAL_SECTION mOverLappedListCS;
	HANDLE mCompletionPort;
	INT mIocpWorkThreadCount;
	HANDLE *mIocpWorkThreads;
protected:
	void AddSockList(CCustomIOCPBaseList *SockList);
	void RemoveSockList(CCustomIOCPBaseList *SockList);
	void FreeOverLappedList();
	void DelOverlapped(PIOCPOverlapped UsedOverlapped);
	PIOCPOverlapped NewOverlapped(CSocketBase *SockObj, OverlappedTypeEnum OverlappedType);
	BOOL PostExitStatus();
public:
	CIOCPManager(int IOCPThreadCount = 0);
	virtual ~CIOCPManager();
	RELEASE_INLINE void LockSockList();

	vector<CCustomIOCPBaseList*> &GetSockList() {
		return mSockList;
	}
	RELEASE_INLINE void UnlockSockList();
	RELEASE_INLINE void LockOverLappedList();
	vector<PIOCPOverlapped> &GetOverLappedList() {
		return mOverLappedList;
	}
	RELEASE_INLINE void UnlockOverLappedList();
	//����
	friend class CSocketBase;
	//����
	friend class CSocketObj;

	friend class CCustomIOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};
//ǰ������
class CIOCPBaseList;
// IOCP�¼�
typedef void (CIOCPBaseList::*EOnIOCPBaseEvent)(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped);
// �����¼�
typedef void (CIOCPBaseList::*EOnListenBaseEvent)(ListenEventEnum EventType, CSocketLst *SockLst);

//�����¼�������������
typedef _LCXLFunctionDelegate<CIOCPBaseList, EOnIOCPBaseEvent> DOnIOCPBaseEvent;
typedef _LCXLFunctionDelegate<CIOCPBaseList, EOnListenBaseEvent> DOnListenBaseEvent;

class CIOCPBaseList :public CCustomIOCPBaseList {
private:
	DOnIOCPBaseEvent mIOCPEvent;
	DOnListenBaseEvent mListenEvent;
protected:
	/// <summary>
	/// IOCP�¼�
	/// </summary>
	virtual void OnIOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped);
	virtual void OnListenEvent(ListenEventEnum EventType, CSocketLst *SockLst);
public:
	CIOCPBaseList(CIOCPManager *AIOCPMgr);
	// �ⲿ�ӿ�
	const DOnIOCPBaseEvent &GetIOCPEvent() {
		return mIOCPEvent;
	}
	void SetIOCPEvent(const DOnIOCPBaseEvent &Value) {
		mIOCPEvent = Value;
	}
	const DOnListenBaseEvent &GetListenEvent() {
		return mListenEvent;
	}
	void SetListenEvent(const DOnListenBaseEvent &Value) {
		mListenEvent = Value;
	}
};

#endif