#ifndef _APP_INTERFACE_H_
#define _APP_INTERFACE_H_

#define _NDIS_CONTROL_CODE(request,method) \
	CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, request, method, FILE_ANY_ACCESS)

#define IOCTL_FILTER_RESTART_ALL            _NDIS_CONTROL_CODE(0, METHOD_BUFFERED)
#define IOCTL_FILTER_RESTART_ONE_INSTANCE   _NDIS_CONTROL_CODE(1, METHOD_BUFFERED)
#define IOCTL_FILTER_ENUERATE_ALL_INSTANCES _NDIS_CONTROL_CODE(2, METHOD_BUFFERED)
#define IOCTL_FILTER_QUERY_ALL_STAT         _NDIS_CONTROL_CODE(3, METHOD_BUFFERED)
#define IOCTL_FILTER_CLEAR_ALL_STAT         _NDIS_CONTROL_CODE(4, METHOD_BUFFERED)
#define IOCTL_FILTER_SET_OID_VALUE          _NDIS_CONTROL_CODE(5, METHOD_BUFFERED)
#define IOCTL_FILTER_QUERY_OID_VALUE        _NDIS_CONTROL_CODE(6, METHOD_BUFFERED)
#define IOCTL_FILTER_CANCEL_REQUEST         _NDIS_CONTROL_CODE(7, METHOD_BUFFERED)
#define IOCTL_FILTER_READ_DRIVER_CONFIG     _NDIS_CONTROL_CODE(8, METHOD_BUFFERED)
#define IOCTL_FILTER_WRITE_DRIVER_CONFIG    _NDIS_CONTROL_CODE(9, METHOD_BUFFERED)
#define IOCTL_FILTER_READ_ADAPTER_CONFIG    _NDIS_CONTROL_CODE(10, METHOD_BUFFERED)
#define IOCTL_FILTER_WRITE_ADAPTER_CONFIG   _NDIS_CONTROL_CODE(11, METHOD_BUFFERED)
#define IOCTL_FILTER_READ_INSTANCE_CONFIG   _NDIS_CONTROL_CODE(12, METHOD_BUFFERED)
#define IOCTL_FILTER_WRITE_INSTANCE_CONFIG  _NDIS_CONTROL_CODE(13, METHOD_BUFFERED)


#define MAX_FILTER_INSTANCE_NAME_LENGTH     256
#define MAX_FILTER_CONFIG_KEYWORD_LENGTH    256
typedef struct _FILTER_DRIVER_ALL_STAT
{
	ULONG          AttachCount;
	ULONG          DetachCount;
	ULONG          ExternalRequestFailedCount;
	ULONG          ExternalRequestSuccessCount;
	ULONG          InternalRequestFailedCount;
} FILTER_DRIVER_ALL_STAT, *PFILTER_DRIVER_ALL_STAT;


typedef struct _FILTER_SET_OID
{
	WCHAR           InstanceName[MAX_FILTER_INSTANCE_NAME_LENGTH];
	ULONG           InstanceNameLength;
	NDIS_OID        Oid;
	NDIS_STATUS     Status;
	UCHAR           Data[sizeof(ULONG)];

}FILTER_SET_OID, *PFILTER_SET_OID;

typedef struct _FILTER_QUERY_OID
{
	WCHAR           InstanceName[MAX_FILTER_INSTANCE_NAME_LENGTH];
	ULONG           InstanceNameLength;
	NDIS_OID        Oid;
	NDIS_STATUS     Status;
	UCHAR           Data[sizeof(ULONG)];

}FILTER_QUERY_OID, *PFILTER_QUERY_OID;

typedef struct _FILTER_READ_CONFIG
{
	_Field_size_bytes_part_(MAX_FILTER_INSTANCE_NAME_LENGTH, InstanceNameLength)
	WCHAR                   InstanceName[MAX_FILTER_INSTANCE_NAME_LENGTH];
	ULONG                   InstanceNameLength;
	_Field_size_bytes_part_(MAX_FILTER_CONFIG_KEYWORD_LENGTH, KeywordLength)
		WCHAR                   Keyword[MAX_FILTER_CONFIG_KEYWORD_LENGTH];
	ULONG                   KeywordLength;
	NDIS_PARAMETER_TYPE     ParameterType;
	NDIS_STATUS             Status;
	UCHAR                   Data[sizeof(ULONG)];
}FILTER_READ_CONFIG, *PFILTER_READ_CONFIG;

typedef struct _FILTER_WRITE_CONFIG
{
	_Field_size_bytes_part_(MAX_FILTER_INSTANCE_NAME_LENGTH, InstanceNameLength)
	WCHAR                   InstanceName[MAX_FILTER_INSTANCE_NAME_LENGTH];
	ULONG                   InstanceNameLength;
	_Field_size_bytes_part_(MAX_FILTER_CONFIG_KEYWORD_LENGTH, KeywordLength)
		WCHAR                   Keyword[MAX_FILTER_CONFIG_KEYWORD_LENGTH];
	ULONG                   KeywordLength;
	NDIS_PARAMETER_TYPE     ParameterType;
	NDIS_STATUS             Status;
	UCHAR                   Data[sizeof(ULONG)];
}FILTER_WRITE_CONFIG, *PFILTER_WRITE_CONFIG;

//��Ӵ���
//��ȡ���е������ӿ����
#define IOCTL_LOADER_ALL_NET_IFINDEX		_NDIS_CONTROL_CODE(0x20, METHOD_BUFFERED)
//��ȡ����IP
#define IOCTL_LOADER_GET_VIRTUAL_IP			_NDIS_CONTROL_CODE(0x21, METHOD_BUFFERED)
//LCXL_IP
//��������IP
#define IOCTL_LOADER_SET_VIRTUAL_IP			_NDIS_CONTROL_CODE(0x22, METHOD_BUFFERED)
//LCXL_IP
//��ȡ�������б�
#define IOCTL_LOADER_GET_SERVER_LIST		_NDIS_CONTROL_CODE(0x23, METHOD_BUFFERED)
typedef struct _APP_SERVER_LIST {
	int server_count;//����������
	LCXL_SERVER_ADDR server_list[0];//��������ַ
} APP_SERVER_LIST, *PAPP_SERVER_LIST;//�������б�
//��ӷ�����
#define IOCTL_LOADER_ADD_SERVER				_NDIS_CONTROL_CODE(0x24, METHOD_BUFFERED)
//ɾ��������
#define IOCTL_LOADER_DEL_SERVER				_NDIS_CONTROL_CODE(0x25, METHOD_BUFFERED)
//!��Ӵ���!

#endif