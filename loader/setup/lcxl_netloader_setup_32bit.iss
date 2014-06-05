; �ű��� Inno Setup �ű��� ���ɡ�
; �����ĵ���ȡ���� INNO SETUP �ű��ļ�����ϸ���ϣ�

#define MyAppName "LCXL Netloader ���ؾ�����"
#define MyAppVersion "1.5"
#define MyAppPublisher "LCXBox"
#define MyAppURL "http://www.lcxbox.com/"
#define MyConfigExeName "LCXLNetLoaderService.exe"

[Setup]
; ע��: AppId ��ֵ��Ψһʶ���������ı�־��
; ��Ҫ������������ʹ����ͬ�� AppId ֵ��
; (�ڱ������е���˵������� -> ���� GUID�����Բ���һ���µ� GUID)
AppId={{51B1C863-B202-45F1-9468-267D9DD0B489}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=����ļ�.rtf
InfoBeforeFile=LCXL Netloader����.rtf
InfoAfterFile=��װ��.txt
OutputDir=..\bin
OutputBaseFilename=lcxl_netloader_setup_32bit
SetupIconFile=SetupIcon.ico
Compression=lzma
SolidCompression=yes

[Messages]
BeveledLabel=��Ȩ���� (C) 2013-2014 �޳���

[Languages]
Name: "default"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "chinesesimp"; MessagesFile: "compiler:Languages\ChineseSimp.isl"
Name: "chinesetrad"; MessagesFile: "compiler:Languages\ChineseTrad.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "english"; MessagesFile: "compiler:Languages\English.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "greek"; MessagesFile: "compiler:Languages\Greek.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "serbiancyrillic"; MessagesFile: "compiler:Languages\SerbianCyrillic.isl"
Name: "serbianlatin"; MessagesFile: "compiler:Languages\SerbianLatin.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Files]
Source: "..\bin\Win32\LCXLNetLoaderService.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\Win32\netloader.inf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\Win32\netloader.sys"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\Win32\netloader_interface.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\Win32\windows��׼��\msvcp120.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\Win32\windows��׼��\msvcr120.dll"; DestDir: "{app}"; Flags: ignoreversion
; ע��: ��Ҫ���κι����ϵͳ�ļ�ʹ�� "Flags: ignoreversion"

[Run]
Filename: "{app}\{#MyConfigExeName}"; Parameters: "install";StatusMsg: "���ڰ�װ��������ط���..."
Filename: "{app}\{#MyConfigExeName}"; Parameters: "startservice";StatusMsg: "����������ط���..."
[UninstallRun]
Filename: "{app}\{#MyConfigExeName}"; Parameters: "uninstall";StatusMsg: "����ж����������ط���..."

[Code]
const
  MF_BYPOSITION=$400;
  function DeleteMenu(HMENU: HWND; uPosition: UINT; uFlags: UINT): BOOL; external 'DeleteMenu@user32.dll stdcall';
  function GetSystemMenu(HWND: hWnd; bRevert: BOOL): HWND; external 'GetSystemMenu@user32.dll stdcall';

procedure InitializeWizard();
begin
 //ɾ��������Ϣ
  DeleteMenu(GetSystemMenu(wizardform.handle,false),8,MF_BYPOSITION);
  DeleteMenu(GetSystemMenu(wizardform.handle,false),7,MF_BYPOSITION);
end;

function UninstallNeedRestart(): Boolean;
begin
  Result := True;
end;