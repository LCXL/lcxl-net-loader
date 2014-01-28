#ifndef _LCXL_SETTING_H_
#define _LCXL_SETTING_H_
/*
author:
LCX
abstract:
�������ͷ�ļ�
*/
#include "lcxl_route.h"
//�����ļ�
#define LOADER_SETTING_FILE_PATH    L"\\SystemRoot\\System32\\drivers\\etc\\lcxl_loader"
#define TAG_MODULE					'MODU'
//�����ļ����ݽṹ
typedef struct _LCXL_MODULE_SETTING_LIST_ENTRY {
	LIST_ENTRY				list_entry;		//�б���
	//ref_count = 0��û��filter�������ļ�����
	//ref_count = 1����filter�������ļ�����
	LONG					ref_count;
	//������ɾ��������
#define ML_DELETE_AFTER_RESTART	0x1
	//�����ô�������״̬
#define ML_ENABLED				0x2
	
	//��������ΨһID
	NET_LUID				miniport_net_luid;
	//��ʶ
	INT						flag;
	//С�˿������Ѻ�����
	PNDIS_STRING			miniport_friendly_name;
	//С�˿���������
	PNDIS_STRING			miniport_name;
	//ģ������
	PNDIS_STRING			filter_module_name;
	//��ʵ��ַ
	LCXL_SERVER_ADDR		real_addr;
	//����IPv4
	IN_ADDR					virtual_ipv4;
	//����IPv6
	IN6_ADDR				virtual_ipv6;
	//����������
	INT						server_count;
	//�������б�
	SERVER_INFO_LIST_ENTRY	server_list;
	// �������б���
	FILTER_LOCK             module_setting_lock;
} LCXL_MODULE_SETTING_LIST_ENTRY, *PLCXL_MODULE_SETTING_LIST_ENTRY;

typedef struct _LCXL_SETTING{
	//ģ���б���
	FILTER_LOCK				module_list_lock;
	INT						module_count;
	LCXL_MODULE_SETTING_LIST_ENTRY	module_list;
} LCXL_SETTING, *PLCXL_SETTING;
//!��Ӵ���!

///<summary>
///���������ļ�
///</summary>
VOID LoadSetting();
//����һ��������Ϣ�ڴ�
PLCXL_MODULE_SETTING_LIST_ENTRY NewModuleSetting();
//�ͷ�������Ϣ�ڴ�
VOID DelModuleSetting(IN PLCXL_MODULE_SETTING_LIST_ENTRY module_setting);

PLCXL_MODULE_SETTING_LIST_ENTRY FindModuleSettingByLUID(IN NET_LUID miniport_net_luid);

//������ģ���м�������
PLCXL_MODULE_SETTING_LIST_ENTRY LoadModuleSetting(IN PNDIS_FILTER_ATTACH_PARAMETERS	attach_paramters);

///<summary>
///���������ļ�
///</summary>
VOID SaveSetting();

extern LCXL_SETTING g_Setting;
#endif


