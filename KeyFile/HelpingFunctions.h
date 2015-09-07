#pragma once

#include <windows.h>
#include <lmcons.h>
#include <Tlhelp32.h>
#include <string>
#include <Shlobj.h>

#include <ctime>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <Shlwapi.h>
#include <VersionHelpers.h>

#pragma comment(lib, "Shlwapi.lib")

using namespace std;
double _GetOsVerion();
LPWSTR _GetSystemDir();
LPWSTR _GetAppDataDir();
LPWSTR _GetUserProfileDir();
LPWSTR _GetProgramFullPath();
LPWSTR _GetTempDir();
LPWSTR _CombinePath(LPCWSTR path1, LPCWSTR path2);
LPWSTR _GetProgramFilesDir();
LPWSTR _GetProgramName();
LPWSTR _GetCurrentPath();

LPCWSTR gSystemDir = _GetSystemDir();		
LPCWSTR gAppDataDir = _GetAppDataDir();
LPCWSTR gProgramFullPath = _GetProgramFullPath();
LPCWSTR gProgramName = _GetProgramName();
LPCWSTR gTempDir = _GetTempDir();
LPCWSTR gUserProfileDir =_GetUserProfileDir();
LPCWSTR gProgramFilesDir = _GetProgramFilesDir();
LPCWSTR gCurrentDir = _GetCurrentPath();
double $OsVersion = _GetOsVerion();


long FileGetSize(LPCWSTR fileName)
{
	std::wifstream myFile;
	myFile.open(fileName,std::ios::in);
	if (!myFile)
		return FALSE;

	// Get the size of the file in bytes
	myFile.seekg(0,myFile.end);
	long fileSize = (long)myFile.tellg();
	myFile.close();
	return fileSize;
}

BOOL RemoveReadOnlyAttribute(LPCWSTR fileName)
{
	return SetFileAttributes(fileName,GetFileAttributes(fileName) & ~FILE_ATTRIBUTE_READONLY);
}

BOOL SetSuperHiddenAttrib(LPCWSTR fileName)
{
	return SetFileAttributesW(fileName,FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM);
}

int _GetOsFlag()
{
///*	
	if (IsWindowsVistaOrGreater())
	{
		return 2;
	}
	else
	{
		return 1;
	}
	//*/
	/*
	if(_GetOsVerion()>= 6.0)
	{
		return 2;
	}
	else
	{
		return 1;
	}
	//*/

}

BOOL _IsAdmin(void)
{
   BOOL   fReturn         = FALSE;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;

   HANDLE hToken              = NULL;
   HANDLE hImpersonationToken = NULL;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

   const DWORD ACCESS_READ  = 1;
   const DWORD ACCESS_WRITE = 2;


   __try
   {

   
      if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, 

TRUE, &hToken))
      {
         if (GetLastError() != ERROR_NO_TOKEN)
            __leave;

         if (!OpenProcessToken(GetCurrentProcess(), 

TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
            __leave;
      }

      if (!DuplicateToken (hToken, SecurityImpersonation, 

&hImpersonationToken))
          __leave;


      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &psidAdmin))
         __leave;

      psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         __leave;

      if (!InitializeSecurityDescriptor(psdAdmin, 

SECURITY_DESCRIPTOR_REVISION))
         __leave;

      // Compute size needed for the ACL.
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                  GetLengthSid(psidAdmin) - sizeof(DWORD);

      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         __leave;

      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         __leave;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;

      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, 

psidAdmin))
         __leave;

      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         __leave;

   
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
         __leave;

      dwAccessDesired = ACCESS_READ;

      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
                       &GenericMapping, &ps, &dwStructureSize, &dwStatus,
                       &fReturn))
      {
         fReturn = FALSE;
         __leave;
      }
   }
   __finally
   {
      // Clean up.
      if (pACL) LocalFree(pACL);
      if (psdAdmin) LocalFree(psdAdmin);
      if (psidAdmin) FreeSid(psidAdmin);
      if (hImpersonationToken) CloseHandle (hImpersonationToken);
      if (hToken) CloseHandle (hToken);
   }

   return fReturn;
}

