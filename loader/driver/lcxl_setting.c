﻿#include "precomp.h"
#include "lcxl_setting.h"

//配置信息
LCXL_SETTING		g_setting;

VOID DelModuleSettingCallBack(PLIST_ENTRY module_setting)
{
	DelModuleSetting(CONTAINING_RECORD(module_setting, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry));
}

VOID LoadSetting()
{
	NTSTATUS status;
	HANDLE file_handle;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING file_path;
	IO_STATUS_BLOCK iosb = { 0 };

	FILE_STANDARD_INFORMATION file_info;

	RtlInitUnicodeString(&file_path, LOADER_SETTING_FILE_PATH);
	InitializeObjectAttributes(&oa, &file_path, OBJ_CASE_INSENSITIVE || OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&file_handle, GENERIC_READ, &oa, &iosb, NULL, FILE_ATTRIBUTE_SYSTEM || FILE_ATTRIBUTE_HIDDEN || FILE_ATTRIBUTE_READONLY, FILE_SHARE_READ, FILE_OPEN, FILE_NON_DIRECTORY_FILE || FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status)) {
		KdPrint(("SYS:LoadSetting:ZwCreateFile Failed:0x%08x, iosb.Info=0x%p\n", status, iosb.Information));
		return;
	}
	//查询文件信息
	status = ZwQueryInformationFile(file_handle, &iosb, &file_info, sizeof(file_info), FileStandardInformation);
	if (NT_SUCCESS(status)) {

		//文件大小不能超过65535
		if (file_info.EndOfFile.QuadPart < 0xFFFF) {
			PUCHAR buf;
			PUCHAR cur_buf;
			INT buf_len;

			buf_len = (INT)file_info.EndOfFile.QuadPart;
			buf = cur_buf = ExAllocatePoolWithTag(NonPagedPool, buf_len, TAG_FILE_BUFFER);
			if (buf_len>0 && buf != NULL) {
				status = ZwReadFile(file_handle, NULL, NULL, NULL, &iosb, buf, buf_len, NULL, NULL);
				if (NT_SUCCESS(status)) {
					INT i;
					INT module_count;

					LockLCXLLockList(&g_setting.module_list);
					//读取模块数量
					cur_buf = LCXLReadFromBuf(cur_buf, &module_count, sizeof(module_count));
					for (i = 0; i < module_count; i++) {
						PLCXL_MODULE_SETTING_LIST_ENTRY module_setting = NULL;
						NET_LUID miniport_net_luid;
						//读取Luid
						cur_buf = LCXLReadFromBuf(cur_buf, &miniport_net_luid.Value, sizeof(miniport_net_luid.Value));
						module_setting = FindModuleSettingByLUID(miniport_net_luid);
						
						if (module_setting == NULL) {
							INT j;
							INT server_count;

							module_setting = NewModuleSetting();
							//读取Luid
							module_setting->miniport_net_luid.Value = miniport_net_luid.Value;
							//写入模块标志信息
							cur_buf = LCXLReadFromBuf(cur_buf, &module_setting->flag, sizeof(module_setting->flag));
							//读取小端口驱动友好名称
							cur_buf = LCXLReadStringFromBuf(cur_buf, &module_setting->miniport_friendly_name);
							//读取小端口驱动名称
							cur_buf = LCXLReadStringFromBuf(cur_buf, &module_setting->miniport_name);
							//读取模块名称
							cur_buf = LCXLReadStringFromBuf(cur_buf, &module_setting->filter_module_name);
							//读取均衡器地址
							cur_buf = LCXLReadFromBuf(cur_buf, &module_setting->real_addr, sizeof(module_setting->real_addr));
							//读取虚拟IPv4地址
							cur_buf = LCXLReadFromBuf(cur_buf, &module_setting->virtual_ipv4, sizeof(module_setting->virtual_ipv4));
							//读取虚拟IPv6地址
							cur_buf = LCXLReadFromBuf(cur_buf, &module_setting->virtual_ipv6, sizeof(module_setting->virtual_ipv6));

							//读取服务器数量
							cur_buf = LCXLReadFromBuf(cur_buf, &server_count, sizeof(server_count));
							//初始化服务器列表
							InitLCXLLockList(&module_setting->server_list, DelServerCallBack);
							
							for (j = 0; j < server_count; j++) {
								PSERVER_INFO_LIST_ENTRY server = NULL;
								server = AllocServer();
								if (server != NULL) {
									cur_buf = LCXLReadFromBuf(cur_buf, &server->info, sizeof(server->info));
									AddtoLCXLLockList(&module_setting->server_list, &server->list_entry);
								} else {
									KdPrint(("SYS:LoadSetting:AllocServer failed.\n"));
								}
							}
							AddtoLCXLLockList(&g_setting.module_list, &module_setting->list_entry);
						}
					}
					UnlockLCXLLockList(&g_setting.module_list);
				}
				ExFreePoolWithTag(buf, TAG_FILE_BUFFER);
			} else {
				KdPrint(("SYS:LoadSetting:ExAllocatePoolWithTag Failed(memsize=%d).\n", buf_len));
			}
		}
	}
	ZwClose(file_handle);
}


