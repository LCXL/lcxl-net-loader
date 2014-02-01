#ifndef _LCXL_SETTING_H_
#define _LCXL_SETTING_H_
/*
author:
LCX
abstract:
�������ͷ�ļ�
*/
#include "lcxl_route.h"
#include "../../common/driver/lcxl_lock_list.h"
//�����ļ�
#define LOADER_SETTING_FILE_PATH    L"\\SystemRoot\\System32\\drivers\\etc\\lcxl_loader"
#define TAG_MODULE					'MODU'
//�����ļ����ݽṹ
typedef struct _LCXL_MODULE_SETTING_LIST_ENTRY {
	LIST_ENTRY			list_entry;		//�б���
	//ref_count = 0��û��filter�������ļ�����
	//ref_count = 1����filter�������ļ�����
	LONG				ref_count;

	
	//��������ΨһID
	NET_LUID			miniport_net_luid;
	//������ɾ��������
#define ML_DELETE_AFTER_RESTART	0x1
	//�����ô�������״̬
#define ML_ENABLED				0x2
	//��ʶ
	INT					flag;
	//С�˿������Ѻ�����
	PNDIS_STRING		miniport_friendly_name;
	//С�˿���������
	PNDIS_STRING		miniport_name;
	//ģ������
	PNDIS_STRING		filter_module_name;
	//��ʵ��ַ
	LCXL_SERVER_ADDR	real_addr;
	//����IPv4
	IN_ADDR				virtual_ipv4;
	//����IPv6
	IN6_ADDR			virtual_ipv6;
	//�������б�
	LCXL_LOCK_LIST		server_list;//SERVER_INFO_LIST_ENTRY
	// �������б���
	FILTER_LOCK			lock;
} LCXL_MODULE_SETTING_LIST_ENTRY, *PLCXL_MODULE_SETTING_LIST_ENTRY;

typedef struct _LCXL_SETTING{
	//ģ���б���
	FILTER_LOCK			lock;

	LCXL_LOCK_LIST		module_list;//LCXL_MODULE_SETTING_LIST_ENTRY
} LCXL_SETTING, *PLCXL_SETTING;


//ɾ��������Ϣ�ص�����
VOID DelModuleSettingCallBack(PLIST_ENTRY module_setting);

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

extern LCXL_SETTING g_setting;
#endif


