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

#define INIT_ROUTE_MEM_MGR() ExInitializeNPagedLookasideList(&g_route_mem_mgr, NULL, NULL, 0, sizeof(LCXL_ROUTE_LIST_ENTRY), TAG_ROUTE, 0)
#define ALLOC_ROUTE() (PLCXL_ROUTE_LIST_ENTRY)ExAllocateFromNPagedLookasideList(&g_route_mem_mgr)
#define FREE_ROUTE(__buf) ExFreeToNPagedLookasideList(&g_route_mem_mgr, __buf)
#define DEL_ROUTE_MEM_MGR() ExDeleteNPagedLookasideList(&g_route_mem_mgr)


///<summary>
///����·����Ϣ����
///</summary>
PLCXL_ROUTE_LIST_ENTRY CreateRouteListEntry(IN PLIST_ENTRY route_list);
///<summary>
///ʼ��·����Ϣ����
///</summary>
void InitRouteListEntry(IN OUT PLCXL_ROUTE_LIST_ENTRY route_info, IN INT ipMode, IN PVOID pIPHeader, IN PTCP_HDR pTcpHeader, IN PSERVER_INFO_LIST_ENTRY server_info);

///<summary>
///��ȡ·����Ϣ��
///</summary>
PLCXL_ROUTE_LIST_ENTRY GetRouteListEntry(IN PLIST_ENTRY route_list, IN INT ipMode, IN PVOID pIPHeader, IN PTCP_HDR pTcpHeader);

///<summary>
///��ȡ·����Ϣ��IPv4
///</summary>
__inline  PLCXL_ROUTE_LIST_ENTRY GetRouteListEntry4(IN PLIST_ENTRY route_list, IN PIPV4_HEADER pIPHeader, IN PTCP_HDR pTcpHeader)
{
	return GetRouteListEntry(route_list, IM_IPV4, pIPHeader, pTcpHeader);
}
///<summary>
///��ȡ·����Ϣ��IPv6
///</summary>
__inline PLCXL_ROUTE_LIST_ENTRY GetRouteListEntry6(IN PLIST_ENTRY route_list, IN PIPV6_HEADER pIPHeader, IN PTCP_HDR pTcpHeader)
{
	return GetRouteListEntry(route_list, IM_IPV6, pIPHeader, pTcpHeader);
}

#endif