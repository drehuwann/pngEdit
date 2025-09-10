#include "utf4win.h"
#ifdef WIN32
/**
 * @brief Converts securely tchar string in cstring.
 * If (ToCstr(in) != in), you should delete the new returned pointer after use.
 * @param tStr : can be const char* or const wchar* 
 * @return tStr if entry was const char*. else returns a new const char* from wchar *entry
 */
const char *ToCstr(LPCTSTR tStr) {
   if constexpr(sizeof(TCHAR) == sizeof(char)) {
      return (const char *)tStr;
   }
   size_t inSize = lstrlen(tStr);
   size_t outSize;
   wcstombs_s(&outSize, nullptr, 0, tStr, inSize);
   auto *toRet = new char[outSize];
   wcstombs_s(&outSize, toRet, outSize, tStr, inSize);
   return (const char *)toRet;
}

/**
 * @brief Converts securely cstring to tchar.
 * If (FromCstr(in) != in), you should delete the new returned pointer after use.
 * @return tStr if _UNICODE is not defined. else returns a new const wchar* from const char *entry
 */
LPCTSTR FromCstr(const char *tStr) {
   if constexpr(sizeof(TCHAR) == sizeof(char)) {
      return (LPCTSTR)tStr;
   }
   size_t inSize = strlen(tStr);
   size_t outSize;
   mbstowcs_s(&outSize, nullptr, 0, tStr, inSize);
   auto toRet = new TCHAR[outSize];
   mbstowcs_s(&outSize, toRet, outSize, tStr, inSize);
   return (LPCTSTR)toRet;
}
#endif  // WIN32