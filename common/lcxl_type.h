#ifndef _LCXL_TYPE_H_
#define _LCXL_TYPE_H_
//author:LCXL
//abstract:������Ӧ�ó����õ��Զ������ݰ��йؽṹ����ͷ�ļ�
//���������������Ҫǰ���ͷ�ļ�lcxl_net.h
//�����Win32������Ҫǰ���WinSock2.h
//#include "driver/lcxl_net.h"
#include <ifdef.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _LCXL_IP {
#define IM_UNKNOWN 0
#define IM_IPV4	1
#define IM_IPV6 2
		int						ip_mode;		//IPģʽ��IPv4����IPv6�� IM_IPV4, IM_IPV6
		union {
			//IP
			IN_ADDR			    ip_4;			//ԴIPv4��ַ
			IN6_ADDR			ip_6;			//ԴIPv6��ַ
		} addr;
	} LCXL_IP, *PLCXL_IP;

	typedef struct _LCXL_SERVER_ADDR {
#define SA_ENABLE_IPV4 0x01//������������IPV4Э��
#define SA_ENABLE_IPV6 0x02//������������IPV6Э��
		UCHAR			status;
		//��ʵ��IP��ַ
		IN_ADDR			ipv4;
		IN6_ADDR		ipv6;
		//MAC��ַ
		IF_PHYSICAL_ADDRESS mac_addr;
	} LCXL_SERVER_ADDR, *PLCXL_SERVER_ADDR;//��������ַ

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
	//��������Ϣ
	typedef struct _LCXL_SERVER_INFO {
#define SS_ENABLED	0x01//��������������״̬
#define SS_ONLINE	0x02//����������
#define SS_DELETED	0x80//�������ѱ�ɾ��
		//������״̬
		UCHAR				status;
		//�������
		CHAR				comment[256];
		//��������ʵIP��ַ
		LCXL_SERVER_ADDR	addr;
	} LCXL_SERVER_INFO, *PLCXL_SERVER_INFO;

#ifdef __cplusplus
}
#endif

#endif