#pragma once

#include "EtUIDialog.h"

class CDnChatBalloonDlg : public CEtUIDialog
{
private :
	CEtUITextBox *	m_pTextBox;
	CEtUIStatic	 *	m_pStaticTail;
	CEtUIStatic  *  m_pStaticMark;

	std::wstring	m_wszChatMsg;
	DWORD			m_dwTextColor;

	float			m_fMaxLineWidth;
	float			m_fDepth;
	float			m_fMaxWitdh;

public:
	CDnChatBalloonDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChatBalloonDlg();

	virtual void InitialUpdate();
	virtual void Initialize( const char *pFileName, bool bShow );
	virtual void Show( bool bShow );
	virtual void Render( float fElapsedTime );
	virtual void OnChangeResolution();

	void SetTextLine( LPCWSTR nText, DWORD dwTextColor, DWORD dwTextFormat = UITEXT_NONE );
	void SetChatMsg( LPCWSTR nText, DWORD dwTextColor, int nWidthSize = 8 , DWORD dwTextFormat = UITEXT_NONE );
	void SetDepth( const float depth );

	void CalcTailPosition();
	DWORD GetTextColor() { return m_dwTextColor; }

	// For speech bubbles used in PR chat, PR text lengths vary,
	// Reported as weird because chat message width keeps changing on each rotation.
	// So adjust the textbox length based on the longest one.
	void SetMaxWidthFromText( LPCWSTR wszText );
};