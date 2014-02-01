#ifndef _LCXL_SERVER_H_
#define _LCXL_SERVER_H_
/*
author:
LCX
abstract:
���������ͷ�ļ�
*/
#include "../../common/driver/lcxl_lock_list.h"
#define TAG_SERVER      'SERV'

//��������Ϣ�б���
typedef struct _SERVER_INFO_LIST_ENTRY
{
	//�б���
	LIST_ENTRY				list_entry;
	//���ü���
	volatile LONG  			ref_count;
	//��
	KSPIN_LOCK				lock;
	//������״̬
	LCXL_SERVER_INFO		info;
	//����������״̬
	LCXL_SERVER_PERFORMANCE	performance;
} SERVER_INFO_LIST_ENTRY, *PSERVER_INFO_LIST_ENTRY;

extern NPAGED_LOOKASIDE_LIST  g_server_mem_mgr;

#define InitServerMemMgr() ExInitializeNPagedLookasideList(&g_server_mem_mgr, NULL, NULL, 0, sizeof(SERVER_INFO_LIST_ENTRY), TAG_SERVER, 0)
__inline PSERVER_INFO_LIST_ENTRY AllocServer()
{
	PSERVER_INFO_LIST_ENTRY resu;
	resu = (PSERVER_INFO_LIST_ENTRY)ExAllocateFromNPagedLookasideList(&g_server_mem_mgr);
	if (resu != NULL) {
		RtlZeroMemory(resu, sizeof(SERVER_INFO_LIST_ENTRY));
		KeInitializeSpinLock(&resu->lock);
		resu->ref_count = 1;
	}
	return resu;
}
//����������
//����:
//__SER:PSERVER_INFO_LIST_ENTRY�����������ݽṹ
//__LockHandleInStack:PKLOCK_QUEUE_HANDLE�����ṹ���˽ṹ���봦��ջ�� 
__inline VOID LockServer(IN PSERVER_INFO_LIST_ENTRY __SER, OUT PKLOCK_QUEUE_HANDLE __LockHandleInStack)
{
	ASSERT(__SER != NULL && __LockHandleInStack != NULL); 
	KeAcquireInStackQueuedSpinLock(&__SER->lock, __LockHandleInStack);
}
//���ӷ���������
__inline LONG IncRefServer(IN PSERVER_INFO_LIST_ENTRY server)
{
	ASSERT(server != NULL);
	return InterlockedIncrement(&server->ref_count);
}
//���ٷ���������
__inline LONG DecRefServer(IN PSERVER_INFO_LIST_ENTRY server)
{
	ASSERT(server != NULL);
	return InterlockedDecrement(&server->ref_count);
}

//����������
//����:
//__LockHandleInStack:PKLOCK_QUEUE_HANDLE �����ṹ���˽ṹ���봦��ջ�� 
__inline VOID  UnLockServer(IN PKLOCK_QUEUE_HANDLE __LockHandleInStack)
{	
	ASSERT(__LockHandleInStack != NULL); 
	KeReleaseInStackQueuedSpinLock(__LockHandleInStack);
}

__inline VOID FreeServer(PSERVER_INFO_LIST_ENTRY server)
{
	ASSERT(server->ref_count == 0);
	ExFreeToNPagedLookasideList(&g_server_mem_mgr, server);
}
#define DelServerMemMgr() ExDeleteNPagedLookasideList(&g_server_mem_mgr)

//ɾ��������Ϣ�ص�����
VOID DelServerCallBack(PLIST_ENTRY server);
///<summary>
//ѡ�������
///</summary>
PSERVER_INFO_LIST_ENTRY SelectBestServer(IN PLCXL_LOCK_LIST server_list, IN INT ipMode, IN PVOID pIPHeader, IN PTCP_HDR pTcpHeader);

#endif