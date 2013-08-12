/*++

Copyright (c) Microsoft Corporation

Module Name:

    Filter.h

Abstract:

    This module contains all prototypes and macros for filter code.

Notes:

--*/
#ifndef _FILT_H
#define _FILT_H

#pragma warning(disable:28930) // Unused assignment of pointer, by design in samples
#pragma warning(disable:28931) // Unused assignment of variable, by design in samples

// TODO: Customize these to hint at your component for memory leak tracking.
// These should be treated like a pooltag.
#define FILTER_REQUEST_ID          'RTLF'
#define FILTER_ALLOC_TAG           'tliF'
#define FILTER_TAG                 'dnTF'

// TODO: Specify which version of the NDIS contract you will use here.
// In many cases, 6.0 is the best choice.  You only need to select a later
// version if you need a feature that is not available in 6.0.
//
// Legal values include:
//    6.0  Available starting with Windows Vista RTM
//    6.1  Available starting with Windows Vista SP1 / Windows Server 2008
//    6.20 Available starting with Windows 7 / Windows Server 2008 R2
//    6.30 Available starting with Windows 8 / Windows Server "8"
// Or, just use NDIS_FILTER_MAJOR_VERSION / NDIS_FILTER_MINOR_VERSION
// to pick up whatever version is defined by your build system
// (for example, "-DNDIS630").
#define FILTER_MAJOR_NDIS_VERSION   NDIS_FILTER_MAJOR_VERSION
#define FILTER_MINOR_NDIS_VERSION   NDIS_FILTER_MINOR_VERSION


//
// Global variables
//
extern NDIS_HANDLE         g_FilterDriverHandle; // NDIS handle for filter driver
extern NDIS_HANDLE         g_FilterDriverObject;
extern NDIS_HANDLE         g_NdisFilterDeviceHandle;
extern PDEVICE_OBJECT      g_DeviceObject;

extern FILTER_LOCK         g_FilterListLock;
extern LIST_ENTRY          g_FilterModuleList;




#if NDISLWF
#define FILTER_FRIENDLY_NAME        L"NDIS Sample LightWeight Filter"
// TODO: Customize this to match the GUID in the INF
#define FILTER_UNIQUE_NAME          L"{5cbf81bd-5055-47cd-9055-a76b2b4e3697}" //unique name, quid name
// TODO: Customize this to match the service name in the INF
#define FILTER_SERVICE_NAME         L"NDISLWF"
//
// The filter needs to handle IOCTLs
//
#define LINKNAME_STRING             L"\\DosDevices\\NDISLWF"
#define NTDEVICE_STRING             L"\\Device\\NDISLWF"
#endif


#if NDISLWF1
#define FILTER_FRIENDLY_NAME        L"NDIS Sample LightWeight Filter 1"
#define FILTER_UNIQUE_NAME          L"{5cbf81be-5055-47cd-9055-a76b2b4e3697}" //unique name, quid name
#define FILTER_SERVICE_NAME         L"NDISLWF1"
//
// The filter needs to handle IOCTRLs
//
#define LINKNAME_STRING             L"\\DosDevices\\NDISLWF1"
#define NTDEVICE_STRING             L"\\Device\\NDISLWF1"
#endif

#if NDISMON
#define FILTER_FRIENDLY_NAME        L"NDIS Sample Monitor LightWeight Filter"
#define FILTER_UNIQUE_NAME          L"{5cbf81bf-5055-47cd-9055-a76b2b4e3697}" //unique name, quid name
#define FILTER_SERVICE_NAME         L"NDISMON"
//
// The filter needs to handle IOCTRLs
//
#define LINKNAME_STRING             L"\\DosDevices\\NDISMON"
#define NTDEVICE_STRING             L"\\Device\\NDISMON"
#endif

#if NDISMON1
#define FILTER_FRIENDLY_NAME        L"NDIS Sample Monitor 1 LightWeight Filter"
#define FILTER_UNIQUE_NAME          L"{5cbf81c0-5055-47cd-9055-a76b2b4e3697}" //unique name, quid name
#define FILTER_SERVICE_NAME         L"NDISMON1"
//
// The filter needs to handle IOCTRLs
//
#define LINKNAME_STRING             L"\\DosDevices\\NDISMON1"
#define NTDEVICE_STRING             L"\\Device\\NDISMON1"
#endif


//
// Types and macros to manipulate packet queue
//
typedef struct _QUEUE_ENTRY
{
    struct _QUEUE_ENTRY * Next;
}QUEUE_ENTRY, *PQUEUE_ENTRY;

