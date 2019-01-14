/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* lum-decoder - open-source Windows transcoder
* MIT-License
**********************************************************
* Module Name: Windows Media Foundation
**********************************************************
* MFReader.h
* Windows Media Foundation implementation
**********************************************************/
#pragma once
#include "lum-decoder.h"

VOID
DeleteMFReader();

BOOL
InitMFReader();

BOOL
IsSupportedByWMF(
	LPCWSTR lpPath
);

BOOL
DecodeByWMF(
	LPCWSTR lpPath,
	LPCWSTR lpPathOut,
	size_t* OutSize,
	WAVEFORMATEX* waveFormat,
	FILE_TYPE* pTypes
);
