#ifndef _LCXL_ROUTE_H_
#define _LCXL_ROUTE_H_

/*
author:
LCX
abstract:
·�����ͷ�ļ�
*/

#include "lcxl_server.h"

#define TAG_ROUTE       'ROUT'
//·����Ϣ
typedef struct _LCXL_ROUTE_LIST_ENTRY
{
	LIST_ENTRY		        list_entry;		//�б���
#define RS_NONE     0x00
#define RS_NORMAL   0x01					//����
#define RS_LAST_ACK 0x02					//���ڵȴ����һ��ACK��
#define RS_CLOSED   0x03					//�����ѹر�
	int                     status;         //����״̬
	LCXL_IP					src_ip;
	//TCP
	unsigned short	        src_port;		//Դ�˿ں�
	unsigned short	        dst_port;		//Ŀ�Ķ˿ں�
	PSERVER_INFO_LIST_ENTRY dst_server;	    //Ŀ�������
} LCXL_ROUTE_LIST_ENTRY, *PLCXL_ROUTE_LIST_ENTRY;

extern NPAGED_LOOKASIDE_LIST  g_route_mem_mgr;

#define InitRouteMemMgr() ExInitializeNPagedLookasideList(&g_route_mem_mgr, NULL, NULL, 0, sizeof(LCXL_ROUTE_LIST_ENTRY), TAG_ROUTE, 0)
__inline PLCXL_ROUTE_LIST_ENTRY AllocRoute()
{
	PLCXL_ROUTE_LIST_ENTRY resu;

	resu = (PLCXL_ROUTE_LIST_ENTRY)ExAllocateFromNPagedLookasideList(&g_route_mem_mgr);
	if (resu != NULL) {
		RtlZeroMemory(resu, sizeof(LCXL_ROUTE_LIST_ENTRY));
		resu->status = RS_NONE;
	}
	return resu;
}
#define FreeRoute(__buf) ExFreeToNPagedLookasideList(&g_route_mem_mgr, __buf)
#define DelRouteMemMgr() ExDeleteNPagedLookasideList(&g_route_mem_mgr)


///<summary>
///����·����Ϣ����
///</summary>
PLCXL_ROUTE_LIST_ENTRY CreateRouteListEntry(IN PLIST_ENTRY route_list);
///<summary>
///ʼ��·����Ϣ����
///</summary>
VOID InitRouteListEntry(IN OUT PLCXL_ROUTE_LIST_ENTRY route_info, IN INT ipMode, IN PVOID pIPHeader, IN PTCP_HDR pTcpHeader, IN PSERVER_INFO_LIST_ENTRY server_info);

///<summary>
///��ȡ·����Ϣ��
///</summary>
PLCXL_ROUTE_LIST_ENTRY GetRouteListEntry(IN PLIST_ENTRY route_list, IN INT ipMode, IN PVOID pIPHeader, IN PTCP_HDR pTcpHeader);

#endif