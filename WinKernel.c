/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* lum-decoder - open-source Windows transcoder
* MIT-License
**********************************************************
* Module Name: Windows kernel
**********************************************************
* WinKernel.c
* Windows kernel implementation
**********************************************************/
#include "stdafx.h"
#include "lum-decoder.h"
#include <process.h>

WCHAR Buffer[260] = { 0 };

BOOL
OpenDialog(LPCWSTR lpPath)
{
	// set params to struct
	OPENFILENAMEW openFile = { NULL };
	openFile.lStructSize = sizeof(OPENFILENAMEW);
	openFile.nMaxFile = MAX_PATH;
	openFile.lpstrFile = lpPath;
	openFile.lpstrFilter = 
		L"AAC media (.m4a)\0*.m4a\0MPEG-4 video (.mp4)\0*.mp4\0MPEG-3 audio (.mp3)\0*.mp3\0Free Lossless Audio Codec audio (.flac)\0*.flac\0Apple Lossless Audio Codec audio (.alac)\0*.alac\0OPUS audio (.opus)\0*.opus\0Vorbis audio (.ogg)\0*.ogg\0PCM audio (.wav)\0*.wav\0AIFF audio (.aiff)\0*.aiff\0AIF audio (.aif)\0*.aif\0AVI video (.avi)\0*.avi\0All files\0ms*.*";
	openFile.lpstrTitle = L"Open media files";
	openFile.nFilterIndex = 1;
	openFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// open file dialog
	return GetOpenFileNameW(&openFile);
}

FILE_TYPE
SaveDialog(LPCWSTR lpPath)
{
	// set params to struct
	OPENFILENAMEW openFile = { NULL };
	openFile.lStructSize = sizeof(OPENFILENAMEW);
	openFile.nMaxFile = MAX_PATH;
	openFile.lpstrFile = lpPath;
	openFile.lpstrFilter =
		L"AAC media (.m4a)\0*.m4a\0MPEG-4 video (.mp4)\0*.mp4\0MPEG-3 audio (.mp3)\0*.mp3\0Free Lossless Audio Codec audio (.flac)\0*.flac\0Apple Lossless Audio Codec audio (.alac)\0*.alac\0OPUS audio (.opus)\0*.opus\0Vorbis audio (.ogg)\0*.ogg\0PCM audio (.wav)\0*.wav\0AIFF audio (.aiff)\0*.aiff\0AIF audio (.aif)\0*.aif\0AVI video (.avi)\0*.avi\0All files\0ms*.*";
	openFile.lpstrTitle = L"Open media files";
	openFile.nFilterIndex = 1;
	
	openFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// open file dialog
	if (GetSaveFileNameW(&openFile))
	{
		switch (openFile.nFilterIndex)
		{
		case 1:
		case 2:
			return AAC_TYPE;
		case 3:
			return MP3_TYPE;
		case 4:
			return FLAC_TYPE;
		case 5:
			return ALAC_TYPE;
		case 6:
			return OPUS_TYPE;
		case 7:
			return OGG_TYPE;
		case 8:
		case 10:
			return WAV_TYPE;
		case 9:
			return AIFF_TYPE;
		default:
			return UNKNOWN_TYPE;
			break;
		}
	}

	return UNKNOWN_TYPE;
}

