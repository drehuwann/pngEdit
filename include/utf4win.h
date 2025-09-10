#pragma once

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <CommCtrl.h>

const char *ToCstr(LPCTSTR tStr);
LPCTSTR FromCstr(const char *tStr);
#endif  // WIN32