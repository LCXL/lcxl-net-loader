#ifndef _LCXL_LOCK_LIST_H_
#define _LCXL_LOCK_LIST_H_
//author:LCXL
//abstract:��������ʹ�õ��б�ṹ��������������б�������
#include <wdm.h>
#define TAG_TO_BE 'TOBE'
//ɾ���б���ص�������
//ע�⣺�˺���������DISPATCH_LEVEL ��
typedef VOID (*DEL_LIST_ENTRY_FUNC) (IN PLIST_ENTRY list_entry);

//Ҫ���/ɾ�����б���
typedef struct _LCXL_TO_BE_LIST {
	LIST_ENTRY list_entry;

	//Ҫ���/ɾ�����б���
	PLIST_ENTRY to_be_list_entry;
} LCXL_TO_BE_LIST, *PLCXL_TO_BE_LIST;

typedef struct _LCXL_LOCK_LIST {
	//�б�ֻ�е�������ʱ����ܷ��ʣ�ͨ��GetListofLCXLLockList���
	LIST_ENTRY				list;
	//�б���������ֻ�е�������ʱ����ܷ��ʣ�ͨ��GetListCountofLCXLLockList���
	INT						list_count;
	//������б�
	LIST_ENTRY				to_be_add_list;//LCXL_TO_BE_LIST
	//��ɾ���б�
	LIST_ENTRY				to_be_del_list;//LCXL_TO_BE_LIST
	//�ڴ������
	NPAGED_LOOKASIDE_LIST	to_be_mem_mgr;
	//������
	KSPIN_LOCK				lock;
	//�������Ĵ���
	BOOLEAN					lock_count;
	//ɾ���б���ص�������
	//ע�⣺�˺���������DISPATCH_LEVEL ��
	DEL_LIST_ENTRY_FUNC		del_func;
} LCXL_LOCK_LIST, *PLCXL_LOCK_LIST;

__inline VOID InitLCXLLockList(IN PLCXL_LOCK_LIST lock_list, IN DEL_LIST_ENTRY_FUNC del_func)
{
	ASSERT(lock_list != NULL && del_func != NULL);
	lock_list->lock_count = 0;
	lock_list->list_count = 0;
	InitializeListHead(&lock_list->list);
	InitializeListHead(&lock_list->to_be_add_list);
	InitializeListHead(&lock_list->to_be_del_list);
	KeInitializeSpinLock(&lock_list->lock);
	ExInitializeNPagedLookasideList(&lock_list->to_be_mem_mgr, NULL, NULL, 0, sizeof(LCXL_TO_BE_LIST), TAG_TO_BE, 0);
}

//����LCXL�б������֮���б��������������ģ����Ҳ�����ĵ�ǰ���ж����󼶱�IRQL��
__inline VOID LockLCXLLockList(IN PLCXL_LOCK_LIST lock_list)
{
	//����֮ǰ��IRQL
	KLOCK_QUEUE_HANDLE		queue_handle;
	KeAcquireInStackQueuedSpinLock(&lock_list->lock, &queue_handle);
	lock_list->lock_count++;
	KeReleaseInStackQueuedSpinLock(&queue_handle);
}

//��ȡ�б�������������ʱ���ȡ
__inline PLIST_ENTRY GetListofLCXLLockList(IN PLCXL_LOCK_LIST lock_list)
{
	ASSERT(lock_list->lock_count > 0);
	return &lock_list->list;
}

//��ȡ�б�������������������ʱ���ȡ
__inline INT GetListCountofLCXLLockList(IN PLCXL_LOCK_LIST lock_list)
{
	ASSERT(lock_list->lock_count > 0);
	return lock_list->list_count;
}

