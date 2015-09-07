#pragma once
#include <Windows.h>
#include <fstream>

/*
#define RES_SEND     L"EXESEND"
#define RES_KEY      L"EXEKEY"
#define RES_LAU      L"EXELAU"
#define RES_MY		 L"MYEXE"
*/
int ExtendResError = 0;
BOOL _DecryptResAndSaveToFile(LPCWSTR lpszOutputPath,int resName,int encKey)
{
	HRSRC resHandle = FindResource(NULL, MAKEINTRESOURCE(resName), MAKEINTRESOURCE(10));
	if (resHandle == NULL || resHandle == 0)
	{
		return FALSE;
	}

	DWORD resSize = SizeofResource(NULL, resHandle);
	PVOID resMemPointer = LockResource(LoadResource(0, resHandle));

	if (resMemPointer == NULL && resSize < 1)
	{
		return FALSE;
	}
	
	//Resource Decrypt
	char *resData = new char[resSize];
	memcpy(resData, resMemPointer, resSize);
	for (DWORD i = 0; i < resSize; i++)
	{

		resData[i] = (char)resData[i] ^ encKey;
	}

	std::ofstream f(lpszOutputPath, std::ios::out | std::ios::binary);
	f.write(resData, resSize);
	f.close();

	return TRUE;
}
BOOL _SaveResToFile(LPCWSTR lpszOutputPath,LPCWSTR resName, int resType = 10, int resLang = 2057, LPCWSTR DLL=NULL,BOOL isSuperHidden = FALSE)
{
	HMODULE hInstance = NULL;
	HRSRC infoBlock = NULL;
	HGLOBAL globalMemoryBlock = NULL;
	LPVOID memoryPointer = NULL;
	DWORD gResSize = 0;

	//if Null Then it gets its own resource
	if (DLL != NULL)
	{
		hInstance = LoadLibraryExW(DLL,NULL,LOAD_LIBRARY_AS_DATAFILE);
		if (hInstance == NULL)
		{
			return FALSE;
		}
	}

	if (resLang != 0)
	{
		infoBlock = FindResourceExW(hInstance,MAKEINTRESOURCEW(resType),resName,resLang);
	}
	else
	{
		infoBlock = FindResourceW(hInstance,MAKEINTRESOURCEW(resType),resName);
	}
	if (infoBlock == NULL)
	{
		if (DLL != NULL)
		{
			FreeLibrary(hInstance);
		}
		return FALSE;
	}
	
	gResSize = SizeofResource(hInstance,infoBlock);
	if (gResSize == 0)
	{
		if (DLL != NULL)
		{
			FreeLibrary(hInstance);
		}
		return FALSE;
	}

	globalMemoryBlock = LoadResource(hInstance,infoBlock);
	if (globalMemoryBlock == NULL)
	{
		if (DLL != NULL)
		{
			FreeLibrary(hInstance);
		}
		return FALSE;
	}

	memoryPointer = LockResource(globalMemoryBlock);
	if (memoryPointer == NULL)
	{
		if (DLL != NULL)
		{
			FreeLibrary(hInstance);
		}
		return FALSE;
	}
	DWORD fileAttribs = FILE_ATTRIBUTE_NORMAL;
	if (isSuperHidden)
	{
		fileAttribs |= FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
	}
	HANDLE hFile = CreateFile  (lpszOutputPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,fileAttribs,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
    DWORD dwByteWritten;
	WriteFile(hFile, memoryPointer , gResSize , &dwByteWritten , NULL);
    CloseHandle(hFile);
    FreeResource(globalMemoryBlock);


	if (DLL != NULL)
	{
		FreeLibrary(hInstance);
	}

	return TRUE;
}

BOOL _ExtractFromRes(LPCWSTR destFile,LPCWSTR resName)
{
	//////wprintf(L"Extract Res \"%s\" To: %s\n",resName,file);
	return _SaveResToFile(destFile,resName, 10, 2057,NULL);
}

BOOL _ResourceExists(LPCWSTR resName,int resType,int resLang = 2057,LPCWSTR exeFile = NULL)
{
	HMODULE hInstance = NULL;
	HRSRC infoBlock = NULL;


	if (exeFile != NULL)
	{
		hInstance = LoadLibraryExW(exeFile,NULL,LOAD_LIBRARY_AS_DATAFILE);
		if (hInstance == NULL)
		{
			return FALSE;
		}
	}

	if (resLang != 0)
	{
		infoBlock = FindResourceExW(hInstance,MAKEINTRESOURCEW(resType),resName,resLang);
	}
	else
	{
		infoBlock = FindResourceW(hInstance,MAKEINTRESOURCEW(resType),resName);
	}

	if (exeFile != NULL)
	{
			FreeLibrary(hInstance);
	}
	if (infoBlock == NULL)
	{		
		return FALSE;
	}

	return TRUE;
}

BOOL _ResourceUpdate(LPCWSTR filePath,LPCWSTR resName, int resType = 10, int resLang = 2057, LPCWSTR resData=NULL)
{
	BYTE *pBuffer = NULL;
	if (!FileExists(filePath))
	{
		return FALSE;
	}
	if(resData == NULL)
	{
		if (!_ResourceExists(resName,resType,resLang,filePath))
		{
			return TRUE;
		}
	}

	DWORD resSize = 0;
	if (resData != NULL)
	{

		// Get the bmp into memory
		HANDLE hFile   = CreateFile(resData, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		resSize = GetFileSize(hFile, NULL);
		//reading image into global memory.

		pBuffer = new BYTE[resSize];
		DWORD dwBytesRead;
		ReadFile(hFile, pBuffer, resSize, &dwBytesRead, NULL);
		CloseHandle(hFile);

	}

	HANDLE handle = BeginUpdateResource(filePath,FALSE);
	if (handle == NULL)
	{
		return FALSE;
	}
	BOOL isUpdated = UpdateResource(handle,MAKEINTRESOURCEW(resType),resName,resLang,(LPVOID)pBuffer,resSize);
	EndUpdateResourceW(handle,!isUpdated);
	return isUpdated;
}

BOOL _DeleteResource(LPCWSTR filePath,LPCWSTR resName, int resType = 10, int resLang = 2057)
{
	return _ResourceUpdate(filePath,resName,resType,resLang,NULL);
}