typedef struct _QUEUE_HEADER
{
    PQUEUE_ENTRY     Head;
    PQUEUE_ENTRY     Tail;
} QUEUE_HEADER, PQUEUE_HEADER;


#if TRACK_RECEIVES
UINT         filterLogReceiveRefIndex = 0;
ULONG_PTR    filterLogReceiveRef[0x10000];
#endif

#if TRACK_SENDS
UINT         filterLogSendRefIndex = 0;
ULONG_PTR    filterLogSendRef[0x10000];
#endif

#if TRACK_RECEIVES
#define   FILTER_LOG_RCV_REF(_O, _Instance, _NetBufferList, _Ref)    \
    {\
        filterLogReceiveRef[filterLogReceiveRefIndex++] = (ULONG_PTR)(_O); \
        filterLogReceiveRef[filterLogReceiveRefIndex++] = (ULONG_PTR)(_Instance); \
        filterLogReceiveRef[filterLogReceiveRefIndex++] = (ULONG_PTR)(_NetBufferList); \
        filterLogReceiveRef[filterLogReceiveRefIndex++] = (ULONG_PTR)(_Ref); \
        if (filterLogReceiveRefIndex >= (0x10000 - 5))                    \
        {                                                              \
            filterLogReceiveRefIndex = 0;                                 \
        }                                                              \
    }
#else
#define   FILTER_LOG_RCV_REF(_O, _Instance, _NetBufferList, _Ref)
#endif

#if TRACK_SENDS
#define   FILTER_LOG_SEND_REF(_O, _Instance, _NetBufferList, _Ref)    \
    {\
        filterLogSendRef[filterLogSendRefIndex++] = (ULONG_PTR)(_O); \
        filterLogSendRef[filterLogSendRefIndex++] = (ULONG_PTR)(_Instance); \
        filterLogSendRef[filterLogSendRefIndex++] = (ULONG_PTR)(_NetBufferList); \
        filterLogSendRef[filterLogSendRefIndex++] = (ULONG_PTR)(_Ref); \
        if (filterLogSendRefIndex >= (0x10000 - 5))                    \
        {                                                              \
            filterLogSendRefIndex = 0;                                 \
        }                                                              \
    }

#else
#define   FILTER_LOG_SEND_REF(_O, _Instance, _NetBufferList, _Ref)
#endif


//
// DEBUG related macros.
//
#if DBG
#define FILTER_ALLOC_MEM(_NdisHandle, _Size)    \
    filterAuditAllocMem(                        \
            _NdisHandle,                        \
           _Size,                               \
           __FILENUMBER,                        \
           __LINE__);

#define FILTER_FREE_MEM(_pMem)                  \
    filterAuditFreeMem(_pMem);

#else
#define FILTER_ALLOC_MEM(_NdisHandle, _Size)     \
    NdisAllocateMemoryWithTagPriority(_NdisHandle, _Size, FILTER_ALLOC_TAG, LowPoolPriority)

#define FILTER_FREE_MEM(_pMem)    NdisFreeMemory(_pMem, 0, 0)

#endif //DBG

#if DBG_SPIN_LOCK
#define FILTER_INIT_LOCK(_pLock)                          \
    filterAllocateSpinLock(_pLock, __FILENUMBER, __LINE__)

#define FILTER_FREE_LOCK(_pLock)       filterFreeSpinLock(_pLock)


#define FILTER_ACQUIRE_LOCK(_pLock, DispatchLevel)  \
    filterAcquireSpinLock(_pLock, __FILENUMBER, __LINE__, DisaptchLevel)

#define FILTER_RELEASE_LOCK(_pLock, DispatchLevel)      \
    filterReleaseSpinLock(_pLock, __FILENUMBER, __LINE__, DispatchLevel)

#else
#define FILTER_INIT_LOCK(_pLock)      NdisAllocateSpinLock(_pLock)

#define FILTER_FREE_LOCK(_pLock)      NdisFreeSpinLock(_pLock)

#define FILTER_ACQUIRE_LOCK(_pLock, DispatchLevel)              \
    {                                                           \
        if (DispatchLevel)                                      \
        {                                                       \
            NdisDprAcquireSpinLock(_pLock);                     \
        }                                                       \
        else                                                    \
        {                                                       \
            NdisAcquireSpinLock(_pLock);                        \
        }                                                       \
    }

