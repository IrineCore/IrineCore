#pragma once

#define HEADNAME_LINEMAX 3
class MAHeadNameRender 
{
public:
	MAHeadNameRender();
	virtual ~MAHeadNameRender();

	enum HeadNameElementTypeEnum {
		String,
		Texture,
//		TextureNString, // PRE_ADD_NEWCOMEBACK
	};
	enum HeadNameFontColorType {
		Normal,
		Appellation,
	};

protected:
	float m_fScale;
	float m_fIncreaseHeight;

	struct HeadNameStruct {
		HeadNameElementTypeEnum Type;

		std::wstring szStr;
		int nFontIndex;
		int nFontHeight;
		DWORD dwFontColor;
		DWORD dwShadowColor;
		float fHeightRevision;

		EtTextureHandle hTexture;
		EtVector2 vSize;
		SUICoord Coord;

		float fWidth;
	};
	std::vector<HeadNameStruct> m_VecHeadNameList[HEADNAME_LINEMAX];
#ifdef PRE_ADD_SHOW_MONACTION
	HeadNameStruct m_HighestNameForDebug;
#endif

protected:
	float GetWidth( HeadNameStruct *pStruct, bool bIncreaseSpace );
	void DrawElement( HeadNameStruct *pStruct, EtVector3 vPos, bool bIncreaseSpace );

	//void DrawElementEx( HeadNameStruct *pStruct, EtVector3 vPos, bool bIncreaseSpace ); // Render both texture and font. PRE_ADD_NEWCOMEBACK

public:
	bool Initialize( CDnActor *pActor );

	void SetIncreaseHeight( float fValue ) { m_fIncreaseHeight = fValue; }
	float GetIncreaseHeight(){return m_fIncreaseHeight;};

	void AddHeadNameElement( char cLine, WCHAR *wszStr, int nFontIndex, int nFontHeight, DWORD dwFontColor, DWORD dwShadowColor );
	void AddHeadNameElement( char cLine, EtTextureHandle hTexture, float fWidth, float fHeight, DWORD dwColor, float fHeightRevision = 0.f );
	void AddHeadNameElement( char cLine, EtTextureHandle hTexture, EtVector2 &vSize, DWORD dwColor, float fHeightRevision = 0.f );
	void AddHeadNameElement( char cLine, EtTextureHandle hTexture, EtVector2 &vSize, int nCountX, int nCountY, int nIndex, DWORD dwColor, float fHeightRevision = 0.f );
	void AddHeadNameElement( char cLine, EtTextureHandle hTexture, float fWidth, float fHeight, int nCountX, int nCountY, int nIndex, DWORD dwColor, float fHeightRevision = 0.f );
#ifdef PRE_ADD_SHOW_MONACTION
	void AddHeadNameElement_Highest(WCHAR *wszStr, int nFontIndex, int nFontHeight, DWORD dwFontColor, DWORD dwShadowColor);
#endif

#ifdef PRE_ADD_NEWCOMEBACK
	//void AddHeadNameElement( char cLine, EtTextureHandle hTexture, EtVector2 &vSize, float fHeightRevision,  WCHAR *wszStr, int nFontIndex, int nFontHeight, DWORD dwFontColor, DWORD dwShadowColor );
#endif // PRE_ADD_NEWCOMEBACK

	void Render();

	static void GetHeadIconFontColor( CDnPlayerActor *pActor, HeadNameFontColorType Type, DWORD &dwFontColor, DWORD &dwFontColorS );
#ifdef PRE_ADD_VIP
	static void GetVIPHeadIconFontColor(CDnPlayerActor* pActor, DWORD &dwFontColor, DWORD &dwFontColorS, bool bCheckStage);
#endif
};