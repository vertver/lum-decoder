#pragma once
#include "resource.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfplay.h>
#include <mfreadwrite.h>

#define _RELEASE(x) if (x && x->lpVtbl) { x->lpVtbl->Release(x);  x->lpVtbl = NULL; }

typedef enum
{
	AAC_TYPE,
	AIFF_TYPE,
	ALAC_TYPE,
	MP3_TYPE,
	FLAC_TYPE,
	OGG_TYPE,
	OPUS_TYPE,
	WMA_TYPE,
	WAV_TYPE,
	UNKNOWN_TYPE
} FILE_TYPE;

typedef struct  
{
	FILE_TYPE FileTypeInput;
	FILE_TYPE FileTypeOutput;
	LPCWSTR lpPathIn;
	LPCWSTR lpPathOut;
} PROCESS_PROC;

typedef void ThreadFunc(void *);

VOID DeleteMFReader();
BOOL OpenDialog(LPCWSTR lpPath);
BOOL SaveDialog(LPCWSTR lpPath);
FILE_TYPE IsFileIsAudio(LPCWSTR PathToFile);
BOOL Process(FILE_TYPE FileTypeInput, FILE_TYPE FileTypeOutput, LPCWSTR lpPathIn, LPCWSTR lpPathOut);
LPVOID ReAllocFile(LPVOID pVoid1, LPVOID pVoid2, size_t size1, size_t size2);
LPCWSTR GetTempDir(); 
VOID DeleteTempDir();
DWORD CreateUserThread(ThreadFunc* pFunction, LPVOID pArgs, LPCSTR pName);
DWORD WINAPIV ProcessProc(LPVOID pData);
VOID SetUserThreadName(DWORD dwThreadId, LPSTR lpName);