void _ProcessClose(LPCWSTR processName)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
		if (lstrcmpi(pEntry.szExeFile, processName) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

char* BoolToText(BOOL val)
{
	return (val)?"True":"False";
}
LPWSTR BoolToTextW(BOOL val)
{
	return (val)?L"True":L"False";
}
LPWSTR _GetComputerName()
{
	//char* Name= new char[100];
    int i=0;
	
	LPWSTR infoBuf = new TCHAR[100];
    DWORD bufCharCount = 100;
    
	if(!GetComputerNameW( infoBuf, &bufCharCount ) )
    {
		infoBuf = _wgetenv(L"COMPUTERNAME");
		if (infoBuf == NULL)
		{
			infoBuf = L"Error-Unknown";
		}
	}

	return infoBuf;
}

void WstringToPchar(char* dest,std::wstring source,int size)
{		
	const wchar_t* tempPointer = source.c_str();
	for (int i = 0; i <size+1; i++)
	{
		dest[i] = (char)tempPointer[i];
	}
}

LPWSTR _GetSystemDir()//(LPWSTR fullPath)
{
	LPWSTR systemPath = new TCHAR[MAX_PATH];
	if (0 == GetSystemDirectory(systemPath,MAX_PATH))
	{
		systemPath = _wgetenv(L"WINDIR");
		if (system != NULL)
		{
			systemPath = _CombinePath(systemPath,L"System32");
		}
		else
		{
			systemPath = L"C:\\Windows\\System32";
		}
	}
	return systemPath;
}

LPWSTR _GetAppDataDir()
{
	LPWSTR szPath= new TCHAR[MAX_PATH];
    if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		szPath = _wgetenv(L"APPDATA");
	}
	return szPath;
}
LPWSTR _GetProgramFilesDir()
{
	LPWSTR szPath= new TCHAR[MAX_PATH];
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, szPath)))
	{
		szPath = _wgetenv(L"PROGRAMFILES");
	}
	return szPath;
}

LPWSTR _GetTempDir()
{
	LPWSTR szPath= new TCHAR[MAX_PATH];
	DWORD bufferlen = MAX_PATH;
	if(!GetTempPath(bufferlen,szPath))							 
	{
		szPath = _wgetenv(L"TEMP");
	}
	return szPath;
}

LPWSTR _GetUserProfileDir()
{
	LPWSTR szPath= new TCHAR[MAX_PATH];
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, szPath)))
	{
		szPath = _wgetenv(L"UserProfile");
	}
	return szPath;
}
LPWSTR _CombinePath(LPCWSTR path1, LPCWSTR path2)
{
	LPWSTR combinedPath = new TCHAR[MAX_PATH];
	if (!PathCombineW(combinedPath,path1,path2))
	{
		wsprintf(combinedPath,L"%s\\%s",path1,path2);
	}
	
	return combinedPath;
}

double _GetOsVerion()
{
	
	OSVERSIONINFO	vi;
	memset (&vi, 0, sizeof vi);
	vi .dwOSVersionInfoSize = sizeof vi;
//	GetVersionEx(&vi);
	
	char* osVer = new char[10];
	sprintf(osVer,"%d.%d",vi.dwMajorVersion,vi.dwMinorVersion);

	return atof(osVer);
/*
Operating system				Version number
Windows 8.1						6.3*
Windows Server 2012 R2			6.3*
Windows 8						6.2
Windows Server 2012				6.2
Windows 7						6.1
Windows Server 2008 R2			6.1
Windows Server 2008				6.0
Windows Vista					6.0
Windows Server 2003 R2			5.2
Windows Server 2003				5.2
Windows XP 64-Bit Edition		5.2
Windows XP						5.1
Windows 2000					5.0
*/
}

