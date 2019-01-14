/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* lum-decoder - open-source Windows transcoder
* MIT-License
**********************************************************
* Module Name: lum kernel
**********************************************************
* lum-decoder.c
* lum kernel implementation
**********************************************************/
#include "stdafx.h"
#include "lum-decoder.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;                   
WCHAR szTitle[MAX_LOADSTRING];                
WCHAR szWindowClass[MAX_LOADSTRING];            

WCHAR szPathIn[260] = { 0 };
WCHAR szPathOut[260] = { 0 };

FILE_TYPE FileTypeIn = UNKNOWN_TYPE;
FILE_TYPE FileTypeOut = UNKNOWN_TYPE;

PROCESS_PROC pProc = { 0 };

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: code here

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSDECODER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	GetTempDir();

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	SetUserThreadName(GetCurrentThreadId(), "Lum main thread");

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSDECODER));

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		Sleep(1);
    }

	DeleteMFReader();
	DeleteTempDir();

    return GetLastError();
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize			= sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSDECODER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(GetStockObject(DC_PEN));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSDECODER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, 0, 380, 500, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	HWND hWnd2 = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_FORMVIEW), hWnd, WndProc);

	ShowWindow(hWnd2, nCmdShow);
	UpdateWindow(hWnd2);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDC_EXITBUT:
			PostQuitMessage(0);
			break;
		case IDC_INOPEN:
		{
			BOOL isDlg = OpenDialog(szPathIn);
			HWND hDlg = GetDlgItem(hWnd, IDC_INPATH);

			if (isDlg && hDlg && hDlg != INVALID_HANDLE_VALUE)
			{
				SetWindowTextW(hDlg, szPathIn);
			}
		}
			break;
		case IDC_OUTSAVE:
		{
			FILE_TYPE fileDlg = SaveDialog(szPathOut);
			HWND hDlg = GetDlgItem(hWnd, IDC_OUTPATH);

			if (hDlg && hDlg != INVALID_HANDLE_VALUE)
			{
				FileTypeOut = IsFileIsAudio(szPathOut);
				if (FileTypeOut == UNKNOWN_TYPE)
				{
					switch (fileDlg)
					{
					case MP3_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".mp3");
						break;
					case FLAC_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".flac");
						break;
					case ALAC_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".alac");
						break;
					case AAC_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".m4a");
						break;
					case WAV_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".wav");
						break;
					case AIFF_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".aiff");
						break;
					case OPUS_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".opus");
						break;
					case OGG_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".ogg");
						break;
					case WMA_TYPE:
						wcscat_s(szPathOut, MAX_PATH, L".wma");
						break;
					default:
						break;
					}

					FileTypeOut = fileDlg;
				}

				SetWindowTextW(hDlg, szPathOut);
			}
		}
			break;
		case IDC_INPATH:
		{
			HWND hDlg = GetDlgItem(hWnd, IDC_INPATH);

			if (hDlg && hDlg != INVALID_HANDLE_VALUE)
			{
				HWND hLabel = GetDlgItem(hWnd, IDC_FMTIN);

				if (GetWindowTextW(hDlg, szPathIn, MAX_PATH))
				{				
					FileTypeIn = IsFileIsAudio(szPathIn);
					if (hLabel && hLabel != INVALID_HANDLE_VALUE)
					{
						switch (FileTypeIn)
						{
						case MP3_TYPE:
							SetWindowTextW(hLabel, L"Format: MPEG-3");
							break;
						case FLAC_TYPE:
							SetWindowTextW(hLabel, L"Format: Free Lossless Audio Codec");
							break;
						case ALAC_TYPE:
							SetWindowTextW(hLabel, L"Format: Apple Lossless Audio Codec");
							break;
						case AAC_TYPE:
							SetWindowTextW(hLabel, L"Format: Advanced Audio Codec");
							break;
						case WAV_TYPE:
							SetWindowTextW(hLabel, L"Format: PCM Audio (little-endian)");
							break;
						case AIFF_TYPE:
							SetWindowTextW(hLabel, L"Format: PCM Audio (big-endian)");
							break;
						case OPUS_TYPE:
							SetWindowTextW(hLabel, L"Format: OPUS");
							break;
						case OGG_TYPE:
							SetWindowTextW(hLabel, L"Format: Vorbis OGG");
							break;
						case WMA_TYPE:
							SetWindowTextW(hLabel, L"Format: WMA Codec");
							break;
						default:
							SetWindowTextW(hLabel, L"Format: ");
							break;
						}
					}

				}
				else
				{
					FileTypeIn = UNKNOWN_TYPE;
					
					SetWindowTextW(hLabel, L"Format: ");
				}
			}
		}
		break;
		case IDC_OUTPATH:
		{
			HWND hDlg = GetDlgItem(hWnd, IDC_OUTPATH);

			if (hDlg && hDlg != INVALID_HANDLE_VALUE)
			{
				if (GetWindowTextW(hDlg, szPathOut, MAX_PATH) && (FileTypeOut = IsFileIsAudio(szPathOut) != UNKNOWN_TYPE))
				{
					FileTypeOut = IsFileIsAudio(szPathOut);
				}

				else
				{
					FileTypeOut = UNKNOWN_TYPE;
				}
			}
		}
		break;
		case IDC_PROCESS:
			pProc.FileTypeInput = FileTypeIn;
			pProc.FileTypeOutput = FileTypeOut;
			pProc.lpPathIn = szPathIn;
			pProc.lpPathOut = szPathOut;

			CreateUserThread((ThreadFunc*)ProcessProc, &pProc, "Lum decoder thread");
			break;
		default:
			Sleep(1);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // TODO: HDC code here
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_SIZE:
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
