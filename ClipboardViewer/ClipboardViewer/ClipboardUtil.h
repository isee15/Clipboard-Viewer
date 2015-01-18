#include <tchar.h>
#include "./gac/vlpp.h"

vl::WString GetClipFormatName(int uFormat, int htmlFormat);

vl::WString ConvertUtf8(char* strUtf8);

vl::WString IntToString(int i);

vl::WString IntToFormatString(const TCHAR *format, int i);

vl::WString CharFillString(const TCHAR ch, int length);

vl::WString ConvertMBCS2Utf8(char* strUtf8);
