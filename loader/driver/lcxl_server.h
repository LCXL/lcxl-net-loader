#ifndef _LCXL_SERVER_H_
#define _LCXL_SERVER_H_
/*
author:
LCX
abstract:
���������ͷ�ļ�
*/

#define TAG_SERVER      'SERV'

//����������
typedef struct _LCXL_SERVER_PERFORMANCE
{
	//���������ƽ������ʱ�䣬ʱ�䵥λΪ΢�us��
	//Windows��ʹ��KeQueryPerformanceCounter
	unsigned long       process_time;
	//���ڴ���
	unsigned long long  total_memory;
	//��ǰʹ���ڴ�
	unsigned long long  cur_memory;
	//CPUʹ���ʣ����Ϊ1
	double              cpu_usage;
} LCXL_SERVER_PERFORMANCE, *PLCXL_SERVER_PERFORMANCE;

typedef struct _LCXL_SERVER_INFO {
#define SS_ENABLED	0x01//��������������״̬
#define SS_ONLINE	0x02//����������
	//������״̬
	UCHAR				status;
	//�������
	CHAR				computer_name[256];
	//��������ʵIP��ַ
	LCXL_SERVER_ADDR	addr;
} LCXL_SERVER_INFO, *PLCXL_SERVER_INFO;

//��������Ϣ�б���
typedef struct _SERVER_INFO_LIST_ENTRY
{
	//�б���
	LIST_ENTRY				list_entry;
	//���ü���
	volatile LONG  			ref_count;
	//������״̬
	LCXL_SERVER_INFO		info;
	//����������״̬
	LCXL_SERVER_PERFORMANCE	performance;
} SERVER_INFO_LIST_ENTRY, *PSERVER_INFO_LIST_ENTRY;

extern NPAGED_LOOKASIDE_LIST  g_server_mem_mgr;

#define INIT_SERVER_MEM_MGR() ExInitializeNPagedLookasideList(&g_server_mem_mgr, NULL, NULL, 0, sizeof(SERVER_INFO_LIST_ENTRY), TAG_SERVER, 0)
#define ALLOC_SERVER() (PSERVER_INFO_LIST_ENTRY)ExAllocateFromNPagedLookasideList(&g_server_mem_mgr)
#define FREE_SERVER(__buf) ExFreeToNPagedLookasideList(&g_server_mem_mgr, __buf)
#define DEL_SERVER_MEM_MGR() ExDeleteNPagedLookasideList(&g_server_mem_mgr)

///<summary>
//ѡ�������
///</summary>
PSERVER_INFO_LIST_ENTRY SelectServer(IN PLIST_ENTRY server_list, IN INT ipMode, IN PVOID pIPHeader, IN PTCP_HDR pTcpHeader);

#endif