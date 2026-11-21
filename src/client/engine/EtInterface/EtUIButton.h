#pragma once
#include "EtUIControl.h"
#include "EtUISound.h"

class CEtUIButton : public CEtUIControl
{
public:
	CEtUIButton( CEtUIDialog *pParent );
	virtual ~CEtUIButton(void);

protected:	
	int m_nToggleButtonIndex;
	//CEtUISound::emUI_SOUND m_emSoundIndex;
	int m_nSoundIndex;

	SUICoord m_MoveButtonCoord;
	SUICoord m_BaseCoordLeft;
	SUICoord m_BaseCoordMid;
	SUICoord m_BaseCoordRight;
	SUICoord m_MoveBaseCoordLeft;
	SUICoord m_MoveBaseCoordMid;
	SUICoord m_MoveBaseCoordRight;

	SUICoord m_uvCoordLeft;
	SUICoord m_uvCoordMid;
	SUICoord m_uvCoordRight;
	SUICoord m_MouseCoord;

	int m_nButtonID;	// Note : This value can be set in the tool to distinguish buttons by index or value.
	//blondy In online games, button disable time is set long to prevent sending messages too often; added for this feature
	bool  IsDisable;             //Whether disabled  
	bool  IsEnableBackUp;       //Save previous state  	
	float m_ElapsedDisableTime;	// Elapsed time since disabled 		
	float m_DisableTime; //Must be able to set this value in the tool
	//blondy end	
	bool m_bRightButton;
	bool m_bEnableKeyboard;
	bool	m_bBlink;
	float	m_fBlinkTime;
	int		m_nBlinkAlpha;

	// PRE_MOD_CASHSHOP_GIFTBOTTON_TWINKLE_SPEEDUP
	int		m_nBlendTime;

public:
	void SetButtonID( int nButtonID ) { m_nButtonID = nButtonID; }
	int GetButtonID() { return m_nButtonID; }	
	SUICoord GetMouseCoord() {return m_MouseCoord;}
	void SetBlink( bool bBlink );
	bool IsBlink() { return m_bBlink; }

	// PRE_MOD_CASHSHOP_GIFTBOTTON_TWINKLE_SPEEDUP
	inline void	SetBlendTime( int BlendTime = 1000 ) { m_nBlendTime = BlendTime; }

public:
	virtual void MouseEnter( bool bEnter );

	// Overridden to use SetTextWithEllipsis function
	virtual void SetText( const std::wstring &strText , bool bFit = false, float fScaleX = 1.0f, float fScaleY = 1.0f) override;

	// Brought directly from EtUIStatic. To automatically append ... when text exceeds length during overseas localization.
	bool SetTextWithEllipsis( const wchar_t *wszText, const wchar_t *wszSymbol );

	virtual void ClearText() { m_szText.clear(); m_strRenderText.clear(); if( m_Property.ButtonProperty.bAutoTooltip ) ClearTooltipText(); }

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual bool CanHaveFocus() { return true; }
	virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );

	void PlayButtonSound();
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void OnHotkey();
	virtual void Render( float fElapsedTime );
	virtual void UpdateRects();
	virtual void Process( float fElapsedTime );
	virtual void Enable( bool bEnable ) ;

	void UpdateBlink( float fElapsedTime );
	void SetDisableTime( float fDisableTime){m_DisableTime = fDisableTime;};
	void EnableKeyboard( bool bEnable ) { m_bEnableKeyboard = bEnable; }
};
