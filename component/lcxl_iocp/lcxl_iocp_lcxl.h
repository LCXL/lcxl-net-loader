#ifndef _LCXL_IOCP_LCXL_H_
#define _LCXL_IOCP_LCXL_H_

#include "lcxl_iocp_base.h"

typedef struct _SendDataRec {
private:
	ULONG mTotalLen;
	PVOID mTotalData;
	PVOID mData;
	ULONG mDataLen;
public:
	PVOID GetData() {
		return mData;
	};
	ULONG GetDataLen() {
		return mDataLen;
	};
	/// <summary>
	/// ������������ת��Ϊ����¼�����ݽṹ
	/// </summary>
	BOOL Assign(PVOID _TotalData, ULONG _TotalLen);
public:
	friend class CLLSockObj;
} SendDataRec, *PSendDataRec;

class CLLSockLst :public CSocketLst {
protected:
	virtual void CreateSockObj(CSocketObj* &SockObj);// ����
public:
	virtual ~CLLSockLst() {

	}
};

///	<summary>
///	  LCXLЭ���socket��
///	</summary>
class CLLSockObj : public CSocketObj {
private:
	PVOID mBuf;
	ULONG mCurDataLen;
	ULONG mBufLen;
	/// <summary>
	/// ���յ�������
	/// </summary>
	PVOID mRecvData;
	ULONG mRecvDataLen;
	/// <summary>
	/// �Ƿ����һ������������
	/// </summary>
	BOOL mIsRecvAll;
protected:
	// ��ʼ��
	virtual BOOL Init();
public:
	virtual PVOID GetRecvData();
	virtual long GetRecvDataLen();
protected:
	PVOID GetRecvBuf() {
		return mBuf;
	};
public:
	// ����
	virtual ~CLLSockObj();
	// SendData֮ǰ����
	BOOL SendData(const SendDataRec &ASendDataRec);
	BOOL SendData(PVOID Data, ULONG DataLen);
	// ��ȡ�������ݵ�ָ��
	void GetSendData(ULONG DataLen, SendDataRec &ASendDataRec);
	// ֻ��û�е���SendData��ʱ��ſ����ͷţ�����SendData֮�󽫻��Զ��ͷš�
	void FreeSendData(const SendDataRec &ASendDataRec);
	BOOL GetIsRecvAll() {
		return mIsRecvAll;
	}
public:
	friend class CCustomIOCPLCXLList;
};

class CCustomIOCPLCXLList : public CCustomIOCPBaseList {
private:
	virtual void OnIOCPEvent(IocpEventEnum EventType, CSocketObj *SockObj, PIOCPOverlapped Overlapped);
protected:
	virtual void OnIOCPEvent(IocpEventEnum EventType, CLLSockObj *SockObj, PIOCPOverlapped Overlapped);
};

class CIOCPLCXLList;

// IOCP�¼�
typedef void (CIOCPLCXLList::*EOnIOCPLCXLEvent)(IocpEventEnum EventType, CLLSockObj *SockObj, PIOCPOverlapped Overlapped);
//�����¼�������������
typedef _LCXLFunctionDelegate<CIOCPLCXLList, EOnIOCPLCXLEvent> DOnIOCPLCXLEvent;
// IOCP�����¼�
typedef void (CIOCPLCXLList::*EOnListenLCXLEvent)(ListenEventEnum EventType, CLLSockLst *SockLst);
//�����¼�������������
typedef _LCXLFunctionDelegate<CIOCPLCXLList, EOnListenLCXLEvent> DOnListenLCXLEvent;


// LCXLЭ��ʵ����
class CIOCPLCXLList : public CCustomIOCPBaseList{
private:
	DOnIOCPLCXLEvent mIOCPEvent;
	DOnListenLCXLEvent mListenEvent;
protected:
	virtual void OnIOCPEvent(IocpEventEnum EventType, CLLSockObj *SockObj, PIOCPOverlapped Overlapped);
	// �����¼�
	virtual void OnListenEvent(ListenEventEnum EventType, CSocketLst *SockLst);
public:
	DOnIOCPLCXLEvent GetIOCPEvent() {
		return mIOCPEvent;
	};
	void SetIOCPEvent(const DOnIOCPLCXLEvent &Value) {
		mIOCPEvent = Value;
	};
	DOnListenLCXLEvent GetListenEvent() {
		return mListenEvent;
	};
	void SetListenEvent(const DOnListenLCXLEvent &Value) {
		mListenEvent = Value;
	};
};

#endif // !_LCXL_IOCP_LCXL_H_
