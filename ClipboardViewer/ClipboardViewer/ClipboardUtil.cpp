#include "ClipboardUtil.h"
#include <windows.h>
#include <string>
#include <cwchar>
#define BOM8A 0xEF
#define BOM8B 0xBB
#define BOM8C 0xBF

TCHAR *StrClipboardFormats[17] = {
	_T("CF_TEXT"),
	_T("CF_BITMAP"),
	_T("CF_METAFILEPICT"),
	_T("CF_SYLK"),
	_T("CF_DIF"),
	_T("CF_TIFF"),
	_T("CF_OEMTEXT"),
	_T("CF_DIB"),
	_T("CF_PALETTE"),
	_T("CF_PENDATA"),
	_T("CF_RIFF"),
	_T("CF_WAVE"),
	_T("CF_UNICODETEXT"),
	_T("CF_ENHMETAFILE"),
	_T("CF_HDROP"),
	_T("CF_LOCALE"),
	_T("CF_DIBV5")
};

vl::WString GetClipFormatName(int uFormat, int htmlFormat)
{
	if (uFormat <= 17)
	{
		return StrClipboardFormats[uFormat - 1];
	}
	else if (uFormat == htmlFormat)
	{
		return _T("CF_HTML");
	}
	else
	{
		TCHAR szFormatName[256];
		if (GetClipboardFormatName(uFormat, szFormatName,
			sizeof(szFormatName)))
		{
			return szFormatName;
		}
		return _T("NONE");
	}
}

vl::WString ConvertUtf8(char* strUtf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, NULL, 0);
	if (len == 0)
		return vl::WString();
	LPWSTR lpBuffer = new WCHAR[len];
	MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, lpBuffer, len);
	return vl::WString(lpBuffer);
}

vl::WString ConvertMBCS2Utf8(char* strUtf8)
{
	int nLen = MultiByteToWideChar(CP_ACP, 0, strUtf8, -1, NULL, 0);
	WCHAR* lpszW = new WCHAR[nLen];
	int nRtn = MultiByteToWideChar(CP_ACP, 0, strUtf8, -1, lpszW, nLen);
	return vl::WString(lpszW);
}

vl::WString IntToString(int i)
{
	TCHAR s[64] = { 0 };
	swprintf_s(s, 64, L"%d", i);
	return vl::WString(s);
}

vl::WString IntToFormatString(const TCHAR *format, int i)
{
	TCHAR s[64] = { 0 };
	swprintf_s(s, 64, format, i);
	return vl::WString(s);
}

vl::WString CharFillString(const TCHAR ch, int length)
{
	TCHAR *s = new TCHAR[length + 1];
	for (int i = 0; i < length; i++)
	{
		s[i] = ch;
	}
	s[length] = L'\0';
	return vl::WString(s);
}