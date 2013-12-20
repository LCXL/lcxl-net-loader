#ifndef _LCXL_IOCP_BASE_H_
#define _LCXL_IOCP_BASE_H_

#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <queue>
#include <tchar.h>
#include <assert.h>

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

#define tstring basic_string<TCHAR>

tstring inttostr(int value);
tstring int64tostr(INT64 value);

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
class SocketBase;
// ����socket�࣬Ҫʵ�ֲ�ͬ�Ĺ��ܣ���Ҫ�̳в�ʵ��������
class SocketLst;
//typedef IOCPOverlapped *PIOCPOverlapped;
class SocketObj;
class IOCPBaseList;
class IOCPBase2List;
class IOCPManager;

typedef SocketObj *PSocketObj;

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
	SocketBase *AssignedSockObj;
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

class SocketBase {
protected:
	int mRefCount;
	int mUserRefCount;
	SocketInitStatus mIniteStatus;
	SOCKET mSock;
	IOCPBaseList *mOwner;
	HANDLE mIOComp;
	PIOCPOverlapped mAssignedOverlapped;
	UINT_PTR mTag;
	virtual BOOL Init() = 0;
	int InternalIncRefCount(int Count=1, BOOL UserMode=FALSE);
	int InternalDecRefCount(int Count=1, BOOL UserMode=FALSE);
public:
	SocketBase();
	virtual ~SocketBase();
	virtual void Close();
	//Property
	IOCPBaseList *GetOwner() {
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
	friend class IOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

class SocketLst: public SocketBase {
private:
	int mPort;
	PVOID mLstBuf;
	DWORD mLstBufLen;
	int mSocketPoolSize;
protected:
	BOOL Accept();
	virtual BOOL Init();
	virtual void CreateSockObj(PSocketObj &SockObj);
public:
	SocketLst();
	virtual ~SocketLst();
	//Property
	int GetPort() {
		return mPort;
	}
	int GetSocketPoolSize() {
		return mSocketPoolSize;
	}
	void SetSocketPoolSize(int Value);
	BOOL StartListen(IOCPBaseList &IOCPList, int Port, u_long InAddr = INADDR_ANY);
	friend class IOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

/// <summary>
/// Socket�࣬һ�����һ���׽���
/// </summary>
class SocketObj: public SocketBase {
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
	SocketObj();
	virtual ~SocketObj();
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
	BOOL ConnectSer(IOCPBaseList &IOCPList, LPCTSTR SerAddr, int Port, int IncRefNumber);
	//Windowsƽ̨��ʹ��WSAAddressToString 
	string GetRemoteIP() {
		string Address;
		WORD Port;
		GetRemoteAddr(Address, Port);
		return Address;
	}
	WORD GetRemotePort() {
		string Address;
		WORD Port;
		GetRemoteAddr(Address, Port);
		return Port;
	}
	BOOL GetRemoteAddr(string &Address, WORD &Port);
	string GetLocalIP() {
		string Address;
		WORD Port;
		GetLocalAddr(Address, Port);
		return Address;
	}
	WORD GetLocalPort() {
		string Address;
		WORD Port;
		GetLocalAddr(Address, Port);
		return Port;
	}
	BOOL GetLocalAddr(string &Address, WORD &Port);
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
	friend class IOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

/// <summary>
/// �洢Socket�б���࣬ǰ��Ϊ��TSocketMgr��
/// </summary>
class IOCPBaseList{
private:
	HANDLE mCanDestroyEvent;
	BOOL mIsFreeing;
	IOCPManager *mOwner;
	int mLockRefNum;
	RTL_CRITICAL_SECTION mSockBaseCS;
	vector<SocketBase*> mSockBaseList;
	queue<SocketBase*> mSockBaseAddList;
	queue<SocketBase*> mSockBaseDelList;
	vector<SocketObj*> mSockObjList;
	vector<SocketLst*> mSockLstList;
	
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
	BOOL AddSockBase(SocketBase *SockBase);
	/// <summary>
	/// �Ƴ�sockbase����� �б���������socket������ɾ��������
	/// </summary>
	BOOL RemoveSockBase(SocketBase *SockBase);
	/// <summary>
	/// ��ʼ��SockBase
	/// </summary>
	BOOL InitSockBase(SocketBase *SockBase);
	/// <summary>
	/// �ͷ�sockbase���������¼�����ʱsockbase�����Ѿ����б����Ƴ�
	/// </summary>
	BOOL FreeSockBase(SocketBase *SockBase);
	/// <summary>
	/// ��IOCP��������ע��SockBase
	/// </summary>
	RELEASE_INLINE BOOL IOCPRegSockBase(SocketBase *SockBase);
	void WaitForDestroyEvent();
	/// <summary>
	/// ����Ƿ�����ͷ�
	/// </summary>
	void CheckCanDestroy();
	/// <summary>
	/// IOCP�¼�
	/// </summary>
	virtual void OnIOCPEvent(IocpEventEnum EventType, SocketObj *SockObj, PIOCPOverlapped Overlapped);
	virtual void OnListenEvent(ListenEventEnum EventType, SocketLst *SockLst);
public:
	IOCPBaseList(IOCPManager *AIOCPMgr);
	virtual ~IOCPBaseList();
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
	IOCPManager *GetOwner() {
		return mOwner;
	}
	vector<SocketBase*> &GetSockBaseList() {
		return mSockBaseList;
	}
	vector<SocketLst*> &GetSockLstList() {
		return mSockLstList;
	}
	vector<SocketObj*> &GetSockObjList() {
		return mSockObjList;
	}
	/// <summary>
	/// ��ȡ����IP��ַ�б�
	/// </summary>
	/// <param name="Addrs">
	/// ��ȡ���ip��ַ������б���
	/// </param>
	static void GetLocalAddrs(vector<tstring> &Addrs);
	//���������
	friend class SocketBase;       
	friend class SocketLst;
	friend class SocketObj;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

class IOCPManager {
private:
	WSADATA mwsaData;
	vector<IOCPBaseList*> mSockList;
	RTL_CRITICAL_SECTION mSockListCS;
	vector<PIOCPOverlapped> mOverLappedList;
	RTL_CRITICAL_SECTION mOverLappedListCS;
	HANDLE mCompletionPort;
	INT mIocpWorkThreadCount;
	HANDLE *mIocpWorkThreads;
protected:
	void AddSockList(IOCPBaseList *SockList);
	void RemoveSockList(IOCPBaseList *SockList);
	void FreeOverLappedList();
	void DelOverlapped(PIOCPOverlapped UsedOverlapped);
	PIOCPOverlapped NewOverlapped(SocketBase *SockObj, OverlappedTypeEnum OverlappedType);
	BOOL PostExitStatus();
public:
	IOCPManager(int IOCPThreadCount = 0);
	virtual ~IOCPManager();
	RELEASE_INLINE void LockSockList();

	vector<IOCPBaseList*> &GetSockList() {
		return mSockList;
	}
	RELEASE_INLINE void UnlockSockList();
	RELEASE_INLINE void LockOverLappedList();
	vector<PIOCPOverlapped> &GetOverLappedList() {
		return mOverLappedList;
	}
	RELEASE_INLINE void UnlockOverLappedList();
	//����
	friend class SocketBase;
	//����
	friend class SocketObj;

	friend class IOCPBaseList;
	friend unsigned __stdcall IocpWorkThread(void *CompletionPortID);
};

// IOCP�¼�
typedef void (IOCPBase2List::*EOnIOCPEvent)(IocpEventEnum EventType, SocketObj *SockObj, PIOCPOverlapped Overlapped);
// �����¼�
typedef void (IOCPBase2List::*EOnListenEvent)(ListenEventEnum EventType, SocketLst *SockLst);

//ǰ������
class IOCPBase2List;
//�����¼�������������
typedef _LCXLFunctionDelegate<IOCPBase2List, EOnIOCPEvent> DOnIOCPEvent;
typedef _LCXLFunctionDelegate<IOCPBase2List, EOnListenEvent> DOnListenEvent;

class IOCPBase2List :public IOCPBaseList {
private:
	DOnIOCPEvent mIOCPEvent;
	DOnListenEvent mOnListenEvent;
protected:
	/// <summary>
	/// IOCP�¼�
	/// </summary>
	virtual void OnIOCPEvent(IocpEventEnum EventType, SocketObj *SockObj, PIOCPOverlapped Overlapped);
	virtual void OnListenEvent(ListenEventEnum EventType, SocketLst *SockLst);
public:
	IOCPBase2List(IOCPManager *AIOCPMgr);
	// �ⲿ�ӿ�
	const DOnIOCPEvent &GetIOCPEvent() {
		return mIOCPEvent;
	}
	void SetIOCPEvent(const DOnIOCPEvent &Value) {
		mIOCPEvent = Value;
	}
	const DOnListenEvent &GetListenEvent() {
		return mOnListenEvent;
	}
	void SetListenEvent(const DOnListenEvent &Value) {
		mOnListenEvent = Value;
	}
};

#endif