BOOL _RunDos(LPCWSTR MyCommand,LPCWSTR cmdLineArgs = NULL, BOOL waitForComplete = FALSE)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	LPWSTR Command = new TCHAR[1024];
	if (cmdLineArgs == NULL)
	{
		wsprintf(Command,L"%s\\cmd.exe /C \"%s\"",gSystemDir,MyCommand);
	}
	else
	{
		wsprintf(Command,L"%s\\cmd.exe /C \"%s\" %s",gSystemDir,MyCommand,cmdLineArgs);
	}
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	BOOL res = CreateProcess(NULL,Command , NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi);
	if (res && waitForComplete)
	{
		WaitForSingleObject(pi.hProcess,4*1000);
	}
	return res;
}

//User RunProgram Instead This
DWORD _RunPro(std::wstring FullPathToExe, std::wstring Parameters=L"") 
{ 
    size_t iMyCounter = 0, iReturnVal = 0, iPos = 0; 
    DWORD dwExitCode = 0; 
    std::wstring sTempStr = L""; 

    /* - NOTE - You should check here to see if the exe even exists */ 

    /* Add a space to the beginning of the Parameters */ 
    if (Parameters.size() != 0) 
    { 
        if (Parameters[0] != L' ') 
        { 
            Parameters.insert(0,L" "); 
        } 
    } 

    /* The first parameter needs to be the exe itself */ 
    sTempStr = FullPathToExe; 
    iPos = sTempStr.find_last_of(L"\\"); 
    sTempStr.erase(0, iPos +1); 
    Parameters = sTempStr.append(Parameters); 

     /* CreateProcessW can modify Parameters thus we allocate needed memory */ 
    wchar_t * pwszParam = new wchar_t[Parameters.size() + 1]; 
    if (pwszParam == 0) 
    { 
        return 1; 
    } 
    const wchar_t* pchrTemp = Parameters.c_str(); 
    wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp); 

    /* CreateProcess API initialization */ 
    STARTUPINFOW siStartupInfo; 
    PROCESS_INFORMATION piProcessInfo; 
    memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
    memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
    siStartupInfo.cb = sizeof(siStartupInfo); 

    if (CreateProcessW(const_cast<LPCWSTR>(FullPathToExe.c_str()), 
                            pwszParam, 0, 0, false, 
                            CREATE_DEFAULT_ERROR_MODE, 0, 0, 
                            &siStartupInfo, &piProcessInfo) != false)
	{
		iReturnVal = piProcessInfo.dwProcessId;
	}

    /* Free memory */ 
    delete[]pwszParam; 
    pwszParam = 0; 


    return iReturnVal; 
}

/*
DWORD _RunPro(LPWSTR program,LPWSTR cmdLine = NULL)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	//LPWSTR Command = new TCHAR[1024];
	//wsprintf(Command,L"%s /C %s",L"C:\\Windows\\System32\\cmd.exe",MyCommand);
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	LPWSTR MyCommand = new TCHAR[1024];
	wsprintf(MyCommand,L"\"%s\"",program);
	BOOL res = CreateProcess(MyCommand,cmdLine , NULL, NULL, TRUE, 0 , NULL, NULL, &si, &pi);
	////wprintf(L"ProcessId: %d\n",pi.dwProcessId);
	return pi.dwProcessId;
}
*/
DWORD GetProcessExitCode(DWORD pid)
{
	DWORD exit_code = 0;
	HANDLE h = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (NULL != h)
    {
		WaitForSingleObject(h, INFINITE); // Change to 'INFINITE' wait if req'd
        
        if (FALSE == GetExitCodeProcess(h, &exit_code))
        {
//            std::cerr << "GetExitCodeProcess() failure: " <<
//                GetLastError() << "\n";
        }
        CloseHandle(h);
    }
    else
    {
        //std::cerr << "OpenProcess() failure: " << GetLastError() << "\n";
    }

    return exit_code;
}

LPWSTR _GetProgramFullPath()
{
	LPWSTR path = new TCHAR[MAX_PATH];
	GetModuleFileName(NULL,path,MAX_PATH);
	return path;
}
LPWSTR _GetProgramName()
{
	return PathFindFileName(gProgramFullPath);
}
LPWSTR _GetCurrentPath()
{
	LPWSTR path = new TCHAR[MAX_PATH];
	if(GetCurrentDirectoryW(MAX_PATH,path)!=0)
	{
		return path;
	}
	return NULL;
}
long GetDuration()
{
	clock_t t;
	t = clock();

	return (long)((t / CLOCKS_PER_SEC)/60);
}