PLCXL_MODULE_SETTING_LIST_ENTRY FindModuleSettingByLUID(IN NET_LUID miniport_net_luid)
{
	BOOLEAN is_found = FALSE;
	PLCXL_MODULE_SETTING_LIST_ENTRY module = NULL;

	NdisAcquireSpinLock(&g_setting.lock);
	module = CONTAINING_RECORD(GetListofLCXLLockList(&g_setting.module_list)->Flink, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry);
	while (&module->list_entry != GetListofLCXLLockList(&g_setting.module_list)) {
		if (module->miniport_net_luid.Value == miniport_net_luid.Value) {
			is_found = TRUE;
			break;
		}
		module = CONTAINING_RECORD(module->list_entry.Flink, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry);
	}

	NdisReleaseSpinLock(&g_setting.lock);
	return is_found ? module : NULL;
}


PLCXL_MODULE_SETTING_LIST_ENTRY LoadModuleSetting(IN PNDIS_FILTER_ATTACH_PARAMETERS	attach_paramters)
{
	
	PLCXL_MODULE_SETTING_LIST_ENTRY module;
	BOOLEAN is_new_module;
	module = FindModuleSettingByLUID(attach_paramters->BaseMiniportNetLuid);
	
	is_new_module = module == NULL;
	//如果没有找到
	if (is_new_module) {
		module = NewModuleSetting();
		if (module != NULL) {
			module->miniport_net_luid = attach_paramters->BaseMiniportNetLuid;
		}
	}
	if (module != NULL) {
		//更新module中的信息
		//更新MAC信息
		//保存MAC地址（一个问题，当用户手动修改了MAC地址，会怎样- -）
		module->real_addr.mac_addr.Length = sizeof(module->real_addr.mac_addr.Address) < attach_paramters->MacAddressLength ? sizeof(module->real_addr.mac_addr.Address) : attach_paramters->MacAddressLength;
		NdisMoveMemory(module->real_addr.mac_addr.Address, attach_paramters->CurrentMacAddress, module->real_addr.mac_addr.Length);
		//更新小端口驱动相关信息
		LCXLFreeString(module->miniport_friendly_name);
		module->miniport_friendly_name = LCXLNewString(attach_paramters->BaseMiniportInstanceName);
		LCXLFreeString(module->miniport_name);
		module->miniport_name = LCXLNewString(attach_paramters->BaseMiniportName);
		LCXLFreeString(module->filter_module_name);
		module->filter_module_name = LCXLNewString(attach_paramters->FilterModuleGuidName);

		//引用计数加1
		InterlockedIncrement(&module->ref_count);
		if (is_new_module) {
			AddtoLCXLLockList(&g_setting.module_list, &module->list_entry);
		}
		
	}
	return module;
}