//����б��LCXL�����б��У�����������������UnlockLCXLLockList����ִ��ʱ�������
__inline VOID AddtoLCXLLockList(IN PLCXL_LOCK_LIST lock_list, IN PLIST_ENTRY list_entry)
{
	//����֮ǰ��IRQL
	KLOCK_QUEUE_HANDLE		queue_handle;
	KeAcquireInStackQueuedSpinLock(&lock_list->lock, &queue_handle);
	
	if (lock_list->lock_count > 0) {
		PLCXL_TO_BE_LIST new_list_entry;
		//��������������б�����ӵ�������б���
		new_list_entry = (PLCXL_TO_BE_LIST)ExAllocateFromNPagedLookasideList(&lock_list->to_be_mem_mgr);
		new_list_entry->to_be_list_entry = list_entry;
		InsertTailList(&lock_list->to_be_add_list, &new_list_entry->list_entry);
	} else {
		//û��ס����ֱ�����
		lock_list->list_count++;
		InsertTailList(&lock_list->list, list_entry);
	}
	KeReleaseInStackQueuedSpinLock(&queue_handle);
}
//��LCXL�����б�ɾ��ָ�����б������������������UnlockLCXLLockList����ִ��ʱ����ɾ��
__inline VOID DelFromLCXLLockList(IN PLCXL_LOCK_LIST lock_list, IN PLIST_ENTRY list_entry)
{
	//����֮ǰ��IRQL
	KLOCK_QUEUE_HANDLE		queue_handle;
	KeAcquireInStackQueuedSpinLock(&lock_list->lock, &queue_handle);

	if (lock_list->lock_count > 0) {
		PLCXL_TO_BE_LIST del_list_entry;
		//��������������б�����ӵ���ɾ���б���
		del_list_entry = (PLCXL_TO_BE_LIST)ExAllocateFromNPagedLookasideList(&lock_list->to_be_mem_mgr);
		del_list_entry->to_be_list_entry = list_entry;
		InsertTailList(&lock_list->to_be_del_list, &del_list_entry->list_entry);
	} else {
		//û��ס����ֱ��ɾ��
		lock_list->list_count--;
		RemoveEntryList(list_entry);
		lock_list->del_func(list_entry);
	}

	KeReleaseInStackQueuedSpinLock(&queue_handle);
}

//�����б������д���ӵ��б�����ɾ�����б�����������������/ɾ��
__inline VOID UnlockLCXLLockList(IN PLCXL_LOCK_LIST lock_list)
{
	//����֮ǰ��IRQL
	KLOCK_QUEUE_HANDLE		queue_handle;
	KeAcquireInStackQueuedSpinLock(&lock_list->lock, &queue_handle);

	lock_list->lock_count--;
	if (lock_list->lock_count == 0) {
		PLCXL_TO_BE_LIST list_entry;
		
		while (list_entry = CONTAINING_RECORD(lock_list->to_be_del_list.Flink, LCXL_TO_BE_LIST, list_entry), &list_entry->list_entry != &lock_list->to_be_del_list) {
			RemoveEntryList(&list_entry->list_entry);
			//���б���ɾ����ɾ������
			lock_list->list_count--;
			RemoveEntryList(list_entry->to_be_list_entry);
			lock_list->del_func(list_entry->to_be_list_entry);
			
			ExFreeToNPagedLookasideList(&lock_list->to_be_mem_mgr, list_entry);
		}

		while (list_entry = CONTAINING_RECORD(lock_list->to_be_add_list.Flink, LCXL_TO_BE_LIST, list_entry), &list_entry->list_entry != &lock_list->to_be_add_list) {
			RemoveEntryList(&list_entry->list_entry);
			//������ӵ�����뵽�б���
			lock_list->list_count++;
			InsertTailList(&lock_list->list, list_entry->to_be_list_entry);
			
			ExFreeToNPagedLookasideList(&lock_list->to_be_mem_mgr, list_entry);
		}
	}
	KeReleaseInStackQueuedSpinLock(&queue_handle);
}
//ɾ��LCXL�����б�
__inline VOID DelLCXLLockList(IN PLCXL_LOCK_LIST lock_list)
{
	ASSERT(lock_list != NULL);
	ExDeleteNPagedLookasideList(&lock_list->to_be_mem_mgr);
}
#endif