#pragma once
#include <windows.h>
#include <string>
#include <atlbase.h>

void _RegWrite(HKEY keyName,LPCWSTR subKeyName,DWORD type,LPCWSTR valueName,wstring strValue)
{

	HKEY hkey;
	long regOpenResult;
	regOpenResult = RegOpenKeyEx(keyName,subKeyName, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hkey);

	RegSetValueEx(hkey,valueName,0,type,(const BYTE*) strValue.c_str(), MAX_PATH);
	RegCloseKey(hkey);
}

void _RegWrite(HKEY keyName,LPCWSTR subKeyName,DWORD type,LPCWSTR valueName,int strValue)
{

	HKEY hkey;
	long regOpenResult;
	regOpenResult = RegOpenKeyEx(keyName,subKeyName, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hkey);

	RegSetValueEx(hkey,valueName,0,type,(const BYTE*) &strValue, sizeof(strValue));
	RegCloseKey(hkey);
}

BOOL GetDWORDRegKey(HKEY keyName,const std::wstring &subKeyName, const std::wstring &strValueName, DWORD &nValue)
{
	HKEY hKey;
	DWORD type;
	LONG lRes = RegOpenKeyExW(keyName,subKeyName.c_str(), 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS != lRes)
	{
		return FALSE;
	}
    DWORD dwBufferSize(sizeof(DWORD));
    DWORD nResult(0);
    LONG nError = ::RegQueryValueExW(hKey,
        strValueName.c_str(),
        0,
		&type,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
		RegCloseKey(hKey);
		if (type!=REG_DWORD && type!=REG_DWORD_BIG_ENDIAN && type!=REG_DWORD_LITTLE_ENDIAN && type!=REG_QWORD && type!=REG_QWORD_LITTLE_ENDIAN)
		{
			return FALSE;
		}

        nValue = nResult;
		return TRUE;
    }
	RegCloseKey(hKey);
	return FALSE;
}


BOOL GetBoolRegKey(HKEY keyName,const std::wstring &subKeyName, const std::wstring &strValueName, bool &bValue)
{
    DWORD nResult(0);
	LONG nError = GetDWORDRegKey(keyName,subKeyName, strValueName.c_str(), nResult);
	if (nError)
    {
        bValue = (nResult != 0) ? true : false;
		return TRUE;
    }
	return FALSE;
}



BOOL GetStringRegKey(HKEY keyName,const std::wstring &subKeyName ,const std::wstring &strValueName, std::wstring &strValue)
{
	HKEY hKey;
	DWORD type;
	LONG lRes = RegOpenKeyExW(keyName,subKeyName.c_str(), 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS != lRes)
	{
		return FALSE;
	}
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, &type, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
		RegCloseKey(hKey);
		if (type!=REG_SZ && type!=REG_EXPAND_SZ && type!=REG_MULTI_SZ)
		{
			return FALSE;
		}
        strValue = szBuffer;
		return TRUE;
    }
	RegCloseKey(hKey);
	return FALSE;
}

void DeleteRegValue(HKEY key,wstring subKey,wstring value)
{
	CRegKey reg;
	reg.Open(key,subKey.c_str(),KEY_ALL_ACCESS);
	reg.DeleteValue(value.c_str());
	reg.Close();
}

void _HideSystemFiles()
{
	DWORD lastError = GetLastError();
	SHELLSTATE ss;
	ZeroMemory(&ss,sizeof(ss));
	ss.fShowSysFiles=FALSE;
	ss.fShowSuperHidden=FALSE;
	SHGetSetSettings(&ss,SSF_SHOWSUPERHIDDEN|SSF_SHOWSYSFILES,TRUE);

	_RegWrite(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",REG_DWORD,L"ShowSuperHidden",0);
}