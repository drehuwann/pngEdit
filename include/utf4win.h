#pragma once

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#include <commctrl.h>

const char *ToCstr(LPCTSTR tStr);
LPCTSTR FromCstr(const char *tStr);
#endif  // WIN32