#include "stdafx.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
CIniFile g_IniFile;

CIniFile::CIniFile(void)
{
	ZeroMemory(m_wszFileName, sizeof(m_wszFileName));
}

CIniFile::~CIniFile(void)
{
}

BOOL CIniFile::Open(LPCTSTR fileName)
{
	if (!fileName)
		return FALSE;

	_tcsncpy(m_wszFileName, fileName, MAX_PATH);

	return TRUE;
}

BOOL CIniFile::Close(void)
{
	return TRUE;
}

BOOL CIniFile::GetValue(LPCTSTR keyName, LPCTSTR valueName, LPINT value)
{
	if (!keyName || !valueName || !value)
		return FALSE;

	*value = GetPrivateProfileInt(keyName, valueName, 0, m_wszFileName);

	return TRUE;
}

BOOL CIniFile::GetValue(LPCTSTR keyName, LPCTSTR valueName, LPINT value, int defaultValue)
{
	if (!keyName || !valueName || !value)
		return FALSE;

	*value = GetPrivateProfileInt(keyName, valueName, defaultValue, m_wszFileName);

	return TRUE;
}

BOOL CIniFile::GetValue(LPCTSTR keyName, LPCTSTR valueName, LPTSTR value, int nlength)
{
	if (!keyName || !valueName || !value)
		return FALSE;

	GetPrivateProfileString(keyName, valueName, _T(""), value, nlength, m_wszFileName);

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
