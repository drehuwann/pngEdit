#include "utf4win.h"
#ifdef WIN32
/**
 * @brief Converts securely tchar string in cstring.
 * If (ToCstr(in) != in), you should delete the new returned pointer after use.
 * @param tStr : can be const char* or const wchar* 
 * @return tStr if entry was const char*. else returns a new const char* from wchar *entry
 */
const char *ToCstr(LPCTSTR tStr) {
   if (sizeof(TCHAR) == sizeof(char)) {
      return (const char *)tStr;
   }
   size_t inSize = lstrlen(tStr);
   size_t outSize;
   wcstombs_s(&outSize, nullptr, 0, tStr, inSize);
   char *toRet = new char[outSize];
   wcstombs_s(&outSize, toRet, outSize, tStr, inSize);
   return (const char *)toRet;
}
#endif  // WIN32