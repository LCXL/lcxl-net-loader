#ifndef _REF_LOCK_LIST_H_
#define _REF_LOCK_LIST_H_
//author:LCXL
//abstract:�����õ�LCXL���б�
#include "lcxl_lock_list.h"

typedef struct _REF_LIST_ENTRY {
	LIST_ENTRY	list_entry;
	//���ü���
	LONG		ref_count;
} REF_LIST_ENTRY, *PREF_LIST_ENTRY;//�����õ��б���

//************************************
// ���: ��LIST_ENTRYת����REF_LIST_ENTRY
// ����: PREF_LIST_ENTRY
// ����: IN PLIST_ENTRY list_entry
//************************************
__inline PREF_LIST_ENTRY GetRefListEntry(IN PLIST_ENTRY	list_entry)
{
	return CONTAINING_RECORD(list_entry, REF_LIST_ENTRY, list_entry);
}
//�����б�������
//������
//list:LCXL���б�
//list_entry:�б����Ҫע�����list_entry������list��
__inline LONG IncRefListEntry(IN PLCXL_LOCK_LIST list, IN PREF_LIST_ENTRY list_entry)
{
	ASSERT(list != NULL);
	ASSERT(list_entry != NULL);
	UNREFERENCED_PARAMETER(list);
	return InterlockedIncrement(&list_entry->ref_count);
}
//�����б�������
//������
//list:LCXL���б�
//list_entry:�б����Ҫע�����list_entry������list��
//��ʾ����list_entry����Ϊ0ʱ�����б����ɾ��
__inline LONG DecRefListEntry(IN PLCXL_LOCK_LIST list, IN PREF_LIST_ENTRY list_entry)
{
	LONG ref_count;

	ASSERT(list != NULL);
	ASSERT(list_entry != NULL);
	ref_count = InterlockedDecrement(&list_entry->ref_count);
	if (ref_count <= 0) {
		ASSERT(ref_count == 0);
		DelFromLCXLLockList(list, &list_entry->list_entry);
	}
	return ref_count;
}


#endif