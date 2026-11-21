#pragma once

#include <usp10.h>

#define UNISCRIBE_DLLNAME	"\\usp10.dll"

//#define UI_MAX_EDITBOXLENGTH 0xffff
#define MAX_COMPSTRING_SIZE		256

class CEtUIUniBuffer
{
public:
	CEtUIUniBuffer( int nInitialSize = 1 );
	virtual ~CEtUIUniBuffer(void);

protected:
	WCHAR *m_pwszBuffer;
	int m_nBufferSize;
	int m_nMaxEditBoxLength;

	bool m_bAnalyseRequired;
	SCRIPT_STRING_ANALYSIS m_Analysis;

	//ID3DXFont *m_pFont;
	static HINSTANCE s_hDll;

	static HRESULT WINAPI Dummy_ScriptApplyDigitSubstitution( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* ) { return E_NOTIMPL; }
	static HRESULT WINAPI Dummy_ScriptStringAnalyse( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* ) { return E_NOTIMPL; }
	static HRESULT WINAPI Dummy_ScriptStringCPtoX( SCRIPT_STRING_ANALYSIS, int, BOOL, int* ) { return E_NOTIMPL; }
	static HRESULT WINAPI Dummy_ScriptStringXtoCP( SCRIPT_STRING_ANALYSIS, int, int*, int* ) { return E_NOTIMPL; }
	static HRESULT WINAPI Dummy_ScriptStringFree( SCRIPT_STRING_ANALYSIS* ) { return E_NOTIMPL; }
	static const SCRIPT_LOGATTR* WINAPI Dummy_ScriptString_pLogAttr( SCRIPT_STRING_ANALYSIS ) { return NULL; }
	static const int* WINAPI Dummy_ScriptString_pcOutChars( SCRIPT_STRING_ANALYSIS ) { return NULL; }

	static HRESULT ( WINAPI *_ScriptApplyDigitSubstitution )( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* );
	static HRESULT ( WINAPI *_ScriptStringAnalyse )( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* );
	static HRESULT ( WINAPI *_ScriptStringCPtoX )( SCRIPT_STRING_ANALYSIS, int, BOOL, int* );
	static HRESULT ( WINAPI *_ScriptStringXtoCP )( SCRIPT_STRING_ANALYSIS, int, int*, int* );
	static HRESULT ( WINAPI *_ScriptStringFree )( SCRIPT_STRING_ANALYSIS* );
	static const SCRIPT_LOGATTR* ( WINAPI *_ScriptString_pLogAttr )( SCRIPT_STRING_ANALYSIS );
	static const int* ( WINAPI *_ScriptString_pcOutChars )( SCRIPT_STRING_ANALYSIS );

public:
	static void Initialize();
	static void Uninitialize();

	//void SetFont( ID3DXFont *pFont ) { m_pFont = pFont; }
	int GetBufferSize() { return m_nBufferSize; }
	bool SetBufferSize( int nSize );
	int GetTextSize() const { return lstrlenW( m_pwszBuffer ); }
	const WCHAR *GetBuffer() const { return m_pwszBuffer; }
	WCHAR* GetModifyBuffer() { return m_pwszBuffer; }
	void SetBuffer( WCHAR* pBuffer, int nBufferSize );
	const WCHAR &operator[]( int n ) const { return m_pwszBuffer[n]; }
	WCHAR &operator[]( int n );
	void Clear();
	// A plain clear does not completely erase the buffer, so create a clear function that also clears the memory.
	void ClearMemory();

	bool RemoveChar( int nIndex );
	bool IsEnableInsertString( int nIndex, const WCHAR *pStr, int nCount = -1 ) const;
	bool IsEnableInsertChar( int nIndex, WCHAR wChar ) const;
	bool InsertString( int nIndex, const WCHAR *pStr, int nCount = -1, bool bIgnoreValidityCheck = false );
	bool InsertChar( int nIndex, WCHAR wChar, bool bIgnoreValidityCheck = false );
	bool SetText( LPCWSTR wszText );

	HRESULT CPtoX( int nCP, BOOL bTrail, int *pX );
	HRESULT XtoCP( int nX, int *pCP, int *pnTrail );
	void GetPriorItemPos( int nCP, int *pPrior );
	void GetNextItemPos( int nCP, int *pPrior );

	// Note : Modify the maximum input length. Default is 256.
	void SetMaxEditBoxLength( int nMaxLength )		
	{
		if( nMaxLength > MAX_COMPSTRING_SIZE )
			nMaxLength = MAX_COMPSTRING_SIZE;
		m_nMaxEditBoxLength = nMaxLength;
	}
	int GetMaxEditBoxLength() const					{ return m_nMaxEditBoxLength; }
	bool IsMaxEditBoxLength() const					{ return ((GetTextSize()+1)>=m_nMaxEditBoxLength); }
	void SetAnalyseRequired( bool bAnalyseRequired ) { m_bAnalyseRequired = bAnalyseRequired; }

	bool RemoveCharThai( int nIndex );
	bool SetTextThai( LPCWSTR wszText );
	bool IsEnableInsertCharThai( int nIndex, WCHAR wChar ) const;
	bool IsEnableInsertStringThai( int nIndex, const WCHAR *pStr ) const;
	bool InsertCharThai( int nIndex, WCHAR wChar );
	bool InsertStringThai( int nIndex, const WCHAR *pStr );

protected:
	HRESULT Analyse();
};
