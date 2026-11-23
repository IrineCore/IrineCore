#include "stdafx.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
CIniFile g_IniFile;

namespace {

static const TCHAR kIniMissingSentinel[] = _T("__IRINE_INI_KEY_MISSING__");

static bool IniFileExists(LPCTSTR path)
{
	if (!path || !path[0])
		return false;
	DWORD attr = GetFileAttributes(path);
	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

} // anonymous namespace

CIniFile::CIniFile(void)
{
	ZeroMemory(m_wszFileName, sizeof(m_wszFileName));
	ZeroMemory(m_wszDistFileName, sizeof(m_wszDistFileName));
}

CIniFile::~CIniFile(void)
{
}

BOOL CIniFile::Open(LPCTSTR fileName)
{
	if (!fileName)
		return FALSE;

	_tcsncpy(m_wszFileName, fileName, MAX_PATH - 1);
	m_wszFileName[MAX_PATH - 1] = _T('\0');

	// Derive optional .dist fallback path (e.g. "./config/DNGame.ini.dist")
	ZeroMemory(m_wszDistFileName, sizeof(m_wszDistFileName));
	size_t len = _tcslen(m_wszFileName);
	if (len + 5 < MAX_PATH)
	{
		_tcscpy(m_wszDistFileName, m_wszFileName);
		_tcscat(m_wszDistFileName, _T(".dist"));
	}

	// If local override does not exist yet but .dist does, read seamlessly from .dist
	if (!IniFileExists(m_wszFileName) && IniFileExists(m_wszDistFileName))
	{
		_tcsncpy(m_wszFileName, m_wszDistFileName, MAX_PATH - 1);
		m_wszFileName[MAX_PATH - 1] = _T('\0');
		m_wszDistFileName[0] = _T('\0');
	}

	return TRUE;
}

BOOL CIniFile::Close(void)
{
	return TRUE;
}

BOOL CIniFile::GetValue(LPCTSTR keyName, LPCTSTR valueName, LPINT value)
{
	return GetValue(keyName, valueName, value, 0);
}

BOOL CIniFile::GetValue(LPCTSTR keyName, LPCTSTR valueName, LPINT value, int defaultValue)
{
	if (!keyName || !valueName || !value)
		return FALSE;

	TCHAR probe[64] = { 0 };
	GetPrivateProfileString(keyName, valueName, kIniMissingSentinel, probe, _countof(probe), m_wszFileName);
	if (_tcscmp(probe, kIniMissingSentinel) != 0)
	{
		*value = GetPrivateProfileInt(keyName, valueName, defaultValue, m_wszFileName);
		return TRUE;
	}

	if (m_wszDistFileName[0] && IniFileExists(m_wszDistFileName))
	{
		*value = GetPrivateProfileInt(keyName, valueName, defaultValue, m_wszDistFileName);
		return TRUE;
	}

	*value = defaultValue;
	return TRUE;
}

BOOL CIniFile::GetValue(LPCTSTR keyName, LPCTSTR valueName, LPTSTR value, int nlength)
{
	if (!keyName || !valueName || !value || nlength <= 0)
		return FALSE;

	TCHAR probe[512] = { 0 };
	GetPrivateProfileString(keyName, valueName, kIniMissingSentinel, probe, _countof(probe), m_wszFileName);
	if (_tcscmp(probe, kIniMissingSentinel) != 0)
	{
		GetPrivateProfileString(keyName, valueName, _T(""), value, nlength, m_wszFileName);
		return TRUE;
	}

	if (m_wszDistFileName[0] && IniFileExists(m_wszDistFileName))
	{
		GetPrivateProfileString(keyName, valueName, _T(""), value, nlength, m_wszDistFileName);
		return TRUE;
	}

	value[0] = _T('\0');
	return TRUE;
}

BOOL CIniFile::SetValue(LPCTSTR keyName, LPCTSTR valueName, int value)
{
	if (!keyName || !valueName)
		return FALSE;

	TCHAR Value[16]	= {0,};
	_itot(value, Value, 10);

	WritePrivateProfileString(keyName, valueName, Value, m_wszFileName);

	return TRUE;
}

BOOL CIniFile::SetValue(LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value)
{
	if (!keyName || !valueName || !value)
		return FALSE;

	WritePrivateProfileString(keyName, valueName, value, m_wszFileName);

	return TRUE;
}
