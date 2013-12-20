#ifndef _DRV_INTERFACE_TYPE_H_
#define _DRV_INTERFACE_TYPE_H_

#include "../../common/lcxl_type.h"
#pragma warning(disable:4200)

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INSTANCE_NAME_LENGTH     256

//��ȡ���е������ӿ����
//IOCTL_LOADER_ALL_MODULE
//output

typedef struct _APP_MODULE_INFO {
#define AMS_NONE		0x00
#define AMS_NORMAL		0x01
#define AMS_NO_SETTING	0x02
#define AMS_NO_FILTER	0x03
	//ģ��ģʽ
	INT					app_module_status;
	//С�˿�����ifindex
	NET_IFINDEX			miniport_if_index;
	//��������ΨһID
	NET_LUID			miniport_net_luid;
	//��ʵ��ַ
	LCXL_SERVER_ADDR	real_addr;
	//����IPv4
	IN_ADDR				virtual_ipv4;
	//����IPv6
	IN6_ADDR			virtual_ipv6;
	//ģ������
	USHORT				filter_module_name_len;
	WCHAR				filter_module_name[MAX_INSTANCE_NAME_LENGTH];
	//С�˿������Ѻ�����
	USHORT				miniport_friendly_name_len;
	WCHAR				miniport_friendly_name[MAX_INSTANCE_NAME_LENGTH];
	//С�˿���������
	USHORT				miniport_name_len;
	WCHAR				miniport_name[MAX_INSTANCE_NAME_LENGTH];
	//����������
	INT					server_count;

} APP_MODULE_INFO, *PAPP_MODULE_INFO;

//�������б�
//IOCTL_LOADER_GET_SERVER_LIST
//input NET_IFINDEX                     MiniportIfIndex;
//output
typedef struct _APP_SERVER_LIST {
	int server_count;//����������
	LCXL_SERVER_ADDR server_list[0];//��������ַ
} APP_SERVER_LIST, *PAPP_SERVER_LIST;

#ifdef __cplusplus
}
#endif

#endif