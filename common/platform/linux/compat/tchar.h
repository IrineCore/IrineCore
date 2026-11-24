// =============================================================================
// IrineCore — common/platform/linux/compat/tchar.h
// Cross-platform TCHAR macro mapping for Linux (-fshort-wchar / MBCS).
// =============================================================================
#pragma once

#include "windows.h"

#ifndef __T
#if defined(UNICODE) || defined(_UNICODE)
#define __T(x) L##x
#else
#define __T(x) x
#endif
#endif

#ifndef _T
#define _T(x) __T(x)
#endif
#ifndef _TEXT
#define _TEXT(x) __T(x)
#endif

#if defined(UNICODE) || defined(_UNICODE)
using _TCHAR  = WCHAR;
using _TUCHAR = uint16_t;
#define _tcslen      wcslen
#define _tcsnlen     wcsnlen
#define _tcscpy      wcscpy
#define _tcscpy_s    wcscpy_s
#define _tcsncpy     wcsncpy
#define _tcsncpy_s   wcsncpy_s
#define _tcscat      wcscat
#define _tcscat_s    wcscat_s
#define _tcsncat     wcsncat
#define _tcsncat_s   wcsncat_s
#define _tsplitpath  _wsplitpath
#define _tsplitpath_s _wsplitpath_s
#define _tcscmp      wcscmp
#define _tcsncmp     wcsncmp
#define _tcsicmp     _wcsicmp
#define _tcsnicmp    _wcsnicmp
#define _tcschr      wcschr
#define _tcsrchr     wcsrchr
#define _tcsstr      wcsstr
#define _tcstok      wcstok
#define _tcslwr      _wcslwr
#define _tcsupr      _wcsupr
#define _tcslwr_s    _wcslwr_s
#define _tcsupr_s    _wcsupr_s
#define _ttoi        _wtoi
#define _ttol        _wtol
#define _ttof        _wtof
#define _ttoi64      _wtoi64
#define _itot        _itow
#define _itot_s      _itow_s
#define _tcstol      wcstol
#define _tcstoul     wcstoul
#define _tcstod      wcstod
#define _tprintf     wprintf
#define _ftprintf    fwprintf
#define _stprintf    swprintf
#define _stprintf_s  swprintf_s
#define _sntprintf   _snwprintf
#define _sntprintf_s _snwprintf_s
#define _vstprintf   vswprintf
#define _vstprintf_s vswprintf_s
#define _vsntprintf  _vsnwprintf
#define _vsntprintf_s _vsnwprintf_s
#define _stscanf     swscanf
#define _stscanf_s   swscanf
#define _tfopen      _wfopen
#define _tfopen_s    _wfopen_s
#define _fgetts      fgetws
#define _fputts      fputws
#define _taccess     _waccess
#define _tmkdir      _wmkdir
#define _tstat       _wstat
#define _tcsftime    wcsftime
#define _istspace    iswspace
#define _istdigit    iswdigit
#define _istalpha    iswalpha
#define _istalnum    iswalnum
#define _totlower    towlower
#define _totupper    towupper
#else
using _TCHAR  = char;
using _TUCHAR = unsigned char;
#define _tcslen      strlen
#define _tcsnlen     strnlen
#define _tcscpy      strcpy
#define _tcscpy_s    strcpy_s
#define _tcsncpy     strncpy
#define _tcsncpy_s   strncpy_s
#define _tcscat      strcat
#define _tcscat_s    strcat_s
#define _tcsncat     strncat
#define _tcsncat_s   strncat_s
#define _tsplitpath  _splitpath
#define _tsplitpath_s _splitpath_s
#define _tcscmp      strcmp
#define _tcsncmp     strncmp
#define _tcsicmp     _stricmp
#define _tcsnicmp    _strnicmp
#define _tcschr      strchr
#define _tcsrchr     strrchr
#define _tcsstr      strstr
#define _tcstok      strtok
#define _tcslwr      _strlwr
#define _tcsupr      _strupr
#define _tcslwr_s    _strlwr_s
#define _tcsupr_s    _strupr_s
#define _ttoi        atoi
#define _ttol        atol
#define _ttof        atof
#define _ttoi64      _atoi64
#define _itot        _itoa
#define _itot_s      _itoa_s
#define _tcstol      strtol
#define _tcstoul     strtoul
#define _tcstod      strtod
#define _tprintf     printf
#define _ftprintf    fprintf
#define _stprintf    sprintf
#define _stprintf_s  sprintf_s
#define _sntprintf   _snprintf
#define _sntprintf_s _snprintf_s
#define _vstprintf   vsprintf
#define _vstprintf_s vsprintf_s
#define _vsntprintf  _vsnprintf
#define _vsntprintf_s _vsnprintf_s
#define _stscanf     sscanf
#define _stscanf_s   sscanf
#define _tfopen      fopen
#define _tfopen_s    fopen_s
#define _fgetts      fgets
#define _fputts      fputs
#define _taccess     _access
#define _tmkdir      _mkdir
#define _tstat       stat
#define _tcsftime    strftime
#define _istspace    isspace
#define _istdigit    isdigit
#define _istalpha    isalpha
#define _istalnum    isalnum
#define _totlower    tolower
#define _totupper    toupper
#endif
