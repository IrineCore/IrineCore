#pragma once

#include "EtUIControl.h"

#define FIX_STATICCONTROL_FOCUS_PROCESS
#define PRE_UISTATIC_PARTTEXT_COLOR
#define PRE_MOD_UISTATIC_PARTTEXT_COLOR

class CEtUIStatic : public CEtUIControl
{
public:
	CEtUIStatic( CEtUIDialog *pParent );
	virtual ~CEtUIStatic(void);

protected:
	SUICoord m_BaseCoord[9];
	SUICoord m_BaseUVCoord[9];

	bool	m_bBlink;
	float	m_fBlinkTime;
	int		m_nBlinkAlpha;
	bool	m_bManualControlColor;
	float	m_fWidthScale;
	float	m_fHeightScale;
	SUIColor m_BgTextColor;
	float	m_fRotate;
	bool	m_bForceBlend;
#ifdef FIX_STATICCONTROL_FOCUS_PROCESS
#else
	bool  m_bCanHaveFocus;
#endif
	bool m_bButton;

	SUICoord m_FontCoord;
#ifdef PRE_UISTATIC_PARTTEXT_COLOR
	struct SPartTextInfo
	{
		std::wstring text;
		DWORD dwTextColor;
		DWORD dwBGColor;
		float fWidth;
		float fPartWidthOffset;
		DWORD dwFontFormat;
	};
	std::list<SPartTextInfo> m_PartTextColorInfoList;
#endif

public:
	// In some states, changing the alpha value is unnecessary (EnemyGaugeDlg); in this case true.
	void	SetShadowColor( DWORD dwColor, bool bSkipAlpha = false );
	void	SetTextColor( DWORD dwColor, bool bSkipAlpha = false );
	void	SetBgTextColor( DWORD dwColor, UI_CONTROL_STATE State = UI_STATE_NORMAL );

	void	SetTextureColor( DWORD dwColor );
	void	SetManualControlColor(bool bManual)		{ m_bManualControlColor = bManual; }
	void	SetTextureScale( float fScale )			{ m_fWidthScale = m_fHeightScale = fScale; }
	void	SetTextureWidthScale( float fScale )	{ m_fWidthScale = fScale; }
	void	SetTextureHeithScale( float fScale )	{ m_fHeightScale = fScale; }

	// For Static control, there will be times when we want to change the color value specified in the tool.
	void	SetTextureColor( DWORD dwColor, int nElementIndex, UI_CONTROL_STATE emState = UI_STATE_NORMAL );

	void SetRotate( float fRotate ) {m_fRotate = fRotate;}

	DWORD	GetTextColor();
	DWORD	GetShadowColor();
	DWORD	GetTextureColor();
	DWORD	GetCurTextureColor();
	int		GetBlinkAlpha()					{ return m_nBlinkAlpha; }

	void SetBlendRate( float fRate )	{ m_fBlendRate = fRate; }
	void ForceBlend() { m_bForceBlend = true; }
	// Note : The static control blinks when Blink becomes true.
	void SetBlink( bool bBlink );
	bool IsBlink() { return m_bBlink; }

	// Note : Add ellipsis when static control space is insufficient, e.g. "Hello wor..."
	bool SetTextWithEllipsis( const wchar_t *wszText, const wchar_t *wszSymbol );

#ifdef PRE_UISTATIC_PARTTEXT_COLOR
	virtual void ClearText();
#else
	virtual void ClearText() { m_szText.clear(); m_strRenderText.clear(); if( m_Property.StaticProperty.bAutoTooltip ) ClearTooltipText(); }
#endif

	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );

#ifdef FIX_STATICCONTROL_FOCUS_PROCESS
#else
	void SetCanHaveFocus( bool bFocus ) {m_bCanHaveFocus = bFocus;}
#endif
	SUICoord *GetFontCoord() { return &m_FontCoord; }

	const wchar_t* GetPropertyString();

	void SetButton( bool bButton ) { m_bButton = bButton;}

#ifdef PRE_UISTATIC_PARTTEXT_COLOR
	void SetPartColorText(const std::wstring& colorText, DWORD dwTextColor, DWORD dwBGColor);
	void ClearPartColorText();
#endif

public:	
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual void UpdateRects();
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );
	virtual void SetText( const std::wstring &strText , bool bFit = false, float fScaleX = 1.0f, float fScaleY = 1.0f) override;
#ifdef FIX_STATICCONTROL_FOCUS_PROCESS
#else
	virtual bool CanHaveFocus() { return m_bCanHaveFocus; }
#endif

private:
#ifdef PRE_UISTATIC_PARTTEXT_COLOR
	void MakePartColorText(std::list<SPartTextInfo>::iterator& iterToInsert, const std::wstring& orgText, const std::wstring colorText, SUIElement* pElement, const SPartTextInfo& partTextInfo);
#endif
};
