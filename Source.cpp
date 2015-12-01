#pragma comment(lib, "wininet")
#include <windows.h>
#include <wininet.h>
#include <string>

#define APP_ID TEXT("") // https://e.developer.yahoo.co.jp からアプリケーションIDを取得してください。
TCHAR szClassName[] = TEXT("Window");

LPTSTR UrlEncode(LPCTSTR lpszSrc)
{
	const DWORD dwTextLength = WideCharToMultiByte(CP_UTF8, 0, lpszSrc, -1, 0, 0, 0, 0);
	LPSTR lpszText = (LPSTR)GlobalAlloc(GMEM_FIXED, sizeof(CHAR)*(dwTextLength + 1));
	WideCharToMultiByte(CP_UTF8, 0, lpszSrc, -1, lpszText, dwTextLength, NULL, NULL);
	lpszText[dwTextLength] = 0;
	DWORD i;
	DWORD j;
	for (i = j = 0; i < dwTextLength && lpszText[i]; ++i)
	{
		if (lpszText[i] == '.' || lpszText[i] == '_' || lpszText[i] == '-')
		{
			++j;
		}
		else if (lpszText[i] == ' ')
		{
			++j;
		}
		else
		{
			j += 3;
		}
	}
	LPTSTR lpszDst = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR) * (j + 1));
	for (i = j = 0; i < dwTextLength && lpszText[i]; ++i)
	{
		if (lpszText[i] == '.' || lpszText[i] == '_' || lpszText[i] == '-')
		{
			lpszDst[j] = lpszText[i];
			++j;
		}
		else if (lpszText[i] == ' ')
		{
			lpszDst[j] = '+';
			++j;
		}
		else
		{
			j += wsprintf(&lpszDst[j], TEXT("%%%02X"), lpszText[i] & 0xFF);
		}
	}
	lpszDst[j] = 0;
	GlobalFree(lpszText);
	return lpszDst;
}