FILE_TYPE
IsFileIsAudio(LPCWSTR PathToFile)
{
	int StringSize = 0;
	int TypeSize = 0;
	LPCWSTR szTypes[] = {
	L"mp3", L"MP3", L"wav", L"WAV", L"flac", L"FLAC", L"aiff", L"AIFF", L"aif", L"AIF", L"ogg", L"OGG", L"opus", L"OPUS", L"alac", L"ALAC", L"aac", L"AAC", L"m4a", L"M4A" };

	if (PathToFile)
	{
		for (size_t i = wcslen(PathToFile); i > 0; i--)
		{
			WCHAR cbSymbol = PathToFile[i];

			if (cbSymbol == L'.') { break; }

			TypeSize++;
		}

		if (TypeSize)
		{
			for (size_t i = 0; i < 20; i++)
			{
				LPCWSTR CurrentType = szTypes[i];
				LPCWSTR FileType = (PathToFile + wcslen(PathToFile) - (TypeSize - 1));
				if (!wcscmp(CurrentType, FileType))
				{
					switch (i)
					{
					case 0:
					case 1:
						return MP3_TYPE;
					case 2:
					case 3:
						return WAV_TYPE;
					case 4:
					case 5:
						return FLAC_TYPE;
					case 6:
					case 7:
						return AIFF_TYPE;
					case 8:
					case 9:
						return AIFF_TYPE;
					case 10:
					case 11:
						return OGG_TYPE;
					case 12:
					case 13:
						return OPUS_TYPE;
					case 14:
					case 15:
						return ALAC_TYPE;
					case 16:
					case 17:
						return AAC_TYPE;
					case 18:
					case 19:
						return AAC_TYPE;
					default:
						return UNKNOWN_TYPE;
						break;
					}
				}
			}
		}
	}

	return UNKNOWN_TYPE;
}

LPVOID
ReAllocFile(LPVOID pVoid1, LPVOID pVoid2, size_t size1, size_t size2)
{
	LPVOID pData = HeapAlloc(GetProcessHeap(), 0, size1 + size2);

	if (pVoid1) { memcpy(pData, pVoid1, size1); }
	if (pVoid2) { memcpy((LPVOID)((size_t)(pData)+size1), pVoid2, size2); }

	if (pVoid1) { HeapFree(GetProcessHeap(), 0, pVoid1); }

	return pData;
}

LPCWSTR
GetTempDir()
{
	static BOOL isFirst = TRUE;

	if (isFirst)
	{
		BOOL bDir = FALSE;
		WIN32_FIND_DATAW findData = { 0 };

		GetCurrentDirectoryW(520, Buffer);
		isFirst = FALSE;

		wcscat_s(Buffer, MAX_PATH, L"\\Temp");
		HANDLE hFind = FindFirstFileW(Buffer, &findData);

		// if file doesn't exist
		if (!hFind || hFind == INVALID_HANDLE_VALUE)
		{
			SetLastError(0);		// no error. That's OK
			CreateDirectoryW(Buffer, NULL);
		}
		else
		{
			// check for dir
			bDir = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0);
			FindClose(hFind);
			
			// if current path is file - delete file and create directory
			if (!bDir)
			{
				DeleteFileW(Buffer);
				CreateDirectoryW(Buffer, NULL);
			}
		}

	}

	return Buffer;
}

VOID
DeleteTempDir()
{
	LPCWSTR lpTemp = GetTempDir();
	RemoveDirectoryW(lpTemp);
}

typedef HRESULT(WINAPI *SETTHREADDESCRIPTION)(HANDLE handle, PCWSTR name);

typedef struct tagTHREAD_NAME
{
	DWORD dwType;
	LPCSTR lpName;
	DWORD dwThreadID;
	DWORD dwFlags;
} THREAD_NAME, PTHREAD_NAME;

VOID
SetUserThreadName(
	DWORD dwThreadId,
	LPSTR lpName
)
{
	THREAD_NAME threadName = { NULL };
	threadName.dwType = 0x1000;
	threadName.lpName = lpName;
	threadName.dwThreadID = dwThreadId;
	threadName.dwFlags = NULL;

	__try { RaiseException(0x406D1388, 0, sizeof(threadName) / sizeof(ULONG_PTR), (ULONG_PTR*)&threadName); }
	__except (EXCEPTION_CONTINUE_EXECUTION) { }
}

DWORD
CreateUserThread(
	ThreadFunc* pFunction, 
	LPVOID pArgs,
	LPCSTR pName
)
{
	HANDLE hThread = (HANDLE)_beginthread((_beginthread_proc_type)pFunction, 0, pArgs);
	DWORD dwThread = GetThreadId(hThread);

	SetUserThreadName(dwThread, pName);

	return dwThread;
}

