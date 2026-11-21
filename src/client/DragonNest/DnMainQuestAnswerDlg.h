#pragma once

#include "DnCustomDlg.h"

class CDnMainQuestAnswerDlg : public CDnCustomDlg
{
public:
	CDnMainQuestAnswerDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMainQuestAnswerDlg();

protected:
	CEtUITextureControl *m_pTextureImage;
	EtTextureHandle m_hImageTexture;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );

	void SetAnswerImage( const char *szFileName );
};
