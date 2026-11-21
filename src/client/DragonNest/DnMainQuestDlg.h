#pragma once

#include "DnCustomDlg.h"

class CDnMainQuestAnswerDlg;
class CDnMainQuestRewardDlg;

class CDnMainQuestDlg : public CDnCustomDlg
{
public:
	CDnMainQuestDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMainQuestDlg();

protected:
	CEtUIStatic *m_pStaticBack;
	CEtUIStatic *m_pStatic0;
	CEtUIStatic *m_pStaticNpcName;
	CEtUITextBox *m_pTextBoxMain;
	CEtUIButton *m_pBtnClose;
	CEtUIButton *m_pBtnReturn;

	CDnMainQuestAnswerDlg *m_pAnswerDlg;
	CDnMainQuestRewardDlg *m_pRewardDlg;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetNpcTalkText( const WCHAR *wszNpcName, const WCHAR *wszContent );
	CDnMainQuestAnswerDlg *GetAnswerDlg() const { return m_pAnswerDlg; }
	CDnMainQuestRewardDlg *GetRewardDlg() const { return m_pRewardDlg; }
};
