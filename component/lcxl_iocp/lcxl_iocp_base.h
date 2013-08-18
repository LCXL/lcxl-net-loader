#ifndef _LCXL_IOCP_BASE_H_

void inline OutputDebugStr(const TCHAR *DebugInfo) 
{
	OutputDebugString(DebugInfo);
}

#ifdef _DEBUG
#define RELEASE_INLINE
#else
#define RELEASE_INLINE inline
#endif // DEBUG


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
//Socket��
class SocketBase;
// ����socket�࣬Ҫʵ�ֲ�ͬ�Ĺ��ܣ���Ҫ�̳в�ʵ��������
class SocketLst;
//typedef IOCPOverlapped *PIOCPOverlapped;
class SocketObj;
class IOCPBaseList;
class IOCPManager;
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
	SocketBase AssignedSockObj;
	LPVOID GetRecvData();
	DWORD GetRecvDataLen();
	DWORD GetCurSendDataLen();
	LPVOID GetSendData();
	DWORD GetTotalSendDataLen();
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
	IOCPBaseList mOwner;
	HANDLE mIOComp;
	PIOCPOverlapped mAssignedOverlapped;
	UINT_PTR mTag;
	virtual BOOL Init() = 0;
	int InternalIncRefCount(int Count=1, BOOL UserMode=FALSE);
	int InternalDecRefCount(int Count=1, BOOL UserMode=FALSE);
public:
	SocketBase();
	~SocketBase();
	//Property
	IOCPBaseList GetOwner();
	SOCKET GetSocket();
	SocketInitStatus GetIniteStatus();
	UINT_PTR GetTag();
	void SetTag(UINT_PTR Value);
	int IncRefCount(int Count=1);
	int DecRefCount(int Count=1);
};

class SockLst: public SocketBase {
private:
	int mPort;
	LPVOID mLstBuf;
	DWORD mLstBufLen;
	int mSocketPoolSize;
	void SetSocketPoolSize(const int Value);
protected:
	BOOL Accept();
	virtual BOOL Init();
	virtual void CreateSockObj(SocketObj &SockObj);
public:
	SockLst();
	~SockLst();
	//Property
	int GetPort();
	int GetSocketPoolSize();
	void SetSocketPoolSize(int Value);
	BOOL StartListen(IOCPBaseList IOCPList, int Port, u_long InAddr = INADDR_ANY);
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
	vector<PIOCPOverlapped> mSendDataQueue;
	BOOL WSARecv();
	BOOL WSASend(PIOCPOverlapped Overlapped);
protected:
	virtual BOOL Init();
public:
	SocketObj();
	~SocketObj();
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
	BOOL ConnectSer(IOCPBaseList IOCPList, const TCHAR *SerAddr, int Port, int IncRefNumber);
	//Windowsƽ̨��ʹ��WSAAddressToString 
	BOOL GetRemoteAddr(_Inout_ LPTSTR AddressString, DWORD &AddressStringLength, WORD &Port);
	BOOL GetLocalAddr(_Inout_ LPTSTR AddressString, DWORD &AddressStringLength, WORD &Port);
	LPVOID GetRecvBuf();
	void SetRecvBufLenBeforeInit(DWORD NewRecvBufLen);
	BOOL SendData(LPVOID Data, DWORD DataLen, BOOL UseGetSendDataFunc = FALSE);
	LPVOID GetSendData(DWORD DataLen);
	void FreeSendData(LPVOID Data);
	void SetKeepAlive(BOOL IsOn, int KeepAliveTime = 50000, int KeepAliveInterval = 30000);
	BOOL IsSerSocket();
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
	vector<SocketBase*> mSockBaseAddList;
	vector<SocketBase*> mSockBaseDelLis;
	vector<SocketObj*> mSockObjList;
	vector<SockLst*> mSockLstList;
	
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
	virtual void OnIOCPEvent(IocpEventEnum EventType, SocketObj SockObj, PIOCPOverlapped Overlapped);
	virtual void OnListenEvent(ListenEventEnum EventType, SocketLst SockLst);
public:
	IOCPBaseList();
	~IOCPBaseList();
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
	/// �ر����е�Socket����������socket�ͷǼ���socket
	/// </summary>
	void CloseAllSockLst();

	/// <summary>
	/// �����ӵ����
	/// </summary>
	IOCPManager *GetOwner();
	vector<SocketBase*> *GetSockBaseList();
	vector<SockLst*> *GetSockLstList();
	vector<SocketObj*> *GetSockObjList();
	/// <summary>
	/// ��ȡ����IP��ַ�б�
	/// </summary>
	/// <param name="Addrs">
	/// ��ȡ���ip��ַ������б���
	/// </param>
	static void GetLocalAddrs(vector<std::string> &Addrs);
};

class IOCPManager {
private:
	WSADATA mwsaData;
	vector<IOCPBaseList*> mSockList;
	RTL_CRITICAL_SECTION mSockListCS;
	vector<PIOCPOverlapped> mOverLappedList;
	RTL_CRITICAL_SECTION mOverLappedListCS;
	HANDLE mCompletionPort;
	vector<HANDLE> mIocpWorkThreads;
protected:
	void AddSockList(IOCPBaseList SockList);
	void RemoveSockList(IOCPBaseList SockList);
	void FreeOverLappedList();
	void DelOverlapped(PIOCPOverlapped UsedOverlapped);
	PIOCPOverlapped NewOverlapped(SocketBase *SockObj,OverlappedTypeEnum OverlappedType);
	BOOL PostExitStatus();
public:
	IOCPManager();
	~IOCPManager();
	RELEASE_INLINE void LockSockList();

	vector<IOCPBaseList*> *GetSockList();
	RELEASE_INLINE void UnlockSockList();
	RELEASE_INLINE void LockOverLappedList();
	vector<PIOCPOverlapped> *GetOverLappedList;
	RELEASE_INLINE void UnlockOverLappedList();
};

#endif