#pragma once
#include <vector>

struct stControlPositionData
{
	int		nID;
	
	int		nTextID;
	int		nType;
	int		nEnum;
	RECT	RectPos;
};

class CDnControlData
{
private:
	CDnControlData();
	~CDnControlData();


	CString m_szLanguageParam; // Multi-language for EU.
	std::vector<stControlPositionData>	m_vecCtrlPosData;


	bool	LoadXMLFile(char* szFilePath);
	bool	LoadResourceFromXMLFile(int nID, LPCTSTR szFileName);
	void	ParsingRectData(WCHAR* szPosition, RECT& rt);
	bool	SetPositionData(CRect* pRt, int nMax);
	RECT	GetCtrlRect(int enumIndex);

public:
	static CDnControlData &GetInstance()
	{
		static CDnControlData obj;
		return obj;
	}
	
	CString GetFilePath( WCHAR* szFilePath );

	// Multi-language for EU
	void	SetLanguageParam(CString str) { m_szLanguageParam = str; }
	CString	GetMulitiLanguageFilePath();
	int		GetMulitiLanguageResourceID(); // Europe: Return the resource ID matching the country.
	
	// Get and set control position.
	HRESULT SetControlDataFromLocalFile( CString szFilePath , CRect* pRect, int nMaxCnt );				 // XML of local file
	HRESULT SetControlDataFromResource( int nResourceID, LPCTSTR szFileName, CRect* pRect, int nMaxCnt );// Resource XML
	/* arg1: resource ID, arg2: output filename, arg3: control rect, arg4: total control count */
};

#define DNCTRLDATA CDnControlData::GetInstance()