BOOL GetCoordinate(LPCTSTR lpszAddress, double *pdLon, double *pdLat)
{
	BOOL bRet = FALSE;
	HINTERNET hInternet;
	HINTERNET hHttpSession;
	HINTERNET hHttpRequest;
	TCHAR sStr[1024];
	const TCHAR hdrs[] = TEXT("Content-Type: application/x-www-form-urlencoded");
	hInternet = InternetOpen(TEXT("WININET Test Program"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL)
	{
		goto END0;
	}
	lstrcpy(sStr, TEXT("/OpenLocalPlatform/V1/geoCoder?appid="));
	lstrcat(sStr, APP_ID);
	lstrcat(sStr, TEXT("&query="));
	LPTSTR lpszUrlAddress = UrlEncode(lpszAddress);
	lstrcat(sStr, lpszUrlAddress);
	GlobalFree(lpszUrlAddress);
	hHttpSession = InternetConnect(hInternet, TEXT("geo.search.olp.yahooapis.jp"), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hHttpSession)
	{
		goto END2;
	}
	hHttpRequest = HttpOpenRequest(hHttpSession, TEXT("GET"), sStr, NULL, 0, NULL, INTERNET_FLAG_RELOAD, 0);
	if (!hHttpRequest)
	{
		goto END3;
	}
	if (HttpSendRequest(hHttpRequest, 0, 0, 0, 0) == FALSE)
	{
		goto END1;
	}
	{
		std::string string;
		CHAR BufSizeText[1000];
		for (;;)
		{
			DWORD ReadSize = 1000;
			BOOL bResult = InternetReadFile(hHttpRequest, BufSizeText, 1000 - 1, &ReadSize);
			if (!bResult)
			{
				goto END1;
			}
			if (!ReadSize)break;
			BufSizeText[ReadSize] = 0;
			string += BufSizeText;
		}
		std::string::size_type Pos1(string.find("<Coordinates>") + 13);
		if (Pos1 != std::string::npos)
		{
			std::string::size_type Pos2 = string.find("</Coordinates>", Pos1);
			if (Pos2 != std::string::npos)
			{
				std::string sub = string.substr(Pos1, Pos2 - Pos1);
				sscanf_s(sub.c_str(), "%lf,%lf", pdLon, pdLat);
				bRet = TRUE;
			}
		}
	}
END1:
	InternetCloseHandle(hHttpRequest);
END3:
	InternetCloseHandle(hHttpSession);
END2:
	InternetCloseHandle(hInternet);
END0:
	return bRet;
}

LPTSTR GetStation(const double *pdLon, const double *pdLat)
{
	LPWSTR pwsz = NULL;
	HINTERNET hInternet;
	HINTERNET hHttpSession;
	HINTERNET hHttpRequest;
	TCHAR sStr[1024];
	const TCHAR hdrs[] = TEXT("Content-Type: application/x-www-form-urlencoded");
	hInternet = InternetOpen(TEXT("WININET Test Program"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL)
	{
		goto END0;
	}
	swprintf_s(sStr, TEXT("/api/xml?method=getStations&x=%lf&y=%lf"), *pdLon, *pdLat);
	hHttpSession = InternetConnect(hInternet, TEXT("express.heartrails.com"), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hHttpSession)
	{
		goto END2;
	}
	hHttpRequest = HttpOpenRequest(hHttpSession, TEXT("GET"), sStr, NULL, 0, NULL, INTERNET_FLAG_RELOAD, 0);
	if (!hHttpRequest)
	{
		goto END3;
	}
	if (HttpSendRequest(hHttpRequest, 0, 0, 0, 0) == FALSE)
	{
		goto END1;
	}
	{
		std::string string;
		CHAR BufSizeText[1000];
		for (;;)
		{
			DWORD ReadSize = 1000;
			BOOL bResult = InternetReadFile(hHttpRequest, BufSizeText, 1000 - 1, &ReadSize);
			if (!bResult)
			{
				goto END1;
			}
			if (!ReadSize)break;
			BufSizeText[ReadSize] = 0;
			string += BufSizeText;
		}
		std::string::size_type  Pos(string.find("\n"));
		while (Pos != std::string::npos)
		{
			string.replace(Pos, 1, "\r\n");
			Pos = string.find("\n", Pos + 2);
		}
		const DWORD len = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, 0, 0);
		pwsz = (LPWSTR)GlobalAlloc(0, len*sizeof(WCHAR));
		MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, pwsz, len);
	}
END1:
	InternetCloseHandle(hHttpRequest);
END3:
	InternetCloseHandle(hHttpSession);
END2:
	InternetCloseHandle(hInternet);
END0:
	return pwsz;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1;
	static HWND hEdit2;
	switch (msg)
	{
	case WM_CREATE:
		hEdit1 = CreateWindow(TEXT("EDIT"), TEXT("東京都千代田区一ツ橋一丁目1番1号"), WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 10, 512, 32, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		CreateWindow(TEXT("BUTTON"), TEXT("取得"), WS_VISIBLE | WS_CHILD, 10, 50, 512, 32, hWnd, (HMENU)100, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindow(TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 10, 90, 512, 512, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == 100)
		{
			const DWORD dwTextLen = GetWindowTextLength(hEdit1);
			if (dwTextLen)
			{
				LPTSTR lpszAddress = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR)*(dwTextLen + 1));
				GetWindowText(hEdit1, lpszAddress, dwTextLen + 1);
				double dLon, dLat;
				if (GetCoordinate(lpszAddress, &dLon, &dLat))
				{
					LPTSTR lpszStation = GetStation(&dLon, &dLat);
					SetWindowText(hEdit2, lpszStation);
					GlobalFree(lpszStation);
				}
				GlobalFree(lpszAddress);
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("住所から最寄り駅を検索（Yahoo! Web API と express.heartrails.com を使用）"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}