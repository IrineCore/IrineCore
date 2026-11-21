#pragma once
#include "EtUIDialog.h"


// CaptionDlg with the same functionality as DnCaptionDlg.
// : Added CEtUIStatic items used as Captions are stored in a vector and processed up to the UI-defined count.
class CDnCaptionExDlg : public CEtUIDialog
{
public:
	CDnCaptionExDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCaptionExDlg(void);

protected:

	std::vector< CEtUIStatic * > m_vecCaptions;	
	float m_fShowTime;
	float m_fShowRatio;
	float m_fElapsedTime;
	float m_fConst;
	EtColor m_TextColor;
	
public:
	void SetCaption( std::vector< const wchar_t * > & vecStrs, DWORD dwColor, float fFadeTime );	
	void CloseCaption();


public:
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );
};
