#pragma once
#include "EtUIEditBox.h"
#include "EtUIIME.h"

class CEtUIIMEEditBox : public CEtUIEditBox, public CEtUIIME
{
public:
	CEtUIIMEEditBox( CEtUIDialog *pParent );
	virtual ~CEtUIIMEEditBox(void);

protected:
	DWORD m_dwCompTargetNonBkColor;
	DWORD m_dwCompTargetBkColor;
	DWORD m_dwCompTargetColor;
	DWORD m_dwCompTargetNonColor;
	SUICoord m_IndicatorCoord;

	// Note: Prevent the composing string from being too long and pushing the completed string out of view.
	// Otherwise m_nFirstVisible would exceed the string length.
	bool m_bFirstVisibleOver;
	int m_nTempCompSize;

	// When rendering Japanese candidate list, render below the composing characters by default,
	// If it exceeds the dialog area, render it on top.
	// Change this to true to force rendering below.
	bool m_bForceBottomPosition;

public:
	void SetCompTextColor( DWORD dwColor ) { m_Property.IMEEditBoxProperty.dwCompTextColor = dwColor; }
	void CheckIMEToggleState();
	void CancelIMEComposition();
	void GetIMECompositionString(std::wstring& str);

	int GetMaxChar() const;
	void SetMaxChar( DWORD dwChar );

	void ForceRenderCandidateReadingWindowBottomPosition( bool bForce ) { m_bForceBottomPosition = bForce; }

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void SetTemplate( int nTemplateIndex );

	virtual bool MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void Focus( bool fFocus );

	virtual void ClearText();
	virtual void Show( bool bShow );

	void PumpMessage();

	virtual void UpdateRects();

	virtual void Render( float fElapsedTime );
	virtual void RenderCandidateReadingWindow( float fElapsedTime, bool bReading );
	virtual void RenderComposition( float fElapsedTime );
	virtual void RenderIndicator( float fElapsedTime );

protected:
	void TruncateCompString( bool bUseBackSpace = true, int iNewStrLen = 0 );
	void FinalizeString( bool bSend );
	void SendCompString();
	// Note : Adjusts the string when it overflows during IME character composition.
	//		Does not modify the actual current cursor; only adjusts the start position of the string.
	//		Pass the number of characters currently being composed.
	void PlaceCaret( int nCP );
};
