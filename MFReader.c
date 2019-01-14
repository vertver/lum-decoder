/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* lum-decoder - open-source Windows transcoder
* MIT-License
**********************************************************
* Module Name: Windows Media Foundation
**********************************************************
* MFReader.c
* Windows Media Foundation implementation
**********************************************************/
#pragma once
#include "stdafx.h"
#include "lum-decoder.h"
#include <mfidl.h>

IMFAttributes* pAttribute = NULL;
IMFSourceReader* pSourceReader = NULL;
IMFMediaType* pMediaType = NULL;
IMFMediaType* pSecondMediaType = NULL;
IMFPresentationDescriptor* pPresentDesc = NULL;
IMFSourceResolver* pSrcResolver = NULL;
IUnknown* pSrc = NULL;
size_t uDuration;

VOID
DeleteMFReader()
{
	_RELEASE(pSecondMediaType);
	_RELEASE(pMediaType);
	_RELEASE(pAttribute);
}

BOOL
InitMFReader()
{
	static BOOL isMFLoaded = FALSE;
	HRESULT hr = 0;

	if (!isMFLoaded)
	{
		hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
		if (FAILED(hr)) { return FALSE; }
		else { isMFLoaded = TRUE; }
	}

	return TRUE;
}

BOOL
IsSupportedByWMF(
	LPCWSTR lpPath
)
{
	HRESULT hr = 0;
	IMFMediaSource* pMediaSrc = NULL;
	WAVEFORMATEX* waveFormat = { 0 };

	{
		MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

		// create source resolver to get media source
		hr = MFCreateSourceResolver(&pSrcResolver);
		hr = pSrcResolver->lpVtbl->CreateObjectFromURL(pSrcResolver, lpPath, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &pSrc);
	}

	if (!pSrc) 
	{ 
		_RELEASE(pSrcResolver);
		return FALSE;
	}

	// query with media source
	hr = pSrc->lpVtbl->QueryInterface(pSrc, &IID_IMFMediaSource, &pMediaSrc);

	// get file duration (100 nanosecs count)
	hr = pMediaSrc->lpVtbl->CreatePresentationDescriptor(pMediaSrc, &pPresentDesc);
	hr = pPresentDesc->lpVtbl->GetUINT64(pPresentDesc, &MF_PD_DURATION, &uDuration);
	uDuration /= 10000;		// convert to milliseconds

	// open source reader
	hr = MFCreateSourceReaderFromMediaSource(pMediaSrc, NULL, &pSourceReader);

	// select first and deselect all other streams
	hr = pSourceReader->lpVtbl->SetStreamSelection(pSourceReader, (DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
	hr = pSourceReader->lpVtbl->SetStreamSelection(pSourceReader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

	DWORD32 dwSizeOfWaveFormat = 0;

	// get media type
	pSourceReader->lpVtbl->GetNativeMediaType(pSourceReader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, &pMediaType);
	MFCreateWaveFormatExFromMFMediaType(pMediaType, &waveFormat, &dwSizeOfWaveFormat, 0);

	WAVEFORMATEX pData = { 0 };
	memcpy(&pData, waveFormat, sizeof(WAVEFORMATEX));
	CoTaskMemFree(waveFormat);

	if (pData.wBitsPerSample != 24) 
	{ 
		return TRUE; 
	}

	return FALSE;
}

BOOL
DecodeByWMF(
	LPCWSTR lpPath,
	LPCWSTR lpPathOut,
	size_t* OutSize,
	WAVEFORMATEX* waveFormat,
	FILE_TYPE* pTypes
)
{
	GUID MajorType = { 0 };
	GUID SubType = { 0 };
	BOOL isCompressed = FALSE;
	WAVEFORMATEX* pWave = NULL;

	pMediaType->lpVtbl->GetGUID(pMediaType, &MF_MT_MAJOR_TYPE, &MajorType);
	pMediaType->lpVtbl->GetGUID(pMediaType, &MF_MT_MAJOR_TYPE, &SubType);
	pMediaType->lpVtbl->IsCompressedFormat(pMediaType, &isCompressed);

	if (!memcmp(&SubType, &MFAudioFormat_Float, sizeof(GUID)) || !memcmp(&SubType, &MFAudioFormat_PCM, sizeof(GUID)) || !isCompressed)
	{
		pMediaType->lpVtbl->SetGUID(pMediaType, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio);
		pMediaType->lpVtbl->SetGUID(pMediaType, &MF_MT_SUBTYPE, &MFAudioFormat_PCM);
	}
	else
	{
		MFCreateMediaType(&pSecondMediaType);
		pSecondMediaType->lpVtbl->SetGUID(pSecondMediaType, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio);
		pSecondMediaType->lpVtbl->SetGUID(pSecondMediaType, &MF_MT_SUBTYPE, &MFAudioFormat_PCM);
		pSourceReader->lpVtbl->SetCurrentMediaType(pSourceReader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pSecondMediaType);
	}

	IMFMediaType* pUncompressedAudioType = NULL;
	DWORD32 dwSizeOfWaveFormat = 0;

	pSourceReader->lpVtbl->GetCurrentMediaType(pSourceReader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pUncompressedAudioType);
	MFCreateWaveFormatExFromMFMediaType(pUncompressedAudioType, &pWave, &dwSizeOfWaveFormat, 0);

	memcpy(waveFormat, pWave, sizeof(WAVEFORMATEX));
	CoTaskMemFree(pWave);

	pSourceReader->lpVtbl->SetStreamSelection(pSourceReader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

	IMFSample* pSample = NULL;
	IMFMediaBuffer* pBuffer = NULL;
	BYTE* localAudioData = NULL;
	DWORD dwLocalAudioDataLength = 0;
	DWORD FullSize = 0;
	DWORD dwSampleCount = 0;
	DWORD flags = 0;
	LPVOID pBuf = NULL;

	HANDLE hFile = CreateFileW(lpPathOut, GENERIC_ALL, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	while (TRUE)
	{
		pSourceReader->lpVtbl->ReadSample(pSourceReader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, NULL, &flags, NULL, &pSample);

		// check whether the data is still valid
		if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) { break; }
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) { break; }
		if (!pSample) { continue; }

		pSample->lpVtbl->ConvertToContiguousBuffer(pSample, &pBuffer);
		pBuffer->lpVtbl->Lock(pBuffer, &localAudioData, NULL, &dwLocalAudioDataLength);

		WriteFile(hFile, localAudioData, dwLocalAudioDataLength, &dwSampleCount, NULL);
		FullSize += dwLocalAudioDataLength;

		pBuffer->lpVtbl->Unlock(pBuffer);
		localAudioData = NULL;
	}

	*OutSize = FullSize;

	// release all stuff
	_RELEASE(pPresentDesc);
	_RELEASE(pSrcResolver);
	_RELEASE(pSrc);

	FlushFileBuffers(hFile);
	CloseHandle(hFile);

	return TRUE;
}