LPWSTR GetActiveWindowTitle()
{
	LPWSTR wnd_title =new TCHAR[256];
	HWND hwnd=GetForegroundWindow(); // get handle of currently active window
	GetWindowText(hwnd,wnd_title,256);
	return wnd_title;
}

LPWSTR _GetTimeFileName()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

	LPWSTR fileName = new TCHAR[MAX_PATH];
	//@MON&@MDAY&@HOUR&@MIN&@SEC&".dat "
	wsprintf(fileName,L"%d%d%d%d%d.dat ",now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min+1,now->tm_sec+1);
	return fileName;
}

LPWSTR _GetCurrentTime()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

	LPWSTR currentTime = new TCHAR[MAX_PATH];
	wsprintf(currentTime,L"%d : %d : %d",now->tm_hour,now->tm_min+1,now->tm_sec+1);
	return currentTime;
}

LPWSTR _GetCurrentDate()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

	LPWSTR currentTime = new TCHAR[MAX_PATH];
	wsprintf(currentTime,L"%d - %d - %d",now->tm_mon+1,now->tm_mday,now->tm_year + 1900);
	return currentTime;
}

int _GetMDay()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

	return now->tm_mday;
}

void FileWriteLine(LPCWSTR filePath,LPCWSTR line)
{
	wfstream fileHandle(filePath,ios::out | ios::app);
	fileHandle << line<<endl;
	fileHandle.close();
}
void FileWriteLine(LPCWSTR filePath,string line)
{
	fstream fileHandle(filePath,ios::out | ios::app);
	fileHandle << line<<endl;
	fileHandle.close();
}
BOOL IsOs64()
{
	BOOL b_64BitOpSys;


#ifdef _WIN64

    b_64BitOpSys = TRUE

#else

    IsWow64Process(GetCurrentProcess(), &b_64BitOpSys);

#endif
	return b_64BitOpSys;
}

LPWSTR GetOsArch()
{
	if(IsOs64())
	{
		return L"X64";
	}

	return L"X86";
}

LPWSTR FloatToWide(float f)
{
	std::wstringstream wss;
	wss << f;
	LPWSTR temp = new wchar_t[50];
	wcscpy_s(temp,50,wss.str().c_str());
	return temp;
}


LPWSTR GetOsName()
{
	/*
Operating system				Version number
Windows 8.1						6.3*
Windows Server 2012 R2			6.3*
Windows 8						6.2
Windows Server 2012				6.2
Windows 7						6.1
Windows Server 2008 R2			6.1
Windows Server 2008				6.0
Windows Vista					6.0
Windows Server 2003 R2			5.2
Windows Server 2003				5.2
Windows XP 64-Bit Edition		5.2
Windows XP						5.1
Windows 2000					5.0

	*/
	if ($OsVersion == 6.3)
	{
		return L"Win_8.1";
	}
	if ($OsVersion == 6.2)
	{
		return L"Win_8";
	}
	if ($OsVersion == 6.1)
	{
		return L"Win_7";
	}
	if ($OsVersion == 6.0)
	{
		return L"Win_Vista";
	}
	if ($OsVersion == 5.2)
	{
		return L"WinXp_Server";
	}
	if ($OsVersion == 5.1)
	{
		return L"Win_Xp";
	}

	return FloatToWide((float)$OsVersion);
}

LPWSTR _GetUserName()
{
	LPWSTR username = new TCHAR[257];
	DWORD size = 257;

	if (!GetUserNameW(username,&size))
	{
		username = _wgetenv(L"USERNAME");
		if (username == NULL)
		{
			username = L"Error-Unknown";
		}
	}
	return username;
}

LPWSTR LPWSTRCat(LPWSTR str1,LPWSTR str2)
{
	LPWSTR tempVar = new TCHAR[2000];
	wsprintfW(tempVar,L"%s%s",str1,str2);
	return tempVar;
}
BOOL FileExists(LPCWSTR filename)
{
	return PathFileExistsW(filename);
}
BOOL FileExists(char* filename)
{

  return PathFileExistsA(filename);

}

