#ifndef _LCXL_FUNC_H_
#define _LCXL_FUNC_H_
/*
author:
LCX
abstract:
 һЩ�����ĺ���
*/
#define TAG_FILE_BUFFER				'FISR'
//��Ӵ���

PUNICODE_STRING LCXLNewString(IN PUNICODE_STRING sour);
VOID LCXLFreeString(PUNICODE_STRING dest);

__inline PUCHAR LCXLReadFromBuf(IN PUCHAR cur_buf, IN OUT PVOID data, IN INT datalen)
{
	RtlCopyMemory(data, cur_buf, datalen);
	return cur_buf + datalen;
}
//�ӻ������ж�ȡ�ַ�����ע�⣬��������LCXLNewString������str��������֮����Ҫʹ��LCXLFreeString�ͷ�str
__inline PUCHAR LCXLReadStringFromBuf(IN PUCHAR cur_buf, OUT PUNICODE_STRING *str)
{
	UNICODE_STRING data;
	cur_buf = LCXLReadFromBuf(cur_buf, &data.Length, sizeof(data.Length));
	data.MaximumLength = data.Length;
	data.Buffer = (PWCH)cur_buf;
	*str = LCXLNewString(&data);
	return cur_buf + data.Length;
}
//������ȡ�ַ���
_inline PUCHAR LCXLSkipReadStringFromBuf(IN PUCHAR cur_buf)
{
	USHORT str_len;

	cur_buf = LCXLReadFromBuf(cur_buf, &str_len, sizeof(str_len));
	return cur_buf + str_len;
}
//д�����ݵ���������
__inline PUCHAR LCXLWriteToBuf(IN PUCHAR cur_buf, IN PVOID data, IN INT datalen)
{
	RtlCopyMemory(cur_buf, data, datalen);
	return cur_buf + datalen;
}

//д���ַ�������������
__inline PUCHAR LCXLWriteStringToBuf(IN PUCHAR cur_buf, IN PUNICODE_STRING data)
{
	if (data != NULL) {
		return LCXLWriteToBuf(LCXLWriteToBuf(cur_buf, &data->Length, sizeof(data->Length)), data->Buffer, data->Length);
	} else {
		USHORT Length = 0;
		return LCXLWriteToBuf(cur_buf, &Length, sizeof(Length));
	}
	
}

//VOID LCXLTransARP

#endif