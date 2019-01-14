#include "stdafx.h"
#include "lum-decoder.h"
#include "MFReader.h"
#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

VOID
CreateWaveFile(LPVOID pData, size_t DataSize, HANDLE hFile, WAVEFORMATEX* waveFormat)
{
	DWORD dwSampleCount = 0;
	LARGE_INTEGER largeInt = { NULL };

	// set header
	DWORD dwHead[] = { MAKEFOURCC('R', 'I', 'F', 'F'), NULL, MAKEFOURCC('W', 'A', 'V', 'E'), MAKEFOURCC('f', 'm', 't', ' '), sizeof(WAVEFORMATEX) };
	DWORD dwHeadData[] = { MAKEFOURCC('d', 'a', 't', 'a'), NULL };
	DWORD dwHeaderSize = 0;

	// write first data
	WriteFile(hFile, dwHead, sizeof(dwHead), &dwSampleCount, NULL);
	WriteFile(hFile, waveFormat, sizeof(WAVEFORMATEX), &dwSampleCount, NULL);
	WriteFile(hFile, dwHeadData, sizeof(dwHeadData), &dwSampleCount, NULL);

	// get size of header 
	dwHeaderSize = sizeof(dwHead) + sizeof(WAVEFORMATEX) + sizeof(dwHeadData);

	// write full audio data
	WriteFile(hFile, pData, DataSize, &dwSampleCount, NULL);

	// get size of header
	largeInt.QuadPart = dwHeaderSize - sizeof(DWORD);

	// set pointer pos to write file size
	SetFilePointerEx(hFile, largeInt, NULL, FILE_BEGIN);
	WriteFile(hFile, &DataSize, sizeof(DWORD), &dwSampleCount, NULL);

	DWORD dwRIFFFileSize = dwHeaderSize + DataSize - 8;
	largeInt.QuadPart = sizeof(FOURCC);

	// finally write a file
	SetFilePointerEx(hFile, largeInt, NULL, FILE_BEGIN);
	WriteFile(hFile, &dwRIFFFileSize, sizeof(dwRIFFFileSize), &dwSampleCount, NULL);

	FlushFileBuffers(hFile);

	HeapFree(GetProcessHeap(), 0, pData);
	CloseHandle(hFile);
}

DWORD
WINAPIV
ProcessProc(LPVOID pData)
{
	PROCESS_PROC* pProc = (PROCESS_PROC*)pData;
	return Process(pProc->FileTypeInput, pProc->FileTypeOutput, pProc->lpPathIn, pProc->lpPathOut);
}

BOOL
Process(
	FILE_TYPE FileTypeInput,
	FILE_TYPE FileTypeOutput,
	LPCWSTR lpPathIn,
	LPCWSTR lpPathOut
)
{
	BOOL isDecoded = FALSE;
	DWORD dwSampleCount = 0;
	LPVOID pData = NULL;
	size_t FileSize = 0;
	static BOOL isLoaded = FALSE;
	LPVOID pOutData = NULL;
	WAVEFORMATEX waveFormat = { 0 };
	FILE_TYPE type = UNKNOWN_TYPE;
	WCHAR lpTemp[MAX_PATH] = { 0 };
	SYSTEMTIME sysTime = { 0 };
	DWORD prevTime = timeGetTime();

	if (FileTypeInput == MP3_TYPE)
	{
		memcpy(lpTemp, GetTempDir(), MAX_PATH * 2);
		GetSystemTime(&sysTime);

		__try
		{
			_snwprintf_s(
				lpTemp, 
				MAX_PATH, 
				MAX_PATH,
				L"%s\\temp_%u_%u_%u_%u_%u_%u.raw",
				lpTemp,
				sysTime.wYear, 
				sysTime.wMonth, 
				sysTime.wDay, 
				sysTime.wHour, 
				sysTime.wMinute,
				sysTime.wMilliseconds
			);
		}
		__except (EXCEPTION_CONTINUE_EXECUTION)
		{
		}

		if (!isLoaded)
		{
			InitMFReader();
		}

		if (!IsSupportedByWMF(lpPathIn))
		{
			DeleteMFReader();
		}

		if (DecodeByWMF(lpPathIn, lpTemp, &FileSize, &waveFormat, &type))
		{
			isDecoded = TRUE;
		}
	}

	if (isDecoded)
	{
		LONG toPos = 0;
		LARGE_INTEGER largeInt = { NULL };

		HANDLE hFile = CreateFileW(lpTemp, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		pData = HeapAlloc(GetProcessHeap(), 0, FileSize);

		ReadFile(hFile, pData, FileSize, &dwSampleCount, NULL);
		DeleteFileW(lpTemp);
		CloseHandle(hFile);

		hFile = CreateFileW(lpPathOut, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		// get null position
		largeInt.QuadPart = 0;

		// set pointer pos to write file size
		SetFilePointerEx(hFile, largeInt, NULL, FILE_BEGIN);

		if (FileTypeOutput == WAV_TYPE)
		{
			CreateWaveFile(pData, FileSize, hFile, &waveFormat);
		}
	}

	WCHAR szMsg[256] = { 0 };
	DWORD msSecs = timeGetTime() - prevTime;
	_snwprintf_s(szMsg, 256, 256, L"File decoding is complete. (%u.%u seconds)", msSecs / 1000, msSecs % 1000);
	MessageBoxW(NULL, szMsg, L"", MB_ICONINFORMATION | MB_OK);

	return TRUE;
}