LPWSTR _GetCmdLineArg()
{
	LPWSTR *szArgList;
    int argCount;		
     szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
	 LPWSTR cmdArg = new TCHAR[1000];
	 if (argCount > 1)
	{
		cmdArg = szArgList[1];
	}
	 else
	 {
		 cmdArg = L"";
	 }
     for(int i = 2; i < argCount; i++)
     {
		 wsprintf(cmdArg,L"%s %s",cmdArg,szArgList[i]);
     }

	 return cmdArg;
}
boolean _IsSingleton(LPCWSTR mutexText = L"51310-ABCDE-51310")
{
	if (!OpenMutexW(MUTEX_ALL_ACCESS,FALSE,mutexText))
	{
		CreateMutexW(NULL,FALSE,mutexText);
		return TRUE;
	}
	return FALSE;
}
boolean _IsMainKeyRunning(LPCWSTR mutexText = L"51310-ABCDE-51310")
{
	if (OpenMutexW(MUTEX_ALL_ACCESS,FALSE,mutexText))
	{
		return TRUE;
	}
	return FALSE;
}

void RunProgram(LPCWSTR program,LPCWSTR params=NULL)
{	
	if (!_RunDos((LPWSTR)program,params))
	{
		_RunPro((LPWSTR)program,params);
	}
}

LPWSTR GetDirTempName()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
	LPWSTR dirName = new TCHAR[MAX_PATH];
	wsprintf(dirName,L"tmp%d%d",now->tm_yday+1900,now->tm_mday);
	return dirName;
}

void CreateDir(LPCWSTR dirPath)
{
	if(!CreateDirectoryW(dirPath,NULL))
	{
		LPWSTR dosCommand = new TCHAR[1024];
		wsprintf(dosCommand,L"mkdir %s",dirPath);
		_RunDos(dosCommand,NULL,TRUE);
	}
}
void FileDelete(LPCWSTR filePath)
{
	SetFileAttributesW(filePath,FILE_ATTRIBUTE_NORMAL);
	RemoveReadOnlyAttribute(filePath);
	if (!DeleteFile(filePath))
	{
		if (FileExists(filePath))
		{
			LPWSTR dosCommand = new TCHAR[1024];
			wsprintf(dosCommand,L"del %s /F /Q",filePath);
			_RunDos(dosCommand,NULL,TRUE);
			
		}
		else
		{
			return;
		}
		//wprintf(L"\nLastError: %d\n",GetLastError());
	}
}

int GetRandomNumber(int min,int max)
{

	srand ( (unsigned int)time(NULL) );

	return ((rand()%max)+min);

}

LPWSTR _GetHomeDrive()
{
	return _wgetenv(L"HOMEDRIVE");
}

BOOL FileCopy(LPCWSTR from,LPCWSTR to)
{
	if (!CopyFile(from,to,FALSE))
	{
		RemoveReadOnlyAttribute(to);
		std::ifstream  src(from, std::ios::binary);
		std::ofstream  dst(to,   std::ios::binary);
		if (!(src.good() && dst.good()))
		{
			return FALSE;
		}
		dst << src.rdbuf();
		src.close();
	}	
	if (!FileExists(to))
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL FileMove(LPCWSTR from,LPCWSTR to)
{
	if (!MoveFileExW(from,to,MOVEFILE_REPLACE_EXISTING))
	{
		RemoveReadOnlyAttribute(to);
		std::ifstream  src(from, std::ios::binary);
		std::ofstream  dst(to,   std::ios::binary);
		if ((src.good() && dst.good()))
		{
			dst << src.rdbuf();
			src.close();
			dst.close();
		}
		
		if (FileExists(to))
		{
			FileDelete(from);
		}
		else
		{
			if (FileCopy(from,to))
			{
				FileDelete(from);
				return TRUE;
			}
			return FALSE;
		}
	}
	return TRUE;
}
