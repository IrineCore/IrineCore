#pragma once
#include "EtUIDialog.h"

class CDnSlideCaptionDlg : public CEtUIDialog
{
	struct SSlideInfo
	{
		std::wstring m_wszMsg;
		DWORD m_dwColor;
		float m_fRemainShowTime;

		SSlideInfo()
			: m_dwColor(0xFFFFFFFF)
			, m_fRemainShowTime(0.0f)
		{
		}

		void SetInfo( LPCWSTR wszMsg, DWORD dwColor, float fShowTime )
		{
			WCHAR wszText[512];
			//swprintf_s( wszText, _countof(wszText), wszMsg );	// fix Format String Bug
			_wcscpy( wszText, _countof(wszText), wszMsg, (int)wcslen(wszMsg) );
			int nLen = (int)wcslen( wszText );
			for( int i = 0; i < nLen; ++i ) {
				if( wszText[i] == '\n' )
					wszText[i] = ' ';
			}
			m_wszMsg = wszText;
			m_dwColor = dwColor;
			m_fRemainShowTime = fShowTime;
		}
	};

public:
	CDnSlideCaptionDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSlideCaptionDlg(void);

protected:
	SUICoord m_uiBlind;

	EtColor m_srcColor;
	EtColor m_destColor;
	EtColor m_renderColor;
	float m_fConst;
	float m_fFadeTime;

	CEtUIStatic *m_pCation;
	std::list<SSlideInfo> m_listSlideInfo;

	// Manage the text to set with a separate index so slides in the list can take turns displaying it.
	int m_nLastSlideIndex;

	// Slide for rendering. Kept separately so even if removed from list mid-slide, it remains visible until it passes.
	SSlideInfo m_RenderSlideInfo;
	float m_fTextWidth;
	bool m_bShowSlideText;	// true if currently visible

	// Checking whether to render by alpha value seems to cause issues in some cases.
	bool m_bRender;

	virtual void UpdateBlind();
	void SetCaption( LPCWSTR wszMsg, DWORD dwColor );

public:
	void AddCaption( LPCWSTR wszMsg, DWORD dwColor, float fShowTime );
	void CloseCaption( bool bForce );	// If bForce is true, the currently shown slide disappears immediately.

	void DelCaption( LPCWSTR wszMsg );
	void DelCaption( int nIndex );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );

	virtual void OnChangeResolution();
};