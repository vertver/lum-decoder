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
