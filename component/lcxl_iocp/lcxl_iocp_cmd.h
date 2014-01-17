#ifndef _LCXL_IOCP_CMD_H_
#define _LCXL_IOCP_CMD_H_

#include "lcxl_iocp_lcxl.h"

typedef struct _CMDDataRec {
private:
	ULONG mTotalLen;
	PVOID mTotalData;
	PVOID mData;
	ULONG mDataLen;
public:
	WORD GetCMD();
	void SetCMD(const WORD Value);
	PVOID GetData() {
		return mData;
	}
	ULONG GetDataLen() {
		return mDataLen;
	}
	BOOL Assign(PVOID _TotalData, ULONG _TotalLen);
public:
	friend class CCmdSockObj;
} CMDDataRec, *PCMDDataRec;

class CCmdSockLst :public CLLSockLst {
protected:
	virtual void CreateSockObj(CSocketObj* &SockObj);// ����
public:
	virtual ~CCmdSockLst() {

	}
};

///	<summary>
///	  ���������ͨѶЭ��Socket��ʵ��
///	</summary>
class CCmdSockObj :public CLLSockObj {
public:
	///	<remarks>
	///	  SendData֮ǰҪ����
	///	</remarks>
	BOOL SendData(const CMDDataRec ASendDataRec);

	///	<remarks>
	///	  SendData֮ǰҪ����
	///	</remarks>
	BOOL SendData(WORD CMD, PVOID Data, ULONG DataLen);

	///	<remarks>
	///	  SendData֮ǰҪ����
	///	</remarks>
	BOOL SendData(WORD CMD, PVOID Data[], ULONG DataLen[], INT DataCount);

	///	<summary>
	///	  ��ȡ�������ݵ�ָ��
	///	</summary>
	void GetSendData(ULONG DataLen, CMDDataRec &ASendDataRec);

	///	<summary>
	///	  ֻ��û�е���SendData��ʱ��ſ����ͷţ�����SendData֮�󽫻��Զ��ͷš�
	///	</summary>
	///	<param name="SendDataRec">
	///	  Ҫ�ͷŵ�����
	///	</param>
	void FreeSendData(const CMDDataRec &ASendDataRec);
	inline static void GetSendDataFromOverlapped(PIOCPOverlapped Overlapped, CMDDataRec &ASendDataRec);
};

class CIOCPCMDList;

// IOCP�¼�
typedef void (CIOCPCMDList::*EOnIOCPCMDEvent)(IocpEventEnum EventType, CCmdSockObj *SockObj, PIOCPOverlapped Overlapped);
//�����¼�������������
typedef _LCXLFunctionDelegate<CIOCPCMDList, EOnIOCPCMDEvent> DOnIOCPCMDEvent;
// IOCP�����¼�
typedef void (CIOCPCMDList::*EOnListenCMDEvent)(ListenEventEnum EventType, CCmdSockLst *SockLst);
//�����¼�������������
typedef _LCXLFunctionDelegate<CIOCPCMDList, EOnListenCMDEvent> DOnListenCMDEvent;

///	<summary>
///	  ���������ͨѶЭ��Socket���б��ʵ��
///	</summary>
class CIOCPCMDList :public CCustomIOCPLCXLList{
private:
	DOnIOCPCMDEvent mIOCPEvent;
	DOnListenCMDEvent mListenEvent;
protected:
	/// <summary>
	/// ������¼�
	/// </summary>
	virtual void OnIOCPEvent(IocpEventEnum EventType, CLLSockObj *SockObj, PIOCPOverlapped Overlapped);
	// �����¼�
	virtual void OnListenEvent(ListenEventEnum EventType, CSocketLst *SockLst);
public:
	// �ⲿ�ӿ�
	DOnIOCPCMDEvent GetIOCPEvent() {
		return mIOCPEvent;
	}
	void SetIOCPEvent(const DOnIOCPCMDEvent &Value) {
		mIOCPEvent = Value;
	}
	DOnListenCMDEvent GetListenEvent() {
		return mListenEvent;
	}
	void SetListenEvent(const DOnListenCMDEvent &Value) {
		mListenEvent = Value;
	}
};

#endif