#define FILTER_RELEASE_LOCK(_pLock, DispatchLevel)              \
    {                                                           \
        if (DispatchLevel)                                      \
        {                                                       \
            NdisDprReleaseSpinLock(_pLock);                     \
        }                                                       \
        else                                                    \
        {                                                       \
            NdisReleaseSpinLock(_pLock);                        \
        }                                                       \
    }
#endif //DBG_SPIN_LOCK


#define NET_BUFFER_LIST_LINK_TO_ENTRY(_pNBL)    ((PQUEUE_ENTRY)(NET_BUFFER_LIST_NEXT_NBL(_pNBL)))
#define ENTRY_TO_NET_BUFFER_LIST(_pEnt)         (CONTAINING_RECORD((_pEnt), NET_BUFFER_LIST, Next))

#define InitializeQueueHeader(_QueueHeader)             \
{                                                       \
    (_QueueHeader)->Head = (_QueueHeader)->Tail = NULL; \
}

//
// Macros for queue operations
//
#define IsQueueEmpty(_QueueHeader)      ((_QueueHeader)->Head == NULL)

#define RemoveHeadQueue(_QueueHeader)                   \
    (_QueueHeader)->Head;                               \
    {                                                   \
        PQUEUE_ENTRY pNext;                             \
        ASSERT((_QueueHeader)->Head);                   \
        pNext = (_QueueHeader)->Head->Next;             \
        (_QueueHeader)->Head = pNext;                   \
        if (pNext == NULL)                              \
            (_QueueHeader)->Tail = NULL;                \
    }

#define InsertHeadQueue(_QueueHeader, _QueueEntry)                  \
    {                                                               \
        ((PQUEUE_ENTRY)(_QueueEntry))->Next = (_QueueHeader)->Head; \
        (_QueueHeader)->Head = (PQUEUE_ENTRY)(_QueueEntry);         \
        if ((_QueueHeader)->Tail == NULL)                           \
            (_QueueHeader)->Tail = (PQUEUE_ENTRY)(_QueueEntry);     \
    }

