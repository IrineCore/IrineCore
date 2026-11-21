#pragma once

#include "DnCustomDlg.h"

class CDnQuestNotifierListItemDlg : public CDnCustomDlg
{
public:
	CDnQuestNotifierListItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnQuestNotifierListItemDlg();

protected:
	CEtUIButton *m_pBtnClose;
	CEtUIStatic *m_pStaticMainIcon;
	CEtUIStatic *m_pStaticMainTitle;
	CEtUITextBox *m_pTextBoxMain;
	CEtUIStatic *m_pStaticArrowBase;
	CEtUIStatic *m_pStaticBoard;
	CEtUIStatic *m_pStaticOver;
	CEtUIStatic *m_pStaticSubIcon;
	CEtUIStatic *m_pStaticSubTitle;

	int m_nQuestIndex;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetQuestSummary( int nQuestIndex, const WCHAR *wszTitle, const WCHAR *wszDesc, bool bMainQuest );
	int GetQuestIndex() const { return m_nQuestIndex; }
};