VOID SaveSetting()
{
	NTSTATUS status;
	HANDLE file_handle;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING file_path;
	IO_STATUS_BLOCK iosb = { 0 };
	PLCXL_MODULE_SETTING_LIST_ENTRY module;
	INT buf_len;
	PUCHAR buf;
	PUCHAR cur_buf;
	//打开文件
	RtlInitUnicodeString(&file_path, LOADER_SETTING_FILE_PATH);
	InitializeObjectAttributes(&oa, &file_path, OBJ_CASE_INSENSITIVE || OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(
		&file_handle,
		GENERIC_WRITE,
		&oa,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_SYSTEM || FILE_ATTRIBUTE_HIDDEN || FILE_ATTRIBUTE_READONLY,
		0,
		FILE_OVERWRITE_IF,
		FILE_NON_DIRECTORY_FILE || FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
		);
	if (!NT_SUCCESS(status)) {
		KdPrint(("SYS:SaveSetting:ZwCreateFile Failed:0x%08x, iosb.Info=0x%p\n", status, iosb.Information));
		return;
	}
	buf_len = 0;
	LockLCXLLockList(&g_setting.module_list);
	//获取模块数量
	buf_len +=  sizeof(g_setting.module_list.list_count);
	//先计算需要写入配置文件的长度
	module = CONTAINING_RECORD(GetListofLCXLLockList(&g_setting.module_list)->Flink, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry);
	while (&module->list_entry != GetListofLCXLLockList(&g_setting.module_list)) {
		//如果是重启后删除的，则不保存到配置文件中
		if ((module->flag & MSF_DELETE_AFTER_RESTART) == 0) {
			PSERVER_INFO_LIST_ENTRY server;

			buf_len +=
				sizeof(module->miniport_net_luid.Value) +
				sizeof(module->flag)+
				3 * sizeof(USHORT)+
				module->miniport_friendly_name->Length +
				module->miniport_name->Length +
				module->filter_module_name->Length +
				sizeof(module->real_addr) +
				sizeof(module->virtual_ipv4) +
				sizeof(module->virtual_ipv6);
			
			LockLCXLLockList(&module->server_list);
			buf_len += sizeof(module->server_list.list_count);
			server = CONTAINING_RECORD(GetListofLCXLLockList(&module->server_list)->Flink, SERVER_INFO_LIST_ENTRY, list_entry);
			while (&server->list_entry != GetListofLCXLLockList(&module->server_list)) {
				if ((server->info.status & SS_DELETED) == 0) {
					buf_len += sizeof(server->info);
				}
				
				server = CONTAINING_RECORD(server->list_entry.Flink, SERVER_INFO_LIST_ENTRY, list_entry);
			}
			UnlockLCXLLockList(&module->server_list);
		}
		module = CONTAINING_RECORD(module->list_entry.Flink, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry);
	}

	buf = cur_buf = ExAllocatePoolWithTag(NonPagedPool, buf_len, TAG_FILE_BUFFER);
	//获取模块数量
	cur_buf = LCXLWriteToBuf(cur_buf, &g_setting.module_list.list_count, sizeof(g_setting.module_list.list_count));

	module = CONTAINING_RECORD(GetListofLCXLLockList(&g_setting.module_list)->Flink, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry);
	while (&module->list_entry != GetListofLCXLLockList(&g_setting.module_list)) {
		//如果是重启后删除的，则不保存到配置文件中
		if ((module->flag & MSF_DELETE_AFTER_RESTART) == 0) {
			PSERVER_INFO_LIST_ENTRY server;
			
			//写入Luid
			cur_buf = LCXLWriteToBuf(cur_buf, &module->miniport_net_luid.Value, sizeof(module->miniport_net_luid.Value));
			//写入模块标志信息
			cur_buf = LCXLWriteToBuf(cur_buf, &module->flag, sizeof(module->flag));
			//写入小端口驱动友好名称
			cur_buf = LCXLWriteStringToBuf(cur_buf, module->miniport_friendly_name);
			//写入小端口驱动名称
			cur_buf = LCXLWriteStringToBuf(cur_buf, module->miniport_name);
			//写入模块名称
			cur_buf = LCXLWriteStringToBuf(cur_buf, module->filter_module_name);
			//写入均衡器地址
			cur_buf = LCXLWriteToBuf(cur_buf, &module->real_addr, sizeof(module->real_addr));
			//写入虚拟IPv4地址
			cur_buf = LCXLWriteToBuf(cur_buf, &module->virtual_ipv4, sizeof(module->virtual_ipv4));
			//写入虚拟IPv6地址
			cur_buf = LCXLWriteToBuf(cur_buf, &module->virtual_ipv6, sizeof(module->virtual_ipv6));
			LockLCXLLockList(&module->server_list);
			//写入服务器数量
			cur_buf = LCXLWriteToBuf(cur_buf, &module->server_list.list_count, sizeof(module->server_list.list_count));
			//遍历写入服务器信息
			server = CONTAINING_RECORD(GetListofLCXLLockList(&module->server_list)->Flink, SERVER_INFO_LIST_ENTRY, list_entry);
			while (&server->list_entry != GetListofLCXLLockList(&module->server_list)) {
				//写入服务器信息
				if ((server->info.status & SS_DELETED) == 0) {
					cur_buf = LCXLWriteToBuf(cur_buf, &server->info, sizeof(server->info));
				}
				server = CONTAINING_RECORD(server->list_entry.Flink, SERVER_INFO_LIST_ENTRY, list_entry);
			}
			UnlockLCXLLockList(&module->server_list);
		}
		module = CONTAINING_RECORD(module->list_entry.Flink, LCXL_MODULE_SETTING_LIST_ENTRY, list_entry);
	}
	UnlockLCXLLockList(&g_setting.module_list);
	status = ZwWriteFile(file_handle, NULL, NULL, NULL, &iosb, buf, buf_len, NULL, NULL);
	if (!NT_SUCCESS(status)) {
		KdPrint(("SYS:SaveSetting:ZwCreateFile Failed:0x%08x, iosb.Info=0x%p\n", status, iosb.Information));
	}
	ExFreePoolWithTag(buf, TAG_FILE_BUFFER);
	ZwClose(file_handle);
}

PLCXL_MODULE_SETTING_LIST_ENTRY NewModuleSetting()
{
	PLCXL_MODULE_SETTING_LIST_ENTRY module_setting;

	module_setting = ExAllocatePoolWithTag(NonPagedPool, sizeof(LCXL_MODULE_SETTING_LIST_ENTRY), TAG_MODULE);
	if (module_setting != NULL) {
		NdisZeroMemory(module_setting, sizeof(LCXL_MODULE_SETTING_LIST_ENTRY));
		NdisAllocateSpinLock(&module_setting->lock);
	}
	
	return module_setting;
}

VOID DelModuleSetting(IN PLCXL_MODULE_SETTING_LIST_ENTRY module_setting)
{
	LCXLFreeString(module_setting->miniport_friendly_name);
	LCXLFreeString(module_setting->miniport_name);
	LCXLFreeString(module_setting->filter_module_name);

	ExFreePoolWithTag(module_setting, TAG_MODULE);
}