#define InsertTailQueue(_QueueHeader, _QueueEntry)                      \
    {                                                                   \
        ((PQUEUE_ENTRY)(_QueueEntry))->Next = NULL;                     \
        if ((_QueueHeader)->Tail)                                       \
            (_QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(_QueueEntry);   \
        else                                                            \
            (_QueueHeader)->Head = (PQUEUE_ENTRY)(_QueueEntry);         \
        (_QueueHeader)->Tail = (PQUEUE_ENTRY)(_QueueEntry);             \
    }

//��Ӵ���
//������·��


#define NDIS_MAC_ADDR_LEN            6

#define NDIS_8021P_TAG_TYPE         0x0081
#define NDIS_IPV4                   0x0008
#define NDIS_IPV6					0x86DD
#include <pshpack1.h>

typedef struct _NDIS_ETH_HEADER
{
	UCHAR       DstAddr[NDIS_MAC_ADDR_LEN];
	UCHAR       SrcAddr[NDIS_MAC_ADDR_LEN];
	USHORT      EthType;

} NDIS_ETH_HEADER;

typedef struct _NDIS_ETH_HEADER UNALIGNED * PNDIS_ETH_HEADER;

#include <poppack.h>

//TCP/IPЭ���йصĽṹ

#ifndef s_addr
typedef struct in_addr {
	union {
		struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { USHORT s_w1,s_w2; } S_un_w;
		ULONG S_addr;
	} S_un;
} IN_ADDR, *PIN_ADDR, FAR *LPIN_ADDR;
#endif

#pragma push(1)
typedef struct IP_HEADER
{

#if LITTLE_ENDIAN
	unsigned char  ip_hl:4;    /* ͷ���� */
	unsigned char  ip_v:4;      /* �汾�� */
#else
	unsigned char   ip_v:4;
	unsigned char   ip_hl:4;     
#endif

	unsigned char  TOS;           // ��������

	unsigned short   TotLen;      // ����ܳ��ȣ�������IP���ĳ���
	unsigned short   ID;          // �����ʶ��Ψһ��ʶ���͵�ÿһ�����ݱ�
	unsigned short   FlagOff;     // ��־
	unsigned char  TTL;           // ����ʱ�䣬����TTL
	unsigned char  Protocol;      // Э�飬������TCP��UDP��ICMP��
	unsigned short Checksum;      // У���
	struct in_addr        iaSrc;  // ԴIP��ַ
	struct in_addr        iaDst;  // Ŀ��PI��ַ

}IP_HEADER, *PIP_HEADER;


typedef struct tcp_header
{
	unsigned short src_port;    //Դ�˿ں�
	unsigned short dst_port;    //Ŀ�Ķ˿ں�
	unsigned int   seq_no;      //���к�
	unsigned int   ack_no;      //ȷ�Ϻ�
#if LITTLE_ENDIAN
	unsigned char reserved_1:4; //����6λ�е�4λ�ײ�����
	unsigned char thl:4;    //tcpͷ������
	unsigned char flag:6;  //6λ��־
	unsigned char reseverd_2:2; //����6λ�е�2λ
#else
	unsigned char thl:4;    //tcpͷ������
	unsigned char reserved_1:4; //����6λ�е�4λ�ײ�����
	unsigned char reseverd_2:2; //����6λ�е�2λ
	unsigned char flag:6;  //6λ��־ 
#endif
	unsigned short wnd_size;   //16λ���ڴ�С
	unsigned short chk_sum;    //16λTCP�����
	unsigned short urgt_p;     //16Ϊ����ָ��

}TCP_HEADER,*PTCP_HEADER;


typedef struct udp_header 
{
	USHORT srcport;   // Դ�˿�
	USHORT dstport;   // Ŀ�Ķ˿�
	USHORT total_len; // ����UDP��ͷ��UDP���ݵĳ���(��λ:�ֽ�)
	USHORT chksum;    // У���

}UDP_HEADER,*PUDP_HEADER;
#pragma push()


#define IP_OFFSET                               0x0E

//IP Э������
#define PROT_ICMP                               0x01 
#define PROT_TCP                                0x06 
#define PROT_UDP                                0x11 
//!��Ӵ���!
//��Ӵ���

//��������Ϣ
typedef struct _SERVER_INFO_LIST_ENTRY
{
	//�б���
	LIST_ENTRY		list_entry;
	//IP
	//��ʵ��IP��ַ
	struct in_addr	ia_real_ip;
	//MAC��ַ����
	USHORT			mac_addr_len;
	//MAC��ַ
	UCHAR			cur_mac_addr[NDIS_MAX_PHYS_ADDRESS_LENGTH];
} SERVER_INFO_LIST_ENTRY, *PSERVER_INFO_LIST_ENTRY;

//·����Ϣ
typedef struct _LCXL_ROUTE_LIST_ENTRY
{
	LIST_ENTRY		list_entry;		//�б���
	//IP
	struct in_addr	ia_src;			//ԴIP��ַ
	//TCP
	unsigned short	src_port;		//Դ�˿ں�
	unsigned short	dst_port;		//Ŀ�Ķ˿ں�
	PLIST_ENTRY		*dst_server;	//Ŀ�������
} LCXL_ROUTE_LIST_ENTRY, *PLCXL_ROUTE_LIST_ENTRY;
//!��Ӵ���!
//
// Enum of filter's states
// Filter can only be in one state at one time
//
typedef enum _FILTER_STATE
{
    FilterStateUnspecified,
    FilterInitialized,
    FilterPausing,
    FilterPaused,
    FilterRunning,
    FilterRestarting,
    FilterDetaching
} FILTER_STATE;


typedef struct _FILTER_REQUEST
{
    NDIS_OID_REQUEST       Request;
    NDIS_EVENT             ReqEvent;
    NDIS_STATUS            Status;
} FILTER_REQUEST, *PFILTER_REQUEST;

//
// Define the filter struct
//
typedef struct _LCXL_FILTER
{
    LIST_ENTRY                     FilterModuleLink;
    //Reference to this filter
    ULONG                           RefCount;

    NDIS_HANDLE                     FilterHandle;
    NDIS_STRING                     FilterModuleName;
    //С�˿������Ѻ�����
    NDIS_STRING                     MiniportFriendlyName;
    //С�˿���������
    NDIS_STRING                     MiniportName;
    //С�˿���������ӿ����
    NET_IFINDEX                     MiniportIfIndex;

    NDIS_STATUS                     Status;
    NDIS_EVENT                      Event;
    ULONG                           BackFillSize;
    FILTER_LOCK                     Lock;    // Lock for protection of state and outstanding sends and recvs

    FILTER_STATE                    State;   // Which state the filter is in
    ULONG                           OutstandingSends;
    ULONG                           OutstandingRequest;
    ULONG                           OutstandingRcvs;
    FILTER_LOCK                     SendLock;
    FILTER_LOCK                     RcvLock;
    QUEUE_HEADER                    SendNBLQueue;
    QUEUE_HEADER                    RcvNBLQueue;


    NDIS_STRING                     FilterName;
    ULONG                           CallsRestart;
    BOOLEAN                         TrackReceives;
    BOOLEAN                         TrackSends;
#if DBG
    BOOLEAN                         bIndicating;
#endif

    PNDIS_OID_REQUEST               PendingOidRequest;
    //��ӵĴ���
    //MAC��ַ����
    USHORT                          mac_addr_len;
    //MAC��ַ
    UCHAR                           cur_mac_addr[NDIS_MAX_PHYS_ADDRESS_LENGTH];
	//����IP
	struct in_addr					ia_virtual_ip;
	//�������б�
	SERVER_INFO_LIST_ENTRY			server_list;
	//·����Ϣ
	LCXL_ROUTE_LIST_ENTRY			route_list;
}LCXL_FILTER, * PLCXL_FILTER;

typedef struct _FILTER_DEVICE_EXTENSION
{
    ULONG            Signature;
    NDIS_HANDLE      Handle;
} FILTER_DEVICE_EXTENSION, *PFILTER_DEVICE_EXTENSION;


#define FILTER_READY_TO_PAUSE(_Filter)      \
    ((_Filter)->State == FilterPausing)

//
// The driver should maintain a list of NDIS filter handles
//
typedef struct _FL_NDIS_FILTER_LIST
{
    LIST_ENTRY              Link;
    NDIS_HANDLE             ContextHandle;
    NDIS_STRING             FilterInstanceName;
} FL_NDIS_FILTER_LIST, *PFL_NDIS_FILTER_LIST;

//
// The context inside a cloned request
//
typedef struct _NDIS_OID_REQUEST *FILTER_REQUEST_CONTEXT,**PFILTER_REQUEST_CONTEXT;


//
// function prototypes
//
DRIVER_INITIALIZE DriverEntry;

FILTER_SET_OPTIONS FilterRegisterOptions;

FILTER_ATTACH FilterAttach;

FILTER_DETACH FilterDetach;

DRIVER_UNLOAD FilterUnload;

FILTER_RESTART FilterRestart;

FILTER_PAUSE FilterPause;

FILTER_OID_REQUEST FilterOidRequest;

FILTER_CANCEL_OID_REQUEST FilterCancelOidRequest;

FILTER_STATUS FilterStatus;

FILTER_DEVICE_PNP_EVENT_NOTIFY FilterDevicePnPEventNotify;

FILTER_NET_PNP_EVENT FilterNetPnPEvent;

FILTER_OID_REQUEST_COMPLETE FilterOidRequestComplete;

FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;

FILTER_RETURN_NET_BUFFER_LISTS FilterReturnNetBufferLists;

FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;

FILTER_RECEIVE_NET_BUFFER_LISTS FilterReceiveNetBufferLists;

FILTER_CANCEL_SEND_NET_BUFFER_LISTS FilterCancelSendNetBufferLists;

FILTER_SET_MODULE_OPTIONS FilterSetModuleOptions;


_IRQL_requires_max_(PASSIVE_LEVEL)
NDIS_STATUS
FilterRegisterDevice(
    VOID
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
FilterDeregisterDevice(
    VOID
    );

DRIVER_DISPATCH FilterDispatch;

DRIVER_DISPATCH FilterDeviceIoControl;

_IRQL_requires_max_(DISPATCH_LEVEL)
PLCXL_FILTER
filterFindFilterModule(
    _In_reads_bytes_(BufferLength)
         PUCHAR                   Buffer,
    _In_ ULONG                    BufferLength
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
NDIS_STATUS
filterDoInternalRequest(
    _In_ PLCXL_FILTER                   FilterModuleContext,
    _In_ NDIS_REQUEST_TYPE            RequestType,
    _In_ NDIS_OID                     Oid,
    _Inout_updates_bytes_to_(InformationBufferLength, *pBytesProcessed)
         PVOID                        InformationBuffer,
    _In_ ULONG                        InformationBufferLength,
    _In_opt_ ULONG                    OutputBufferLength,
    _In_ ULONG                        MethodId,
    _Out_ PULONG                      pBytesProcessed
    );

VOID
filterInternalRequestComplete(
    _In_ NDIS_HANDLE                  FilterModuleContext,
    _In_ PNDIS_OID_REQUEST            NdisRequest,
    _In_ NDIS_STATUS                  Status
    );


#endif  //_FILT_H


