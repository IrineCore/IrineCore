#pragma once
#include "DnCustomDlg.h"

// rotha - Dialog created for PVP Captain mode //
// After inheriting, multiple can be used by increasing m_vec values //

class CDnCaptionDlg_04 : public CDnCustomDlg
{
public:
	CDnCaptionDlg_04( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCaptionDlg_04(void);

	enum CaptionNum
	{
		FirstCaption,
		SecondCaption,
	};
protected:
	std::vector<CDnMessageStatic*> m_vecStaticMessage;

public:
	void SetCaption( LPCWSTR wszMsg, DWORD dwColor, float fFadeTime , int bCaptionNum);
	void CloseCaption();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
};